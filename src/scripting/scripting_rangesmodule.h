#ifndef SCRIPTING_RANGESMODULE_H
#define SCRIPTING_RANGESMODULE_H

/*
 * SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QObject>

namespace SubtitleComposer {
namespace Scripting {
class RangesModule : public QObject
{
	Q_OBJECT

public:
	RangesModule(QObject *parent = 0);

public slots:
	QObject * newRange(int firstIndex, int lastIndex);

	QObject * newLowerRange(int index);
	QObject * newUpperRange(int index);

	QObject * newUptoLastSelectedRange();
	QObject * newFromFirstSelectedRange();

	QObject * newEmptyRangeList();
	QObject * newSelectionRangeList();
};
}
}
#endif
