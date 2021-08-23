/*
 * SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SCRIPTING_RANGE_H
#define SCRIPTING_RANGE_H

#include "core/range.h"

#include <QObject>

namespace SubtitleComposer {
namespace Scripting {
class Range : public QObject
{
	Q_OBJECT

public slots:
	int start() const;
	int end() const;
	int length() const;

	bool contains(int index) const;
	bool contains(const QObject *range) const;

private:
	friend class RangesModule;
	friend class RangeList;
	friend class Subtitle;

	Range(const SubtitleComposer::Range &range, QObject *parent);

	SubtitleComposer::Range m_backend;
};
}
}
#endif
