/*
    SPDX-FileCopyrightText: 2003 Fabrice Bellard
    SPDX-FileCopyrightText: 2020 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "videodecoder.h"

#include "videoplayer/backend/ffplayer.h"
#include "videoplayer/backend/videostate.h"

extern "C" {
#include "libavutil/rational.h"
}

using namespace SubtitleComposer;

VideoDecoder::VideoDecoder(VideoState *state, QObject *parent)
	: Decoder(parent),
	  m_vs(state),
	  m_timeBase(0.),
	  m_frameDropsEarly(0)
{
}

int
VideoDecoder::getVideoFrame(AVFrame *frame)
{
	const int gotPicture = decodeFrame(frame, nullptr);
	if(gotPicture <= 0)
		return gotPicture;

	frame->sample_aspect_ratio = av_guess_sample_aspect_ratio(m_vs->fmtContext, m_vs->vidStream, frame);

	if(frame->pts != AV_NOPTS_VALUE) {
		const double dPts = m_timeBase * frame->pts;
		if(m_vs->seekDecoder > 0. && !std::isnan(dPts) && m_vs->seekDecoder > dPts) {
			m_frameDropsEarly++;
			av_frame_unref(frame);
			return 0;
		}
		if(m_vs->framedrop > 0 || (m_vs->framedrop && m_vs->masterSyncType() != AV_SYNC_VIDEO_MASTER)) {
			const double diff = dPts - m_vs->masterTime();
			if(!std::isnan(diff) && diff < 0 && fabs(diff) < AV_NOSYNC_THRESHOLD
			&& pktSerial() == m_vs->vidClk.serial() && m_queue->nbPackets()) {
				m_frameDropsEarly++;
				av_frame_unref(frame);
				return 0;
			}
		}
	}

	return gotPicture;
}

int
VideoDecoder::queuePicture(AVFrame *srcFrame, double pts, double duration, int64_t pos, int serial)
{
#if defined(DEBUG_SYNC)
	printf("frame_type=%c pts=%0.3f\n", av_get_picture_type_char(src_frame->pict_type), pts);
#endif

	Frame *vp = m_frameQueue->peekWritable();
	if(!vp)
		return -1;

	vp->sar = srcFrame->sample_aspect_ratio;
	vp->uploaded = false;

	vp->width = srcFrame->width;
	vp->height = srcFrame->height;
	vp->format = srcFrame->format;

	vp->pts = pts;
	vp->duration = duration;
	vp->pos = pos;
	vp->serial = serial;

	av_frame_move_ref(vp->frame, srcFrame);
	m_frameQueue->push();
	return 0;
}

void
VideoDecoder::run()
{
	AVFrame *frame = av_frame_alloc();
	m_timeBase = av_q2d(m_vs->vidStream->time_base);

	if(!frame)
		return;

	const AVRational fps = av_guess_frame_rate(m_vs->fmtContext, m_vs->vidStream, nullptr);
	const double frameDuration = fps.num ? double(fps.den) / fps.num : 0.0;

	for(;;) {
		int ret = getVideoFrame(frame);
		if(ret < 0)
			break;
		if(!ret)
			continue;

		double pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * m_timeBase;
		ret = queuePicture(frame, pts, frameDuration, frame->pkt_pos, pktSerial());
		av_frame_unref(frame);

		if(ret < 0)
			break;
	}

	av_frame_free(&frame);
}
