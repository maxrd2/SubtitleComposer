#ifndef PLAYERVIDEOWIDGET_H
#define PLAYERVIDEOWIDGET_H

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

#include <QtCore/QPoint>
#include <QtCore/QSize>
#include <QtGui/QWidget>

class QResizeEvent;
class QPaintEvent;
class QMouseEvent;
class QWheelEvent;

namespace SubtitleComposer {
class VideoWidget : public QWidget
{
	Q_OBJECT

public:
	explicit VideoWidget(QWidget *parent);
	VideoWidget(QWidget *videoLayer, QWidget *parent);
	virtual ~VideoWidget();

	QWidget * videoLayer();

	int videoWidth() const;
	int videoHeight() const;
	double videoDAR() const;

	/**
	 * @brief setVideoResolution
	 * @param width
	 * @param height
	 * @param dar if <= 0, it's ignored (the dar becomes width/height)
	 */
	void setVideoResolution(int width, int height, double dar = 0.0);

	virtual QSize sizeHint() const;
	virtual QSize minimumSizeHint() const;

	double desktopAspectRatio();
	QSize desktopSize();

	void updateVideoLayerGeometry();

public slots:
	virtual void setMouseTracking(bool enable);

signals:
	void doubleClicked(const QPoint &point);
	void rightClicked(const QPoint &point);
	void leftClicked(const QPoint &point);
	void wheelUp();
	void wheelDown();

protected:
	void init(bool setVideoLayerAttributes);

	virtual void resizeEvent(QResizeEvent *e);

	virtual void mouseReleaseEvent(QMouseEvent *e);
	virtual void mouseDoubleClickEvent(QMouseEvent *e);
	virtual void wheelEvent(QWheelEvent *e);

protected:
	QWidget *m_videoLayer;

	int m_videoWidth;
	int m_videoHeight;
	double m_videoDAR;
};
}
#endif
