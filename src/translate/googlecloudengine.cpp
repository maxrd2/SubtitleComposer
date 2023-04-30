/*
    SPDX-FileCopyrightText: 2023 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "googlecloudengine.h"

#include "appglobal.h"
#include "application.h"
#include "helpers/common.h"
#include "scconfig.h"

#include <QByteArray>
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>

#include <KLocalizedString>
#include <KMessageBox>

#include <memory>
#include <type_traits>

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

using namespace SubtitleComposer;

GoogleCloudEngine::GoogleCloudEngine(QObject *parent)
	: TranslateEngine(parent),
	  m_netManager(new QNetworkAccessManager(this)),
	  m_ui(new Ui::GoogleCloudEngine)
{
}

GoogleCloudEngine::~GoogleCloudEngine()
{
	delete m_ui;
}

// The name of the function to free an EVP_MD_CTX changed in OpenSSL 1.1.0.
#if OPENSSL_VERSION_NUMBER < 0x10100000L
inline std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_destroy)>
getDigestCtx()
{
	return std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_destroy)>(EVP_MD_CTX_create(), &EVP_MD_CTX_destroy);
}
#else
inline std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)>
getDigestCtx()
{
	return std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)>(EVP_MD_CTX_new(), &EVP_MD_CTX_free);
}
#endif

#include <QScopedPointer>

static QByteArray
signStringWithPem(const QByteArray &str, const QByteArray &pem_contents, const EVP_MD *digestType)
{
	auto digestCtx = getDigestCtx();
	if(!digestCtx) {
		qWarning() << "Couldn't create OpenSSL digest context.";
		return QByteArray();
	}

	auto pemBuffer = std::unique_ptr<BIO, decltype(&BIO_free)>(
				BIO_new_mem_buf(pem_contents.data(), pem_contents.length()),
				&BIO_free);
	if(!pemBuffer) {
		qWarning() << "Couldn't create PEM buffer";
		return QByteArray();
	}

	auto privateKey = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>(
				PEM_read_bio_PrivateKey(pemBuffer.get(), nullptr, nullptr, nullptr),
				&EVP_PKEY_free);
	if(!privateKey) {
		qWarning() << "Failed to extract private key from PEM";
		return QByteArray();
	}

	if(!EVP_DigestSignInit(digestCtx.get(), nullptr, digestType, nullptr, privateKey.get())) {
		qWarning() << "could not initialize PEM digest. ";
		return QByteArray();
	}

	if(!EVP_DigestSignUpdate(digestCtx.get(), str.data(), str.length())) {
		qWarning() << "could not update PEM digest. ";
		return QByteArray();
	}

	std::size_t signatureSize = 0;
	if(!EVP_DigestSignFinal(digestCtx.get(), nullptr, &signatureSize)) {
		qWarning() << "could not finalize PEM digest (1/2). ";
		return QByteArray();
	}

	QByteArray signature(signatureSize, char(0));
	if(!EVP_DigestSignFinal(digestCtx.get(), reinterpret_cast<unsigned char *>(signature.data()), &signatureSize)) {
		qWarning() << "could not finalize PEM digest (2/2). ";
		return QByteArray();
	}

	return signature;
}

static constexpr auto URLSafeBase64 = QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals;

static QByteArray
makeJWTAssertion(const QByteArray &header, const QByteArray &payload, const QByteArray &pem)
{
	const QByteArray body = header.toBase64(URLSafeBase64)
			+ '.' + payload.toBase64(URLSafeBase64);
	QByteArray sig = signStringWithPem(body, pem, EVP_sha256());
	if(sig.isEmpty())
		return QByteArray();
	return body + '.' + sig.toBase64(URLSafeBase64);
}

static void
showError(QNetworkReply *res)
{
	QString errText;
	const QJsonDocument doc = QJsonDocument::fromJson(res->readAll());
	if(doc.isObject()) {
		const QJsonObject err = doc[$("error")].toObject();
		if(!err.empty()) {
			errText = i18n("Remote service error %1 %2\n%3",
							err[$("code")].toInt(),
							err[$("status")].toString(),
							err[$("message")].toString());
		}
	}
	if(errText.isEmpty()) {
		const int httpCode = res->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
		if(httpCode) {
			errText = i18n("HTTP Error %1 - %2\n%3",
					httpCode,
					res->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString(),
					QString(res->readAll()));
		} else {
			errText = i18n("Network Error %1\n%2",
					int(res->error()),
					QString(res->readAll()));
		}
	}
	qWarning() << "Google Cloud Engine Error" << errText;
	KMessageBox::error(app()->mainWindow(), errText, i18n("Google Cloud Engine Error"));
}

void
GoogleCloudEngine::settings(QWidget *widget)
{
	m_ui->setupUi(widget);
	m_ui->serviceJSON->setUrl(QUrl(SCConfig::gctServiceJSON()));

	connect(m_ui->btnLogin, &QPushButton::clicked, this, &GoogleCloudEngine::login);

	m_ui->grpSettings->setEnabled(false);
	emit engineReady(false);

	if(!SCConfig::gctServiceJSON().isEmpty())
		login();
}

void
GoogleCloudEngine::login()
{
	QUrl url = m_ui->serviceJSON->url();

	if(!parseJSON())
		return;

	const bool authenticated = url.toLocalFile() == SCConfig::gctServiceJSON()
			&& !SCConfig::gctAccessToken().isEmpty()
			&& SCConfig::gctTokenExpires() > QDateTime::currentDateTime();
	// NOTE there is this backend to retrieve token details
	// https://www.googleapis.com/oauth2/v3/tokeninfo?access_token=<access_token>
	if(authenticated) {
		languagesUpdate();
	} else {
		SCConfig::setGctServiceJSON(url.toLocalFile());
		authenticate();
	}
}

bool
GoogleCloudEngine::parseJSON()
{
	QFile file(SCConfig::gctServiceJSON());
	if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		KMessageBox::error(app()->mainWindow(),
			i18n("Error opening file '%1'", SCConfig::gctServiceJSON()),
			i18n("Google Cloud Engine Error"));
		return false;
	}
	const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
	file.close();

	if(doc.isNull()) {
		KMessageBox::error(app()->mainWindow(),
			i18n("Error parsing JSON from file '%1'", SCConfig::gctServiceJSON()),
			i18n("Google Cloud Engine Error"));
		return false;
	}

	m_projectId = doc[$("project_id")].toString();
	m_clientEmail = doc[$("client_email")].toString();
	m_privateKey = doc[$("private_key")].toString();
	m_privateKeyId = doc[$("private_key_id")].toString();
	m_subject = doc[$("subject")].toString();
	m_tokenUrl = doc[$("token_uri")].toString();
	return true;
}

bool
GoogleCloudEngine::authenticate()
{
	// Authenticate with Service Account OAuth2
	// https://developers.google.com/identity/protocols/oauth2/service-account#httprest

	using clk = std::chrono::system_clock;
	clk::time_point tp = clk::now();

	QJsonObject header;
	header.insert($("alg"), $("RS256"));
	header.insert($("typ"), $("JWT"));
	header.insert($("kid"), m_privateKeyId);
	QJsonObject payload;
	payload.insert($("iss"), m_clientEmail);
	payload.insert($("scope"), $("https://www.googleapis.com/auth/cloud-platform"));
	payload.insert($("aud"), m_tokenUrl);
	payload.insert($("iat"), static_cast<qint64>(clk::to_time_t(tp)));
	payload.insert($("exp"), static_cast<qint64>(clk::to_time_t(tp + std::chrono::seconds(3600))));
	if(!m_subject.isEmpty())
		payload.insert($("sub"), m_subject);

	QByteArray assertionData = makeJWTAssertion(
		QJsonDocument(header).toJson(QJsonDocument::Compact),
		QJsonDocument(payload).toJson(QJsonDocument::Compact),
		m_privateKey.toUtf8());
	QUrlQuery postData;
	postData.addQueryItem($("grant_type"), $("urn:ietf:params:oauth:grant-type:jwt-bearer"));
	postData.addQueryItem($("assertion"), assertionData);

	QNetworkRequest request(m_tokenUrl);
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

	QNetworkReply *res = m_netManager->post(request, postData.query(QUrl::FullyEncoded).toUtf8());
	connect(res, &QNetworkReply::finished, this, &GoogleCloudEngine::authenticated);

	return true;
}

void
GoogleCloudEngine::authenticated()
{
	QNetworkReply *res = qobject_cast<QNetworkReply *>(sender());
	res->deleteLater();

	if(res->error() == QNetworkReply::NoError) {
		const QJsonDocument doc = QJsonDocument::fromJson(res->readAll());
		SCConfig::setGctAccessToken(doc[$("access_token")].toString());
		SCConfig::setGctTokenExpires(QDateTime::currentDateTime().addSecs(doc[$("expires_in")].toInt()));
		SCConfig::setGctTokenType(doc[$("token_type")].toString());
		languagesUpdate();
	} else {
		showError(res);
	}
}

bool
GoogleCloudEngine::languagesUpdate()
{
	QNetworkRequest request($("https://translation.googleapis.com/v3/projects/%1/locations/global/supportedLanguages").arg(m_projectId));
	request.setRawHeader("Authorization", QByteArray("Bearer ") + SCConfig::gctAccessToken().toUtf8());

	QNetworkReply *res = m_netManager->get(request);
	connect(res, &QNetworkReply::finished, this, &GoogleCloudEngine::languagesUpdated);

	return true;
}

void
GoogleCloudEngine::languagesUpdated()
{
	QNetworkReply *res = qobject_cast<QNetworkReply *>(sender());
	res->deleteLater();

	if(res->error() == QNetworkReply::NoError) {
		m_ui->langSource->clear();
		m_ui->langSource->addItem(i18n("Autodetect Language"), QString());
		m_ui->langTranslation->clear();

		const QJsonDocument doc = QJsonDocument::fromJson(res->readAll());
		const QJsonArray langs = doc[$("languages")].toArray();
		for(auto it = langs.cbegin(); it != langs.cend(); ++it) {
			const QString langCode = it->toObject().value($("languageCode")).toString();
			const QString langTitle = QLocale(langCode).nativeLanguageName();
			const QString ttl = langCode % $(" - ") % langTitle;
			if(it->toObject().value($("supportSource")).toBool()) {
				m_ui->langSource->addItem(ttl, langCode);
				if(langCode == SCConfig::gctLangSource())
					m_ui->langSource->setCurrentIndex(m_ui->langSource->count() - 1);
			}
			if(it->toObject().value($("supportTarget")).toBool()) {
				m_ui->langTranslation->addItem(ttl, langCode);
				if(langCode == SCConfig::gctLangTrans())
					m_ui->langTranslation->setCurrentIndex(m_ui->langTranslation->count() - 1);
			}
		}
		m_ui->grpSettings->setEnabled(true);
		emit engineReady(true);
	} else {
		showError(res);
	}
}

void
GoogleCloudEngine::translate(QVector<QString> &textLines)
{
	SCConfig::setGctLangSource(m_ui->langSource->currentData().toString());
	SCConfig::setGctLangTrans(m_ui->langTranslation->currentData().toString());

	QNetworkRequest request($("https://translation.googleapis.com/v3/projects/%1:translateText").arg(m_projectId));
	request.setRawHeader("Authorization", QByteArray("Bearer ") + SCConfig::gctAccessToken().toUtf8());
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json; charset=utf-8");

	QJsonObject reqData;
	if(!m_ui->langSource->currentData().isValid())
		reqData.insert($("sourceLanguageCode"), m_ui->langSource->currentData().toString());
	reqData.insert($("targetLanguageCode"), m_ui->langTranslation->currentData().toString());

	int *reqCount = new int;
	auto translateDone = [=](){
		if(--(*reqCount) == 0) {
			delete reqCount;
			emit translated();
		}
	};

	int line = 0;
	*reqCount = 1;
	while(line != textLines.size()) {
		// NOTE there is 100000 char/minute limit: https://cloud.google.com/translate/quotas
		// We're doing multiple requests with each sending 100 lines for translations - that should help
		// throttling things to under 100k/min - if not do proper throttling and waiting
		constexpr const int lineLimit = 100;
		const int off = line;
		QJsonArray textArray;
		for(;;) {
			textArray.append(textLines.at(line));
			if(++line % lineLimit == 0 || line == textLines.size())
				break;
		}
		reqData.insert($("contents"), textArray);

		(*reqCount)++;
		QNetworkReply *res = m_netManager->post(QNetworkRequest(request), QJsonDocument(reqData).toJson(QJsonDocument::Compact));
		connect(res, &QNetworkReply::finished, this, [=, &textLines](){
			res->deleteLater();
			if(res->error() == QNetworkReply::NoError) {
				const QJsonDocument doc = QJsonDocument::fromJson(res->readAll());
				const QJsonArray tta = doc[$("translations")].toArray();
				for(int i = 0, n = tta.size(); i < n; i++)
					textLines[off + i] = tta.at(i).toObject().value($("translatedText")).toString();
			} else {
				showError(res);
			}
			translateDone();
		});
	}
	translateDone();
}
