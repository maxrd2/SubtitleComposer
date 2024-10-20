/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "videowidget.h"

#include "appglobal.h"
#include "videoplayer/backend/glrenderer.h"
#include "videoplayer/videoplayer.h"

#include <QApplication>
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMoveEvent>
#include <QMouseEvent>
#include <QScreen>

#include <QDebug>

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
#include <QDesktopWidget>
#endif


using namespace SubtitleComposer;

VideoWidget::VideoWidget(QWidget *parent)
	: QWidget(parent),
	  m_videoWidth(0),
	  m_videoHeight(0),
	  m_videoDAR(0)
{
	setFocusPolicy(Qt::NoFocus);
	QPalette pal(palette());
	pal.setColor(QPalette::Window, Qt::black);
	setAutoFillBackground(true);
	setPalette(pal);
}

VideoWidget::~VideoWidget()
{}

int
VideoWidget::videoWidth() const
{
	return m_videoWidth;
}

int
VideoWidget::videoHeight() const
{
	return m_videoHeight;
}

double
VideoWidget::videoDAR() const
{
	return m_videoDAR;
}

void
VideoWidget::setVideoResolution(int width, int height, double dar)
{
	if(width <= 0 || height <= 0) {
		qWarning() << "invalid video width or height reported";
		return;
	}

	m_videoWidth = width;
	m_videoHeight = height;
	m_videoDAR = dar > 0.0 ? dar : (double)width / height;

	updateVideoLayerGeometry();
}

void
VideoWidget::setMouseTracking(bool enable)
{
	QWidget::setMouseTracking(enable);
	videoPlayer()->renderer()->setMouseTracking(enable);
}

void
VideoWidget::resizeEvent(QResizeEvent *)
{
	updateVideoLayerGeometry();
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#define globalPosition globalPos
#endif

void
VideoWidget::mouseReleaseEvent(QMouseEvent *e)
{
	// FIXME: ignore doubleclicks
	Q_ASSERT(e->type() != QEvent::MouseButtonDblClick);
	if(e->button() == Qt::LeftButton) {
		e->accept();
		emit leftClicked(e->globalPosition());
	} else if(e->button() == Qt::RightButton) {
		e->accept();
		emit rightClicked(e->globalPosition());
	} else {
		e->ignore();
	}
}

void
VideoWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
	e->accept();
	emit doubleClicked(e->globalPosition());
}

void
VideoWidget::wheelEvent(QWheelEvent *e)
{
	e->accept();
	if(e->angleDelta().y() >= 0)
		emit wheelUp();
	else
		emit wheelDown();
}

QSize
VideoWidget::desktopSize()
{
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
	QRect rect = QApplication::desktop()->screenGeometry();
#else
	QRect rect = screen()->geometry();
#endif
	return QSize(rect.width(), rect.height());
}

double
VideoWidget::desktopAspectRatio()
{
	QSize size = desktopSize();
	return (double)size.width() / size.height();
}

#include "appglobal.h"
#include "videoplayer/videoplayer.h"
#include "videoplayer/backend/glrenderer.h"

void
VideoWidget::updateVideoLayerGeometry()
{
	QSize frameSize = size();

	int videoLayerX = 0;
	int videoLayerY = 0;
	int videoLayerWidth = frameSize.width();
	int videoLayerHeight = frameSize.height();

	if(m_videoDAR > 0.0) {
		if(m_videoDAR >= ((double)videoLayerWidth / videoLayerHeight)) {
			videoLayerHeight = (int)(videoLayerWidth / m_videoDAR);
			videoLayerY = (frameSize.height() - videoLayerHeight) / 2;
		} else {
			videoLayerWidth = (int)(videoLayerHeight * m_videoDAR);
			videoLayerX = (frameSize.width() - videoLayerWidth) / 2;
		}
	}

	videoPlayer()->renderer()->setGeometry(videoLayerX, videoLayerY, videoLayerWidth, videoLayerHeight);

	videoPlayer()->renderer()->update();

	update();
}

QSize
VideoWidget::sizeHint() const
{
	return QSize(m_videoWidth, m_videoHeight);
}

QSize
VideoWidget::minimumSizeHint() const
{
	return QSize(0, 0);
}


