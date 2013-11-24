#ifndef PLAYERWIDGET_H
#define PLAYERWIDGET_H

/***************************************************************************
 *   Copyright (C) 2007-2009 Sergio Pistone (sergio_pistone@yahoo.com.ar)  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "../core/time.h"
#include "../core/subtitle.h"
#include "../core/subtitleline.h"

#include <QtCore/QPoint>
#include <QtGui/QWidget>

class LayeredWidget;
class TextOverlayWidget;
class AttachableWidget;
class TimeEdit;
class QGridLayout;
class QGroupBox;
class QLabel;
class QToolButton;
class QSlider;

namespace SubtitleComposer {
class Player;

class PlayerWidget : public QWidget
{
	Q_OBJECT

public:
	PlayerWidget(QWidget *parent);
	virtual ~PlayerWidget();

	void loadConfig();
	void saveConfig();

	bool fullScreenMode() const;
	void setFullScreenMode(bool fullScreenMode);

	SubtitleLine * playingLine();
	SubtitleLine * overlayLine();

	void plugActions();

	virtual bool eventFilter(QObject *object, QEvent *event);

public slots:
	void setSubtitle(Subtitle *subtitle = 0);
	void setTranslationMode(bool enabled);
	void setShowTranslation(bool showTranslation);

	void increaseFontSize(int points = 1);
	void decreaseFontSize(int points = 1);

signals:
	void playingLineChanged(SubtitleLine *line);

protected:
	virtual void timerEvent(QTimerEvent *event);

private:
	static QToolButton * toolButton(QWidget *parent, const char *name);
	static QToolButton * createToolButton(QWidget *parent, const char *name, int size);

	void updateOverlayLine(const Time &videoPosition);
	void setOverlayLine(SubtitleLine *line);
	void setPlayingLine(SubtitleLine *line);

	void updatePositionEditVisibility();

private slots:
	void invalidateOverlayLine();

	void onVolumeSliderValueChanged(int value);
	void onSeekSliderValueChanged(int value);
	void onSeekSliderMoved(int value);
	void onSeekSliderPressed();
	void onSeekSliderReleased();
	void onPositionEditValueChanged(int position);

	void onPlayerOptionChanged(const QString &option, const QString &value);

	void onPlayerFileOpened(const QString &filePath);
	void onPlayerFileOpenError(const QString &filePath);
	void onPlayerFileClosed();
	void onPlayerPlaybackError(const QString &errorMessage);
	void onPlayerPlaying();
	void onPlayerStopped();
	void onPlayerPositionChanged(double seconds);
	void onPlayerLengthChanged(double seconds);
	void onPlayerFramesPerSecondChanged(double fps);
	void onPlayerVolumeChanged(double volume);

	void onPlayerLeftClicked(const QPoint &point);
	void onPlayerRightClicked(const QPoint &point);
	void onPlayerDoubleClicked(const QPoint &point);

	void onPlayerBackendInitialized();

private:
	Subtitle *m_subtitle;
	bool m_translationMode;
	bool m_showTranslation;
	SubtitleLine *m_overlayLine;            // the line being shown or to be shown next
	SubtitleLine *m_playingLine;            // the line being shown or the last one shown

	int m_fullScreenTID;
	bool m_fullScreenMode;
	Player *m_player;

	Time m_lastSearchedLineToShowTime;
	Time m_lastCheckedTime;

	QGridLayout *m_mainLayout;

	LayeredWidget *m_layeredWidget;
	TextOverlayWidget *m_textOverlay;
	AttachableWidget *m_fullScreenControls;

	QSlider *m_seekSlider;
	QSlider *m_fsSeekSlider;
	QLabel *m_fsPositionLabel;
	QString m_lengthString;
	int m_updatePositionControls;           // "true" when >0
	bool m_updateVideoPosition;

	QSlider *m_volumeSlider;
	QSlider *m_fsVolumeSlider;
	bool m_updateVolumeControls;
	bool m_updatePlayerVolume;

	QGroupBox *m_infoControlsGroupBox;
	QLabel *m_positionLabel;
	TimeEdit *m_positionEdit;
	bool m_showPositionTimeEdit;
	QLabel *m_lengthLabel;
	QLabel *m_fpsLabel;

	QPoint m_savedCursorPos;                // for hidding the mouse on full screen mode
	QPoint m_currentCursorPos;
};
}
#endif
