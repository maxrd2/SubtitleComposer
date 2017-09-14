#ifndef VOBSUBINPUTPROCESSDIALOG_H
#define VOBSUBINPUTPROCESSDIALOG_H

/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
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

#include "core/subtitle.h"

#include <QDialog>
#include <QHash>
#include <QExplicitlySharedDataPointer>

namespace Ui {
class VobSubInputProcessDialog;
}

namespace SubtitleComposer {
class VobSubInputProcessDialog : public QDialog
{
	Q_OBJECT

public:
	class Frame;
	typedef QExplicitlySharedDataPointer<Frame> FramePtr;
	class Piece;
	typedef QExplicitlySharedDataPointer<Piece> PiecePtr;
	class Line;
	typedef QExplicitlySharedDataPointer<Line> LinePtr;

	VobSubInputProcessDialog(Subtitle *subtitle, void *vob, void *spu, QWidget *parent = 0);
	~VobSubInputProcessDialog();

	bool symFileOpen(const QString &filename);
	bool symFileSave(const QString &filename);

	virtual bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
	void onOkClicked();
	void onAbortClicked();
	void onPrevImageClicked();
	void onNextImageClicked();
	void onPrevSymbolClicked();
	void onNextSymbolClicked();
	void onSymbolCountChanged(int symbolCount);

private:
	friend class VobSubInputFormat;
	Ui::VobSubInputProcessDialog *ui;

	Q_INVOKABLE void processFrames(void *vob, void *spu);
	Q_INVOKABLE void processNextFrame(void *vob, void *spu, quint32 lastStartPts);
	Q_INVOKABLE void processNextImage();
	void processCurrentPiece();
	void updateCurrentPiece();
	void processNextPiece();
	void recognizePiece();

	SString currentText();
	PiecePtr currentNormalizedPiece(int symbolCount);
	void currentSymbolCountSet(int symbolCount);

	QList<FramePtr> m_frames;
	QList<FramePtr>::iterator m_frameCurrent;

	Subtitle *m_subtitle;

	qint32 m_spaceWidth;

	QList<PiecePtr> m_pieces;
	QList<PiecePtr>::iterator m_pieceCurrent;

	QHash<Piece, SString> m_recognizedPieces;
	qint32 m_recognizedPiecesMaxSymbolLength;
};
}

#endif // VOBSUBINPUTPROCESSDIALOG_H
