/*
    SPDX-FileCopyrightText: 2023 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "deeplengine.h"

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

DeepLEngine::DeepLEngine(QObject *parent)
	: TranslateEngine(parent),
	  m_netManager(new QNetworkAccessManager(this)),
	  m_ui(new Ui::DeepLEngine)
{
}

DeepLEngine::~DeepLEngine()
{
	delete m_ui;
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
	qWarning() << "DeepL Engine Error" << errText;
	KMessageBox::error(app()->mainWindow(), errText, i18n("DeepL Engine Error"));
}

void
DeepLEngine::settings(QWidget *widget)
{
	m_ui->setupUi(widget);
	m_ui->authKey->setText(SCConfig::dltAuthKey());
	m_ui->apiDomain->setText(SCConfig::dltApiDomain());

	connect(m_ui->btnConnect, &QPushButton::clicked, this, [&](){
		SCConfig::setDltAuthKey(m_ui->authKey->text());
		SCConfig::setDltApiDomain(m_ui->apiDomain->text());
		languagesUpdate();
	});

	m_ui->grpSettings->setEnabled(false);
	emit engineReady(false);

	if(!SCConfig::dltAuthKey().isEmpty())
		languagesUpdate();
}

bool
DeepLEngine::languagesUpdate()
{
	ProgressLock pl(this, i18n("Updating languages..."));
	QNetworkRequest req;

	req.setUrl($("https://%1/v2/languages?type=source").arg(SCConfig::dltApiDomain()));
	req.setRawHeader("Authorization", QByteArray("DeepL-Auth-Key ") + SCConfig::dltAuthKey().toUtf8());
	sendRequest(m_netManager, req, QByteArray(), [this](QNetworkReply *r){ languagesUpdated(r); });

	req.setUrl($("https://%1/v2/languages?type=target").arg(SCConfig::dltApiDomain()));
	req.setRawHeader("Authorization", QByteArray("DeepL-Auth-Key ") + SCConfig::dltAuthKey().toUtf8());
	sendRequest(m_netManager, req, QByteArray(), [this](QNetworkReply *r){ languagesUpdated(r); });

	return true;
}

void
DeepLEngine::languagesUpdated(QNetworkReply *res)
{
	const bool isSource = res->request().url().query().endsWith($("=source"));
	if(res->error() == QNetworkReply::NoError) {
		if(isSource) {
			m_ui->langSource->clear();
			m_ui->langSource->addItem(i18n("Autodetect Language"), QString());
		} else {
			m_ui->langTranslation->clear();
		}

		const QJsonDocument doc = QJsonDocument::fromJson(res->readAll());
		const QJsonArray langs = doc.array();
		for(auto it = langs.cbegin(); it != langs.cend(); ++it) {
			const QJsonObject o = it->toObject();
			const QString langCode = o.value($("language")).toString();
			const QString langTitle = o.value($("name")).toString()/*QLocale(langCode).nativeLanguageName()*/;
			const QString ttl = langCode % $(" - ") % langTitle;
			if(isSource) {
				m_ui->langSource->addItem(ttl, langCode);
				if(langCode == SCConfig::dltLangSource())
					m_ui->langSource->setCurrentIndex(m_ui->langSource->count() - 1);
			} else {
				m_ui->langTranslation->addItem(ttl, langCode);
				if(langCode == SCConfig::dltLangTrans())
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
DeepLEngine::translate(QVector<QString> &textLines)
{
	SCConfig::setDltLangSource(m_ui->langSource->currentData().toString());
	SCConfig::setDltLangTrans(m_ui->langTranslation->currentData().toString());

	ProgressLock pl(this, i18n("Translating lines..."));

	QNetworkRequest request($("https://%1/v2/translate").arg(SCConfig::dltApiDomain()));
	request.setRawHeader("Authorization", QByteArray("DeepL-Auth-Key ") + SCConfig::dltAuthKey().toUtf8());
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

	QUrlQuery commonPost;
	if(!SCConfig::dltLangSource().isEmpty())
		commonPost.addQueryItem($("source_lang"), SCConfig::dltLangSource());
	commonPost.addQueryItem($("target_lang"), SCConfig::dltLangTrans());
	const QByteArray commonPostData = commonPost.query(QUrl::FullyEncoded).toUtf8();

	int line = 0;
	while(line != textLines.size()) {
		// Request body size must not exceed 128 KiB (128 · 1024 bytes)
		constexpr const int sizeLimit = 128 << 10;
		const int off = line;

		QByteArray postData = commonPostData;
		for(;;) {
			static const char key[] = "&text=";
			const QByteArray val = QUrl::toPercentEncoding(textLines.at(line));
			const QString &ln = textLines.at(line);
			if(postData.size() + sizeof(key) - 1 + ln.size() >= sizeLimit)
				break;
			postData.append(key, sizeof(key) - 1).append(val);
			if(++line == textLines.size())
				break;
		}

		sendRequest(m_netManager, request, postData, [=, &textLines](QNetworkReply *res){
			if(res->error() == QNetworkReply::NoError) {
				const QJsonDocument doc = QJsonDocument::fromJson(res->readAll());
				const QJsonArray tta = doc[$("translations")].toArray();
				for(int i = 0, n = tta.size(); i < n; i++)
					textLines[off + i] = tta.at(i).toObject().value($("text")).toString();
			} else {
				showError(res);
			}
		});
	}
}
