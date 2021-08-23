#ifndef SUBTITLECOLORDIALOG_H
#define SUBTITLECOLORDIALOG_H
/*
 * SPDX-FileCopyrightText: 2007-2012 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2013-2018 Mladen Milinkovic <max@smoothware.net>
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

#include <QColorDialog>

namespace SubtitleComposer {
class SubtitleColorDialog : public QColorDialog
{
	Q_OBJECT

public:
	explicit SubtitleColorDialog(QWidget *parent = 0);
	explicit SubtitleColorDialog(const QColor &initial, QWidget *parent = 0);
	~SubtitleColorDialog();

	static QColor getColor(const QColor &initial, QWidget *parent, const QString &title, ColorDialogOptions options = ColorDialogOptions());
	static QColor getColor(const QColor &initial = Qt::white, QWidget *parent = 0);

protected:
	bool defaultColorSelected;

public slots:
	void acceptDefaultColor();
};
}

#endif /* SUBTITLECOLORDIALOG_H */
