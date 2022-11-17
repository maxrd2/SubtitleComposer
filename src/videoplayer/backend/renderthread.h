/*
    SPDX-FileCopyrightText: 2003 Fabrice Bellard
    SPDX-FileCopyrightText: 2020-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RENDERTHREAD_H
#define RENDERTHREAD_H

#include <QThread>

struct SwsContext;
struct AVFrame;
struct AVPixFmtDescriptor;

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
	void videoImageDisplay();
	void videoAudioDisplay();

private:
	VideoState *m_vs;
	bool m_isYUV;
	bool m_isPlanar;
};
}

#endif // RENDERTHREAD_H
