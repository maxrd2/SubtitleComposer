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

#ifndef VIDEODECODER_H
#define VIDEODECODER_H

#include "videoplayer/backend/decoder.h"

namespace SubtitleComposer {
class VideoState;

class VideoDecoder : public Decoder
{
	Q_OBJECT

public:
	VideoDecoder(VideoState *state, QObject *parent = nullptr);

private:
	void run() override;

	int getVideoFrame(AVFrame *frame);
	int queuePicture(AVFrame *srcFrame, double pts, double duration, int64_t pos, int serial);

	VideoState *m_vs;

	double m_timeBase;

	int m_frameDropsEarly;
};
}

#endif // VIDEODECODER_H
