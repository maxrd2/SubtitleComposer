/*
    SPDX-FileCopyrightText: 2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef SUBTITLEPOSITIONWIDGET_H
#define SUBTITLEPOSITIONWIDGET_H

#include <QWidget>

namespace Ui {
class SubtitlePositionWidget;
}

namespace SubtitleComposer {
class SubtitleLine;
struct SubtitleRect;

class SubtitlePositionWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SubtitlePositionWidget(QWidget *parent = nullptr);
	~SubtitlePositionWidget();

	void setCurrentLine(SubtitleLine *line);

	void updatePosition(const SubtitleRect &pos);

private:
	void onPosTop(double value);
	void onPosBottom(double value);
	void onPosLeft(double value);
	void onPosRight(double value);
	void onHAlignLeft(bool checked);
	void onHAlignCenter(bool checked);
	void onHAlignRight(bool checked);
	void onVAlignTop(bool checked);
	void onVAlignBottom(bool checked);

private:
	Ui::SubtitlePositionWidget *ui;

	SubtitleLine *m_currentLine;
};

}

#endif // SUBTITLEPOSITIONWIDGET_H
