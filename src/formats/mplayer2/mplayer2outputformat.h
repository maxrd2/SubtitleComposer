#ifndef MPLAYER2OUTPUTFORMAT_H
#define MPLAYER2OUTPUTFORMAT_H

/*
 * SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2010-2019 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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

#include "formats/outputformat.h"
#include "core/richdocument.h"
#include "core/subtitleiterator.h"

namespace SubtitleComposer {
class MPlayer2OutputFormat : public OutputFormat
{
	friend class FormatManager;

protected:
	QString dumpSubtitles(const Subtitle &subtitle, bool primary) const override
	{
		QString ret;

		for(SubtitleIterator it(subtitle); it.current(); ++it) {
			const SubtitleLine *line = it.current();

			QString text = (primary ? line->primaryDoc() : line->secondaryDoc())->toPlainText();

			ret += m_lineBuilder.arg(static_cast<long>((line->showTime().toMillis() / 100.0) + 0.5))
					.arg(static_cast<long>((line->hideTime().toMillis() / 100.0) + 0.5))
					.arg(text.replace('\n', '|'));
		}
		return ret;
	}

	MPlayer2OutputFormat() :
		OutputFormat(QStringLiteral("MPlayer2"), QStringList(QStringLiteral("mpl"))),
		m_lineBuilder(QStringLiteral("[%1][%2]%3\n"))
	{}

	const QString m_lineBuilder;
};
}

#endif
