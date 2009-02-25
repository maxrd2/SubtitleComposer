#ifndef INPUTDIALOG_H
#define INPUTDIALOG_H

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

#include <KDialog>

class QLineEdit;
class KIntNumInput;

namespace SubtitleComposer
{
	class TextInputDialog : public KDialog
	{
		Q_OBJECT

		public:

			TextInputDialog( const QString& caption, const QString& label, QWidget* parent=0 );
			TextInputDialog( const QString& caption, const QString& label, const QString& value, QWidget* parent=0 );

			const QString value() const;

		public slots:

			void setValue( const QString& value );

		private:

			void init( const QString& caption, const QString& label, const QString& value );

		private slots:

			void onLineEditTextChanged( const QString& text );

		private:

			QLineEdit* m_lineEdit;
	};

	class IntInputDialog : public KDialog
	{
		Q_OBJECT

		public:

			IntInputDialog( const QString& caption, const QString& label, QWidget* parent=0 );
			IntInputDialog( const QString& caption, const QString& label, int min, int max, QWidget* parent=0 );
			IntInputDialog( const QString& caption, const QString& label, int min, int max, int value, QWidget* parent=0 );

			int minimum() const;
			int maximum() const;
			int value() const;

		public slots:

			void setMinimum( int minimum );
			void setMaximum( int maximum );
			void setValue( int value );

		private:

			void init( const QString& caption, const QString& label, int min, int max, int value );

		private:

			KIntNumInput* m_intNumInput;
	};

}

#endif
