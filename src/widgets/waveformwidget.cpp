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

#include <QDebug>

#include <KLocalizedString>

#define MAX_WINDOW_ZOOM 6000
#define DRAG_TOLERANCE (double(10 * m_samplesPerPixel / SAMPLE_RATE_MILIS))

using namespace SubtitleComposer;

WaveformWidget::WaveformWidget(QWidget *parent)
	: QWidget(parent),
	  m_mediaFile(QString()),
	  m_streamIndex(-1),
	  m_stream(new StreamProcessor()),
	  m_subtitle(NULL),
	  m_timeStart(0.),
	  m_timeCurrent(0.),
	  m_timeEnd(MAX_WINDOW_ZOOM),
	  m_waveformChannels(0),
	  m_waveform(NULL),
	  m_waveformGraphics(new QWidget(this)),
	  m_progressWidget(new QWidget(this)),
	  m_samplesPerPixel(0),
	  m_waveformZoomed(NULL),
	  m_waveformZoomedSize(0),
	  m_waveformZoomedOffset(0),
	  m_draggedLine(NULL),
	  m_draggedPos(DRAG_NONE),
	  m_draggedTime(0.)
{
	m_waveformGraphics->setAttribute(Qt::WA_OpaquePaintEvent, true);
	m_waveformGraphics->setAttribute(Qt::WA_NoSystemBackground, true);
	m_waveformGraphics->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_waveformGraphics->setMouseTracking(true);
	m_waveformGraphics->installEventFilter(this);
	m_waveformGraphics->show();

	QHBoxLayout *toolbarLayout = new QHBoxLayout();
	toolbarLayout->setMargin(0);
	toolbarLayout->setSpacing(2);
	toolbarLayout->addWidget(createToolButton(QStringLiteral(ACT_WAVEFORM_ZOOM_IN)));
	toolbarLayout->addWidget(createToolButton(QStringLiteral(ACT_WAVEFORM_ZOOM_OUT)));
	toolbarLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Preferred));

	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setMargin(0);
	mainLayout->setSpacing(5);
	mainLayout->addWidget(m_waveformGraphics);
	mainLayout->addLayout(toolbarLayout);

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

	connect(VideoPlayer::instance(), &VideoPlayer::positionChanged, this, &WaveformWidget::onPlayerPositionChanged);
	connect(m_stream, &StreamProcessor::streamProgress, this, &WaveformWidget::onStreamProgress);
	connect(m_stream, &StreamProcessor::streamFinished, this, &WaveformWidget::onStreamFinished);
	// Using Qt::DirectConnection here makes WaveformWidget::onStreamData() to execute in GStreamer's thread
	connect(m_stream, &StreamProcessor::audioDataAvailable, this, &WaveformWidget::onStreamData, Qt::DirectConnection);
}

void
WaveformWidget::initActions()
{
	Application *app = Application::instance();
	const quint32 size = windowSize();
	QToolButton *btn;

	btn = findChild<QToolButton *>(QStringLiteral(ACT_WAVEFORM_ZOOM_IN));
	btn->setDefaultAction(app->action(ACT_WAVEFORM_ZOOM_IN));
	btn->setEnabled(m_waveformZoomed && size > MAX_WINDOW_ZOOM);

	btn = findChild<QToolButton *>(QStringLiteral(ACT_WAVEFORM_ZOOM_OUT));
	btn->setDefaultAction(app->action(ACT_WAVEFORM_ZOOM_OUT));
	btn->setEnabled(m_waveformZoomed && size < m_waveformChannelSize / SAMPLE_RATE_MILIS);
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
		initActions();
		m_timeEnd = m_timeStart + size;
		updateZoomData();
		m_waveformGraphics->update();
	}
}

void
WaveformWidget::zoomIn()
{
	quint32 winSize = windowSize();
	if(winSize <= MAX_WINDOW_ZOOM)
		return;
	setWindowSize(winSize / 2);
}

void
WaveformWidget::zoomOut()
{
	quint32 winSize = windowSize();
	quint32 totalLength = m_waveformChannelSize / SAMPLE_RATE_MILIS;
	if(winSize >= totalLength)
		return;
	setWindowSize(winSize * 2);
}

