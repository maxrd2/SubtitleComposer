#ifndef TEXTDEMUX_H
#define TEXTDEMUX_H

/*
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

#include <QExplicitlySharedDataPointer>
#include <QObject>

QT_FORWARD_DECLARE_CLASS(QWidget)
QT_FORWARD_DECLARE_CLASS(QProgressBar)

namespace SubtitleComposer {
class Subtitle;
class StreamProcessor;

class TextDemux : public QObject
{
	Q_OBJECT

public:
	explicit TextDemux(QWidget *parent=NULL);

	void demuxFile(Subtitle *subtitle, const QString filename, int textStreamIndex);

	QWidget * progressWidget();

signals:
	void onError(const QString &message);

private slots:
	void onStreamData(const QString &text, quint64 msecStart, quint64 msecDuration);
	void onStreamProgress(quint64 msecPos, quint64 msecLength);
	void onStreamError(int code, const QString &message, const QString &debug);
	void onStreamFinished();

private:
	QExplicitlySharedDataPointer<Subtitle> m_subtitle;
	QExplicitlySharedDataPointer<Subtitle> m_subtitleTemp;
	StreamProcessor *m_streamProcessor;

	QWidget *m_progressWidget;
	QProgressBar *m_progressBar;
};
}

#endif // TEXTDEMUX_H
