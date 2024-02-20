/*
    SPDX-FileCopyrightText: 2023 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef DEEPLENGINE_H
#define DEEPLENGINE_H

#include "ui_deeplengine.h"

#include "translate/translateengine.h"

#include <QDateTime>
#include <QList>

QT_FORWARD_DECLARE_CLASS(QNetworkAccessManager)

namespace SubtitleComposer {

class DeepLEngine : public TranslateEngine
{
	Q_OBJECT

public:
	explicit DeepLEngine(QObject *parent = nullptr);
	virtual ~DeepLEngine();

	QString name() const override { return QStringLiteral("DeepL"); }

	void settings(QWidget *widget) override;
	void translate(QVector<QString> &textLines) override;

private:
	bool languagesUpdate();
	void languagesUpdated(QNetworkReply *res);

private:
	QNetworkAccessManager *m_netManager;
	Ui::DeepLEngine *m_ui;
};
} // namespace SubtitleComposer

#endif // DEEPLENGINE_H
