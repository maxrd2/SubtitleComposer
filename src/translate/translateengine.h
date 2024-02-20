/*
    SPDX-FileCopyrightText: 2023 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef TRANSLATEENGINE_H
#define TRANSLATEENGINE_H

#include <QObject>
#include <QProgressDialog>
#include <QString>
#include <QVector>
#include <QWidget>

#include <klocalizedstring.h>

#include <functional>

QT_FORWARD_DECLARE_CLASS(QNetworkAccessManager);
QT_FORWARD_DECLARE_CLASS(QNetworkRequest)
QT_FORWARD_DECLARE_CLASS(QNetworkReply)

namespace SubtitleComposer {

class TranslateEngine : public QObject
{
	Q_OBJECT

public:
	explicit TranslateEngine(QObject *parent=nullptr);

	virtual QString name() const = 0;

	virtual void settings(QWidget *widget) = 0;
	virtual void translate(QVector<QString> &textLines) = 0;

signals:
	void engineReady(bool status);
	void translated();

protected:
	void sendRequest(QNetworkAccessManager *nm, const QNetworkRequest &request, const QByteArray &data, std::function<void(QNetworkReply*)> callback);

	struct ProgressLock {
		ProgressLock(TranslateEngine *e, const QString &progressText);
		~ProgressLock();
		TranslateEngine *te;
	};

private:
	void translateStart();
	void translateDone();

	struct ProgressHelper;

	QProgressDialog *m_progress;
	ProgressHelper *m_ph;
};
} // namespace SubtitleComposer

#endif // TRANSLATEENGINE_H
