/*
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WAVEFORMWIDGET_H
#define WAVEFORMWIDGET_H

#include "core/time.h"
#include "core/subtitle.h"
#include "gui/waveform/wavesubtitle.h"
#include "videoplayer/waveformat.h"

#include <QExplicitlySharedDataPointer>
#include <QTimer>
#include <QWidget>

#include <list>

QT_FORWARD_DECLARE_CLASS(QRegion)
QT_FORWARD_DECLARE_CLASS(QPolygon)
QT_FORWARD_DECLARE_CLASS(QProgressBar)
QT_FORWARD_DECLARE_CLASS(QToolButton)
QT_FORWARD_DECLARE_CLASS(QScrollBar)
QT_FORWARD_DECLARE_CLASS(QPropertyAnimation)
QT_FORWARD_DECLARE_CLASS(QToolButton)
QT_FORWARD_DECLARE_CLASS(QBoxLayout)

namespace SubtitleComposer {
class WaveBuffer;
class WaveRenderer;
struct WaveZoomData;

class WaveformWidget : public QWidget
{
	Q_OBJECT

public:
	explicit WaveformWidget(QWidget *parent = nullptr);
	virtual ~WaveformWidget();

	void setZoom(quint32 val);
	inline quint32 zoom() const { return m_zoom; }

	inline double windowSize() const { return (m_timeEnd - m_timeStart).toMillis(); }
	double windowSizeInner(double *autoScrollPadding = nullptr) const;

	QWidget *progressWidget();

	QWidget *toolbarWidget();

	inline bool autoScroll() const { return m_autoScroll; }

	inline const Time & rightMousePressTime() const { return m_timeRMBPress; }
	inline const Time & rightMouseReleaseTime() const { return m_timeRMBRelease; }

	SubtitleLine * subtitleLineAtMousePosition() const;

	inline const Time & rightMouseSoonerTime() const {
		return m_timeRMBPress > m_timeRMBRelease ? m_timeRMBRelease : m_timeRMBPress;
	}
	inline const Time & rightMouseLaterTime() const {
		return m_timeRMBPress > m_timeRMBRelease ? m_timeRMBPress : m_timeRMBRelease;
	}

	inline void zoomIn() { setZoom(zoom() / 2); }
	inline void zoomOut() { setZoom(zoom() * 2); }

signals:
	void doubleClick(Time time);
	void middleMouseDown(Time time);
	void middleMouseMove(Time time);
	void middleMouseUp(Time time);
	void dragStart(SubtitleLine *line, DragPosition dragPosition);
	void dragEnd(SubtitleLine *line, DragPosition dragPosition);

public slots:
	void setSubtitle(Subtitle *subtitle = 0);
	void setAudioStream(const QString &mediaFile, int audioStream);
	void setNullAudioStream(quint64 msecVideoLength);
	void clearAudioStream();
	void setAutoscroll(bool autoscroll);
	void setScrollPosition(double milliseconds);
	void onSubtitleChanged();
	void setTranslationMode(bool enabled);
	void setShowTranslation(bool showTranslation);

private slots:
	void onPlayerPositionChanged(double seconds);
	void onScrollBarValueChanged(int value);
	void onHoverScrollTimeout();

private:
	void leaveEvent(QEvent *event) override;
	bool eventFilter(QObject *obj, QEvent *event) override;
	void showContextMenu(QPoint pos);

	void onWaveformResize(quint32 span);
	void onWaveformRotate(bool vertical);

	void handleTimeUpdate(quint32 msSpan);

	QToolButton * createToolButton(const QString &actionName, int iconSize=16);
	void updateActions();
	void updateVisibleLines();
	Time timeAt(int y);
	DragPosition draggableAt(double posTime, WaveSubtitle **result);
	bool scrollToTime(const Time &time, bool scrollToPage);

	void updatePointerTime(int pos);
	bool mousePress(int pos, Qt::MouseButton button);
	bool mouseRelease(int pos, Qt::MouseButton button, const QPointF &globalPos);

private:
	QString m_mediaFile;
	int m_streamIndex;
	QExplicitlySharedDataPointer<Subtitle> m_subtitle;

	Time m_timeStart;
	Time m_timeCurrent;
	Time m_timeEnd;
	quint32 m_zoom; // reflects samples/pixel

	Time m_timeRMBPress;
	Time m_timeRMBRelease;
	bool m_RMBDown;

	bool m_MMBDown;

	QScrollBar *m_scrollBar;
	QPropertyAnimation *m_scrollAnimation;
	bool m_autoScroll;
	bool m_autoScrollPause;
	double m_hoverScrollAmount;
	QTimer m_hoverScrollTimer;

	QWidget *m_toolbar;

	WaveRenderer *m_waveformGraphics;

	QWidget *m_progressWidget;
	QProgressBar *m_progressBar;

	std::list<WaveSubtitle *> m_visibleLines;
	bool m_visibleLinesDirty;

	WaveSubtitle *m_draggedLine;

	QBoxLayout *m_widgetLayout;

	Time m_pointerTime;

	QToolButton *m_btnZoomIn;
	QToolButton *m_btnZoomOut;
	QToolButton *m_btnAutoScroll;

	bool m_translationMode;
	bool m_showTranslation;

	friend class WaveBuffer;
	WaveBuffer *m_wfBuffer;

	WaveZoomData **m_zoomData;
	quint32 m_zoomDataLen;

	friend class WaveRenderer;
};
}
#endif
