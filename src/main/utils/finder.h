#ifndef FINDER_H
#define FINDER_H

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

#include "../../core/subtitle.h"
#include "../../core/subtitleline.h"

#include <QtCore/QObject>

class QGroupBox;
class QRadioButton;
class KFind;
class KFindDialog;

namespace SubtitleComposer
{
	class SubtitleIterator;

	class Finder : public QObject
	{
		Q_OBJECT

		public:

			explicit Finder( QWidget* parent=0 );
			virtual ~Finder();

			QWidget* parentWidget();

		public slots:

			void setSubtitle( Subtitle* subtitle=0 );
			void setTranslationMode( bool enabled );

			void find( const RangeList& selectionRanges, int currentIndex, const QString& text=QString(), bool findBackwards=false );
			bool findNext();
			bool findPrevious();

		signals:

			void found( SubtitleLine* line, bool primary, int startIndex, int endIndex );

		private slots:

			void invalidate();

			void onLinePrimaryTextChanged( const SString& text );
			void onLineSecondaryTextChanged( const SString& text );

			void onHighlight( const QString& text, int matchingIndex, int matchedLength );

			void onIteratorSynchronized( int firstIndex, int lastIndex, bool inserted );

		private:

			void advance();

		private:

			Subtitle* m_subtitle;
			bool m_translationMode;
			bool m_feedingPrimary;

			KFind* m_find;
			KFindDialog* m_dialog;
			QGroupBox* m_targetGroupBox;
			QRadioButton* m_targetRadioButtons[SubtitleLine::TextTargetSIZE];
			SubtitleIterator* m_iterator;
			SubtitleLine* m_dataLine;
			bool m_instancesFound;
			int m_allSearchedIndex;
	};
}

#endif
