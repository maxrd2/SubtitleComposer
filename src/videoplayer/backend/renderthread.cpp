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

#include "renderthread.h"

#include <QMutex>

#include "videoplayer/backend/videostate.h"
#include "videoplayer/backend/glrenderer.h"

extern "C" {
#include "libavutil/time.h"
#include "libavformat/avformat.h"
#include "libavutil/pixdesc.h"
}


using namespace SubtitleComposer;

RenderThread::RenderThread(VideoState *state, QObject *parent)
	: QThread(parent),
	  m_vs(state),
	  m_lastFormat(-1)
{
}

void
RenderThread::run()
{
	double remaining_time = 0.0;
	for(;;) {
		if(remaining_time > 0.0)
			av_usleep((int64_t)(remaining_time * double(AV_TIME_BASE)));
		remaining_time = REFRESH_RATE;
		if(isInterruptionRequested())
			break;
		if(m_vs->showMode != SHOW_MODE_NONE && (!m_vs->paused || m_vs->forceRefresh))
			videoRefresh(&remaining_time);
	}
}

// called to display each frame
void
RenderThread::videoRefresh(double *remainingTime)
{
	double time = 0.0;

	if(!m_vs->paused && m_vs->masterSyncType() == AV_SYNC_EXTERNAL_CLOCK && m_vs->realTime)
		m_vs->checkExternalClockSpeed();

#ifdef AUDIO_VISUALIZATION
	if(m_vs->showMode != SHOW_MODE_VIDEO && m_vs->audStream) {
		time = av_gettime_relative() / double(AV_TIME_BASE);
		if(m_vs->forceRefresh || m_vs->last_vis_time + m_vs->rdftspeed < time) {
			videoDisplay();
			m_vs->last_vis_time = time;
		}
		*remainingTime = FFMIN(*remainingTime, m_vs->last_vis_time + m_vs->rdftspeed - time);
	}
#endif

	if(m_vs->vidStream) {
retry:
		if(m_vs->vidFQ.nbRemaining() == 0) {
			// nothing to do, no picture to display in the queue
		} else {
			double last_duration, duration, delay;
			Frame *vp, *lastvp;

			/* dequeue the picture */
			lastvp = m_vs->vidFQ.peekLast();
			vp = m_vs->vidFQ.peek();

			if(vp->serial != m_vs->vidPQ.serial()) {
				m_vs->vidFQ.next();
				goto retry;
			}

			if(lastvp->serial != vp->serial)
				m_vs->frameTimer = av_gettime_relative() / double(AV_TIME_BASE);

			if(m_vs->paused)
				goto display;

			// compute nominal last_duration
			last_duration = vpDuration(lastvp, vp);
			delay = computeTargetDelay(last_duration);

			time = av_gettime_relative() / double(AV_TIME_BASE);
			if(time < m_vs->frameTimer + delay) {
				*remainingTime = FFMIN(m_vs->frameTimer + delay - time, *remainingTime);
				goto display;
			}

			m_vs->frameTimer += delay;
			if(delay > 0 && time - m_vs->frameTimer > AV_SYNC_THRESHOLD_MAX)
				m_vs->frameTimer = time;

			m_vs->vidFQ.m_mutex->lock();
			if(!isnan(vp->pts))
				updateVideoPts(vp->pts, vp->pos, vp->serial);
			m_vs->vidFQ.m_mutex->unlock();

			if(m_vs->vidFQ.nbRemaining() > 1) {
				Frame *nextvp = m_vs->vidFQ.peekNext();
				duration = vpDuration(vp, nextvp);
				if(!m_vs->step
				&& (m_vs->framedrop > 0 || (m_vs->framedrop && m_vs->masterSyncType() != AV_SYNC_VIDEO_MASTER))
				&& time > m_vs->frameTimer + duration) {
					m_vs->frameDropsLate++;
					m_vs->vidFQ.next();
					goto retry;
				}
			}

#ifdef VIDEO_SUBTITLE
			if(m_vs->subStream) {
				while(m_vs->subFQ.nbRemaining() > 0) {
					Frame *sp = m_vs->subFQ.peek();
					Frame *sp2 = m_vs->subFQ.nbRemaining() > 1 ? m_vs->subFQ.peekNext() : nullptr;

					if(sp->serial != m_vs->subPQ.serial()
					   || (m_vs->vidClk.pts() > (sp->pts + ((float)sp->sub.end_display_time / 1000)))
					   || (sp2 && m_vs->vidClk.pts() > (sp2->pts + ((float)sp2->sub.start_display_time / 1000)))) {
						if(sp->uploaded) {
							int i;
							for(i = 0; i < (int)sp->sub.num_rects; i++) {
								AVSubtitleRect *sub_rect = sp->sub.rects[i];
								uint8_t *pixels;
								int pitch, j;

								if(!SDL_LockTexture(is->sub_texture, (SDL_Rect *)sub_rect, (void **)&pixels, &pitch)) {
									for(j = 0; j < sub_rect->h; j++, pixels += pitch)
										memset(pixels, 0, sub_rect->w << 2);
									SDL_UnlockTexture(is->sub_texture);
								}
							}
						}
						m_vs->subFQ.next();
					} else {
						break;
					}
				}
			}
#endif

			m_vs->vidFQ.next();
			m_vs->forceRefresh = true;

			if(m_vs->step && !m_vs->paused)
				m_vs->demuxer->pauseToggle();
		}
display:
		// display picture
		if(m_vs->forceRefresh && m_vs->showMode == SHOW_MODE_VIDEO && m_vs->vidFQ.m_rIndexShown)
			videoDisplay();
	}
	m_vs->forceRefresh = false;
	m_vs->notifyPosition();
}

