/*
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "waveformwidget.h"

#include "appglobal.h"
#include "application.h"
#include "scconfig.h"
#include "core/subtitleline.h"
#include "videoplayer/videoplayer.h"
#include "actions/useraction.h"
#include "actions/useractionnames.h"
#include "gui/treeview/lineswidget.h"
#include "gui/waveform/wavebuffer.h"
#include "gui/waveform/waverenderer.h"
#include "gui/waveform/zoombuffer.h"

#include <QRect>
#include <QPainter>
#include <QPaintEvent>
#include <QPolygon>
#include <QRegion>
#include <QThread>

#include <QProgressBar>
#include <QLabel>
#include <QMenu>
#include <QBoxLayout>
#include <QToolButton>
#include <QScrollBar>
#include <QPropertyAnimation>
#include <QDebug>

#include <KLocalizedString>

using namespace SubtitleComposer;

#define ZOOM_MIN (1 << 3)

WaveformWidget::WaveformWidget(QWidget *parent)
	: QWidget(parent),
	  m_mediaFile(QString()),
	  m_streamIndex(-1),
	  m_subtitle(nullptr),
	  m_timeStart(0.),
	  m_timeCurrent(0.),
	  m_timeEnd(WaveBuffer::MAX_WINDOW_ZOOM()),
	  m_zoom(1 << 6),
	  m_RMBDown(false),
	  m_MMBDown(false),
	  m_scrollBar(nullptr),
	  m_scrollAnimation(nullptr),
	  m_autoScroll(true),
	  m_autoScrollPause(false),
	  m_hoverScrollAmount(.0),
	  m_waveformGraphics(new WaveRenderer(this)),
	  m_progressWidget(new QWidget(this)),
	  m_visibleLinesDirty(true),
	  m_draggedLine(nullptr),
	  m_widgetLayout(nullptr),
	  m_translationMode(false),
	  m_showTranslation(false),
	  m_wfBuffer(new WaveBuffer(this)),
	  m_zoomData(nullptr),
	  m_zoomDataLen(0)
{
	m_widgetLayout = new QBoxLayout(QBoxLayout::LeftToRight);
	m_widgetLayout->setContentsMargins(0, 0, 0, 0);
	m_widgetLayout->setSpacing(0);

	m_waveformGraphics->installEventFilter(this);
	m_widgetLayout->addWidget(m_waveformGraphics);

	connect(m_wfBuffer->zoomBuffer(), &ZoomBuffer::zoomedBufferReady, m_waveformGraphics, QOverload<>::of(&QWidget::update));

	m_scrollBar = new QScrollBar(Qt::Vertical, this);
	m_scrollBar->setPageStep(windowSize());
	m_scrollBar->setRange(0, windowSize());
	m_scrollBar->installEventFilter(this);
	m_widgetLayout->addWidget(m_scrollBar);

	m_scrollAnimation = new QPropertyAnimation(m_scrollBar, QByteArrayLiteral("value"), this);
	m_scrollAnimation->setDuration(150);

	m_btnZoomOut = createToolButton(QStringLiteral(ACT_WAVEFORM_ZOOM_OUT));
	m_btnZoomIn = createToolButton(QStringLiteral(ACT_WAVEFORM_ZOOM_IN));
	m_btnAutoScroll = createToolButton(QStringLiteral(ACT_WAVEFORM_AUTOSCROLL));

	QHBoxLayout *toolbarLayout = new QHBoxLayout();
	toolbarLayout->setContentsMargins(0, 0, 0, 0);
	toolbarLayout->setSpacing(2);
	toolbarLayout->addWidget(m_btnZoomOut);
	toolbarLayout->addWidget(m_btnZoomIn);
	toolbarLayout->addSpacerItem(new QSpacerItem(2, 2, QSizePolicy::Preferred, QSizePolicy::Preferred));
	toolbarLayout->addWidget(m_btnAutoScroll);
	toolbarLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Preferred));

	m_toolbar = new QWidget(this);
	m_toolbar->setLayout(toolbarLayout);

	QBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setSpacing(5);
	mainLayout->addWidget(m_toolbar);
	mainLayout->addLayout(m_widgetLayout);

	setMinimumWidth(300);

	// Progress Bar
	m_progressWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
	m_progressWidget->hide();

	QLabel *label = new QLabel(i18n("Generating waveform"), m_progressWidget);

	m_progressBar = new QProgressBar(m_progressWidget);
	m_progressBar->setMinimumWidth(300);
	m_progressBar->setTextVisible(true);

	QLayout *layout = new QBoxLayout(QBoxLayout::LeftToRight, m_progressWidget);
	layout->setContentsMargins(1, 0, 1, 0);
	layout->setSpacing(1);
	layout->addWidget(label);
	layout->addWidget(m_progressBar);

	connect(m_scrollBar, &QScrollBar::valueChanged, this, &WaveformWidget::onScrollBarValueChanged);

	connect(VideoPlayer::instance(), &VideoPlayer::positionChanged, this, &WaveformWidget::onPlayerPositionChanged);

	m_hoverScrollTimer.setInterval(50);
	m_hoverScrollTimer.setSingleShot(false);
	connect(&m_hoverScrollTimer, &QTimer::timeout, this, &WaveformWidget::onHoverScrollTimeout);

	connect(app(), &Application::actionsReady, this, &WaveformWidget::updateActions);
	connect(m_wfBuffer, &WaveBuffer::waveformUpdated, this, [this]() {
		onWaveformResize(m_waveformGraphics->span());
	});
}

void
WaveformWidget::updateActions()
{
	const Application *app = SubtitleComposer::app();

	const quint32 span = m_waveformGraphics->span();
	const quint32 maxZoom = span ? m_wfBuffer->lengthSamples() / span : 0;

	m_btnZoomIn->setDefaultAction(app->action(ACT_WAVEFORM_ZOOM_IN));
	m_btnZoomIn->setEnabled(m_zoom > ZOOM_MIN);

	m_btnZoomOut->setDefaultAction(app->action(ACT_WAVEFORM_ZOOM_OUT));
	m_btnZoomOut->setEnabled(m_zoom < maxZoom);

	QAction *action = app->action(ACT_WAVEFORM_AUTOSCROLL);
	action->setChecked(m_autoScroll);
	m_btnAutoScroll->setDefaultAction(action);
	m_btnAutoScroll->setEnabled(m_wfBuffer->waveformDuration() > 0);
}

WaveformWidget::~WaveformWidget()
{
	clearAudioStream();
}

double
WaveformWidget::windowSizeInner(double *autoScrollPadding) const
{
	const double winSize = windowSize();
	const double scrollPad = winSize * double(SCConfig::wfAutoscrollPadding()) / 100.;
	if(autoScrollPadding)
		*autoScrollPadding = scrollPad;
	const double innerSize = winSize - 2. * scrollPad;
	return qMax(innerSize, 1.);
}

void
WaveformWidget::setZoom(quint32 val)
{
	const quint32 span = m_waveformGraphics->span();
	if(span) {
		const quint32 maxZoom = m_wfBuffer->lengthSamples() / span;
		if(maxZoom && val > maxZoom)
			val = maxZoom;
	}
	if(val < ZOOM_MIN)
		val = ZOOM_MIN;

	if(m_zoom == val)
		return;

	m_wfBuffer->zoomBuffer()->setZoomScale(val);

	const quint32 samMS = m_wfBuffer->sampleRateMillis();
	if(samMS) {
		const qint32 msSpanOld = m_zoom * span / samMS;
		m_zoom = val;

		const quint32 msSpanNew = val * span / samMS;
		m_timeStart.shift((msSpanOld - qint32(msSpanNew)) / 2);
		m_timeEnd = m_timeStart.shifted(msSpanNew);
		handleTimeUpdate(msSpanNew);
	} else {
		m_zoom = val;
	}

	updateActions();
}

void
WaveformWidget::handleTimeUpdate(quint32 msSpan)
{
	// make sure start time is good
	const quint32 msDuration = m_wfBuffer->waveformDuration() * 1000;
	const quint32 maxTime = msDuration < msSpan ? 0 : msDuration - msSpan;
	if(m_timeStart.toMillis() > maxTime) {
		m_timeStart.setMillisTime(maxTime);
		m_timeEnd.setMillisTime(maxTime + msSpan);
	}

	QSignalBlocker s(m_scrollBar);
	m_scrollBar->setPageStep(msSpan);
	m_scrollBar->setRange(0, maxTime);
	m_scrollBar->setValue(m_timeStart.toMillis());

	const quint16 chans = m_wfBuffer->channels();
	if(chans) {
		m_wfBuffer->zoomBuffer()->setZoomScale(m_zoom);
		if(!m_zoomData)
			m_zoomData = new WaveZoomData *[chans];
		m_wfBuffer->zoomBuffer()->zoomedBuffer(m_timeStart.toMillis(), m_timeEnd.toMillis(), m_zoomData, &m_zoomDataLen);
	}

	m_visibleLinesDirty = true;
	m_waveformGraphics->update();
}

void
WaveformWidget::onWaveformResize(quint32 span)
{
	const quint32 samMS = m_wfBuffer->sampleRateMillis();
	if(samMS) {
		const double windowSize = m_zoom * span / samMS;
		m_timeEnd = m_timeStart.shifted(windowSize);
		handleTimeUpdate(windowSize);
		updateActions();
	} else {
		m_visibleLinesDirty = true;
		m_waveformGraphics->update();
	}
}

void
WaveformWidget::onWaveformRotate(bool vertical)
{
	if(vertical) {
		m_widgetLayout->setDirection(QBoxLayout::LeftToRight);
		m_scrollBar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
		m_scrollBar->setOrientation(Qt::Vertical);
	} else {
		m_widgetLayout->setDirection(QBoxLayout::TopToBottom);
		m_scrollBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
		m_scrollBar->setOrientation(Qt::Horizontal);
	}
}

void
WaveformWidget::setAutoscroll(bool autoscroll)
{
	m_autoScroll = autoscroll;
	app()->action(ACT_WAVEFORM_AUTOSCROLL)->setChecked(m_autoScroll);
}

void
WaveformWidget::onScrollBarValueChanged(int value)
{
	double winSize = windowSize();
	m_timeStart = value;
	m_timeEnd = m_timeStart.shifted(winSize);
	handleTimeUpdate(winSize);
}

void
WaveformWidget::setSubtitle(Subtitle *subtitle)
{
	if(m_subtitle) {
		disconnect(m_subtitle.constData(), &Subtitle::primaryChanged, this, &WaveformWidget::onSubtitleChanged);
		disconnect(m_subtitle.constData(), &Subtitle::secondaryChanged, this, &WaveformWidget::onSubtitleChanged);
		disconnect(m_subtitle.constData(), &Subtitle::lineAnchorChanged, this, &WaveformWidget::onSubtitleChanged);
	}

	m_subtitle = subtitle;

	if(m_subtitle) {
		connect(m_subtitle.constData(), &Subtitle::primaryChanged, this, &WaveformWidget::onSubtitleChanged);
		connect(m_subtitle.constData(), &Subtitle::secondaryChanged, this, &WaveformWidget::onSubtitleChanged);
		connect(m_subtitle.constData(), &Subtitle::lineAnchorChanged, this, &WaveformWidget::onSubtitleChanged);
	}

	m_visibleLines.clear();
	m_visibleLinesDirty = true;

	m_waveformGraphics->update();
}

void
WaveformWidget::onSubtitleChanged()
{
	m_visibleLinesDirty = true;
	m_waveformGraphics->update();
}

QWidget *
WaveformWidget::progressWidget()
{
	return m_progressWidget;
}

QWidget *
WaveformWidget::toolbarWidget()
{
	return m_toolbar;
}

void
WaveformWidget::setAudioStream(const QString &mediaFile, int audioStream)
{
	if(m_mediaFile == mediaFile && audioStream == m_streamIndex)
		return;

	clearAudioStream();

	m_mediaFile = mediaFile;
	m_streamIndex = audioStream;

	m_wfBuffer->setAudioStream(m_mediaFile, m_streamIndex);
}

void
WaveformWidget::setNullAudioStream(quint64 msecVideoLength)
{
	clearAudioStream();

	m_timeStart.setMillisTime(0.);
	m_timeStart.setMillisTime(msecVideoLength);
	// will do onTimeUpdated() during event from m_wfBuffer->setNullAudioStream()
	m_wfBuffer->setNullAudioStream(msecVideoLength);
}

void
WaveformWidget::clearAudioStream()
{
	m_wfBuffer->clearAudioStream();

	m_mediaFile.clear();
	m_streamIndex = -1;

	delete[] m_zoomData;
	m_zoomData = nullptr;
	m_zoomDataLen = 0;
}

void
WaveformWidget::updateVisibleLines()
{
	if(!m_subtitle || !m_visibleLinesDirty)
		return;

	m_visibleLinesDirty = false;

	auto it = m_visibleLines.begin();
	while(it != m_visibleLines.end()) {
		if(*it == m_draggedLine) {
			it = m_visibleLines.erase(it);
		} else if(!(*it)->line()->intersectsTimespan(m_timeStart, m_timeEnd)) {
			delete *it;
			it = m_visibleLines.erase(it);
		} else {
			++it;
		}
	}

	it = m_visibleLines.begin();
	for(int i = 0, n = m_subtitle->count(); i < n; i++) {
		SubtitleLine *sub = m_subtitle->at(i);
		const bool isDragged = m_draggedLine != nullptr && sub == m_draggedLine->line();
		if(!sub->intersectsTimespan(m_timeStart, m_timeEnd) && !isDragged)
			continue;
		const Time showTime = isDragged ? m_draggedLine->showTime() : sub->showTime();
		while(it != m_visibleLines.end() && (*it)->showTime() < showTime) {
			if((*it)->line() == sub)
				break;
			++it;
		}
		if(it == m_visibleLines.end() || (*it)->line() != sub) {
			if(isDragged) {
				m_visibleLines.emplace(it, m_draggedLine);
				it = m_visibleLines.begin();
			} else {
				it = m_visibleLines.emplace(it, new WaveSubtitle(sub, m_waveformGraphics));
				++it;
			}
		}
	}
}

void
WaveformWidget::leaveEvent(QEvent */*event*/)
{
	m_pointerTime.setMillisTime(std::numeric_limits<double>::max());

	if(m_autoScrollPause) {
		if(!m_RMBDown)
			m_autoScrollPause = false;
		if(m_autoScroll && !m_draggedLine && !m_autoScrollPause)
			scrollToTime(m_timeCurrent, true);
	}

	m_waveformGraphics->update();
}

