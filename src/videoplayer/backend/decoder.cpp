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

#include "decoder.h"

#include "videoplayer/backend/ffplayer.h"
#include "videoplayer/backend/packetqueue.h"
#include "videoplayer/backend/framequeue.h"

#include <QWaitCondition>


using namespace SubtitleComposer;

Decoder::Decoder(QObject *parent)
	: QThread(parent),
	  m_reorderPts(-1),
	  m_pkt(nullptr),
	  m_queue(nullptr),
	  m_frameQueue(nullptr),
	  m_avCtx(nullptr),
	  m_pktSerial(0),
	  m_finished(0),
	  m_emptyQueueCond(nullptr),
	  m_startPts(0)
{
	memset(&m_pkt, 0, sizeof(m_pkt));
}

void
Decoder::init(AVCodecContext *avctx, PacketQueue *pq, FrameQueue *fq, QWaitCondition *emptyQueueCond)
{
	m_queue = pq;
	m_frameQueue = fq;
	m_avCtx = avctx;
	m_pktSerial = -1;
	m_finished = 0;
	av_packet_free(&m_pkt);
	m_emptyQueueCond = emptyQueueCond;
	m_startPts = AV_NOPTS_VALUE;
}

void
Decoder::start()
{
	m_queue->start();
	QThread::start();
}

int
Decoder::decodeFrame(AVFrame *frame, AVSubtitle *sub)
{
	int ret = AVERROR(EAGAIN);

	for(;;) {
		if(m_queue->m_serial == m_pktSerial) {
			do {
				if(m_queue->m_abortRequest)
					return -1;

				switch(m_avCtx->codec_type) {
				case AVMEDIA_TYPE_VIDEO:
					ret = avcodec_receive_frame(m_avCtx, frame);
					if(ret >= 0) {
						if(m_reorderPts == -1)
							frame->pts = frame->best_effort_timestamp;
						else if(!m_reorderPts)
							frame->pts = frame->pkt_dts;
					}
					break;
				case AVMEDIA_TYPE_AUDIO:
					ret = avcodec_receive_frame(m_avCtx, frame);
					if(ret >= 0) {
						AVRational tb = AVRational{ 1, frame->sample_rate };
						if(frame->pts != AV_NOPTS_VALUE)
							frame->pts = av_rescale_q(frame->pts, m_avCtx->pkt_timebase, tb);
						else if(m_nextPts != AV_NOPTS_VALUE)
							frame->pts = av_rescale_q(m_nextPts, m_nextPtsTb, tb);
						if(frame->pts != AV_NOPTS_VALUE) {
							m_nextPts = frame->pts + frame->nb_samples;
							m_nextPtsTb = tb;
						}
					}
					break;
				default:
					break;
				}
				if(ret == AVERROR_EOF) {
					m_finished = m_pktSerial;
					avcodec_flush_buffers(m_avCtx);
					return 0;
				}
				if(ret >= 0)
					return 1;
			} while(ret != AVERROR(EAGAIN));
		}

		AVPacket *pkt = nullptr;
		for(;;) {
			if(m_queue->m_nbPackets == 0)
				m_emptyQueueCond->wakeOne();
			if(m_pkt) {
				pkt = m_pkt;
				m_pkt = nullptr;
			} else if(m_queue->get(&pkt, 1, &m_pktSerial) < 0) {
				return -1;
			}
			if(m_queue->m_serial == m_pktSerial)
				break;
			av_packet_free(&pkt);
		}

		if(pkt->data == FFPlayer::flushPkt()) {
			avcodec_flush_buffers(m_avCtx);
			m_finished = 0;
			m_nextPts = m_startPts;
			m_nextPtsTb = m_startPtsTb;
		} else if(m_avCtx->codec_type == AVMEDIA_TYPE_SUBTITLE) {
			int gotFrame = 0;
			ret = avcodec_decode_subtitle2(m_avCtx, sub, &gotFrame, pkt);
			if(ret < 0) {
				ret = AVERROR(EAGAIN);
			} else if(gotFrame) {
				ret = 0;
				if(!pkt->data) {
					m_pkt = pkt;
					pkt = nullptr;
				}
			} else {
				ret = pkt->data ? AVERROR(EAGAIN) : AVERROR_EOF;
			}
		} else if(avcodec_send_packet(m_avCtx, pkt) == AVERROR(EAGAIN)) {
			av_log(m_avCtx, AV_LOG_ERROR, "Receive_frame and send_packet both returned EAGAIN, which is an API violation.\n");
			m_pkt = pkt;
			pkt = nullptr;
		}
		av_packet_free(&pkt);
	}
}

void
Decoder::destroy()
{
	av_packet_free(&m_pkt);
	avcodec_free_context(&m_avCtx);
}

void
Decoder::abort()
{
	m_queue->abort();
	if(m_frameQueue)
		m_frameQueue->signal();
	wait();
	m_queue->flush();
}
