/*
    SPDX-FileCopyrightText: 2003 Fabrice Bellard
    SPDX-FileCopyrightText: 2020 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SUBTITLEDECODER_H
#define SUBTITLEDECODER_H

#include "videoplayer/backend/decoder.h"

namespace SubtitleComposer {
class SubtitleDecoder : public Decoder
{
	Q_OBJECT

public:
	SubtitleDecoder(QObject *parent = nullptr);

private:
	void run() override;
};
}

#endif // SUBTITLEDECODER_H