bool
WaveformWidget::eventFilter(QObject */*obj*/, QEvent *event)
{
	switch(event->type()) {
	case QEvent::Wheel: {
		QPoint delta = static_cast<QWheelEvent *>(event)->angleDelta() / 8;
		if(delta.isNull())
			delta = static_cast<QWheelEvent *>(event)->pixelDelta();
		if(delta.isNull())
			return false;

		m_autoScrollPause = true;

		m_scrollBar->setValue(m_timeStart.shifted(-4 * double(delta.ry()) * windowSize() / m_waveformGraphics->span()).toMillis());
		return true; // stop wheel events from propagating
	}
	case QEvent::MouseButtonPress:
		m_autoScrollPause = true;
		return false;

	default:
		return false;
	}
}

Time
WaveformWidget::timeAt(int y)
{
	return m_timeStart + double(y * qint32(windowSize()) / m_waveformGraphics->span());
}

DragPosition
WaveformWidget::draggableAt(double posTime, WaveSubtitle **result)
{
	if(result)
		*result = nullptr;

	if(!m_wfBuffer->sampleRateMillis())
		return DRAG_NONE;

	double msTolerance = double(10 * m_wfBuffer->millisPerPixel());

	DragPosition dragMode = DRAG_NONE;
	for(WaveSubtitle *sub: m_visibleLines) {
		DragPosition dm = sub->draggableAt(posTime, &msTolerance);
		if(dm > DRAG_FORBIDDEN || dragMode == DRAG_NONE) {
			dragMode = dm;
			if(result && dm > DRAG_FORBIDDEN)
				*result = sub;
		}
	}

	return dragMode;
}