// display the current picture, if any
void
RenderThread::videoDisplay()
{
	m_vs->videoWidth = m_vs->glRenderer->width();
	m_vs->videoHeight = m_vs->glRenderer->height();

	if(m_vs->audStream && m_vs->showMode != SHOW_MODE_VIDEO)
		videoAudioDisplay();
	else if(m_vs->vidStream)
		videoImageDisplay();
}

double
RenderThread::vpDuration(Frame *vp, Frame *nextvp)
{
	if(vp->serial == nextvp->serial) {
		double duration = nextvp->pts - vp->pts;
		if(isnan(duration) || duration <= 0 || duration > m_vs->maxFrameDuration)
			return vp->duration;
		else
			return duration;
	} else {
		return 0.0;
	}
}

void
RenderThread::updateVideoPts(double pts, int64_t /*pos*/, int serial)
{
	// update current video pts
	m_vs->vidClk.set(pts, serial);
	m_vs->extClk.syncTo(&m_vs->vidClk);
}

double
RenderThread::computeTargetDelay(double delay)
{
	double sync_threshold, diff = 0;

	// update delay to follow master synchronisation source
	if(m_vs->masterSyncType() != AV_SYNC_VIDEO_MASTER) {
		// if video is slave, we try to correct big delays by duplicating or deleting a frame
		diff = m_vs->vidClk.get() - m_vs->masterTime();

		// skip or repeat frame. We take into account the delay to compute the threshold.
		// I still don't know if it is the best guess
		sync_threshold = FFMAX(AV_SYNC_THRESHOLD_MIN, FFMIN(AV_SYNC_THRESHOLD_MAX, delay));
		if(!isnan(diff) && fabs(diff) < m_vs->maxFrameDuration) {
			if(diff <= -sync_threshold)
				delay = FFMAX(0, delay + diff);
			else if(diff >= sync_threshold && delay > AV_SYNC_FRAMEDUP_THRESHOLD)
				delay = delay + diff;
			else if(diff >= sync_threshold)
				delay = 2 * delay;
		}
	}

	av_log(nullptr, AV_LOG_TRACE, "video: delay=%0.3f A-V=%f\n", delay, -diff);

	return delay;
}

