/*
    Copyright (C) 2001, S.R.Haque <srhaque@iee.org>.
    Copyright (C) 2002, David Faure <david@mandrakesoft.com>
    Copyright (C) 2004, Arend van Beelen jr. <arend@auton.nl>
    This file is part of the KDE project

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
 */

#ifndef KFIND_P_H
#define KFIND_P_H

#include "kfind.h"

#include <kdialog.h>

#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QPointer>
#include <QtCore/QString>

struct KFind::Private {
	Private(KFind *q) :
		q(q),
		findDialog(0),
		currentId(0),
		customIds(false),
		patternChanged(false),
		matchedPattern(""),
		emptyMatch(0) {}

	~Private()
	{
		delete dialog;
		data.clear();
		delete emptyMatch;
		emptyMatch = 0;
	}

	struct Match {
		Match() : dataId(-1),
			index(-1),
			matchedLength(-1) {}

		bool isNull() const
		{
			return index == -1;
		}

		Match(int _dataId, int _index, int _matchedLength) : dataId(_dataId),
			index(_index),
			matchedLength(_matchedLength)
		{
			Q_ASSERT(index != -1);
		}

		int dataId;
		int index;
		int matchedLength;
	};

	struct Data {
		Data() : id(-1),
			dirty(false) {}

		Data(int id, const QString &text, bool dirty = false) :
			id(id),
			text(text),
			dirty(dirty) {}

		int id;
		QString text;
		bool dirty;
	};

	void init(const QString &pattern);
	void startNewIncrementalSearch();

	static bool isInWord(QChar ch);
	static bool isWholeWords(const QString &text, int starts, int matchedLength);

	void _k_slotFindNext();
	void _k_slotDialogClosed();

	KFind *q;
	QPointer<QWidget> findDialog;
	int currentId;
	bool customIds : 1;
	bool patternChanged : 1;
	QString matchedPattern;
	QHash<QString, Match> incrementalPath;
	Match *emptyMatch;
	QList<Data> data;            // used like a vector, not like a linked-list

	QString pattern;
	QRegExp *regExp;
	KDialog *dialog;
	long options;
	unsigned matches;

	QString text;                           // the text set by setData
	int index;
	int matchedLength;
	bool dialogClosed : 1;
	bool lastResult : 1;
};

#endif                                                  // KFIND_P_H
