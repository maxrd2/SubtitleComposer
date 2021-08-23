#ifndef YOUTUBECAPTIONSOUTPUTFORMAT_H
#define YOUTUBECAPTIONSOUTPUTFORMAT_H

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
class YouTubeCaptionsOutputFormat : public OutputFormat
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
			ret += QString::asprintf("%d\n%02d:%02d:%02d,%03d,%02d:%02d:%02d,%03d\n",
										 it.index() + 1, showTime.hours(),
										 showTime.minutes(),
										 showTime.seconds(),
										 showTime.millis(),
										 hideTime.hours(),
										 hideTime.minutes(),
										 hideTime.seconds(),
										 hideTime.millis()
										 );

			const SString text = (primary ? line->primaryDoc() : line->secondaryDoc())->toRichText();

			// TODO does the format actually supports styled text?
			// if so, does it use standard HTML style tags?
			ret += text.richString();

			ret += QStringLiteral("\n\n");
		}
		return ret;
	}

	YouTubeCaptionsOutputFormat() :
		OutputFormat(QStringLiteral("YouTube Captions"), QStringList(QStringLiteral("sbv"))),
		m_dialogueBuilder(QStringLiteral("%1%2%3%4%5%6%7\n\n"))
	{}

	const QString m_dialogueBuilder;
};
}

#endif
