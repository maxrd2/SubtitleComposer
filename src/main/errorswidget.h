#ifndef ERRORSWIDGET_H
#define ERRORSWIDGET_H

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

#include "../core/rangelist.h"
#include "../core/subtitle.h"
#include "../widgets/treeview.h"

#include <QtCore/QList>
#include <QtCore/QAbstractItemModel>

class QTimer;

namespace SubtitleComposer
{
	class ErrorsModel;

	class ErrorsModelNode
	{
		public:

			~ErrorsModelNode();

			inline const ErrorsModel* model() const { return m_model; }

			inline const SubtitleLine* line() const { return m_line; }

			inline int errorCount() const { return m_errorCount; }
			inline bool isMarked() const { return m_marked; }

			inline bool isVisible() const { return m_errorCount; }

		private:

			ErrorsModelNode( ErrorsModel* model, const SubtitleLine* line );

			void update();

		private:

			ErrorsModel* m_model;
			const SubtitleLine* m_line;
			int m_errorCount;
			bool m_marked;

			friend class ErrorsModel;
	};


	class ErrorsModel : public QAbstractItemModel
	{
		Q_OBJECT

		friend class ErrorsModelNode;

		public:

			enum { Number=0, ErrorCount, UserMark, ColumnCount };

			explicit ErrorsModel( QObject* parent=0 );
			virtual ~ErrorsModel();

			int mapModelIndexToLineIndex( const QModelIndex& modelIndex ) const;

			int mapLineIndexToModelL1Row( int lineIndex ) const;
			int mapModelL1RowToLineIndex( int modelL1Row ) const;

			int mapModelL2RowToLineErrorID( int modelL2Row, int lineErrorFlags ) const;
			int mapModelL2RowToLineErrorFlag( int modelL2Row, int lineErrorFlags ) const;

			inline Subtitle* subtitle() const { return m_subtitle; };
			void setSubtitle( Subtitle* subtitle );

			inline const ErrorsModelNode* node( int lineIndex ) const { return m_nodes.at( lineIndex ); }

			inline int lineWithErrorsCount() const { return m_lineWithErrorsCount; };
			inline int errorCount() const { return m_errorCount; };
			inline int markCount() const { return m_markCount; };

			virtual int rowCount( const QModelIndex& parent=QModelIndex() ) const;
			virtual int columnCount( const QModelIndex& parent=QModelIndex() ) const;

			virtual QModelIndex index( int row, int column, const QModelIndex& index=QModelIndex() ) const;
			virtual QModelIndex parent( const QModelIndex& index ) const;

			virtual QVariant headerData( int section, Qt::Orientation orientation, int role=Qt::DisplayRole ) const;

			virtual QVariant data( const QModelIndex& index, int role ) const;

		signals:

			void dataChanged();
			void dataChanged( const QModelIndex& topLeft, const QModelIndex& bottomRight );

			void statsChanged();

		private slots:

			void onLinesInserted( int firstIndex, int lastIndex );
			void onLinesRemoved( int firstIndex, int lastIndex );

			void onLineErrorsChanged( SubtitleLine* line );

			void emitDataChanged();

		private:

			static const QIcon& markIcon();
			static const QIcon& errorIcon();

			void markLineChanged( int lineIndex );
			void updateLineErrors( SubtitleLine* line, int errorFlags );

			void incrementVisibleLinesCount( int delta );
			void incrementErrorsCount( int delta );
			void incrementMarksCount( int delta );

		private:

			Subtitle* m_subtitle;

			const SubtitleLine LEVEL1_LINE;

			QList<ErrorsModelNode*> m_nodes;

			QTimer* m_statsChangedTimer;
			int m_lineWithErrorsCount;
			int m_errorCount;
			int m_markCount;

			QTimer* m_dataChangedTimer;
			int m_minChangedLineIndex;
			int m_maxChangedLineIndex;
	};

	class ErrorsWidget : public TreeView
	{
		Q_OBJECT

		public:

			ErrorsWidget( QWidget* parent );
			virtual ~ErrorsWidget();

			void loadConfig();
			void saveConfig();

			SubtitleLine* currentLine();

			int lineSelectedErrorFlags( int lineIndex );

			RangeList selectionRanges() const;

			inline ErrorsModel* model() const { return static_cast<ErrorsModel*>( TreeView::model() ); }

		public slots:

			void setSubtitle( Subtitle* subtitle=0 );

			void setCurrentLine( SubtitleLine* line, bool clearSelection=true );

			void expandAll(); // reimplemented because currently calling QTreeView has many bad side effects

		signals:

			void currentLineChanged( SubtitleLine* line );
			void lineDoubleClicked( SubtitleLine* line );

		protected:

			virtual void contextMenuEvent( QContextMenuEvent* event );
			virtual void mouseDoubleClickEvent( QMouseEvent* event );
			virtual void keyPressEvent( QKeyEvent* event );
			virtual void showEvent( QShowEvent* event );
			virtual void hideEvent( QHideEvent* event );

		protected slots:

			void onCurrentRowChanged( const QModelIndex& currentIndex );

		private:

			Subtitle* m_subtitle;
	};
}

#endif
