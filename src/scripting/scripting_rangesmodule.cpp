/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "scripting_rangesmodule.h"
#include "scripting_rangelist.h"
#include "scripting_range.h"
#include "application.h"
#include "gui/treeview/lineswidget.h"

using namespace SubtitleComposer;

Scripting::RangesModule::RangesModule(QObject *parent) :
	QObject(parent)
{}

QObject *
Scripting::RangesModule::newRange(int firstIndex, int lastIndex)
{
	if(firstIndex > lastIndex || firstIndex < 0 || lastIndex < 0)
		return 0;
	return new Scripting::Range(SubtitleComposer::Range(firstIndex, lastIndex), this);
}

QObject *
Scripting::RangesModule::newLowerRange(int index)
{
	return new Scripting::Range(SubtitleComposer::Range::lower(index), this);
}

QObject *
Scripting::RangesModule::newUpperRange(int index)
{
	return new Scripting::Range(SubtitleComposer::Range::upper(index), this);
}

QObject *
Scripting::RangesModule::newUptoLastSelectedRange()
{
	int index = app()->linesWidget()->lastSelectedIndex();
	return index < 0 ? 0 : new Scripting::Range(SubtitleComposer::Range::lower(index), this);
}

QObject *
Scripting::RangesModule::newFromFirstSelectedRange()
{
	int index = app()->linesWidget()->firstSelectedIndex();
	return index < 0 ? 0 : new Scripting::Range(SubtitleComposer::Range::upper(index), this);
}

QObject *
Scripting::RangesModule::newEmptyRangeList()
{
	return new Scripting::RangeList(SubtitleComposer::RangeList(), this);
}

QObject *
Scripting::RangesModule::newSelectionRangeList()
{
	return new Scripting::RangeList(app()->linesWidget()->selectionRanges(), this);
}


