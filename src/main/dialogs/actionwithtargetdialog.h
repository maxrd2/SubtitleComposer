#ifndef ACTIONWITHTARGETDIALOG_H
#define ACTIONWITHTARGETDIALOG_H

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
#include "../../core/subtitle.h"

#include <QtGui/QWidget>

#include <KLocale>

class QGridLayout;
class QGroupBox;
class QButtonGroup;

namespace SubtitleComposer
{
	class ActionWithTargetDialog : public ActionDialog
	{
		Q_OBJECT

		public:

			typedef enum { AllLines=0, Selection, FromSelected, UpToSelected, None } LinesTarget;

			/// LINES TARGET
			LinesTarget selectedLinesTarget() const;
			void setSelectedLinesTarget( ActionWithTargetDialog::LinesTarget target );

			bool isLinesTargetEnabled( LinesTarget target ) const;
			void setLinesTargetEnabled( LinesTarget target, bool enabled );

			/// TEXTS TARGET
			// the active text target when translationMode is false
			Subtitle::TextTarget nonTranslationModeTarget() const;
			void setNonTranslationModeTarget( Subtitle::TextTarget target );

			Subtitle::TextTarget selectedTextsTarget() const;
			void setSelectedTextsTarget( Subtitle::TextTarget target );

			bool isTextsTargetEnabled( Subtitle::TextTarget target ) const;
			void setTextsTargetEnabled( Subtitle::TextTarget target, bool enabled );

		public slots:

			virtual int exec();
			virtual void show();

		protected:

			explicit ActionWithTargetDialog( const QString& title, QWidget* parent=0 );

			bool selectionTargetOnlyMode() const;
			bool translationMode() const;

			QGroupBox* createTargetsGroupBox( const QString& title=i18n( "Apply To" ), bool addToLayout=true );

			void setTargetsButtonsHiddenState( QButtonGroup* targetButtonGroup, bool hidden );
			void updateTargetsGroupBoxHiddenState();

			void createLineTargetsButtonGroup();
			void createTextTargetsButtonGroup();

			virtual void setSelectionTargetOnlyMode( bool value );
			virtual void setTranslationMode( bool value );

		private:

			void _setSelectionTargetOnlyMode( bool value, bool force );
			void _setTranslationMode( bool enabled, bool force );

		private slots:

			void onDefaultButtonClicked();

		protected:

			QGroupBox* m_targetGroupBox;
			QGridLayout* m_targetLayout;

			QButtonGroup* m_lineTargetsButtonGroup;
			QButtonGroup* m_textTargetsButtonGroup;

		private:

			bool m_selectionTargetOnlyMode;
			bool m_selectionTargetWasChecked;

			bool m_translationMode;
			Subtitle::TextTarget m_nonTranslationModeTarget;
	};


	class ActionWithLinesTargetDialog : public ActionWithTargetDialog
	{
		Q_OBJECT

		public:

			explicit ActionWithLinesTargetDialog( const QString& title, QWidget* parent=0 );
			ActionWithLinesTargetDialog( const QString& title, const QString& desc, QWidget* parent=0 );

		public slots:

			virtual int exec();
	};

	class ActionWithTextsTargetDialog : public ActionWithTargetDialog
	{
		Q_OBJECT

		public:

			explicit ActionWithTextsTargetDialog( const QString& title, QWidget* parent=0 );
			ActionWithTextsTargetDialog( const QString& title, const QString& desc, QWidget* parent=0 );

		public slots:

			virtual int exec();
	};

	class ActionWithLinesAndTextsTargetDialog : public ActionWithTargetDialog
	{
		Q_OBJECT

		public:

			explicit ActionWithLinesAndTextsTargetDialog( const QString& title, QWidget* parent=0 );
			ActionWithLinesAndTextsTargetDialog( const QString& title, const QString& desc, QWidget* parent=0 );

		public slots:

			virtual int exec();
	};

}

#endif
