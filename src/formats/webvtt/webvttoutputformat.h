/*
    SPDX-FileCopyrightText: 2021-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WEBVTTOUTPUTFORMAT_H
#define WEBVTTOUTPUTFORMAT_H

#include "formats/outputformat.h"

namespace SubtitleComposer {
class WebVTTOutputFormat : public OutputFormat
{
	friend class FormatManager;

protected:
	QString dumpSubtitles(const Subtitle &subtitle, bool primary) const override;

	WebVTTOutputFormat();
};
}

#endif
