/*
 * SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2010-2020 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef PLAYERVIDEOWIDGET_H
#define PLAYERVIDEOWIDGET_H

#include <QSize>
#include <QWidget>

QT_FORWARD_DECLARE_CLASS(QResizeEvent)
QT_FORWARD_DECLARE_CLASS(QPaintEvent)
QT_FORWARD_DECLARE_CLASS(QMouseEvent)
QT_FORWARD_DECLARE_CLASS(QWheelEvent)
QT_FORWARD_DECLARE_CLASS(QPoint)

namespace SubtitleComposer {
class VideoWidget : public QWidget
{
	Q_OBJECT

public:
	explicit VideoWidget(QWidget *parent);
	virtual ~VideoWidget();

	QWidget * videoLayer() const;
	void setVideoLayer(QWidget *videoLayer);

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

	QSize sizeHint() const override;
	QSize minimumSizeHint() const override;

	double desktopAspectRatio();
	QSize desktopSize();

	void updateVideoLayerGeometry();

public slots:
	void setMouseTracking(bool enable);

signals:
	void doubleClicked(const QPoint &point);
	void rightClicked(const QPoint &point);
	void leftClicked(const QPoint &point);
	void wheelUp();
	void wheelDown();

protected:
	void init(bool setVideoLayerAttributes);

	void resizeEvent(QResizeEvent *e) override;

	void mouseReleaseEvent(QMouseEvent *e) override;
	void mouseDoubleClickEvent(QMouseEvent *e) override;
	void wheelEvent(QWheelEvent *e) override;

protected:
	QWidget *m_videoLayer;

	int m_videoWidth;
	int m_videoHeight;
	double m_videoDAR;
};
}
#endif
