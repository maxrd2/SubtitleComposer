/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "core/subtitle.h"

#include <kxmlguiwindow.h>

namespace SubtitleComposer {
class PlayerWidget;
class SubtitleMetaWidget;
class LinesWidget;
class CurrentLineWidget;
class WaveformWidget;

class MainWindow : public KXmlGuiWindow
{
	Q_OBJECT

	friend class Application;

public:
	MainWindow();
	virtual ~MainWindow();

	void loadConfig();
	void saveConfig();

	QMenu *createPopupMenu() override;

protected:
	bool queryClose() override;

protected:
	PlayerWidget *m_playerWidget;
	SubtitleMetaWidget *m_metaWidget;
	LinesWidget *m_linesWidget;
	CurrentLineWidget *m_curLineWidget;
	WaveformWidget *m_waveformWidget;
};
}
#endif
