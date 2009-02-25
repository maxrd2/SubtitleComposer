#ifndef DURATIONLIMITSDIALOG_H
#define DURATIONLIMITSDIALOG_H

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

#include "actionwithtargetdialog.h"
#include "../../core/time.h"

class QGroupBox;
class QCheckBox;
class TimeEdit;

namespace SubtitleComposer
{
	class DurationLimitsDialog : public ActionWithTargetDialog
	{
		Q_OBJECT

		public:

			DurationLimitsDialog( const Time& minDuration, const Time& maxDuration, QWidget* parent=0 );

			Time minDuration() const;
			Time maxDuration() const;

			bool enforceMaxDuration() const;
			bool enforceMinDuration() const;
			bool preventOverlap() const;

		private slots:

			void onMinDurationValueChanged( int value );
			void onMaxDurationValueChanged( int value );

		private:

			QGroupBox* m_maxGroupBox;
			TimeEdit* m_maxDurationTimeEdit;

			QGroupBox* m_minGroupBox;
			QCheckBox* m_preventOverlapCheckBox;
			TimeEdit* m_minDurationTimeEdit;
	};
}

#endif
