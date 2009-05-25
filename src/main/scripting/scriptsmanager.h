#ifndef SCRIPTSMANAGER_H
#define SCRIPTSMANAGER_H

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

#include <QtCore/QObject>
#include <QtCore/QMap>

#include <KUrl>

class QAction;
class QMenu;
class TreeView;
class KAction;
class KDialog;
class KPushButton;

namespace SubtitleComposer
{
	class Subtitle;

	class ScriptsManager : public QObject
	{
		Q_OBJECT

		public:

			explicit ScriptsManager( QObject* parent=0 );
			virtual ~ScriptsManager();

			QString currentScriptName() const;
			QStringList scriptNames() const;

			virtual bool eventFilter( QObject* object, QEvent* event );

		public slots:

			void setSubtitle( Subtitle* subtitle=0 );

			void showDialog();

			void createScript( const QString& scriptName=QString() );
			void addScript( const KUrl& srcScriptUrl=KUrl() );
			void removeScript( const QString& scriptName=QString() );
			void editScript( const QString& scriptName=QString() );
			void runScript( const QString& scriptName=QString() );
			void reloadScripts();

		private:

			static const QStringList& mimeTypes();
			QMenu* toolsMenu();

		private slots:

			void onToolsMenuActionTriggered( QAction* action );

		private:

			QMap<QString,QString> m_scripts; // name => path
			TreeView* m_scriptsWidget;
			KPushButton* m_runScriptButton;
			KDialog* m_dialog;
	};

	class Debug : public QObject
	{
		Q_OBJECT

		public:

			Debug();
			~Debug();

		public slots:

			void information( const QString& message );
			void warning( const QString& message );
			void error( const QString& message );
	};
}

#endif
