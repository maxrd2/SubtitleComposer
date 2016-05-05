/**
 * Copyright (C) 2010-2016 Mladen Milinkovic <max@smoothware.net>
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

#include "waveformwidget.h"
#include "../core/subtitleline.h"
#include "../videoplayer/videoplayer.h"
#include "application.h"
#include "actions/useractionnames.h"
#include "lineswidget.h"

#include <QRect>
#include <QPainter>
#include <QPaintEvent>
#include <QRegion>
#include <QPolygon>
#include <QThread>

#include <QProgressBar>
#include <QLabel>
#include <QBoxLayout>
#include <QToolButton>
#include <QScrollBar>

#include <QDebug>

#include <KLocalizedString>

#define MAX_WINDOW_ZOOM 3000
#define DRAG_TOLERANCE (double(10 * m_samplesPerPixel / SAMPLE_RATE_MILIS))

using namespace SubtitleComposer;

WaveformWidget::WaveformWidget(QWidget *parent)
	: QWidget(parent),
	  m_mediaFile(QString()),
	  m_streamIndex(-1),
	  m_stream(new StreamProcessor(this)),
	  m_subtitle(Q_NULLPTR),
	  m_timeStart(0.),
	  m_timeCurrent(0.),
	  m_timeEnd(MAX_WINDOW_ZOOM),
	  m_scrollBar(Q_NULLPTR),
	  m_autoScroll(true),
	  m_userScroll(false),
	  m_waveformChannels(0),
	  m_waveform(Q_NULLPTR),
	  m_waveformGraphics(new QWidget(this)),
	  m_progressWidget(new QWidget(this)),
	  m_samplesPerPixel(0),
	  m_waveformZoomed(Q_NULLPTR),
	  m_waveformZoomedSize(0),
	  m_waveformZoomedOffset(0),
	  m_visibleLinesDirty(true),
	  m_draggedLine(Q_NULLPTR),
	  m_draggedPos(DRAG_NONE),
	  m_draggedTime(0.),
	  m_widgetLayout(Q_NULLPTR)
{
	m_vertical = height() > width();

	m_waveformGraphics->setAttribute(Qt::WA_OpaquePaintEvent, true);
	m_waveformGraphics->setAttribute(Qt::WA_NoSystemBackground, true);
	m_waveformGraphics->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_waveformGraphics->setMouseTracking(true);
	m_waveformGraphics->installEventFilter(this);

	m_scrollBar = new QScrollBar(Qt::Vertical);
	m_scrollBar->setPageStep(windowSize());
	m_scrollBar->setRange(0, windowSize());
	m_scrollBar->installEventFilter(this);

	m_btnZoomOut = createToolButton(QStringLiteral(ACT_WAVEFORM_ZOOM_OUT));
	m_btnZoomIn = createToolButton(QStringLiteral(ACT_WAVEFORM_ZOOM_IN));
	m_btnAutoScroll = createToolButton(QStringLiteral(ACT_WAVEFORM_AUTOSCROLL));

	QHBoxLayout *toolbarLayout = new QHBoxLayout();
	toolbarLayout->setMargin(0);
	toolbarLayout->setSpacing(2);
	toolbarLayout->addWidget(m_btnZoomOut);
	toolbarLayout->addWidget(m_btnZoomIn);
	toolbarLayout->addSpacerItem(new QSpacerItem(2, 2, QSizePolicy::Preferred, QSizePolicy::Preferred));
	toolbarLayout->addWidget(m_btnAutoScroll);
	toolbarLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Preferred));

	m_toolbar = new QWidget(this);
	m_toolbar->setLayout(toolbarLayout);

	m_mainLayout = new QVBoxLayout(this);
	m_mainLayout->setMargin(0);
	m_mainLayout->setSpacing(5);
	m_mainLayout->addWidget(m_toolbar);

	setupScrollBar();

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
	connect(m_stream, &StreamProcessor::streamProgress, this, &WaveformWidget::onStreamProgress);
	connect(m_stream, &StreamProcessor::streamFinished, this, &WaveformWidget::onStreamFinished);
	// Using Qt::DirectConnection here makes WaveformWidget::onStreamData() to execute in GStreamer's thread
	connect(m_stream, &StreamProcessor::audioDataAvailable, this, &WaveformWidget::onStreamData, Qt::DirectConnection);

	connect(SCConfig::self(), SIGNAL(configChanged()), this, SLOT(onConfigChanged()));
	onConfigChanged();
}

void
WaveformWidget::onConfigChanged()
{
	m_fontNumber = QFont(SCConfig::wfFontFamily(), SCConfig::wfSubNumberFontSize());
	m_fontNumberHeight = QFontMetrics(m_fontNumber).height();
	m_fontText = QFont(SCConfig::wfFontFamily(), SCConfig::wfSubTextFontSize());

	m_subBorderWidth = SCConfig::wfSubBorderWidth();

	m_subNumberColor = QPen(QColor(SCConfig::wfSubNumberColor()), 0, Qt::SolidLine);
	m_subTextColor = QPen(QColor(SCConfig::wfSubTextColor()), 0, Qt::SolidLine);

	m_waveInner = QPen(QColor(SCConfig::wfInnerColor()), 0, Qt::SolidLine);
	m_waveOuter = QPen(QColor(SCConfig::wfOuterColor()), 0, Qt::SolidLine);

	m_subtitleBack = QColor(SCConfig::wfSubBackground());
	m_subtitleBorder = QColor(SCConfig::wfSubBorder());

	m_selectedBack = QColor(SCConfig::wfSelBackground());
	m_selectedBorder = QColor(SCConfig::wfSelBorder());

	m_playColor = QPen(QColor(SCConfig::wfPlayLocation()), 0, Qt::SolidLine);
	m_mouseColor = QPen(QColor(SCConfig::wfMouseLocation()), 0, Qt::DotLine);
}

void
WaveformWidget::updateActions()
{
	Application *app = Application::instance();
	const quint32 size = windowSize();

	m_btnZoomIn->setDefaultAction(app->action(ACT_WAVEFORM_ZOOM_IN));
	m_btnZoomIn->setEnabled(m_waveformZoomed && size > MAX_WINDOW_ZOOM);

	m_btnZoomOut->setDefaultAction(app->action(ACT_WAVEFORM_ZOOM_OUT));
	m_btnZoomOut->setEnabled(m_waveformZoomed && size < m_waveformChannelSize / SAMPLE_RATE_MILIS);

	QAction *action = app->action(ACT_WAVEFORM_AUTOSCROLL);
	action->setChecked(m_autoScroll);
	m_btnAutoScroll->setDefaultAction(action);
	m_btnAutoScroll->setEnabled(m_waveformZoomed);
}

WaveformWidget::~WaveformWidget()
{
	clearAudioStream();
}

quint32
WaveformWidget::windowSize() const
{
	return (m_timeEnd - m_timeStart).toMillis();
}

void
WaveformWidget::setWindowSize(const quint32 size)
{
	if(size != windowSize()) {
		updateActions();
		m_timeEnd = m_timeStart + size;
		m_visibleLinesDirty = true;
		updateZoomData();
		m_waveformGraphics->update();
		m_scrollBar->setPageStep(size);
		m_scrollBar->setRange(0, m_waveformDuration * 1000 - windowSize());
	}
}

void
WaveformWidget::zoomIn()
{
	quint32 winSize = windowSize();
	if(winSize <= MAX_WINDOW_ZOOM)
		return;
	m_scrollBar->setValue(m_timeStart.toMillis() + winSize / 4);
	setWindowSize(winSize / 2);
}

void
WaveformWidget::zoomOut()
{
	quint32 winSize = windowSize();
	quint32 totalLength = m_waveformChannelSize / SAMPLE_RATE_MILIS;
	if(winSize >= totalLength)
		return;
	m_scrollBar->setValue(m_timeStart.toMillis() - winSize / 2);
	setWindowSize(winSize * 2);
}

void
WaveformWidget::setAutoscroll(bool autoscroll)
{
	m_autoScroll = autoscroll;
	updateActions();
}

void
WaveformWidget::onScrollBarValueChanged(int value)
{
	double winSize = windowSize();
	m_timeStart = value;
	m_timeEnd = m_timeStart.shifted(winSize);

	m_visibleLinesDirty = true;
	m_waveformGraphics->update();
}

void
WaveformWidget::updateZoomData()
{
	int height = m_vertical ? m_waveformGraphics->height() : m_waveformGraphics->width();
	if(!height)
		return;

	double samplesPerPixel = double(SAMPLE_RATE_MILIS * windowSize()) / height;
	if(m_samplesPerPixel != samplesPerPixel) {
		m_samplesPerPixel = samplesPerPixel;
		m_waveformZoomedOffset = 0;
		m_waveformZoomedSize = 0;
		if(m_waveformZoomed) {
			for(quint32 i = 0; i < m_waveformChannels; i++)
				delete[] m_waveformZoomed[i];
			delete[] m_waveformZoomed;
			m_waveformZoomed = Q_NULLPTR;
		}
	}

	if(m_waveformChannels && m_waveform) {
		if(!m_waveformZoomed) {
			m_waveformZoomedOffset = 0;
			m_waveformZoomedSize = m_waveformChannelSize / m_samplesPerPixel;
			if(m_waveformChannelSize % int(m_samplesPerPixel))
				m_waveformZoomedSize++;
			m_waveformZoomed = new ZoomData *[m_waveformChannels];
			for(quint32 i = 0; i < m_waveformChannels; i++)
				m_waveformZoomed[i] = new ZoomData[m_waveformZoomedSize];

			updateActions();
		}

		int iMin = m_waveformZoomedOffset * m_samplesPerPixel;
		int iMax = m_waveformDataOffset / BYTES_PER_SAMPLE / m_waveformChannels;
		qint32 xMin = 65535, xMax = -65535;
//		qint32 xAvg = 0;
//		qreal xRMS = 0;

		for(quint32 ch = 0; ch < m_waveformChannels; ch++) {
			for(int i = iMin; i < iMax; i++) {
				qint32 val = (qint32)m_waveform[ch][i] + SIGNED_PAD;
				if(xMin > val)
					xMin = val;
				if(xMax < val)
					xMax = val;
//				xAvg += val;
//				xRMS += val * val;

				if(i % int(m_samplesPerPixel) == int(m_samplesPerPixel) - 1) {
//					xAvg /= m_samplesPerPixel;
//					xRMS = sqrt(xRMS / m_samplesPerPixel);

					int zi = i / m_samplesPerPixel;
					m_waveformZoomed[ch][zi].min = xMin;
					m_waveformZoomed[ch][zi].max = xMax;

					xMin = 65535;
					xMax = 0;
//					xAvg = 0;
//					xRMS = 0.0;
				}
			}
		}
		m_waveformZoomedOffset = iMax / m_samplesPerPixel;
	}
}

void
WaveformWidget::setSubtitle(Subtitle *subtitle)
{
	if(m_subtitle)
		disconnect(m_subtitle, &Subtitle::primaryChanged, this, &WaveformWidget::onSubtitleChanged);

	m_subtitle = subtitle;

	if(m_subtitle)
		connect(m_subtitle, &Subtitle::primaryChanged, this, &WaveformWidget::onSubtitleChanged);

	m_visibleLines.clear();
	m_visibleLinesDirty = subtitle != Q_NULLPTR;

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

	m_waveformDuration = 0;
	m_waveformDataOffset = 0;

	static WaveFormat waveFormat(8000, 0, BYTES_PER_SAMPLE * 8, true);
	if(m_stream->open(mediaFile) && m_stream->initAudio(audioStream, waveFormat))
		m_stream->start();
}

void
WaveformWidget::clearAudioStream()
{
	m_stream->close();

	m_mediaFile.clear();
	m_streamIndex = -1;

	if(m_waveformZoomed) {
		for(quint32 i = 0; i < m_waveformChannels; i++)
			delete[] m_waveformZoomed[i];
		delete[] m_waveformZoomed;
		m_waveformZoomed = Q_NULLPTR;
	}

	if(m_waveform) {
		for(quint32 i = 0; i < m_waveformChannels; i++)
			delete[] m_waveform[i];
		delete[] m_waveform;
		m_waveform = Q_NULLPTR;
	}

	m_waveformChannels = 0;
}

void
WaveformWidget::onStreamProgress(quint64 msecPos, quint64 msecLength)
{
	if(!m_waveformDuration) {
		m_waveformDuration = msecLength / 1000;
		m_progressBar->setRange(0, m_waveformDuration);
		m_progressWidget->show();
		m_scrollBar->setRange(0, m_waveformDuration * 1000 - windowSize());
	}
	m_progressBar->setValue(msecPos / 1000);
}

void
WaveformWidget::onStreamFinished()
{
	m_progressWidget->hide();
	m_stream->close();
}

void
WaveformWidget::onStreamData(const void *buffer, const qint32 size, const WaveFormat *waveFormat)
{
	// make sure WaveformWidget::onStreamProgress() signal was processed since we're in different thread
	while(!m_waveformDuration)
		QThread::yieldCurrentThread();

	if(!m_waveformChannels) {
		m_waveformChannels = waveFormat->channels();
		m_waveformChannelSize = waveFormat->sampleRate() * (m_waveformDuration + 60); // FIXME: added 60sec not to overflow below
		m_waveform = new SAMPLE_TYPE *[m_waveformChannels];
		for(quint32 i = 0; i < m_waveformChannels; i++)
			m_waveform[i] = new SAMPLE_TYPE[m_waveformChannelSize];
	}

	Q_ASSERT(m_waveformDataOffset + size < m_waveformChannelSize * BYTES_PER_SAMPLE * m_waveformChannels);
	Q_ASSERT(waveFormat->bitsPerSample() == BYTES_PER_SAMPLE * 8);
	Q_ASSERT(waveFormat->sampleRate() == 8000);
	Q_ASSERT(size % BYTES_PER_SAMPLE == 0);

	const SAMPLE_TYPE *sample = reinterpret_cast<const SAMPLE_TYPE *>(buffer);
	int len = size / BYTES_PER_SAMPLE;
	int i = m_waveformDataOffset / BYTES_PER_SAMPLE / m_waveformChannels;
	while(len > 0) {
		for(quint32 c = m_waveformDataOffset / BYTES_PER_SAMPLE % m_waveformChannels; len > 0 && c < m_waveformChannels; c++) {
			qint32 val = *sample++;
			if(i > 0) {
				// simple lowpass filter
				val = (val + m_waveform[c][i - 1]) / 2;
			}
			val &= BYTES_PER_SAMPLE == 1 ? 0x000000ff : 0x0000ffff;
			m_waveform[c][i] = val;
			len--;
		}
		i++;
	}
	m_waveformDataOffset += size;
}


void
WaveformWidget::updateVisibleLines()
{
	if(!m_subtitle || !m_visibleLinesDirty)
		return;

	m_visibleLinesDirty = false;

	m_visibleLines.clear();

	foreach(SubtitleLine *sub, m_subtitle->allLines()) {
		if(sub == m_draggedLine || (sub->showTime() <= m_timeEnd && m_timeStart <= sub->hideTime()))
			m_visibleLines.push_back(sub);
	}
}

void
WaveformWidget::paintGraphics(QPainter &painter)
{
	quint32 msWindowSize = windowSize();
	int widgetHeight = m_waveformGraphics->height();
	int widgetWidth = m_waveformGraphics->width();
	int widgetSpan = m_vertical ? widgetHeight : widgetWidth;

	updateZoomData();

	// FIXME: make visualization types configurable? Min/Max/Avg/RMS
	if(m_waveformZoomed) {
		quint32 yMin = SAMPLE_RATE_MILIS * m_timeStart.toMillis() / m_samplesPerPixel;
		quint32 yMax = SAMPLE_RATE_MILIS * m_timeEnd.toMillis() / m_samplesPerPixel;
		qint32 xMin, xMax;

		qint32 chHalfWidth = (m_vertical ? widgetWidth : widgetHeight) / m_waveformChannels / 2;

		for(quint32 ch = 0; ch < m_waveformChannels; ch++) {
			qint32 chCenter = (ch * 2 + 1) * chHalfWidth;
			for(quint32 i = yMin; i < yMax; i++) {
				if(i >= m_waveformZoomedOffset) {
					xMin = xMax = 0;
				} else {
					xMin = m_waveformZoomed[ch][i].min * 9 / 5 * chHalfWidth / SAMPLE_MAX;
					xMax = m_waveformZoomed[ch][i].max * 9 / 5 * chHalfWidth / SAMPLE_MAX;
				}

				int y = i - yMin;
				painter.setPen(m_waveOuter);
				if(m_vertical)
					painter.drawLine(chCenter - xMax, y, chCenter + xMax, y);
				else
					painter.drawLine(y, chCenter - xMax, y, chCenter + xMax);
				painter.setPen(m_waveInner);
				if(m_vertical)
					painter.drawLine(chCenter - xMin, y, chCenter + xMin, y);
				else
					painter.drawLine(y, chCenter - xMin, y, chCenter + xMin);
			}
		}
	}

	updateVisibleLines();

	const RangeList &selection = Application::instance()->linesWidget()->selectionRanges();
	foreach(const SubtitleLine *sub, m_visibleLines) {
		bool selected = selection.contains(sub->index());
		Time timeShow = sub->showTime();
		Time timeHide = sub->hideTime();
		if(sub == m_draggedLine) {
			if(m_draggedPos == DRAG_LINE) {
				timeShow = m_draggedTime - m_draggedOffset;
				timeHide = timeShow + sub->durationTime();
			} else if(m_draggedPos == DRAG_SHOW) {
				timeShow = m_draggedTime - m_draggedOffset;
			} else if(m_draggedPos == DRAG_HIDE) {
				timeHide = m_draggedTime - m_draggedOffset;
			}
		}
		if(timeShow <= m_timeEnd && m_timeStart <= timeHide) {
			int showY = widgetSpan * (timeShow.toMillis() - m_timeStart.toMillis()) / msWindowSize;
			int hideY = widgetSpan * (timeHide.toMillis() - m_timeStart.toMillis()) / msWindowSize;
			QRect box;
			if(m_vertical)
				box = QRect(0, showY + m_subBorderWidth, widgetWidth, hideY - showY - 2 * m_subBorderWidth);
			else
				box = QRect(showY + m_subBorderWidth, 0, hideY - showY - 2 * m_subBorderWidth, widgetHeight);

			painter.fillRect(box, selected ? m_selectedBack : m_subtitleBack);

			if(m_subBorderWidth) {
				if(m_vertical) {
					painter.fillRect(0, showY, widgetWidth, m_subBorderWidth, selected ? m_selectedBorder : m_subtitleBorder);
					painter.fillRect(0, hideY - m_subBorderWidth, widgetWidth, m_subBorderWidth, selected ? m_selectedBorder : m_subtitleBorder);
				} else {
					painter.fillRect(showY, 0, m_subBorderWidth, widgetHeight, selected ? m_selectedBorder : m_subtitleBorder);
					painter.fillRect(hideY - m_subBorderWidth, 0, m_subBorderWidth, widgetHeight, selected ? m_selectedBorder : m_subtitleBorder);
				}
			}

			painter.setFont(m_fontText);
			painter.setPen(m_subTextColor);
			painter.drawText(box, Qt::AlignCenter, sub->primaryText().string());

			painter.setPen(m_subNumberColor);
			painter.setFont(m_fontNumber);
			if(m_vertical)
				painter.drawText(m_fontNumberHeight / 2, showY + m_fontNumberHeight + 2, QString::number(sub->index() + 2));
			else
				painter.drawText(showY + m_fontNumberHeight / 2, m_fontNumberHeight + 2, QString::number(sub->index() + 2));
		}
	}

	int playY = widgetSpan * (m_timeCurrent - m_timeStart).toMillis() / msWindowSize;
	painter.setPen(m_playColor);
	if(m_vertical)
		painter.drawLine(0, playY, widgetWidth, playY);
	else
		painter.drawLine(playY, 0, playY, widgetHeight);

	painter.setPen(m_subTextColor);
	painter.setFont(m_fontText);
	if(m_vertical) {
		QRect textRect(6, 4, widgetWidth - 12, widgetHeight - 8);
		painter.drawText(textRect, Qt::AlignRight | Qt::AlignTop, m_timeStart.toString());
		painter.drawText(textRect, Qt::AlignRight | Qt::AlignBottom, m_timeEnd.toString());
	} else {
		QRect textRect(4, 6, widgetWidth - 8, widgetHeight - 12);
		painter.drawText(textRect, Qt::AlignLeft | Qt::AlignTop, m_timeStart.toString());
		painter.drawText(textRect, Qt::AlignRight | Qt::AlignTop, m_timeEnd.toString());
	}

	painter.setPen(m_mouseColor);
	playY = widgetSpan * (m_pointerTime - m_timeStart).toMillis() / msWindowSize;
	if(m_vertical)
		painter.drawLine(0, playY, widgetWidth, playY);
	else
		painter.drawLine(playY, 0, playY, widgetHeight);
}

void
WaveformWidget::setupScrollBar()
{
	if(m_widgetLayout)
		m_widgetLayout->deleteLater();

	if(m_vertical) {
		m_widgetLayout = new QHBoxLayout();
		m_scrollBar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
		m_scrollBar->setOrientation(Qt::Vertical);
	} else {
		m_widgetLayout = new QVBoxLayout();
		m_scrollBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
		m_scrollBar->setOrientation(Qt::Horizontal);
	}

	m_widgetLayout->setMargin(0);
	m_widgetLayout->setSpacing(0);
	m_widgetLayout->addWidget(m_waveformGraphics);
	m_widgetLayout->addWidget(m_scrollBar);

	m_mainLayout->insertLayout(0, m_widgetLayout);
}

/*virtual*/ void
WaveformWidget::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);

	bool vertical = height() > width();
	if(m_vertical != vertical) {
		m_vertical = vertical;
		setupScrollBar();
	}

	m_visibleLinesDirty = true;
	updateZoomData();

	m_waveformGraphics->update();
}

