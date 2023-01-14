/*
    SPDX-FileCopyrightText: 2023 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef SUBTITLEVOICEDIALOG_H
#define SUBTITLEVOICEDIALOG_H

#include <QDialog>

namespace Ui {
class SubtitleVoiceDialog;
}

namespace SubtitleComposer {

class SubtitleVoiceDialog : public QDialog
{
	Q_OBJECT

	Q_PROPERTY(QString currentVoice READ currentVoice WRITE setCurrentVoice
	           NOTIFY currentVoiceChanged)

public:
	SubtitleVoiceDialog(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
	SubtitleVoiceDialog(const QString &initial, QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
	virtual ~SubtitleVoiceDialog();

	static QString getVoice(const QStringList &voices,
				const QString &initial = QString(),
				QWidget *parent = nullptr,
				const QString &title = QString());

	void setCurrentVoice(const QString &cssVoice);
	QString currentVoice() const;

	inline QString selectedVoice() const { return currentVoice(); }

signals:
	void currentVoiceChanged(const QString &cssVoice);
	void voiceSelected(const QString &cssVoice);

private:
	Ui::SubtitleVoiceDialog *ui;
};
} // namespace SubtitleComposer

#endif // SUBTITLEVOICEDIALOG_H
