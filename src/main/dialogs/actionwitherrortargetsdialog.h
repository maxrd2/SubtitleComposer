#ifndef ACTIONWITHERRORTARGETSDIALOG_H
#define ACTIONWITHERRORTARGETSDIALOG_H

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
#include "../../core/subtitleline.h"

class QGroupBox;
class QCheckBox;
class QGridLayout;

namespace SubtitleComposer
{
	class ActionWithErrorTargetsDialog : public ActionWithTargetDialog
	{
		Q_OBJECT

		public:

			~ActionWithErrorTargetsDialog();

			int selectedErrorFlags() const;

		protected:

			explicit ActionWithErrorTargetsDialog( const QString& title, QWidget* parent=0 );

			QGroupBox* createErrorsGroupBox( const QString& title );
			void createErrorsButtons( bool showUserMarks, bool showMissingTranslation );

		private slots:

			void selectAllErrorFlags();
			void deselectAllErrorFlags();

		private:

			QGroupBox* m_errorsGroupBox;
			QCheckBox** m_errorsCheckBox;
			QGridLayout* m_errorsLayout;
	};
}

#endif
