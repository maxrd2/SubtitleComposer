#ifndef SUBTITLESORTTEST_H
#define SUBTITLESORTTEST_H
/*
 * SPDX-FileCopyrightText: 2021 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "core/subtitle.h"

#include <QObject>

class SubtitleTest : public QObject
{
	Q_OBJECT

	SubtitleComposer::Subtitle sub;

private slots:
	void testSort_data();
	void testSort();
};

#endif // SUBTITLESORTTEST_H
