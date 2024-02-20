/*
    SPDX-FileCopyrightText: 2023 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef MINTENGINE_H
#define MINTENGINE_H

#include "ui_mintengine.h"

#include "translate/translateengine.h"

#include <map>

#include <QDateTime>
#include <QList>

QT_FORWARD_DECLARE_CLASS(QJsonObject)
QT_FORWARD_DECLARE_CLASS(QNetworkAccessManager)

namespace SubtitleComposer {

class MinTEngine : public TranslateEngine
{
	Q_OBJECT

public:
	explicit MinTEngine(QObject *parent = nullptr);
	virtual ~MinTEngine();

	QString name() const override { return QStringLiteral("MinT machine translation"); }

	void settings(QWidget *widget) override;
	void translate(QVector<QString> &textLines) override;

private:
	void languagesUpdate();
	void languagesRefreshUI();
	QString languageTitle(const QString &code) const;

private:
	QNetworkAccessManager *m_netManager;
	const QJsonObject *m_langs;
	std::map<QString, QString> m_langName;
	Ui::MinTEngine *m_ui;
};
} // namespace SubtitleComposer

#endif // MINTENGINE_H