SubtitleLine *
WaveformWidget::subtitleLineAtMousePosition() const
{
	const Time mouseTime = m_RMBDown ? m_timeRMBRelease : m_pointerTime;
	for(const WaveSubtitle *sub: qAsConst(m_visibleLines)) {
		if(sub->line()->containsTime(mouseTime))
			return sub->line();
	}
	return nullptr;
}

void
WaveformWidget::setScrollPosition(double milliseconds)
{
	if(milliseconds < m_timeStart.toMillis() || milliseconds > m_timeEnd.toMillis()) {
		scrollToTime(milliseconds, true);
		m_visibleLinesDirty = true;
		m_waveformGraphics->update();
	}
}

void
WaveformWidget::onHoverScrollTimeout()
{
	if(!m_draggedLine && !m_RMBDown && !m_MMBDown) {
		m_hoverScrollAmount = .0;
		m_hoverScrollTimer.stop();
		return;
	}

	if(m_hoverScrollAmount == .0)
		return;

	const Time ptrTime(m_pointerTime.toMillis() + m_hoverScrollAmount);
	if(m_draggedLine)
		m_draggedLine->dragUpdate(ptrTime.toMillis());
	if(m_RMBDown)
		m_timeRMBRelease = ptrTime;
	m_scrollBar->setValue(m_timeStart.toMillis() + m_hoverScrollAmount);
}

