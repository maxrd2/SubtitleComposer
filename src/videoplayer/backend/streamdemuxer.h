/*
 * Copyright (c) 2003 Fabrice Bellard
 * Copyright (c) 2020 Mladen Milinkovic <max@smoothware.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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
	bool paused();
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
