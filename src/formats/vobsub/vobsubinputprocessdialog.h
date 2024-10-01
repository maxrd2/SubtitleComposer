/*
    SPDX-FileCopyrightText: 2017-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef VOBSUBINPUTPROCESSDIALOG_H
#define VOBSUBINPUTPROCESSDIALOG_H

#include "core/subtitle.h"

#include "streamprocessor/streamprocessor.h"

#include <QDialog>
#include <QExplicitlySharedDataPointer>
#include <QHash>

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

	VobSubInputProcessDialog(Subtitle *subtitle, qint32 spaceThreshold = 0, QWidget *parent = 0);
	~VobSubInputProcessDialog();

	bool symFileOpen(const QString &filename);
	bool symFileSave(const QString &filename);

	bool eventFilter(QObject *obj, QEvent *event) override;

	void processFrames(StreamProcessor *streamProcessor);

private slots:
	void onOkClicked();
	void onAbortClicked();
	void onPrevImageClicked();
	void onNextImageClicked();
	void onPrevSymbolClicked();
	void onNextSymbolClicked();
	void onSymbolCountChanged(int symbolCount);

	void onStreamData(const QImage &image, quint64 msecStart, quint64 msecDuration);
	void onStreamError(int code, const QString &message, const QString &debug);
	void onStreamFinished();

private:
	friend class VobSubInputFormat;
	Ui::VobSubInputProcessDialog *ui;

	Q_INVOKABLE void processNextImage();
	void processCurrentPiece();
	void updateCurrentPiece();
	void processNextPiece();
	void recognizePiece();

	RichString currentText();
	PiecePtr currentNormalizedPiece(int symbolCount);
	void currentSymbolCountSet(int symbolCount);

	QList<FramePtr> m_frames;
	QList<FramePtr>::iterator m_frameCurrent;

	QExplicitlySharedDataPointer<Subtitle> m_subtitle;

	qint32 m_spaceWidth;
	qint32 m_spaceThreshold;

	QList<PiecePtr> m_pieces;
	QList<PiecePtr>::iterator m_pieceCurrent;

	QHash<Piece, RichString> m_recognizedPieces;
	qint32 m_recognizedPiecesMaxSymbolLength;
};
}

#endif // VOBSUBINPUTPROCESSDIALOG_H
