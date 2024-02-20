/*
    SPDX-FileCopyrightText: 2023 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "mintengine.h"

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
#include <set>
#include <type_traits>

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/pem.h>


using namespace SubtitleComposer;

MinTEngine::MinTEngine(QObject *parent)
	: TranslateEngine(parent),
	  m_netManager(new QNetworkAccessManager(this)),
	  m_langs(nullptr),
	  m_ui(new Ui::MinTEngine)
{
}

MinTEngine::~MinTEngine()
{
	delete m_langs;
	delete m_ui;
}

static void
showError(QNetworkReply *res)
{
	QString errText;
	const QJsonDocument doc = QJsonDocument::fromJson(res->readAll());
	if(doc.isObject()) {
		const QJsonObject err = doc[QLatin1String("detail")].toObject();
		if(!err.empty()) {
			errText = i18n("Remote service error %1 %2\n%3",
							err[QLatin1String("type")].toInt(),
							err[QLatin1String("loc")].toString(),
							err[QLatin1String("message")].toString());
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
	qWarning() << "MinT Engine Error" << errText;
	KMessageBox::error(app()->mainWindow(), errText, i18n("MinT Engine Error"));
}

void
MinTEngine::settings(QWidget *widget)
{
	m_ui->setupUi(widget);

	m_ui->apiURLPrefix->setText(SCConfig::mintURLPrefix());

	connect(m_ui->langSource, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](){
		if(!m_langs || m_langs->empty())
			return;
		SCConfig::setMintLangSource(m_ui->langSource->currentData().toString());
		languagesRefreshUI();
	});

	connect(m_ui->apiURLPrefix, &QLineEdit::textChanged, this, [&](const QString &urlPrefix){
		if(urlPrefix.isEmpty() || urlPrefix.back() == QChar('/')) {
			SCConfig::setMintURLPrefix(urlPrefix);
		} else {
			QSignalBlocker b(m_ui->apiURLPrefix);
			SCConfig::setMintURLPrefix(urlPrefix + QChar('/'));
			m_ui->apiURLPrefix->setText(SCConfig::mintURLPrefix());
		}
		languagesUpdate();
	});

	connect(m_ui->btnUpdate, &QPushButton::clicked, this, [&](){
		languagesUpdate();
	});

	m_ui->grpSettings->setEnabled(false);
	emit engineReady(false);

	if(!SCConfig::mintURLPrefix().isEmpty())
		QMetaObject::invokeMethod(this, &MinTEngine::languagesUpdate, Qt::QueuedConnection);
}

void
MinTEngine::languagesUpdate()
{
	if(SCConfig::mintURLPrefix().isEmpty())
		return;

	ProgressLock pl(this, i18n("Updating languages..."));
	QNetworkRequest req;

	req.setUrl($("https://en.wikipedia.org/w/api.php?action=query&liprop=autonym|name&meta=languageinfo&uselang=en&format=json&origin=*"));
	sendRequest(m_netManager, req, QByteArray(), [this](QNetworkReply *res){
		if(res->error() == QNetworkReply::NoError) {
			m_langName.clear();

			const QJsonDocument doc = QJsonDocument::fromJson(res->readAll());
			const QJsonObject langs = doc[QLatin1String("query")][QLatin1String("languageinfo")].toObject();
			for(auto it = langs.constBegin(); it != langs.constEnd(); ++it)
				m_langName.insert({it.key(), it.value()[QLatin1String("name")].toString()});
			languagesRefreshUI();
		} else {
			showError(res);
		}
	});

	req.setUrl(SCConfig::mintURLPrefix() + $("api/languages"));
	sendRequest(m_netManager, req, QByteArray(), [this](QNetworkReply *res){
		if(res->error() == QNetworkReply::NoError) {
			const QJsonDocument doc = QJsonDocument::fromJson(res->readAll());
			delete m_langs;
			m_langs = new QJsonObject(doc.object());

			if(SCConfig::mintLangSource().isEmpty() && !m_langs->empty())
				SCConfig::setMintLangSource(m_langs->constBegin().key());

			if(SCConfig::mintLangTrans().isEmpty()) {
				const QJsonObject obj = m_langs->constBegin()->toObject();
				if(!obj.empty())
					SCConfig::setMintLangTrans(obj.constBegin().key());
			}

			languagesRefreshUI();
		} else {
			showError(res);
		}
	});
}

QString
MinTEngine::languageTitle(const QString &code) const
{
	auto it = m_langName.find(code);
	if(it != m_langName.cend() && !it->second.isEmpty())
		return code % $(" - ") % it->second;

	QString title = QLocale(code).nativeLanguageName().toLower();
	if(title.isEmpty())
		return code;

	title[0] = title[0].toUpper();
	return code % $(" - ") % title;
}

void
MinTEngine::languagesRefreshUI()
{
	if(!m_langs || m_langs->empty())
		return;

	QSignalBlocker b(m_ui->langSource);

	m_ui->langSource->clear();
	m_ui->langTranslation->clear();

	for(auto it = m_langs->constBegin(); it != m_langs->constEnd(); ++it) {
		const QString srcLangCode = it.key();
		m_ui->langSource->addItem(languageTitle(srcLangCode), srcLangCode);

		if(srcLangCode == SCConfig::mintLangSource()) {
			m_ui->langSource->setCurrentIndex(m_ui->langSource->count() - 1);

			const QJsonObject dstLangs = it->toObject();
			for(auto di = dstLangs.constBegin(); di != dstLangs.constEnd(); ++di) {
				const QString dstLangCode = di.key();
				m_ui->langTranslation->addItem(languageTitle(dstLangCode), dstLangCode);
				if(dstLangCode == SCConfig::mintLangTrans())
					m_ui->langTranslation->setCurrentIndex(m_ui->langTranslation->count() - 1);
			}
		}
	}
	m_ui->grpSettings->setEnabled(true);
	emit engineReady(true);
}

void
MinTEngine::translate(QVector<QString> &textLines)
{
	SCConfig::setMintLangSource(m_ui->langSource->currentData().toString());
	SCConfig::setMintLangTrans(m_ui->langTranslation->currentData().toString());

	if(SCConfig::mintURLPrefix().isEmpty())
		return;

	ProgressLock pl(this, i18n("Translating lines..."));

	QNetworkRequest request(SCConfig::mintURLPrefix() + $("api/translate"));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

	QJsonObject reqData;
	reqData.insert(QLatin1String("format"), QJsonValue(QLatin1String("json")));
	reqData.insert(QLatin1String("source_language"), QJsonValue(SCConfig::mintLangSource()));
	reqData.insert(QLatin1String("target_language"), QJsonValue(SCConfig::mintLangTrans()));

	int line = 0;
	while(line != textLines.size()) {
		// request body size must not exceed some unknown size limit - will use 5K
		constexpr const int sizeLimit = 5 << 10;
		const int off = line;

		QJsonArray transLines;
		uint32_t textSize = 0;
		for(;;) {
			const QString &ln = textLines.at(line);
			textSize += ln.size();
			if(textSize >= sizeLimit)
				break;
			transLines.append(QJsonValue(ln));
			if(++line == textLines.size())
				break;
		}
		QJsonObject data(reqData);
		data.insert(QLatin1String("content"), QJsonValue(QJsonDocument(transLines).toJson(QJsonDocument::Compact).constData()));

		sendRequest(m_netManager, request, QJsonDocument(data).toJson(QJsonDocument::Compact), [=, &textLines](QNetworkReply *res){
			if(res->error() == QNetworkReply::NoError) {
				const QJsonDocument doc = QJsonDocument::fromJson(res->readAll());
				const QString trans = doc[QLatin1String("translation")].toString();
				const QJsonDocument ttd = QJsonDocument::fromJson(trans.toUtf8());
				const QJsonArray tta = ttd.array();
				for(int i = 0, n = tta.size(); i < n; i++)
					textLines[off + i] = tta.at(i).toString();
			} else {
				showError(res);
			}
		});
	}
}