void
WaveformWidget::updateZoomData()
{
	if(!m_waveformGraphics->height())
		return;

	double samplesPerPixel = double(SAMPLE_RATE_MILIS * windowSize()) / m_waveformGraphics->height();
	if(m_samplesPerPixel != samplesPerPixel) {
		m_samplesPerPixel = samplesPerPixel;
		m_waveformZoomedOffset = 0;
		m_waveformZoomedSize = 0;
		if(m_waveformZoomed) {
			for(quint32 i = 0; i < m_waveformChannels; i++)
				delete[] m_waveformZoomed[i];
			delete[] m_waveformZoomed;
			m_waveformZoomed = NULL;
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

			initActions();
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
	m_subtitle = subtitle;
}

QWidget *
WaveformWidget::progressWidget()
{
	return m_progressWidget;
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
		m_waveformZoomed = NULL;
	}

	if(m_waveform) {
		for(quint32 i = 0; i < m_waveformChannels; i++)
			delete[] m_waveform[i];
		delete[] m_waveform;
		m_waveform = NULL;
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
WaveformWidget::paintGraphics(QPainter &painter)
{
	const static QFont fontBig("helvetica", 10);
	const static int fontBigHeight = QFontMetrics(fontBig).height();
	const static QFont fontSubText("helvetica", 9);

	const static QColor subtitleBg(0, 0, 150, 100);
	const static QPen subtitleBorder(QColor(0, 0, 255, 150), 0, Qt::SolidLine);
	const static QPen textWhite(Qt::white, 0, Qt::SolidLine);
	const static QPen mousePointer(QColor(255, 255, 255, 80), 0, Qt::DotLine);
	const static QPen textShadow(QColor(0, 0, 0, 192), 0, Qt::SolidLine);
	const static QPen waveDark(QColor(100, 100, 100, 255), 0, Qt::SolidLine);
	const static QPen waveLight(QColor(150, 150, 150, 255), 0, Qt::SolidLine);

	quint32 msWindowSize = windowSize();
	int widgetHeight = m_waveformGraphics->height();
	int widgetWidth = m_waveformGraphics->width();

	updateZoomData();

	// FIXME: make visualization types configurable? Min/Max/Avg/RMS
	if(m_waveformZoomed) {
		quint32 yMin = SAMPLE_RATE_MILIS * m_timeStart.toMillis() / m_samplesPerPixel;
		quint32 yMax = SAMPLE_RATE_MILIS * m_timeEnd.toMillis() / m_samplesPerPixel;
		qint32 xMin, xMax;

		qint32 chHalfWidth = widgetWidth / m_waveformChannels / 2;

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
				painter.setPen(waveDark);
				painter.drawLine(chCenter - xMax, y, chCenter + xMax, y);
				painter.setPen(waveLight);
				painter.drawLine(chCenter - xMin, y, chCenter + xMin, y);
			}
		}
	}

	m_visibleLines.clear();
	if(m_subtitle) {
		for(int i = 0, n = m_subtitle->linesCount(); i < n; i++) {
			SubtitleLine *sub = m_subtitle->line(i);
			const Time timeShow = sub == m_draggedLine && m_draggedPos == DRAG_SHOW ? m_draggedTime : sub->showTime();
			const Time timeHide = sub == m_draggedLine && m_draggedPos == DRAG_HIDE ? m_draggedTime : sub->hideTime();
			if(timeShow <= m_timeEnd && m_timeStart <= timeHide) {
				m_visibleLines.push_back(sub);

				int showY = widgetHeight * (timeShow.toMillis() - m_timeStart.toMillis()) / msWindowSize;
				int hideY = widgetHeight * (timeHide.toMillis() - m_timeStart.toMillis()) / msWindowSize;
				QRect box(0, showY, widgetWidth, hideY - showY + 1);

				painter.fillRect(box, subtitleBg);

				painter.setPen(subtitleBorder);
				painter.drawLine(0, showY, widgetWidth, showY);
				painter.drawLine(0, hideY, widgetWidth, hideY);

				painter.setFont(fontSubText);
				painter.setPen(textShadow);
				box.adjust(fontBigHeight + 1, fontBigHeight / 2 + 1, -fontBigHeight, -fontBigHeight / 2);
				painter.drawText(box, Qt::AlignCenter, sub->primaryText().string());
				painter.setPen(textWhite);
				box.adjust(-1, -1, 0, 0);
				painter.drawText(box, Qt::AlignCenter, sub->primaryText().string());

				painter.setPen(textWhite);
				painter.setFont(fontBig);
				painter.drawText(fontBigHeight / 2, showY + fontBigHeight * 3 / 2, QString::number(i));
			}
		}
	}

	int playY = widgetHeight * (m_timeCurrent - m_timeStart).toMillis() / msWindowSize;
	painter.setPen(textWhite);
	painter.drawLine(0, playY, widgetWidth, playY);

	QRect textRect(6, 4, m_waveformGraphics->width() - 12, m_waveformGraphics->height() - 8);
	painter.setPen(textWhite);
	painter.setFont(fontSubText);
	painter.drawText(textRect, Qt::AlignRight | Qt::AlignTop, m_timeStart.toString());
	painter.drawText(textRect, Qt::AlignRight | Qt::AlignBottom, m_timeEnd.toString());

	painter.setPen(mousePointer);
	playY = widgetHeight * (m_pointerTime - m_timeStart).toMillis() / msWindowSize;
	painter.drawLine(0, playY, widgetWidth, playY);
}

void
WaveformWidget::resizeEvent(QResizeEvent *e)
{
	QWidget::resizeEvent(e);

	updateZoomData();

	m_waveformGraphics->update();
}

bool
WaveformWidget::eventFilter(QObject *obj, QEvent *ev)
{
	if(obj != m_waveformGraphics)
		return false;

	switch(ev->type()) {
	case QEvent::Paint: {
		QPainter painter(m_waveformGraphics);
		painter.fillRect(reinterpret_cast<QPaintEvent *>(ev)->rect(), Qt::black);
		paintGraphics(painter);
		return true;
	}

	case QEvent::MouseMove: {
		int y = reinterpret_cast<QMouseEvent *>(ev)->y();

		m_pointerTime = timeAt(y);

		if(m_draggedLine) {
			m_draggedTime = m_pointerTime;
		} else {
			WaveformWidget::DragPosition res = subtitleAt(y, NULL);
			if(res == DRAG_SHOW || res == DRAG_HIDE)
				m_waveformGraphics->setCursor(QCursor(Qt::SplitVCursor));
			else
				m_waveformGraphics->unsetCursor();
		}

		m_waveformGraphics->update();

		return true;
	}

	case QEvent::MouseButtonPress: {
		int y = reinterpret_cast<QMouseEvent *>(ev)->y();
		m_draggedPos = subtitleAt(y, &m_draggedLine);
		if(m_draggedPos == DRAG_SHOW) {
			m_draggedTime = m_draggedLine->showTime();
		} else if(m_draggedPos == DRAG_HIDE) {
			m_draggedTime = m_draggedLine->hideTime();
		} else if(m_draggedPos == DRAG_LINE) {
			m_draggedTime = 0.;
			m_draggedPos = DRAG_NONE;
			m_draggedLine = NULL;
		}
		return true;
	}

	case QEvent::MouseButtonRelease: {
		if(m_draggedLine) {
			m_draggedTime = timeAt(reinterpret_cast<QMouseEvent *>(ev)->y());
			if(m_draggedPos == DRAG_SHOW)
				m_draggedLine->setShowTime(m_draggedTime);
			else if(m_draggedPos == DRAG_HIDE)
				m_draggedLine->setHideTime(m_draggedTime);
		}
		m_draggedLine = NULL;
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
//	return m_timeStart.toMillis() + double(y) * m_samplesPerPixel / SAMPLE_RATE_MILIS;
	return m_timeStart.toMillis() + double(y * windowSize() / m_waveformGraphics->height());
}

WaveformWidget::DragPosition
WaveformWidget::subtitleAt(int y, SubtitleLine **result)
{
	double yTime = timeAt(y).toMillis();
	double closestDistance = DRAG_TOLERANCE, currentDistance;
	WaveformWidget::DragPosition closestDrag = DRAG_NONE;
	if(result)
		*result = NULL;

	foreach(auto sub, m_visibleLines) {
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
WaveformWidget::onPlayerPositionChanged(double seconds)
{
	Time playingPosition;
	playingPosition.setSecondsTime(seconds);

	if(m_timeCurrent != playingPosition) {
		m_timeCurrent = playingPosition;

		if(!m_draggedLine) {
			int windowSize = this->windowSize() + 0.5,
				windowPadding = windowSize / 8, // scroll when we reach padding
				windowSizePad = windowSize - 2 * windowPadding;

			if(m_timeCurrent > m_timeEnd.shifted(-windowPadding) || m_timeCurrent < m_timeStart.shifted(windowPadding)) {
				m_timeStart.setMillisTime((int(m_timeCurrent.toMillis() + 0.5) / windowSizePad) * windowSizePad);
				m_timeEnd = m_timeStart.shifted(windowSize);
			}
		}

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