/*virtual*/ void
WaveformWidget::leaveEvent(QEvent */*event*/)
{
	m_pointerTime.setMillisTime(Time::MaxMseconds);

	if(m_userScroll) {
		m_userScroll = false;
		if(m_autoScroll)
			onPlayerPositionChanged(m_timeCurrent.toSeconds());
	} else {
		m_waveformGraphics->update();
	}
}

/*virtual*/ bool
WaveformWidget::eventFilter(QObject *obj, QEvent *event)
{
	if(obj != m_scrollBar && obj != m_waveformGraphics)
		return false;

	switch(event->type()) {
	case QEvent::Wheel: {
		QPoint delta = reinterpret_cast<QWheelEvent *>(event)->angleDelta() / 8;
		if(delta.isNull())
			delta = reinterpret_cast<QWheelEvent *>(event)->pixelDelta();
		if(delta.isNull())
			return false;

		m_userScroll = true;

		m_scrollBar->setValue(m_timeStart.shifted(-4 * double(delta.ry()) * windowSize() / m_waveformGraphics->height()).toMillis());
		return true;
	}
	case QEvent::MouseButtonPress:
		m_userScroll = true;
		break; // do not capture mouse presses

	default:
		break;
	}

	if(obj != m_waveformGraphics)
		return false;

	switch(event->type()) {
	case QEvent::Paint: {
		QPainter painter(m_waveformGraphics);
		painter.fillRect(reinterpret_cast<QPaintEvent *>(event)->rect(), Qt::black);
		paintGraphics(painter);
		return true;
	}

	case QEvent::MouseMove: {
		QMouseEvent *mouse = reinterpret_cast<QMouseEvent *>(event);
		int y = m_vertical ? mouse->y() : mouse->x();

		m_pointerTime = timeAt(y);

		if(m_draggedLine) {
			m_draggedTime = m_pointerTime;
		} else {
			SubtitleLine *sub = Q_NULLPTR;
			WaveformWidget::DragPosition res = subtitleAt(y, &sub);
			if(sub && !m_subtitle->anchoredLines().empty() && m_subtitle->anchoredLines().indexOf(sub) == -1)
				m_waveformGraphics->setCursor(QCursor(Qt::ForbiddenCursor));
			else if(res == DRAG_LINE)
				m_waveformGraphics->setCursor(QCursor(m_vertical ? Qt::SizeVerCursor : Qt::SizeHorCursor));
			else if(res == DRAG_SHOW || res == DRAG_HIDE)
				m_waveformGraphics->setCursor(QCursor(m_vertical ? Qt::SplitVCursor : Qt::SplitHCursor));
			else
				m_waveformGraphics->unsetCursor();
		}

		m_waveformGraphics->update();

		return true;
	}

	case QEvent::MouseButtonDblClick: {
		QMouseEvent *mouse = reinterpret_cast<QMouseEvent *>(event);
		emit doubleClick(timeAt(m_vertical ? mouse->y() : mouse->x()));
		return true;
	}

	case QEvent::MouseButtonPress: {
		QMouseEvent *mouse = reinterpret_cast<QMouseEvent *>(event);
		if(mouse->button() != Qt::LeftButton)
			return false;

		int y = m_vertical ? mouse->y() : mouse->x();
		m_draggedPos = subtitleAt(y, &m_draggedLine);
		if(m_draggedLine && !m_subtitle->anchoredLines().empty() && m_subtitle->anchoredLines().indexOf(m_draggedLine) == -1) {
			m_draggedTime = 0.;
			m_draggedPos = DRAG_NONE;
			m_draggedLine = Q_NULLPTR;
		} else {
			m_pointerTime = timeAt(y);
			m_draggedTime = m_pointerTime;
			if(m_draggedPos == DRAG_LINE || m_draggedPos == DRAG_SHOW)
				m_draggedOffset = m_pointerTime.toMillis() - m_draggedLine->showTime().toMillis();
			else if(m_draggedPos == DRAG_HIDE)
				m_draggedOffset = m_pointerTime.toMillis() - m_draggedLine->hideTime().toMillis();
		}

		if(m_draggedLine)
			emit dragStart(m_draggedLine, m_draggedPos);

		return true;
	}

	case QEvent::MouseButtonRelease: {
		QMouseEvent *mouse = reinterpret_cast<QMouseEvent *>(event);
		if(mouse->button() != Qt::LeftButton)
			return false;

		if(m_draggedLine) {
			m_draggedTime = timeAt(m_vertical ? mouse->y() : mouse->x());
			if(m_draggedPos == DRAG_LINE) {
				m_draggedLine->setHideTime(m_draggedTime - m_draggedOffset + m_draggedLine->durationTime());
				m_draggedLine->setShowTime(m_draggedTime - m_draggedOffset);
			} else if(m_draggedPos == DRAG_SHOW) {
				m_draggedLine->setShowTime(m_draggedTime - m_draggedOffset);
			} else if(m_draggedPos == DRAG_HIDE) {
				m_draggedLine->setHideTime(m_draggedTime - m_draggedOffset);
			}

			emit dragEnd(m_draggedLine, m_draggedPos);
		}
		m_draggedLine = Q_NULLPTR;
		m_draggedPos = DRAG_NONE;
		m_draggedTime = 0.;
		return true;
	}

	default:
		return false;
	}
}

