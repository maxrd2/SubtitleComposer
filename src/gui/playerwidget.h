#ifndef PLAYERWIDGET_H
#define PLAYERWIDGET_H

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

#include "core/time.h"
#include "core/subtitle.h"
#include "core/subtitleline.h"

#include <QExplicitlySharedDataPointer>
#include <QPoint>
#include <QWidget>

QT_FORWARD_DECLARE_CLASS(QGridLayout)
QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QToolButton)
QT_FORWARD_DECLARE_CLASS(QSlider)

class LayeredWidget;
class AttachableWidget;
class TimeEdit;

namespace SubtitleComposer {
class TextOverlayWidget;

class PlayerWidget : public QWidget
{
	Q_OBJECT

public:
	PlayerWidget(QWidget *parent);
	virtual ~PlayerWidget();

	void loadConfig();
	void saveConfig();

	inline bool fullScreenMode() const { return m_fullScreenMode; }
	void setFullScreenMode(bool fullScreenMode);

	inline SubtitleLine * playingLine() { return m_playingLine; }

	bool eventFilter(QObject *object, QEvent *event) override;

	QWidget *infoSidebarWidget();

	void pauseAfterPlayingLine(const SubtitleLine *line);

public slots:
	void setSubtitle(Subtitle *subtitle = 0);
	void setTranslationMode(bool enabled);
	void setShowTranslation(bool showTranslation);

	void increaseFontSize(int size = 1);
	void decreaseFontSize(int size = 1);

signals:
	void playingLineChanged(SubtitleLine *line);

protected:
	void timerEvent(QTimerEvent *event) override;

private:
	static QToolButton * toolButton(QWidget *parent, const char *name);
	static QToolButton * createToolButton(QWidget *parent, const char *name, int size);

	void updatePlayingLine(const Time &videoPosition);
	void setPlayingLine(SubtitleLine *line);

	void updatePositionEditVisibility();

private slots:
	void setPlayingLineFromVideo();

	void onVolumeSliderMoved(int value);
	void onSeekSliderMoved(int value);
	void onPositionEditValueChanged(int position);

	void onConfigChanged();

	void onPlayerFileOpened(const QString &filePath);
	void onPlayerFileOpenError(const QString &filePath, const QString &reason);
	void onPlayerFileClosed();
	void onPlayerPlaybackError(const QString &errorMessage);
	void onPlayerPlaying();
	void onPlayerStopped();
	void onPlayerPositionChanged(double seconds);
	void onPlayerLengthChanged(double seconds);
	void onPlayerFramesPerSecondChanged(double fps);
	void onPlayerPlaybackRateChanged(double rate);
	void onPlayerVolumeChanged(double volume);

	void onPlayerLeftClicked(const QPoint &point);
	void onPlayerRightClicked(const QPoint &point);
	void onPlayerDoubleClicked(const QPoint &point);

private:
	QExplicitlySharedDataPointer<Subtitle> m_subtitle;
	bool m_translationMode;
	bool m_showTranslation;
	SubtitleLine *m_playingLine = nullptr;
	SubtitleLine *m_prevLine = nullptr;
	SubtitleLine *m_nextLine = nullptr;

	const SubtitleLine *m_pauseAfterPlayingLine;

	int m_fullScreenTID;
	bool m_fullScreenMode;

	QGridLayout *m_mainLayout;

	LayeredWidget *m_layeredWidget;
	AttachableWidget *m_fullScreenControls;

	QSlider *m_seekSlider;
	QSlider *m_fsSeekSlider;
	QLabel *m_fsPositionLabel;
	QString m_lengthString;

	QSlider *m_volumeSlider;
	QSlider *m_fsVolumeSlider;

	QWidget *m_infoControlsGroupBox;
	QLabel *m_positionLabel;
	TimeEdit *m_positionEdit;
	bool m_showPositionTimeEdit;
	QLabel *m_lengthLabel;
	QLabel *m_fpsLabel;
	QLabel *m_rateLabel;

	QPoint m_savedCursorPos;                // for hiding the mouse on full screen mode
	QPoint m_currentCursorPos;
};
}
#endif