void
RenderThread::videoAudioDisplay()
{
#ifdef AUDIO_VISUALIZATION
	int i, i_start, x, y1, y, ys, delay, n, nb_display_channels;
	int ch, channels, h, h2;
	int64_t time_diff;
	int rdft_bits, nb_freq;

	for(rdft_bits = 1; (1 << rdft_bits) < 2 * m_vs->height; rdft_bits++);
	nb_freq = 1 << (rdft_bits - 1);

	// compute display index : center on currently output samples
	channels = m_vs->audio_tgt.channels;
	nb_display_channels = channels;
	if(!m_vs->paused) {
		int data_used = m_vs->show_mode == SHOW_MODE_WAVES ? m_vs->width : (2 * nb_freq);
		n = 2 * channels;
		delay = m_vs->audio_write_buf_size;
		delay /= n;

		// to be more precise, we take into account the time spent since the last buffer computation
		if(ffplay::audio_callback_time) {
			time_diff = av_gettime_relative() - ffplay::audio_callback_time;
			delay -= (time_diff * m_vs->audio_tgt.freq) / 1000000;
		}

		delay += 2 * data_used;
		if(delay < data_used)
			delay = data_used;

		i_start = x = compute_mod(m_vs->sample_array_index - delay * channels, SAMPLE_ARRAY_SIZE);
		if(m_vs->show_mode == SHOW_MODE_WAVES) {
			h = INT_MIN;
			for(i = 0; i < 1000; i += channels) {
				int idx = (SAMPLE_ARRAY_SIZE + x - i) % SAMPLE_ARRAY_SIZE;
				int a = m_vs->sample_array[idx];
				int b = m_vs->sample_array[(idx + 4 * channels) % SAMPLE_ARRAY_SIZE];
				int c = m_vs->sample_array[(idx + 5 * channels) % SAMPLE_ARRAY_SIZE];
				int d = m_vs->sample_array[(idx + 9 * channels) % SAMPLE_ARRAY_SIZE];
				int score = a - d;
				if(h < score && (b ^ c) < 0) {
					h = score;
					i_start = idx;
				}
			}
		}

		m_vs->last_i_start = i_start;
	} else {
		i_start = m_vs->last_i_start;
	}

	if(m_vs->show_mode == SHOW_MODE_WAVES) {
//		SDL_SetRenderDrawColor(ffplay::renderer, 255, 255, 255, 255);

		// total height for one channel
		h = m_vs->height / nb_display_channels;
		// graph height / 2
		h2 = (h * 9) / 20;
		for(ch = 0; ch < nb_display_channels; ch++) {
			i = i_start + ch;
			y1 = m_vs->ytop + ch * h + (h / 2); // position of center line
			for(x = 0; x < m_vs->width; x++) {
				y = (m_vs->sample_array[i] * h2) >> 15;
				if(y < 0) {
					y = -y;
					ys = y1 - y;
				} else {
					ys = y1;
				}
				fill_rectangle(m_vs->xleft + x, ys, 1, y);
				i += channels;
				if(i >= SAMPLE_ARRAY_SIZE)
					i -= SAMPLE_ARRAY_SIZE;
			}
		}

//		SDL_SetRenderDrawColor(ffplay::renderer, 0, 0, 255, 255);

		for(ch = 1; ch < nb_display_channels; ch++) {
			y = m_vs->ytop + ch * h;
			fill_rectangle(m_vs->xleft, y, m_vs->width, 1);
		}
	} else {
		nb_display_channels = FFMIN(nb_display_channels, 2);
		if(rdft_bits != m_vs->rdft_bits) {
			av_rdft_end(m_vs->rdft);
			av_free(m_vs->rdft_data);
			m_vs->rdft = av_rdft_init(rdft_bits, DFT_R2C);
			m_vs->rdft_bits = rdft_bits;
			m_vs->rdft_data = (FFTSample *)av_malloc_array(nb_freq, 4 * sizeof(*m_vs->rdft_data));
		}
		if(!m_vs->rdft || !m_vs->rdft_data) {
			av_log(nullptr, AV_LOG_ERROR, "Failed to allocate buffers for RDFT, switching to waves display\n");
			m_vs->show_mode = SHOW_MODE_WAVES;
		} else {
			FFTSample *data[2];
			for(ch = 0; ch < nb_display_channels; ch++) {
				data[ch] = m_vs->rdft_data + 2 * nb_freq * ch;
				i = i_start + ch;
				for(x = 0; x < 2 * nb_freq; x++) {
					double w = (x - nb_freq) * (1.0 / nb_freq);
					data[ch][x] = m_vs->sample_array[i] * (1.0 - w * w);
					i += channels;
					if(i >= SAMPLE_ARRAY_SIZE)
						i -= SAMPLE_ARRAY_SIZE;
				}
				av_rdft_calc(m_vs->rdft, data[ch]);
			}
			// Least efficient way to do this, we should of course
			// directly access it but it is more than fast enough.
			SDL_Rect rect = { .x = m_vs->xpos, .y = 0, .w = 1, .h = m_vs->height };
			uint32_t *pixels;
			int pitch;
			if(!SDL_LockTexture(m_vs->vis_texture, &rect, (void **)&pixels, &pitch)) {
				pitch >>= 2;
				pixels += pitch * m_vs->height;
				for(y = 0; y < m_vs->height; y++) {
					double w = 1 / sqrt(nb_freq);
					int a = sqrt(
						w * sqrt(data[0][2 * y + 0] * data[0][2 * y + 0] + data[0][2 * y + 1] * data[0][2 * y + 1]));
					int b = (nb_display_channels == 2) ? sqrt(w * hypot(data[1][2 * y + 0], data[1][2 * y + 1]))
													   : a;
					a = FFMIN(a, 255);
					b = FFMIN(b, 255);
					pixels -= pitch;
					*pixels = (a << 16) + (b << 8) + ((a + b) >> 1);
				}
				SDL_UnlockTexture(m_vs->vis_texture);
			}
			SDL_RenderCopy(ffplay::renderer, m_vs->vis_texture, nullptr, nullptr);
		}
		if(!m_vs->paused)
			m_vs->xpos++;
		if(m_vs->xpos >= m_vs->width)
			m_vs->xpos = m_vs->xleft;
	}
#endif
}

