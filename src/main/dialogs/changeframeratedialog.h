#ifndef CHANGEFRAMERATEDIALOG_H
#define CHANGEFRAMERATEDIALOG_H

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

class KComboBox;

namespace SubtitleComposer
{
	class ChangeFrameRateDialog : public ActionDialog
	{
		Q_OBJECT

		public:

			explicit ChangeFrameRateDialog( double fromFramesPerSecond, QWidget* parent=0 );

			double fromFramesPerSecond() const;
			void setFromFramesPerSecond( double framesPerSecond );

			double toFramesPerSecond() const;
			void setNewFramesPerSecond( double framesPerSecond );

		private slots:

			void onTextChanged();

		private:

			KComboBox* m_fromFramesPerSecondComboBox;
			KComboBox* m_toFramesPerSecondComboBox;
	};
}

#endif
