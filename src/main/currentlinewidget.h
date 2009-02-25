#ifndef CURRENTLINEWIDGET_H
#define CURRENTLINEWIDGET_H

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
#include "../core/subtitleline.h"

#include <QtGui/QWidget>

class QTimer;
class QGridLayout;
class QLabel;
class QToolButton;
class TimeEdit;
class SimpleRichTextEdit;

namespace SubtitleComposer
{
	class CurrentLineWidget : public QWidget
	{
		Q_OBJECT

		public:

			CurrentLineWidget( QWidget* parent );
			virtual ~CurrentLineWidget();

			QString focusedText() const;

			void loadConfig();
			void saveConfig();

			void setupActions();

		public slots:

			void setSubtitle( Subtitle* subtitle=0 );
			void setCurrentLine( SubtitleLine* line );

			void setTranslationMode( bool enabled );

			void highlightPrimary( int startIndex, int endIndex );
			void highlightSecondary( int startIndex, int endIndex );

		protected slots:

			void onPrimaryTextEditSelectionChanged();
			void onSecondaryTextEditSelectionChanged();

			void onPrimaryTextEditChanged();
			void onSecondaryTextEditChanged();
			void onShowTimeEditChanged( int showTime );
			void onHideTimeEditChanged( int hideTime );
			void onDurationTimeEditChanged( int durationTime );

			void onLinePrimaryTextChanged( const SString& primaryText );
			void onLineSecondaryTextChanged( const SString& secondaryText );
			void onLineShowTimeChanged( const Time& showTime );
			void onLineHideTimeChanged( const Time& hideTime );

			void onSpellingOptionChanged( const QString& option, const QString& value );

			void markUpdateShortcuts();
			void updateShortcuts();

		private:

			QToolButton* createToolButton( const QString& text, const char* icon, QObject* receiver, const char* slot );

			QString buildTextDescription( const QString& text );

		protected:

			SubtitleLine* m_currentLine;
			bool m_translationMode;

			bool m_updateCurrentLine;
			bool m_updateControls;

			TimeEdit* m_showTimeEdit;
			TimeEdit* m_hideTimeEdit;
			TimeEdit* m_durationTimeEdit;

			QLabel* m_textLabels[2];
			SimpleRichTextEdit* m_textEdits[2];
			QToolButton* m_italicButtons[2];
			QToolButton* m_boldButtons[2];
			QToolButton* m_underlineButtons[2];
			QToolButton* m_strikeThroughButtons[2];

			QGridLayout* m_mainLayout;

			QTimer* m_updateShorcutsTimer;
	};
}

#endif
