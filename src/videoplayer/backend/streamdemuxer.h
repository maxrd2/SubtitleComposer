/*
    SPDX-FileCopyrightText: 2003 Fabrice Bellard
    SPDX-FileCopyrightText: 2020-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef STREAMDEMUXER_H
#define STREAMDEMUXER_H

#include <QThread>

namespace SubtitleComposer {
class VideoState;
class FFPlayer;

class StreamDemuxer : public QThread
{
	Q_OBJECT

public:
	static VideoState * open(const char *filename);
	static void close(VideoState *vs);
	void pauseToggle();
	void seek(qint64 time);
	void stepFrame();
	bool abortRequested();
	int relativeStreamIndex(int codecType, int absoluteIndex);
	int absoluteStreamIndex(int codecType, int relativeIndex);
	void selectStream(int codecType, int streamIndex);

private:
	StreamDemuxer(VideoState *vs, QObject *parent = nullptr);

	VideoState *m_vs;

	void run() override;

	int componentOpen(int streamIndex);
	void componentClose(int streamIndex);
	void cycleStream(int codecType);
};
}

#endif // STREAMDEMUXER_H
