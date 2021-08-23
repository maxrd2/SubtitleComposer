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
