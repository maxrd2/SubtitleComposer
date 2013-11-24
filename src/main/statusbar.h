#ifndef STATUSBAR_H
#define STATUSBAR_H

/***************************************************************************
 *   Copyright (C) 2007-2009 Sergio Pistone (sergio_pistone@yahoo.com.ar)  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "../core/subtitle.h"

#include <KStatusBar>

class QProgressBar;
class QToolButton;

namespace SubtitleComposer {
// NOTE "StatusBar" seems to be conflicting with something so we have to use another class name
class StatusBar2 : public KStatusBar
{
	Q_OBJECT

public:
	StatusBar2(QWidget *parent = 0);
	virtual ~StatusBar2();

	void loadConfig();
	void saveConfig();

	void plugActions();

public slots:
	void setSubtitle(Subtitle *subtitle = 0);

	void initDecoding();
	void setDecodingPosition(double position);
	void setDecodingLength(double length);
	void endDecoding();

protected:
	virtual void showEvent(QShowEvent *event);

private:
	QProgressBar *m_decodingProgressBar;
	QToolButton *m_cancelDecodingButton;
};
}
#endif
