#ifndef TMPLAYERINPUTFORMAT_H
#define TMPLAYERINPUTFORMAT_H

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

#include "core/richdocument.h"
#include "formats/inputformat.h"

#include <QRegExp>

namespace SubtitleComposer {
// FIXME TMPlayer Multiline variant

class TMPlayerInputFormat : public InputFormat
{
	friend class FormatManager;

protected:
	bool parseSubtitles(Subtitle &subtitle, const QString &data) const override
	{
		unsigned readLines = 0;

		if(m_regExp.indexIn(data, 0) == -1)
			return false;

		Time previousShowTime(m_regExp.cap(1).toInt(), m_regExp.cap(2).toInt(), m_regExp.cap(3).toInt(), 0);
		QString previousText(m_regExp.cap(4).replace('|', '\n').trimmed());

		int offset = m_regExp.matchedLength();
		for(; m_regExp.indexIn(data, offset) != -1; offset += m_regExp.matchedLength()) {
			Time showTime(m_regExp.cap(1).toInt(), m_regExp.cap(2).toInt(), m_regExp.cap(3).toInt(), 0);
			QString text(m_regExp.cap(4).replace('|', '\n').trimmed());

			// To compensate for the format deficiencies, Subtitle Composer writes empty lines
			// indicating that way the line hide time. We do the same.
			if(!previousText.isEmpty()) {
				SubtitleLine *l = new SubtitleLine(previousShowTime, showTime);
				l->primaryDoc()->setPlainText(previousText);
				subtitle.insertLine(l);

				readLines++;
			}

			previousText = text;
			previousShowTime = showTime;
		}
		if(!previousText.isEmpty()) {
			SubtitleLine *l = new SubtitleLine(previousShowTime, previousShowTime + 2000);
			l->primaryDoc()->setPlainText(previousText);
			subtitle.insertLine(l);

			readLines++;
		}

		return true;
	}

	TMPlayerInputFormat() :
		InputFormat(QStringLiteral("TMPlayer"), QStringList() << QStringLiteral("sub") << QStringLiteral("txt")),
		m_regExp(QStringLiteral("([0-2]?[0-9]):([0-5][0-9]):([0-5][0-9]):([^\n]*)\n?")) {}

	TMPlayerInputFormat(const QString &name, const QStringList &extensions, const QString &regExp) :
		InputFormat(name, extensions),
		m_regExp(regExp) {}

	mutable QRegExp m_regExp;
};

class TMPlayerPlusInputFormat : public TMPlayerInputFormat
{
	friend class FormatManager;

protected:
	TMPlayerPlusInputFormat() :
		TMPlayerInputFormat(QStringLiteral("TMPlayer+"), QStringList() << QStringLiteral("sub") << QStringLiteral("txt"), QStringLiteral("([0-2]?[0-9]):([0-5][0-9]):([0-5][0-9])=([^\n]*)\n?"))
	{}
};
}

#endif
