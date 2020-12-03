#ifndef CURRENTLINEWIDGET_H
#define CURRENTLINEWIDGET_H

/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2019 Mladen Milinkovic <max@smoothware.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "core/subtitle.h"
#include "core/subtitleline.h"

#include <QWidget>

QT_FORWARD_DECLARE_CLASS(QTimer)
QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QToolButton)
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
	void onPrimaryTextEditChanged();
	void onSecondaryTextEditChanged();
	void onShowTimeEditChanged(int showTime);
	void onHideTimeEditChanged(int hideTime);
	void onDurationTimeEditChanged(int durationTime);

	void onLineAnchorChanged(const SubtitleLine *line, bool anchored);

	void onLinePrimaryTextChanged(const SString &primaryText);
	void onLineSecondaryTextChanged(const SString &secondaryText);
	void onLineShowTimeChanged(const Time &showTime);
	void onLineHideTimeChanged(const Time &hideTime);

	void onConfigChanged();

private:
	QToolButton * createToolButton(const QString &text, const char *icon, bool checkable=true);
	QWidget * createLineWidgetBox(int index);

	QString buildTextDescription(const QString &text);

protected:
	Subtitle *m_subtitle;
	SubtitleLine *m_currentLine;
	bool m_translationMode;

	int m_userChangingText = 0;

	TimeEdit *m_showTimeEdit;
	TimeEdit *m_hideTimeEdit;
	TimeEdit *m_durationTimeEdit;

	QWidget *m_boxPrimary = nullptr;
	QWidget *m_boxTranslation = nullptr;
	SimpleRichTextEdit *m_textEdits[2] = {0};
	QLabel *m_textLabels[2] = {0};

//	QTimer *m_updateShorcutsTimer;
};
}
#endif
