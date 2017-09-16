#ifndef WAVEFORMWIDGET_H
#define WAVEFORMWIDGET_H
/*
 * Copyright (C) 2010-2017 Mladen Milinkovic <max@smoothware.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#include "../core/time.h"
#include "../core/subtitle.h"
#include "../videoplayer/waveformat.h"
#include "../streamprocessor/streamprocessor.h"

#include <QWidget>
#include <QList>
#include <QPen>
#include <QColor>
#include <QFont>
#include <QTimer>

QT_FORWARD_DECLARE_CLASS(QRegion)
QT_FORWARD_DECLARE_CLASS(QPolygon)
QT_FORWARD_DECLARE_CLASS(QProgressBar)
QT_FORWARD_DECLARE_CLASS(QToolButton)
QT_FORWARD_DECLARE_CLASS(QScrollBar)
QT_FORWARD_DECLARE_CLASS(QToolButton)
QT_FORWARD_DECLARE_CLASS(QBoxLayout)

// FIXME: make sample size configurable or drop this
/*
#define BYTES_PER_SAMPLE 2
#define SAMPLE_TYPE qint16
#define SIGNED_PAD 0 // 32768
#define SAMPLE_MAX 32768 // 65535
/*/
#define BYTES_PER_SAMPLE 1
#define SAMPLE_TYPE quint8
#define SIGNED_PAD -128 // 0
#define SAMPLE_MAX 128 // 255
//*/
#define SAMPLE_RATE_MILIS (8000 / 1000)

namespace SubtitleComposer {
class WaveformWidget : public QWidget
{
	Q_OBJECT

private:
	enum DragPosition {
		DRAG_NONE = 0,
		DRAG_SHOW,
		DRAG_LINE,
		DRAG_HIDE
	};

public:
	WaveformWidget(QWidget *parent);
	virtual ~WaveformWidget();

	void updateActions();

	quint32 windowSize() const;
	void setWindowSize(const quint32 size);

	QWidget *progressWidget();

	QWidget *toolbarWidget();

	inline bool autoScroll() const { return m_autoScroll; }

	inline const Time & rightMousePressTime() const { return m_timeRMBPress; }
	inline const Time & rightMouseReleaseTime() const { return m_timeRMBRelease; }

signals:
	void doubleClick(Time time);
	void dragStart(SubtitleLine *line, DragPosition dragPosition);
	void dragEnd(SubtitleLine *line, DragPosition dragPosition);

public slots:
	void setSubtitle(Subtitle *subtitle = 0);
	void setAudioStream(const QString &mediaFile, int audioStream);
	void setNullAudioStream(quint64 msecVideoLength);
	void clearAudioStream();
	void zoomIn();
	void zoomOut();
	void setAutoscroll(bool autoscroll);
	void setScrollPosition(int milliseconds);
	void onConfigChanged();
	void onSubtitleChanged();

protected:
	void resizeEvent(QResizeEvent *event);
	void leaveEvent(QEvent *event);
	bool eventFilter(QObject *obj, QEvent *event);
	void showContextMenu(QMouseEvent *event);

private slots:
	void onPlayerPositionChanged(double seconds);
	void onStreamData(const void *buffer, const qint32 size, const WaveFormat *waveFormat);
	void onStreamProgress(quint64 msecPos, quint64 msecLength);
	void onStreamFinished();
	void onScrollBarValueChanged(int value);
	void onHoverScrollTimeout();

private:
	void paintGraphics(QPainter &painter);
	QToolButton * createToolButton(const QString &actionName, int iconSize=16);
	void updateZoomData();
	void updateVisibleLines();
	Time timeAt(int y);
	WaveformWidget::DragPosition subtitleAt(int y, SubtitleLine **result);
	void setupScrollBar();
	bool autoscrollToTime(const Time &time, bool scrollPage);

private:
	QString m_mediaFile;
	int m_streamIndex;

	StreamProcessor *m_stream;
	Subtitle *m_subtitle;

	Time m_timeStart;
	Time m_timeCurrent;
	Time m_timeEnd;

	Time m_timeRMBPress;
	Time m_timeRMBRelease;
	bool m_RMBDown;

	QScrollBar *m_scrollBar;
	bool m_autoScroll;
	bool m_userScroll;
	double m_hoverScrollAmount;
	QTimer m_hoverScrollTimer;

	quint32 m_waveformDuration;
	quint32 m_waveformDataOffset;
	quint32 m_waveformChannels;
	quint32 m_waveformChannelSize;
	SAMPLE_TYPE **m_waveform;

	QWidget *m_toolbar;

	QWidget *m_waveformGraphics;

	QWidget *m_progressWidget;
	QProgressBar *m_progressBar;

	struct ZoomData {
		qint32 min;
		qint32 max;
	};
	double m_samplesPerPixel;
	ZoomData **m_waveformZoomed;
	quint32 m_waveformZoomedSize;
	quint32 m_waveformZoomedOffset;

	QList<SubtitleLine *> m_visibleLines;
	bool m_visibleLinesDirty;

	SubtitleLine *m_draggedLine;
	DragPosition m_draggedPos;
	Time m_draggedTime;
	double m_draggedOffset;

	bool m_vertical;
	QBoxLayout *m_mainLayout;
	QBoxLayout *m_widgetLayout;

	Time m_pointerTime;

	QToolButton *m_btnZoomIn;
	QToolButton *m_btnZoomOut;
	QToolButton *m_btnAutoScroll;

	QFont m_fontNumber;
	int m_fontNumberHeight;
	QFont m_fontText;

	int m_subBorderWidth;

	QPen m_subNumberColor;
	QPen m_subTextColor;

	QPen m_waveInner;
	QPen m_waveOuter;

	QColor m_subtitleBack;
	QColor m_subtitleBorder;

	QColor m_selectedBack;
	QColor m_selectedBorder;

	QPen m_playColor;
	QPen m_mouseColor;
};
}
#endif