void
RenderThread::videoImageDisplay()
{
	Frame *vp = m_vs->vidFQ.peekLast();
#ifdef VIDEO_SUBTITLE
	Frame *sp = nullptr;
	if(m_vs->subStream) {
		if(m_vs->subFQ.nbRemaining() > 0) {
			sp = m_vs->subFQ.peek();

			if(vp->pts >= sp->pts + ((float)sp->sub.start_display_time / 1000)) {
				if(!sp->uploaded) {
					uint8_t *pixels[4];
					int pitch[4];
					int i;
					if(!sp->width || !sp->height) {
						sp->width = vp->width;
						sp->height = vp->height;
					}

					for(i = 0; i < (int)sp->sub.num_rects; i++) {
						AVSubtitleRect *sub_rect = sp->sub.rects[i];

						sub_rect->x = av_clip(sub_rect->x, 0, sp->width);
						sub_rect->y = av_clip(sub_rect->y, 0, sp->height);
						sub_rect->w = av_clip(sub_rect->w, 0, sp->width - sub_rect->x);
						sub_rect->h = av_clip(sub_rect->h, 0, sp->height - sub_rect->y);

						m_vs->subConvertCtx = sws_getCachedContext(m_vs->subConvertCtx,
								sub_rect->w, sub_rect->h, AV_PIX_FMT_PAL8,
								sub_rect->w, sub_rect->h, AV_PIX_FMT_BGRA,
								0, nullptr, nullptr, nullptr);
						if(!m_vs->subConvertCtx) {
							av_log(nullptr, AV_LOG_FATAL, "Cannot initialize the conversion context\n");
							return;
						}
						if(!SDL_LockTexture(is->sub_texture, (SDL_Rect *)sub_rect, (void **)pixels, pitch)) {
							sws_scale(is->sub_convert_ctx, (const uint8_t *const *)sub_rect->data, sub_rect->linesize, 0, sub_rect->h, pixels, pitch);
							SDL_UnlockTexture(is->sub_texture);
						}
					}
					sp->uploaded = 1;
				}
			} else
				sp = nullptr;
		}
	}
#endif

	if(!vp->uploaded) {
		if(uploadTexture(vp->frame) < 0)
			return;
		vp->uploaded = 1;
		vp->flip_v = vp->frame->linesize[0] < 0;
	}

#ifdef VIDEO_SUBTITLE
	if(sp) {
#if USE_ONEPASS_SUBTITLE_RENDER
//		SDL_RenderCopy(ffplay::renderer, is->sub_texture, nullptr, &rect);
#else
		int i;
		double xratio = (double)rect.w / (double)sp->width;
		double yratio = (double)rect.h / (double)sp->height;
		for(i = 0; i < sp->sub.num_rects; i++) {
			SDL_Rect *sub_rect = (SDL_Rect*)sp->sub.rects[i];
			SDL_Rect target = {.x = rect.x + sub_rect->x * xratio,
							   .y = rect.y + sub_rect->y * yratio,
							   .w = sub_rect->w * xratio,
							   .h = sub_rect->h * yratio};
			SDL_RenderCopy(renderer, is->sub_texture, sub_rect, &target);
		}
#endif
	}
#endif
}

