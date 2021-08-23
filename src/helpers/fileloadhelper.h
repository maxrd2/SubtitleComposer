#ifndef FILELOADHELPER_H
#define FILELOADHELPER_H

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

#include <QObject>
#include <QUrl>
#include <QByteArray>

QT_FORWARD_DECLARE_CLASS(QIODevice)

class KJob;

class FileLoadHelper : public QObject
{
	Q_OBJECT

public:
	FileLoadHelper(const QUrl &url);
	~FileLoadHelper();

	const QUrl & url();
	QIODevice * file();

	bool open();
	bool close();

	static bool exists(const QUrl &url);

protected slots:
	void downloadComplete(KJob *job);

private:
	QByteArray m_data;
	QUrl m_url;
	QIODevice *m_file;
};

#endif
