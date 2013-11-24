#ifndef ERRORFINDER_H
#define ERRORFINDER_H

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

#include <QtCore/QObject>

namespace SubtitleComposer {
class FindErrorsDialog;
class SubtitleIterator;

class ErrorFinder : public QObject
{
	Q_OBJECT

public:
	explicit ErrorFinder(QWidget *parent = 0);
	virtual ~ErrorFinder();

	QWidget * parentWidget();

public slots:
	void setSubtitle(Subtitle *subtitle = 0);
	void setTranslationMode(bool enabled);

	void find(const RangeList &selectionRanges, int currentIndex, bool findBackwards = false);
	bool findNext();
	bool findPrevious();

signals:
	void found(SubtitleLine *line);

private:
	void advance(bool advanceIteratorOnFirstStep);

private slots:
	void invalidate();
	void onIteratorSynchronized(int firstIndex, int lastIndex, bool inserted);

private:
	Subtitle *m_subtitle;
	FindErrorsDialog *m_dialog;
	bool m_translationMode;

	int m_targetErrorFlags;
	bool m_findBackwards;
	bool m_selection;
	SubtitleIterator *m_iterator;
	bool m_instancesFound;
	int m_allSearchedIndex;
};
}
#endif