void
WaveformWidget::updatePointerTime(int pos)
{
	m_pointerTime = timeAt(pos);

	if(m_RMBDown) {
		m_timeRMBRelease = m_pointerTime;
		scrollToTime(m_pointerTime, false);
	}

	if(m_MMBDown) {
		scrollToTime(m_pointerTime, false);
		emit middleMouseMove(m_pointerTime);
	}

	if(m_draggedLine) {
		m_draggedLine->dragUpdate(m_pointerTime.toMillis());
		scrollToTime(m_pointerTime, false);
	} else {
		const double posTime = timeAt(pos).toMillis();
		DragPosition res = draggableAt(posTime, nullptr);
		if(res == DRAG_FORBIDDEN)
			m_waveformGraphics->setCursor(QCursor(Qt::ForbiddenCursor));
		else if(res == DRAG_LINE)
			m_waveformGraphics->setCursor(QCursor(m_waveformGraphics->vertical() ? Qt::SizeVerCursor : Qt::SizeHorCursor));
		else if(res == DRAG_SHOW || res == DRAG_HIDE)
			m_waveformGraphics->setCursor(QCursor(m_waveformGraphics->vertical() ? Qt::SplitVCursor : Qt::SplitHCursor));
		else
			m_waveformGraphics->unsetCursor();
	}
}