// copy samples for viewing in editor window
void
RenderThread::updateSampleDisplay(short *samples, int samplesSize)
{
#ifdef AUDIO_VISUALIZATION
	int size = samplesSize / sizeof(int16_t);
	while(size > 0) {
		int len = m_vs->sample_array.capacity() - m_vs->sample_array_index;
		if(len > size)
			len = size;
		memcpy(&m_vs->sample_array[m_vs->sample_array_index], samples, len * sizeof(int16_t));
		samples += len;
		m_vs->sample_array_index += len;
		if(m_vs->sample_array_index >= m_vs->sample_array.capacity())
			m_vs->sample_array_index = 0;
		size -= len;
	}
#else
	Q_UNUSED(samples)
	Q_UNUSED(samplesSize)
#endif
}

void
RenderThread::toggleAudioDisplay()
{
	int next = m_vs->showMode;
	do {
		next = (next + 1) % SHOW_MODE_NB;
	} while((next != m_vs->showMode && next == SHOW_MODE_VIDEO && !m_vs->vidStream) || (next != SHOW_MODE_VIDEO && !m_vs->audStream));
	if(m_vs->showMode != next) {
		m_vs->forceRefresh = true;
		m_vs->showMode = ShowMode(next);
	}
}

bool
RenderThread::validTextureFormat(const AVPixFmtDescriptor *fd)
{
	const uint64_t &f = fd->flags;
	if((f & AV_PIX_FMT_FLAG_BITSTREAM)) {
		qCritical("uploadTexture() failed: unsupported frame format [%s] - bitstream", fd->name);
		return false;
	}
	if((f & AV_PIX_FMT_FLAG_PAL)) {
		qCritical("uploadTexture() failed: unsupported frame format [%s] - palette", fd->name);
		return false;
	}
	if((f & AV_PIX_FMT_FLAG_BE)) {
		qCritical("uploadTexture() failed: unsupported frame format [%s] - bigendian", fd->name);
		return false;
	}

	m_isYUV = !(f & AV_PIX_FMT_FLAG_RGB);
	m_isPlanar = f & AV_PIX_FMT_FLAG_PLANAR;
	if(m_isPlanar && m_isYUV) {
		const quint8 b = fd->comp[0].depth > 8 ? 2 : 1;
		if(fd->comp[0].step != b || fd->comp[1].step != b || fd->comp[2].step != b) {
			qCritical("validTextureFormat() failed: unsupported plane step [%d, %d, %d] %s",
				   fd->comp[0].step, fd->comp[1].step, fd->comp[2].step, fd->name);
			return false;
		}
		if(fd->comp[0].offset || fd->comp[1].offset || fd->comp[2].offset) {
			qCritical("validTextureFormat() failed: unsupported plane offset [%d, %d, %d] %s",
				   fd->comp[0].offset, fd->comp[1].offset, fd->comp[2].offset, fd->name);
			return false;
		}
		if(fd->comp[0].shift || fd->comp[1].shift || fd->comp[2].shift) {
			qCritical("validTextureFormat() failed: unsupported plane shift [%d, %d, %d] %s",
				   fd->comp[0].shift, fd->comp[1].shift, fd->comp[2].shift, fd->name);
			return false;
		}
		if(fd->comp[0].depth != fd->comp[1].depth || fd->comp[0].depth != fd->comp[2].depth) {
			qCritical("validTextureFormat() failed: unsupported plane depths [%d, %d, %d] %s",
				   fd->comp[0].depth, fd->comp[1].depth, fd->comp[2].depth, fd->name);
			return false;
		}
		if(fd->nb_components < 3) {
			qCritical("validTextureFormat() failed: unsupported plane count [%d] %s",
					  fd->nb_components, fd->name);
			return false;
		}
	} else {
		qCritical("validTextureFormat() failed: unsupported frame format [%s]", fd->name);
		return false;
	}
	return true;
}

