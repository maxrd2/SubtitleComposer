#ifndef SUBRIPOUTPUTFORMAT_H
#define SUBRIPOUTPUTFORMAT_H

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
class SubRipOutputFormat : public OutputFormat
{
	friend class FormatManager;

protected:
	QString dumpSubtitles(const Subtitle &subtitle, bool primary) const override
	{
		QString ret;

		for(SubtitleIterator it(subtitle); it.current(); ++it) {
			const SubtitleLine *line = it.current();

			Time showTime = line->showTime();
			Time hideTime = line->hideTime();
			ret += QString::asprintf("%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\n", it.index() + 1, showTime.hours(), showTime.minutes(), showTime.seconds(), showTime.millis(), hideTime.hours(), hideTime.minutes(), hideTime.seconds(), hideTime.millis());

			const SString text = (primary ? line->primaryDoc() : line->secondaryDoc())->toRichText();

			ret += text.richString().replace(QLatin1String("&amp;"), QLatin1String("&")).replace(QLatin1String("&lt;"), QLatin1String("<")).replace(QLatin1String("&gt;"), QLatin1String(">"));

			ret += QStringLiteral("\n\n");
		}
		return ret;
	}

	SubRipOutputFormat() :
		OutputFormat(QStringLiteral("SubRip"), QStringList(QStringLiteral("srt"))),
		m_dialogueBuilder(QStringLiteral("%1%2%3%4%5%6%7\n\n"))
	{}

	const QString m_dialogueBuilder;
};
}

#endif