bool
WaveformWidget::mousePress(int pos, Qt::MouseButton button)
{
	if(button == Qt::RightButton) {
		m_timeRMBPress = m_timeRMBRelease = timeAt(pos);
		m_RMBDown = true;
		return false;
	}

	if(button == Qt::MiddleButton) {
		m_MMBDown = true;
		emit middleMouseDown(timeAt(pos));
		return false;
	}

	if(button != Qt::LeftButton)
		return false;

	const double posTime = timeAt(pos).toMillis();
	DragPosition dragMode = draggableAt(posTime, &m_draggedLine);
	if(m_draggedLine) {
		m_pointerTime.setMillisTime(posTime);
		m_draggedLine->dragStart(dragMode, posTime);
		emit dragStart(m_draggedLine->line(), dragMode);
	}

	return true;
}

bool
WaveformWidget::mouseRelease(int pos, Qt::MouseButton button, QPoint globalPos)
{
	if(button == Qt::RightButton) {
		m_timeRMBRelease = timeAt(pos);
		m_hoverScrollTimer.stop();
		showContextMenu(globalPos);
		m_RMBDown = false;
		return false;
	}

	if(button == Qt::MiddleButton) {
		emit middleMouseUp(timeAt(pos));
		m_hoverScrollTimer.stop();
		m_MMBDown = false;
		return true;
	}

	if(button != Qt::LeftButton)
		return false;

	if(m_draggedLine) {
		DragPosition mode = m_draggedLine->dragEnd(timeAt(pos).toMillis());
		emit dragEnd(m_draggedLine->line(), mode);
		m_draggedLine = nullptr;
	}
	return true;
}

