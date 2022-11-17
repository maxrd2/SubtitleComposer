/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CURRENTLINEWIDGET_H
#define CURRENTLINEWIDGET_H

#include "core/subtitle.h"
#include "core/subtitleline.h"

#include <QExplicitlySharedDataPointer>
#include <QWidget>

QT_FORWARD_DECLARE_CLASS(QTimer)
QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QToolButton)
QT_FORWARD_DECLARE_CLASS(QTextDocument)
class TimeEdit;

namespace SubtitleComposer {
class SimpleRichTextEdit;

class CurrentLineWidget : public QWidget
{
	Q_OBJECT

public:
	CurrentLineWidget(QWidget *parent);
	virtual ~CurrentLineWidget();

	QString focusedText() const;

	void loadConfig();
	void saveConfig();

public slots:
	void setSubtitle(Subtitle *subtitle = nullptr);
	void setCurrentLine(SubtitleLine *line);

	void setTranslationMode(bool enabled);

	void selectPrimaryText(int startIndex, int endIndex);
	void selectTranslationText(int startIndex, int endIndex);

protected slots:
	void onShowTimeEditChanged(int showTime);
	void onHideTimeEditChanged(int hideTime);
	void onDurationTimeEditChanged(int durationTime);

	void onLineAnchorChanged(const SubtitleLine *line, bool anchored);

	void onLineTimesChanged(const Time &showTime, const Time &hideTime);
	void onLineShowTimeChanged(const Time &showTime);
	void onLineHideTimeChanged(const Time &hideTime);

	void onConfigChanged();

private:
	QToolButton * createToolButton(const QString &text, const char *icon, bool checkable=true);
	QWidget * createLineWidgetBox(int index);

	QString buildTextDescription(bool primary);
	void updateLabels();

private:
	QExplicitlySharedDataPointer<const Subtitle> m_subtitle;
	SubtitleLine *m_currentLine;
	bool m_translationMode;

	static QTextDocument m_blankDoc;

	TimeEdit *m_showTimeEdit;
	TimeEdit *m_hideTimeEdit;
	TimeEdit *m_durationTimeEdit;

	QWidget *m_boxPrimary = nullptr;
	QWidget *m_boxTranslation = nullptr;
	SimpleRichTextEdit *m_textEdits[2] = {0};
	QLabel *m_textLabels[2] = {0};
};
}
#endif
