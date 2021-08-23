#ifndef FORMATDATA_H
#define FORMATDATA_H

/*
 * SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>
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

#include <QString>
#include <QMap>

namespace SubtitleComposer {
class FormatData
{
	friend class Format;

public:
	FormatData(const FormatData &formatData) :
		m_formatName(formatData.m_formatName),
		m_data(formatData.m_data) {}

	FormatData & operator=(const FormatData &formatData)
	{
		if(this == &formatData)
			return *this;

		m_formatName = formatData.m_formatName;
		m_data = formatData.m_data;

		return *this;
	}

	inline const QString & formatName()
	{
		return m_formatName;
	}

	inline const QString & value(const QString &key)
	{
		static const QString empty;
		return m_data.contains(key) ? m_data[key] : empty;
	}

	inline void setValue(const QString &key, const QString &value)
	{
		m_data[key] = value;
	}

	inline void clear()
	{
		m_data.clear();
	}

private:
	FormatData(const QString &formatName) : m_formatName(formatName) {}

	QString m_formatName;
	QMap<QString, QString> m_data;
};
}

#endif