Time
WaveformWidget::timeAt(int y)
{
	int height = m_vertical ? m_waveformGraphics->height() : m_waveformGraphics->width();
//	return m_timeStart.toMillis() + double(y) * m_samplesPerPixel / SAMPLE_RATE_MILIS;
	return m_timeStart.toMillis() + double(y * windowSize() / height);
}

WaveformWidget::DragPosition
WaveformWidget::subtitleAt(int y, SubtitleLine **result)
{
	double yTime = timeAt(y).toMillis();
	double closestDistance = DRAG_TOLERANCE, currentDistance;
	WaveformWidget::DragPosition closestDrag = DRAG_NONE;
	if(result)
		*result = Q_NULLPTR;

	updateVisibleLines();

	foreach(SubtitleLine *sub, m_visibleLines) {
		if(sub->showTime() - DRAG_TOLERANCE <= yTime && sub->hideTime() + DRAG_TOLERANCE >= yTime) {
			if(closestDistance > (currentDistance = qAbs(sub->showTime().toMillis() - yTime))) {
				closestDistance = currentDistance;
				closestDrag = DRAG_SHOW;
				if(result)
					*result = sub;
			} else if(closestDistance > (currentDistance = qAbs(sub->hideTime().toMillis() - yTime))) {
				closestDistance = currentDistance;
				closestDrag = DRAG_HIDE;
				if(result)
					*result = sub;
			} else if(closestDrag == DRAG_NONE) {
				closestDistance = DRAG_TOLERANCE;
				closestDrag = DRAG_LINE;
				if(result)
					*result = sub;
			}
		}
	}

	return closestDrag;
}

void
WaveformWidget::setScrollPosition(int milliseconds)
{
	milliseconds -= windowSize() / 2;
	if(milliseconds != m_scrollBar->value()) {
		m_scrollBar->setValue(milliseconds);

		m_visibleLinesDirty = true;
		m_waveformGraphics->update();
	}
}

void
WaveformWidget::onPlayerPositionChanged(double seconds)
{
	Time playingPosition;
	playingPosition.setSecondsTime(seconds);

	if(m_timeCurrent != playingPosition) {
		m_timeCurrent = playingPosition;

		if(m_autoScroll && !m_draggedLine && !m_userScroll) {
			int windowSize = this->windowSize(),
				windowPadding = windowSize / 8, // autoscroll when we reach padding
				windowSizePad = windowSize - 2 * windowPadding;

			if(m_timeCurrent > m_timeEnd.shifted(-windowPadding) || m_timeCurrent < m_timeStart.shifted(windowPadding))
				m_scrollBar->setValue((int(m_timeCurrent.toMillis() + 0.5) / windowSizePad) * windowSizePad);
		}

		m_visibleLinesDirty = true;
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
