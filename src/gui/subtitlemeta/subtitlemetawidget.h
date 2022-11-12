/*
    SPDX-FileCopyrightText: 2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SUBTITLEMETAWIDGET_H
#define SUBTITLEMETAWIDGET_H

#include <QExplicitlySharedDataPointer>
#include <QWidget>

#include "gui/subtitlemeta/subtitlepositionwidget.h"

QT_FORWARD_DECLARE_CLASS(QTabBar);
QT_FORWARD_DECLARE_CLASS(QTextEdit);
class KTextEdit;

namespace SubtitleComposer {
class Subtitle;
class SubtitleLine;

class SubtitleMetaWidget : public QWidget
{
	Q_OBJECT

public:
	SubtitleMetaWidget(QWidget *parent);
	virtual ~SubtitleMetaWidget();

	void setSubtitle(Subtitle *subtitle = nullptr);
	inline void setCurrentLine(SubtitleLine *line) { m_subPosition->setCurrentLine(line); }

	inline QWidget *dockTitleBar() const { return m_dockTitleWidget; };

protected:
	bool eventFilter(QObject *obj, QEvent *event) override;

private:
	QExplicitlySharedDataPointer<Subtitle> m_subtitle;
	QWidget *m_dockTitleWidget;
	QTabBar *m_tabBar;
	QTextEdit *m_cssEdit;
	KTextEdit *m_commentIntroEdit;
	KTextEdit *m_commentTopEdit;
	KTextEdit *m_commentBottomEdit;
	SubtitlePositionWidget *m_subPosition;
};

}
#endif // SUBTITLEMETAWIDGET_H
