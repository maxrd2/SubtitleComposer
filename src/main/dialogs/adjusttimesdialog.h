#ifndef ADJUSTTIMESDIALOG_H
#define ADJUSTTIMESDIALOG_H

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

#include "actiondialog.h"
#include "../../core/time.h"

class TimeEdit;

namespace SubtitleComposer {
class AdjustTimesDialog : public ActionDialog
{
public:
	AdjustTimesDialog(QWidget *parent = 0);

	Time firstLineTime() const;
	void setFirstLineTime(const Time &time);

	Time lastLineTime() const;
	void setLastLineTime(const Time &time);

private:
	TimeEdit *m_firstLineTimeEdit;
	TimeEdit *m_lastLineTimeEdit;
};
}
#endif
