#ifndef PLAYERCONFIGWIDGET_H
#define PLAYERCONFIGWIDGET_H

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

#include "playerconfig.h"
#include "../../config/appconfiggroupwidget.h"

class QCheckBox;
class QFontComboBox;
class KComboBox;
class KColorCombo;
class KIntNumInput;
class TextOverlayWidget;

namespace SubtitleComposer
{
	class PlayerConfigWidget : public AppConfigGroupWidget
	{
		Q_OBJECT

		friend class ConfigDialog;

		public:

			virtual ~PlayerConfigWidget();

			virtual void setControlsFromConfig();
			virtual void setConfigFromControls();

		private slots:

			void onFamilyChanged( const QString& family );
			void onSizeChanged( int size );
			void onPrimaryColorChanged( const QColor& color );
			void onOutlineColorChanged( const QColor& color );
			void onOutlineWidthChanged( int width );

		private:

			explicit PlayerConfigWidget( QWidget* parent=0 );

			PlayerConfig* config() { return static_cast<PlayerConfig*>( m_config ); };

		private:

			KComboBox* m_backendComboBox;
			KIntNumInput* m_seekJumpSecsSpinBox;
			QCheckBox* m_showPositionTimeEditCheckBox;

			QFontComboBox* m_fontComboBox;
			KIntNumInput* m_sizeSpinBox;
			KColorCombo* m_primaryColorComboBox;
			KColorCombo* m_outlineColorComboBox;
			KIntNumInput* m_outlineWidthSpinBox;
			TextOverlayWidget* m_textOverlayWidget;
	};
}

#endif
