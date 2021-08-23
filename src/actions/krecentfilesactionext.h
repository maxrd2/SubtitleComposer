#ifndef KRECENTFILESACTIONEXT_H
#define KRECENTFILESACTIONEXT_H

/*
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

#include <QMap>
#include <QUrl>

#include <KRecentFilesAction>

class KRecentFilesActionExt : public KRecentFilesAction
{
	Q_OBJECT

public:
	explicit KRecentFilesActionExt(QObject *parent);
	virtual ~KRecentFilesActionExt();

	static QString encodingForUrl(const QUrl &url);

	void loadEntries(const KConfigGroup &configGroup);
	void saveEntries(const KConfigGroup &configGroup);

	void addUrl(const QUrl &url, const QString &encoding, const QString &name);
	inline void addUrl(const QUrl &url, const QString &encoding) { addUrl(url, encoding, QString()); }
};

#endif
