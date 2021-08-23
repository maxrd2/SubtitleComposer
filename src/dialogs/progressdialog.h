#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

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

QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QProgressBar)
QT_FORWARD_DECLARE_CLASS(QCloseEvent)
QT_FORWARD_DECLARE_CLASS(QDialogButtonBox)

namespace SubtitleComposer {
class ProgressDialog : public QDialog
{
	Q_OBJECT

public:
	ProgressDialog(const QString &caption, const QString &description, bool allowCancel, QWidget *parent = 0);

	int value() const;
	int minimum() const;
	int maximum() const;
	QString description() const;
	bool isCancellable() const;

protected:
	void closeEvent(QCloseEvent *event) override;

public slots:
	void setMinimum(int minimum);
	void incrementMinimum(int delta);
	void setMaximum(int maximum);
	void incrementMaximum(int delta);
	void setValue(int value);
	void setDescription(const QString &description);
	void setCancellable(bool cancellable);

private:
	QLabel *m_label;
	QProgressBar *m_progressBar;
	QDialogButtonBox *m_buttonBox;
};
}
#endif