int
RenderThread::uploadTexture(AVFrame *frame)
{
	const AVPixFmtDescriptor *fd = av_pix_fmt_desc_get(AVPixelFormat(frame->format));
	if(m_lastFormat != frame->format) {
		if(!validTextureFormat(fd)) {
			requestInterruption();
			return -1;
		}
		m_lastFormat = frame->format;
	}

	if(m_isPlanar && m_isYUV) {
		if(!frame->linesize[0] || !frame->linesize[1] || !frame->linesize[2]) {
			qCritical("uploadTexture() failed: invalid linesize [%d, %d, %d]",
				   frame->linesize[0], frame->linesize[1], frame->linesize[2]);
			return -1;
		}

		QMutexLocker l(m_vs->glRenderer->mutex());

		m_vs->glRenderer->setFrameFormat(frame->width, frame->height,
			fd->comp[0].depth, fd->log2_chroma_w, fd->log2_chroma_h);

		m_vs->glRenderer->setColorspace(frame);

		if(frame->linesize[0] > 0)
			m_vs->glRenderer->setFrameY(frame->data[0], frame->linesize[0]);
		else
			m_vs->glRenderer->setFrameY(frame->data[0] + frame->linesize[0] * (frame->height - 1), -frame->linesize[0]);

		if(frame->linesize[1] > 0)
			m_vs->glRenderer->setFrameU(frame->data[1], frame->linesize[1]);
		else
			m_vs->glRenderer->setFrameU(frame->data[1] + frame->linesize[1] * (AV_CEIL_RSHIFT(frame->height, 1) - 1), -frame->linesize[1]);

		if(frame->linesize[2] > 0)
			m_vs->glRenderer->setFrameV(frame->data[2], frame->linesize[2]);
		else
			m_vs->glRenderer->setFrameV(frame->data[2] + frame->linesize[2] * (AV_CEIL_RSHIFT(frame->height, 1) - 1), -frame->linesize[2]);

		m_vs->glRenderer->update();
	}

	return 0;
}
