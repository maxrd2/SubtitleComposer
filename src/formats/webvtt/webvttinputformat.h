/*
    SPDX-FileCopyrightText: 2021-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WEBVTTINPUTFORMAT_H
#define WEBVTTINPUTFORMAT_H

#include "formats/inputformat.h"

namespace SubtitleComposer {
class WebVTTInputFormat : public InputFormat
{
	friend class FormatManager;

protected:
	bool parseSubtitles(Subtitle &subtitle, const QString &data) const override;

	WebVTTInputFormat();
};
}

#endif
