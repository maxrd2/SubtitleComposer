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

#ifndef PACKETQUEUE_H
#define PACKETQUEUE_H

#include <QObject>

QT_FORWARD_DECLARE_CLASS(QMutex)
QT_FORWARD_DECLARE_CLASS(QWaitCondition)

extern "C" {
#include "libavformat/avformat.h"
}

namespace SubtitleComposer {
class PacketQueue
{
public:
	PacketQueue();

	/**
	 * @brief enqueue a packet
	 * @param pkt packet allocated with av_packet_alloc(), pkt will be set to nullptr
	 * @return 0 if successful; <0 otherwise
	 */
	int put(AVPacket **pkt);
	int putFlushPacket();
	int putNullPacket(int streamIndex);
	int init();
	void flush();
	void destroy();
	void abort();
	void start();
	/**
	 * @brief dequeue a packet
	 * @param pkt will be set to packet which must be freed with av_packet_free()
	 * @param block
	 * @param serial
	 * @return <0 if aborted, 0 if no packet and >0 if packet
	 */
	int get(AVPacket **pkt, int block, int *serial);

	inline int nbPackets() const { return m_nbPackets; }
	inline int size() const { return m_size; }
	inline int64_t duration() const { return m_duration; }
	inline bool abortRequested() const { return m_abortRequest; }
	inline int serial() const { return m_serial; }

private:
	int put_private(AVPacket **pkt);

private:
	struct PacketList {
		AVPacket *pkt;
		PacketList *next;
		int serial;
	};

	PacketList *m_firstPkt, *m_lastPkt;
	int m_nbPackets;
	int m_size;
	int64_t m_duration;
	bool m_abortRequest;
	int m_serial;
	QMutex *m_mutex;
	QWaitCondition *m_cond;

	friend class Decoder;
	friend class FrameQueue;
	friend class Clock;
};
}

#endif // PACKETQUEUE_H
