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

#ifndef RENDERTHREAD_H
#define RENDERTHREAD_H

#include <QThread>

struct SwsContext;
struct AVFrame;

namespace SubtitleComposer {
class VideoState;
struct Frame;

class RenderThread : public QThread
{
	Q_OBJECT

	friend class AudioDecoder;

public:
	explicit RenderThread(VideoState *state, QObject *parent = nullptr);

	void run() override;

private:
	void videoRefresh(double *remainingTime);
	void videoDisplay();
	double vpDuration(Frame *vp, Frame *nextvp);
	void updateVideoPts(double pts, int64_t pos, int serial);
	double computeTargetDelay(double delay);
	void updateSampleDisplay(short *samples, int samplesSize);
	void toggleAudioDisplay();
	int uploadTexture(AVFrame *frame);
	void videoImageDisplay();
	void videoAudioDisplay();

private:
	VideoState *m_vs;
};
}

#endif // RENDERTHREAD_H