bool
WaveformWidget::scrollToTime(const Time &time, bool scrollToPage)
{
	double windowPadding;
	const double windowSize = windowSizeInner(&windowPadding);
	if(m_draggedLine || m_RMBDown || m_MMBDown)
		windowPadding = this->windowSize() / 5.;

	const double topPadding = m_timeStart.toMillis() + windowPadding;
	const double bottomPadding = m_timeEnd.toMillis() - windowPadding;
	if(time <= bottomPadding && time >= topPadding) {
		if(!scrollToPage) {
			m_hoverScrollAmount = .0;
			m_hoverScrollTimer.stop();
		}
		return false;
	}

	if(scrollToPage) {
		const int scrollPosition = int(time.toMillis() / windowSize) * windowSize - windowPadding;
		if(SCConfig::wfSmoothScroll()) {
			m_scrollAnimation->setEndValue(scrollPosition);
			m_scrollAnimation->start();
		} else {
			m_scrollBar->setValue(scrollPosition);
		}
	} else {
		const double bd = time.toMillis() - (time < topPadding ? topPadding : bottomPadding);
		m_hoverScrollAmount = bd * bd * bd / (3. * windowPadding * windowPadding);
		if(!m_hoverScrollTimer.isActive())
			m_hoverScrollTimer.start();
	}

	return true;
}

void
WaveformWidget::onPlayerPositionChanged(double seconds)
{
	Time playingPosition;
	playingPosition.setSecondsTime(seconds);

	if(m_timeCurrent != playingPosition) {
		m_timeCurrent = playingPosition;

		if(m_autoScroll && !m_draggedLine && !m_autoScrollPause)
			scrollToTime(m_timeCurrent, true);

		if(m_waveformGraphics)
			m_waveformGraphics->update();
	}
}

QToolButton *
WaveformWidget::createToolButton(const QString &actionName, int iconSize)
{
	QToolButton *toolButton = new QToolButton(this);
	toolButton->setObjectName(actionName);
	toolButton->setMinimumSize(iconSize, iconSize);
	toolButton->setIconSize(iconSize >= 32 ? QSize(iconSize - 6, iconSize - 6) : QSize(iconSize, iconSize));
	toolButton->setAutoRaise(true);
	toolButton->setFocusPolicy(Qt::NoFocus);
	return toolButton;
}

