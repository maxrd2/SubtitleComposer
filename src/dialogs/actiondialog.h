#ifndef ACTIONDIALOG_H
#define ACTIONDIALOG_H

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

#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>

QT_FORWARD_DECLARE_CLASS(QGridLayout)
QT_FORWARD_DECLARE_CLASS(QVBoxLayout)
QT_FORWARD_DECLARE_CLASS(QGroupBox)

namespace SubtitleComposer {
class ActionDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ActionDialog(const QString &title, QWidget *parent = 0);

public slots:
	virtual int exec() override;
	virtual void show();

protected:
	QGroupBox * createGroupBox(const QString &title = QString(), bool addToLayout = true);
	QGridLayout * createLayout(QGroupBox *groupBox);

protected:
	QWidget *m_mainWidget;
	QVBoxLayout *m_mainLayout;
	QDialogButtonBox *m_buttonBox;
};
}
#endif
