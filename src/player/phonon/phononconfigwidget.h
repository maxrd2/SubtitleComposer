#ifndef PHONONCONFIGWIDGET_H
#define PHONONCONFIGWIDGET_H

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

#include "phononconfig.h"
#include "../../config/appconfiggroupwidget.h"

#include <QtGui/QWidget>

namespace SubtitleComposer
{
	class PhononConfigWidget : public AppConfigGroupWidget
	{
		Q_OBJECT

		friend class PhononBackend;

		public:

			virtual ~PhononConfigWidget();

			virtual void setControlsFromConfig();
			virtual void setConfigFromControls();

		private:

			explicit PhononConfigWidget( QWidget* parent=0 );

			PhononConfig* config() { return static_cast<PhononConfig*>( m_config ); };
	};
}

#endif
