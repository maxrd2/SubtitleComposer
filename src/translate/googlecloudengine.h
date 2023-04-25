/*
    SPDX-FileCopyrightText: 2023 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef GOOGLECLOUDENGINE_H
#define GOOGLECLOUDENGINE_H

#include "ui_googlecloudengine.h"

#include "translate/translateengine.h"

#include <QDateTime>
#include <QList>

QT_FORWARD_DECLARE_CLASS(QNetworkAccessManager)

namespace SubtitleComposer {

class GoogleCloudEngine : public TranslateEngine
{
	Q_OBJECT

public:
	explicit GoogleCloudEngine(QObject *parent = nullptr);
	virtual ~GoogleCloudEngine();

	QString name() const override { return QStringLiteral("Google Cloud"); }

	void settings(QWidget *widget) override;
	void translate(QVector<QString> &textLines) override;

private:
	bool parseJSON();
	void login();
	bool authenticate();
	void authenticated();
	bool languagesUpdate();
	void languagesUpdated();

private:
	QNetworkAccessManager *m_netManager;
	Ui::GoogleCloudEngine *m_ui;

	QString m_projectId;
	QString m_clientEmail;
	QString m_privateKeyId;
	QString m_privateKey;
	QString m_subject;
	QString m_tokenUrl;
};
} // namespace SubtitleComposer

#endif // GOOGLECLOUDENGINE_H