void
WaveformWidget::showContextMenu(QPoint pos)
{
	static QMenu *menu = nullptr;
	static QList<QAction *> needCurrentLine;
	static QList<QAction *> needSubtitle;

	const Application *app = SubtitleComposer::app();
	SubtitleLine *currentLine = subtitleLineAtMousePosition();
	SubtitleLine *selectedLine = app->linesWidget()->currentLine();

	if(!menu) {
		UserActionManager *actionManager = UserActionManager::instance();
		menu = new QMenu(this);

		needCurrentLine.append(
			menu->addAction(QIcon::fromTheme(QStringLiteral("select")), i18n("Select Line"), [&](){
				app->linesWidget()->setCurrentLine(currentLine, true);
			}));
		menu->addSeparator();
		actionManager->addAction(
			menu->addAction(QIcon::fromTheme(QStringLiteral("list-add")), i18n("Insert Line"), [&](){
				const Time timeShow = rightMouseSoonerTime();
				const Time timeHide = rightMouseLaterTime();
				SubtitleLine *newLine = new SubtitleLine(timeShow,
					timeHide.toMillis() - timeShow.toMillis() > SCConfig::minDuration() ? timeHide : timeShow + SCConfig::minDuration());
				m_subtitle->insertLine(newLine);
				app->linesWidget()->setCurrentLine(newLine, true);
			}),
			UserAction::SubOpened);
		needCurrentLine.append(
			menu->addAction(QIcon::fromTheme(QStringLiteral("list-remove")), i18n("Remove Line"), [&](){
				m_subtitle->removeLines(RangeList(Range(currentLine->index())), SubtitleTarget::Both);
				if(selectedLine != currentLine)
					app->linesWidget()->setCurrentLine(selectedLine, true);
			}));
		menu->addSeparator();
		needSubtitle.append(
			menu->addAction(i18n("Join Lines"), this, [&](){
				int startIndex = -1, endIndex = -1;
				const Time startTime = rightMouseSoonerTime();
				const Time endTime = rightMouseLaterTime();
				for(int idx = 0, n = m_subtitle->count(); idx < n; idx++) {
					const SubtitleLine *sub = m_subtitle->at(idx);
					if(sub->intersectsTimespan(startTime, endTime)) {
						if(startIndex == -1 || startIndex > idx)
							startIndex = idx;
						if(endIndex == -1 || endIndex < idx)
							endIndex = idx;
					}
				}
				if(endIndex >= 0 && startIndex != endIndex)
					m_subtitle->joinLines(RangeList(Range(startIndex, endIndex)));
			})
		);
		needCurrentLine.append(
			menu->addAction(i18n("Split Line"), this, [&](){
				// TODO: split the line at exact waveform mouse position
				m_subtitle->splitLines(RangeList(Range(currentLine->index())));
			}));
		menu->addSeparator();
		needCurrentLine.append(
			menu->addAction(i18n("Toggle Anchor"), this, [&](){ m_subtitle->toggleLineAnchor(currentLine); }));
		menu->addAction(app->action(ACT_ANCHOR_REMOVE_ALL));
		menu->addSeparator();
		actionManager->addAction(
			menu->addAction(QIcon::fromTheme(QStringLiteral("set_show_time")), i18n("Set Current Line Show Time"), [&](){
				selectedLine->setShowTime(m_timeRMBRelease);
			}),
			UserAction::HasSelection | UserAction::EditableShowTime);
		actionManager->addAction(
			menu->addAction(QIcon::fromTheme(QStringLiteral("set_hide_time")), i18n("Set Current Line Hide Time"), [&](){
				selectedLine->setHideTime(m_timeRMBRelease);
			}),
			UserAction::HasSelection | UserAction::EditableShowTime);
	}

	foreach(QAction *action, needCurrentLine)
		action->setDisabled(currentLine == nullptr);
	foreach(QAction *action, needSubtitle)
		action->setDisabled(m_subtitle == nullptr);

	menu->exec(pos);
}

void
WaveformWidget::setTranslationMode(bool enabled)
{
	m_translationMode = enabled;

	if(!m_translationMode)
		setShowTranslation(false);
}

void
WaveformWidget::setShowTranslation(bool showTranslation)
{
	if(m_showTranslation != showTranslation) {
		m_showTranslation = showTranslation;
		m_waveformGraphics->update();
	}
}
