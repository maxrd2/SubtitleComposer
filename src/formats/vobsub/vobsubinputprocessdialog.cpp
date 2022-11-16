/*
    SPDX-FileCopyrightText: 2017-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "vobsubinputprocessdialog.h"
#include "ui_vobsubinputprocessdialog.h"

#include "core/richtext/richdocument.h"

#include <functional>

#include <QDebug>
#include <QPainter>
#include <QKeyEvent>

#include <KMessageBox>

#include <QStringBuilder>
#include <QDataStream>
#include <QFile>
#include <QSaveFile>
#include <QStringView>

using namespace SubtitleComposer;

// Private helper classes
class VobSubInputProcessDialog::Frame : public QSharedData
{
public:
	Frame() {}
	Frame(const Frame &other) : QSharedData(other) {}
	~Frame() {}

	bool processPieces();

	static QMap<qint32, qint32> spaceStats;

	quint32 index;
	QImage subImage;
	Time subShowTime;
	Time subHideTime;
	QList<PiecePtr> pieces;
};

class VobSubInputProcessDialog::Piece : public QSharedData
{
public:
	Piece()
		: line(nullptr),
		  top(0),
		  left(0),
		  bottom(0),
		  right(0),
		  symbolCount(1) { }
	Piece(int x, int y)
		: line(nullptr),
		  top(y),
		  left(x),
		  bottom(y),
		  right(x),
		  symbolCount(1) { }
	Piece(const Piece &other)
		: QSharedData(other),
		  line(other.line),
		  top(other.top),
		  left(other.left),
		  bottom(other.bottom),
		  right(other.right),
		  symbolCount(other.symbolCount),
		  pixels(other.pixels) { }
	~Piece() { }

	inline int height() {
		return bottom - top + 1;
	}

	inline bool operator<(const Piece &other) const;
	inline bool operator==(const Piece &other) const;
	inline Piece & operator+=(const Piece &other);


	inline void normalize();

	LinePtr line;
	qint32 top, left, bottom, right;
	qint32 symbolCount;
	RichString text;
	QVector<QPoint> pixels;
};

class VobSubInputProcessDialog::Line : public QSharedData {
public:
	Line(int top, int bottom)
		: top(top),
		  bottom(bottom) { }
	Line(const Line &other)
		: QSharedData(other),
		  top(other.top),
		  bottom(other.bottom) { }

	inline int height() { return bottom - top; }

	inline bool contains(PiecePtr piece) { return top <= piece->bottom && piece->top <= bottom; }
	inline bool intersects(LinePtr line) { return top <= line->bottom && line->top <= bottom; }
	inline void extend(int top, int bottom) {
		if(top < this->top)
			this->top = top;
		if(bottom > this->bottom)
			this->bottom = bottom;
	}

	qint32 top, bottom;
	qint16 baseline;
};

QMap<qint32, qint32> VobSubInputProcessDialog::Frame::spaceStats;

bool
VobSubInputProcessDialog::Frame::processPieces()
{
	QImage pieceBitmap = subImage;
	const int width = pieceBitmap.width();
	const int height = pieceBitmap.height();
	PiecePtr piece;

	QVector<int> ignoredColors = {pieceBitmap.pixelIndex(0, 0)};
	int maxAlpha = 0;
	for(int i = 0; i < pieceBitmap.colorCount(); i++) {
		const int alpha = qAlpha(pieceBitmap.color(i));
		if(maxAlpha < alpha)
			maxAlpha = alpha;
	}
	for(int i = 0; i < pieceBitmap.colorCount(); i++) {
		if(i == ignoredColors.at(0))
			continue;
		const QRgb color = pieceBitmap.color(i);
		if(qAlpha(color) < maxAlpha || qGray(color) <= 127)
			ignoredColors.append(i);
	}

	pieces.clear();

	// build piece by searching non-diagonal adjacent pixels, assigned pixels are
	// removed from pieceBitmap
	std::function<void(int,int)> cutPiece = [&](int x, int y){
		if(piece->top > y)
			piece->top = y;
		if(piece->bottom < y)
			piece->bottom = y;

		if(piece->left > x)
			piece->left = x;
		if(piece->right < x)
			piece->right = x;

		piece->pixels.append(QPoint(x, y));
		pieceBitmap.setPixel(x, y, ignoredColors.at(0));

		if(x < width - 1 && !ignoredColors.contains(pieceBitmap.pixelIndex(x + 1, y)))
			cutPiece(x + 1, y);
		if(x > 0 && !ignoredColors.contains(pieceBitmap.pixelIndex(x - 1, y)))
			cutPiece(x - 1, y);
		if(y < height - 1 && !ignoredColors.contains(pieceBitmap.pixelIndex(x, y + 1)))
			cutPiece(x, y + 1);
		if(y > 0 && !ignoredColors.contains(pieceBitmap.pixelIndex(x, y - 1)))
			cutPiece(x, y - 1);
	};

	// search pieces from top left
	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
			if(!ignoredColors.contains(pieceBitmap.pixelIndex(x, y))) {
				piece = new Piece(x, y);
				cutPiece(x, y);
				pieces.append(piece);
			}
		}
	}

	if(pieces.empty())
		return false;

	// figure out where the lines are
	int maxLineHeight = 0;
	QVector<LinePtr> lines;
	foreach(piece, pieces) {
		foreach(LinePtr line, lines) {
			if(line->contains(piece)) {
				piece->line = line;
				line->extend(piece->top, piece->bottom);
				break;
			}
		}
		if(!piece->line) {
			piece->line = new Line(piece->top, piece->bottom);
			lines.append(piece->line);
		}
		if(maxLineHeight < piece->line->height())
			maxLineHeight = piece->line->height();
	}

	// fix accents of characters going into their own line, merge short lines
	// that are close to next line with next line
	LinePtr lastLine;
	foreach(LinePtr line, lines) {
		if(lastLine && line->top - lastLine->bottom < maxLineHeight / 3 && lastLine->height() < maxLineHeight / 3) {
			foreach(piece, pieces) {
				if(piece->line == lastLine)
					piece->line = line;
			}
		}
		lastLine = line;
	}

	// find out where the symbol baseline is, using most frequent bottom coordinate,
	// otherwise comma and apostrophe could be recognized as same character
	QHash<LinePtr, QHash<qint16, qint16>> bottomCount;
	foreach(piece, pieces)
		bottomCount[piece->line][piece->bottom]++;
	foreach(LinePtr line, lines) {
		qint16 max = 0;
		for(auto i = bottomCount[line].cbegin(); i != bottomCount[line].cend(); ++i) {
			if(i.value() > max) {
				max = i.value();
				line->baseline = i.key();
			}
		}
	}
	foreach(piece, pieces) {
		if(piece->bottom < piece->line->baseline)
			piece->bottom = piece->line->baseline;
	}

	// sort pieces, line by line, left to right, comparison is done in Piece::operator<()
	std::sort(pieces.begin(), pieces.end(), [](const PiecePtr &a, const PiecePtr &b)->bool{
		return *a < *b;
	});

	PiecePtr prevPiece;
	foreach(piece, pieces) {
		if(prevPiece && prevPiece->line == piece->line)
			spaceStats[piece->left - prevPiece->right]++;
		prevPiece = piece;
	}

	return true;
}

inline bool
VobSubInputProcessDialog::Piece::operator<(const Piece &other) const
{
	if(line->top < other.line->top)
		return true;
	if(line->intersects(other.line) && left < other.left)
		return true;
	return false;
}

inline bool
VobSubInputProcessDialog::Piece::operator==(const Piece &other) const
{
	if(bottom - top != other.bottom - other.top)
		return false;
	if(right - left != other.right - other.left)
		return false;
	if(symbolCount != other.symbolCount)
		return false;
	// we assume pixel vectors contain QPoint elements ordered exactly the same
	return pixels == other.pixels;
}

inline VobSubInputProcessDialog::Piece &
VobSubInputProcessDialog::Piece::operator+=(const Piece &other)
{
	if(top > other.top)
		top = other.top;
	if(bottom < other.bottom)
		bottom = other.bottom;

	if(left > other.left)
		left = other.left;
	if(right < other.right)
		right = other.right;

	pixels.append(other.pixels);

	return *this;
}

// write to QDataStream
inline QDataStream &
operator<<(QDataStream &stream, const SubtitleComposer::VobSubInputProcessDialog::Line &line) {
	stream << line.top << line.bottom;
	return stream;
}

inline QDataStream &
operator<<(QDataStream &stream, const SubtitleComposer::VobSubInputProcessDialog::Piece &piece) {
	stream << *piece.line;
	stream << piece.top << piece.left << piece.bottom << piece.right;
	stream << piece.symbolCount;
	stream << piece.text;
	stream << piece.pixels;
	return stream;
}

// read from QDataStream
inline QDataStream &
operator>>(QDataStream &stream, SubtitleComposer::VobSubInputProcessDialog::Line &line) {
	stream >> line.top >> line.bottom;
	return stream;
}

inline QDataStream &
operator>>(QDataStream &stream, SubtitleComposer::VobSubInputProcessDialog::Piece &piece) {
	piece.line = new VobSubInputProcessDialog::Line(0, 0);
	stream >> *piece.line;
	stream >> piece.top >> piece.left >> piece.bottom >> piece.right;
	stream >> piece.symbolCount;
	stream >> piece.text;
	stream >> piece.pixels;
	return stream;
}

inline void
VobSubInputProcessDialog::Piece::normalize()
{
	if(top == 0 && left == 0)
		return;

	for(auto i = pixels.begin(); i != pixels.end(); ++i) {
		i->rx() -= left;
		i->ry() -= top;
	}

	right -= left;
	bottom -= top;
	top = left = 0;
}

inline uint
qHash(const VobSubInputProcessDialog::Piece &piece)
{
	// ignore top and left since this is used on normalized pieces
	return 1000 * piece.right * piece.bottom + piece.pixels.length();
}




// VobSubInputProcessDialog
VobSubInputProcessDialog::VobSubInputProcessDialog(Subtitle *subtitle, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::VobSubInputProcessDialog),
	m_subtitle(subtitle),
	m_recognizedPiecesMaxSymbolLength(0)
{
	ui->setupUi(this);

	connect(ui->btnOk, &QPushButton::clicked, this, &VobSubInputProcessDialog::onOkClicked);
	connect(ui->btnAbort, &QPushButton::clicked, this, &VobSubInputProcessDialog::onAbortClicked);

	connect(ui->styleBold, &QPushButton::toggled, this, [this](bool checked){
		QFont font = ui->lineEdit->font();
		font.setBold(checked);
		ui->lineEdit->setFont(font);
	});
	connect(ui->styleItalic, &QPushButton::toggled, this, [this](bool checked){
		QFont font = ui->lineEdit->font();
		font.setItalic(checked);
		ui->lineEdit->setFont(font);
	});
	connect(ui->styleUnderline, &QPushButton::toggled, this, [this](bool checked){
		QFont font = ui->lineEdit->font();
		font.setUnderline(checked);
		ui->lineEdit->setFont(font);
	});

	connect(ui->symbolCount, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &VobSubInputProcessDialog::onSymbolCountChanged);

	connect(ui->btnPrevSymbol, &QPushButton::clicked, this, &VobSubInputProcessDialog::onPrevSymbolClicked);
	connect(ui->btnNextSymbol, &QPushButton::clicked, this, &VobSubInputProcessDialog::onNextSymbolClicked);
	connect(ui->btnPrevImage, &QPushButton::clicked, this, &VobSubInputProcessDialog::onPrevImageClicked);
	connect(ui->btnNextImage, &QPushButton::clicked, this, &VobSubInputProcessDialog::onNextImageClicked);

	ui->lineEdit->installEventFilter(this);
	ui->lineEdit->setFocus();
}

VobSubInputProcessDialog::~VobSubInputProcessDialog()
{
	delete ui;
}

bool
VobSubInputProcessDialog::symFileOpen(const QString &filename)
{
	QFile file(filename);
	if(!file.open(QIODevice::ReadOnly))
		return false;

	QTextStream stream(&file);
	if(stream.readLine() != QStringLiteral("SubtitleComposer Symbol Matrix v1.0"))
		return false;

	m_recognizedPieces.clear();
	m_recognizedPiecesMaxSymbolLength = 0;

	RichString text;
	Piece piece;
	QString line;
	QChar ch;
	while(stream.readLineInto(&line)) {
		if(line.startsWith(QStringLiteral(".s "))) {
			text.setRichString(QStringView(line).mid(3).trimmed().toString());
		} else if(line.startsWith(QStringLiteral(".d "))) {
			QTextStream data(QStringView(line).mid(3).trimmed().toUtf8());

			// read piece data
			data >> piece.right;
			do { data >> ch; } while(ch != QLatin1Char(','));
			data >> piece.bottom;
			do { data >> ch; } while(ch != QLatin1Char(','));
			data >> piece.symbolCount;

			// skip to point data
			do { data >> ch; } while(ch != QLatin1Char(':'));
			data.skipWhiteSpace();

			// read point data
			piece.pixels.clear();
			const QByteArray pixelData(qUncompress(QByteArray::fromBase64(data.readAll().toUtf8(), QByteArray::Base64Encoding | QByteArray::OmitTrailingEquals)));
			QDataStream pixelDataStream(pixelData);
			while(!pixelDataStream.atEnd()) {
				int x, y;
				pixelDataStream >> x >> y;
				piece.pixels.append(QPoint(x, y));
			}

			// save piece
			if(piece.symbolCount > m_recognizedPiecesMaxSymbolLength)
				m_recognizedPiecesMaxSymbolLength = piece.symbolCount;
			m_recognizedPieces[piece] = text;

			text.clear();
		}
	}

	file.close();

	return true;
}

bool
VobSubInputProcessDialog::symFileSave(const QString &filename)
{
	QSaveFile file(filename);
	if(!file.open(QIODevice::WriteOnly))
		return false;

	QTextStream stream(&file);
	stream << QStringLiteral("SubtitleComposer Symbol Matrix v1.0\n");
	for(auto i = m_recognizedPieces.cbegin(); i != m_recognizedPieces.cend(); ++i) {
		if(!i.value().length())
			continue;
		Piece piece = i.key();
		stream << "\n.s " << i.value().richString();
		stream << QString::asprintf("\n.d %d, %d, %d: ", i.key().right, i.key().bottom, i.key().symbolCount);
		QByteArray pixelData;
		QDataStream pixelDataStream(&pixelData, QIODevice::WriteOnly);
		foreach(QPoint p, i.key().pixels)
			pixelDataStream << p.x() << p.y();
		stream << qCompress(pixelData).toBase64(QByteArray::Base64Encoding | QByteArray::OmitTrailingEquals) << '\n';
	}
	return file.commit();
}

/*virtual*/ bool
VobSubInputProcessDialog::eventFilter(QObject *obj, QEvent *event) /*override*/
{
	if(event->type() == QEvent::FocusOut) {
		ui->lineEdit->setFocus();
		return true;
	}

	if(event->type() == QEvent::KeyPress) {
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
		switch(keyEvent->key()) {
		case Qt::Key_Up:
			ui->symbolCount->setValue(ui->symbolCount->value() + ui->symbolCount->singleStep());
			return true;

		case Qt::Key_Down:
			ui->symbolCount->setValue(ui->symbolCount->value() - ui->symbolCount->singleStep());
			return true;

		case Qt::Key_Left:
			if((keyEvent->modifiers() & Qt::ControlModifier) == 0)
				break;
			if((keyEvent->modifiers() & Qt::ShiftModifier) == 0)
				QMetaObject::invokeMethod(this, "onPrevSymbolClicked", Qt::QueuedConnection);
			else
				QMetaObject::invokeMethod(this, "onPrevImageClicked", Qt::QueuedConnection);
			return true;

		case Qt::Key_Right:
			if((keyEvent->modifiers() & Qt::ControlModifier) == 0)
				break;
			if((keyEvent->modifiers() & Qt::ShiftModifier) == 0)
				QMetaObject::invokeMethod(this, "onNextSymbolClicked", Qt::QueuedConnection);
			else
				QMetaObject::invokeMethod(this, "onNextImageClicked", Qt::QueuedConnection);
			return true;

		case Qt::Key_Space:
		case Qt::Key_Escape:
			return true;

		case Qt::Key_B:
			if((keyEvent->modifiers() & Qt::ControlModifier) == 0)
				break;
			ui->styleBold->toggle();
			return true;

		case Qt::Key_I:
			if((keyEvent->modifiers() & Qt::ControlModifier) == 0)
				break;
			ui->styleItalic->toggle();
			return true;

		case Qt::Key_U:
			if((keyEvent->modifiers() & Qt::ControlModifier) == 0)
				break;
			ui->styleUnderline->toggle();
			return true;
		}
	}

	return QDialog::eventFilter(obj, event);
}

void
VobSubInputProcessDialog::processFrames(StreamProcessor *streamProcessor)
{
	connect(streamProcessor, &StreamProcessor::streamError, this, &VobSubInputProcessDialog::onStreamError);
	connect(streamProcessor, &StreamProcessor::streamFinished, this, &VobSubInputProcessDialog::onStreamFinished);
	connect(streamProcessor, &StreamProcessor::imageDataAvailable, this, &VobSubInputProcessDialog::onStreamData, Qt::BlockingQueuedConnection);

	streamProcessor->start();

	Frame::spaceStats.clear();

	ui->progressBar->setMinimum(0);
	ui->progressBar->setValue(0);

	ui->grpText->setDisabled(true);
	ui->grpNavButtons->setDisabled(true);
}

void
VobSubInputProcessDialog::onStreamData(const QImage &image, quint64 msecStart, quint64 msecDuration)
{
	FramePtr frame(new Frame());
	frame->subShowTime.setMillisTime(double(msecStart));
	frame->subHideTime.setMillisTime(double(msecStart + msecDuration));
	frame->subImage = image;

	ui->subtitleView->setPixmap(QPixmap::fromImage(frame->subImage));
	QCoreApplication::processEvents();

	if(frame->processPieces()) {
		frame->index = m_frames.length();
		ui->progressBar->setMaximum(m_frames.length());
		m_frames.append(frame);
	}
}

void
VobSubInputProcessDialog::onStreamError(int /*code*/, const QString &message, const QString &debug)
{
	QString text = message % QStringLiteral("\n") % debug;
	KMessageBox::error(this, text, i18n("VobSub Error"));
}

void
VobSubInputProcessDialog::onStreamFinished()
{
	m_frameCurrent = m_frames.begin() - 1;

	// average word length in english is 5.1 chars
	const double avgWordLength = 4;

	if(!Frame::spaceStats.empty()) {
		auto itChar = Frame::spaceStats.begin(); // shorter spaces on start
		auto itWord = std::prev(Frame::spaceStats.end()); // longer spaces near end
		qint64 charSpacingSum = itChar.key() * itChar.value();
		quint64 charSpacingCount = itChar.value();
		qint64 wordSpacingSum = itWord.key() * itWord.value();
		quint64 wordSpacingCount = itWord.value();

		while(itChar != itWord) {
			if(charSpacingCount < avgWordLength * wordSpacingCount) {
				// sum up chars
				++itChar;
				charSpacingSum += itChar.key() * itChar.value();
				charSpacingCount += itChar.value();
			} else {
				// sum up words
				--itWord;
				wordSpacingSum += itWord.key() * itWord.value();
				wordSpacingCount += itWord.value();
			}
		}
		m_spaceWidth = wordSpacingSum / wordSpacingCount;
	} else {
		m_spaceWidth = 100;
	}

	ui->grpText->setDisabled(true);
	ui->grpNavButtons->setDisabled(true);
	QMetaObject::invokeMethod(this, "processNextImage", Qt::QueuedConnection);
}

void
VobSubInputProcessDialog::processNextImage()
{
	if(++m_frameCurrent == m_frames.end()) {
		accept();
		return;
	}

	ui->progressBar->setValue((*m_frameCurrent)->index + 1);

	ui->subtitleView->setPixmap(QPixmap::fromImage((*m_frameCurrent)->subImage));

	m_pieces = (*m_frameCurrent)->pieces;
	m_pieceCurrent = m_pieces.begin();

	recognizePiece();
}

void
VobSubInputProcessDialog::processCurrentPiece()
{
	if(m_pieceCurrent == m_pieces.end())
		return;

	ui->grpText->setDisabled(false);
	ui->grpNavButtons->setDisabled(false);

	QPixmap pixmap = QPixmap::fromImage((*m_frameCurrent)->subImage);
	QPainter p(&pixmap);

	QList<PiecePtr>::iterator i = m_pieceCurrent;
	p.setPen(QColor(255, 255, 255, 64));
	p.drawLine(0, (*i)->line->baseline, pixmap.width(), (*i)->line->baseline);

	p.setPen(QColor(255, 0, 0, 200));
	QRect rcVisible(QPoint((*i)->left, (*i)->top), QPoint((*i)->right, (*i)->bottom));
	int n = (*i)->symbolCount;
	for(; n-- && i != m_pieces.end(); ++i) {
		rcVisible |= QRect(QPoint((*i)->left, (*i)->top), QPoint((*i)->right, (*i)->bottom));
		foreach(QPoint pix, (*i)->pixels)
			p.drawPoint(pix);
	}
	rcVisible.adjust((ui->subtitleView->minimumWidth() - rcVisible.width()) / -2, (ui->subtitleView->minimumHeight() - rcVisible.height()) / -2, 0, 0);
	rcVisible.setBottomRight(QPoint(pixmap.width(), pixmap.height()));
	ui->subtitleView->setPixmap(pixmap.copy(rcVisible));

	ui->lineEdit->setFocus();

	ui->symbolCount->setMaximum(m_pieces.end() - m_pieceCurrent);
}

void
VobSubInputProcessDialog::processNextPiece()
{
	m_pieceCurrent += (*m_pieceCurrent)->symbolCount;

	ui->lineEdit->clear();
	ui->symbolCount->setValue(1);

	if(m_pieceCurrent == m_pieces.end()) {
		QString subText;
		PiecePtr piecePrev;
		foreach(PiecePtr piece, m_pieces) {
			if(piecePrev) {
				if(!piecePrev->line->intersects(piece->line))
					subText.append(QChar(QChar::LineFeed));
				else if(piece->left - piecePrev->right > m_spaceWidth)
					subText.append(QChar(QChar::Space));
			}

			subText += piece->text;
			piecePrev = piece;
		}

		SubtitleLine *l = new SubtitleLine((*m_frameCurrent)->subShowTime, (*m_frameCurrent)->subHideTime);
		l->primaryDoc()->setPlainText(subText);
		m_subtitle->insertLine(l);

		ui->grpText->setDisabled(true);
		ui->grpNavButtons->setDisabled(true);
		QMetaObject::invokeMethod(this, "processNextImage", Qt::QueuedConnection);
		return;
	}

	recognizePiece();
}

void
VobSubInputProcessDialog::recognizePiece()
{
	for(int len = m_recognizedPiecesMaxSymbolLength; len > 0; len--) {
		PiecePtr normal = currentNormalizedPiece(len);
		if(len != normal->symbolCount)
			continue;
		if(m_recognizedPieces.contains(*normal)) {
			const RichString text = m_recognizedPieces.value(*normal);
			(*m_pieceCurrent)->text = text;
			currentSymbolCountSet(len);
			processNextPiece();
			return;
		}
	}

	processCurrentPiece();
}

RichString
VobSubInputProcessDialog::currentText()
{
	int style = 0;
	if(ui->styleBold->isChecked())
		style |= RichString::Bold;
	if(ui->styleItalic->isChecked())
		style |= RichString::Italic;
	if(ui->styleUnderline->isChecked())
		style |= RichString::Underline;
	return RichString(ui->lineEdit->text(), style);
}

VobSubInputProcessDialog::PiecePtr
VobSubInputProcessDialog::currentNormalizedPiece(int symbolCount)
{
	PiecePtr normal(new Piece(**m_pieceCurrent));
	normal->symbolCount = 1;
	for(auto piece = m_pieceCurrent; --symbolCount && ++piece != m_pieces.end(); ) {
		*normal += **piece;
		normal->symbolCount++;
	}

	normal->normalize();

	return normal;
}

void
VobSubInputProcessDialog::currentSymbolCountSet(int symbolCount)
{
	if(m_pieceCurrent == m_pieces.end())
		return;

	int n = (*m_pieceCurrent)->symbolCount;
	QList<PiecePtr>::iterator piece = m_pieceCurrent;
	while(--n && ++piece != m_pieces.end())
		(*piece)->symbolCount = 1;

	piece = m_pieceCurrent;
	(*piece)->symbolCount = symbolCount;
	while(--symbolCount && ++piece != m_pieces.end())
		(*piece)->symbolCount = 0;
}

void
VobSubInputProcessDialog::onSymbolCountChanged(int symbolCount)
{
	currentSymbolCountSet(symbolCount);

	processCurrentPiece();
}

void
VobSubInputProcessDialog::onOkClicked()
{
	if((*m_pieceCurrent)->symbolCount > m_recognizedPiecesMaxSymbolLength)
		m_recognizedPiecesMaxSymbolLength = (*m_pieceCurrent)->symbolCount;

	(*m_pieceCurrent)->text = currentText();

	PiecePtr normal = currentNormalizedPiece((*m_pieceCurrent)->symbolCount);
	m_recognizedPieces[*normal] = (*m_pieceCurrent)->text;

	processNextPiece();
}

void
VobSubInputProcessDialog::onAbortClicked()
{
	reject();
}

void
VobSubInputProcessDialog::onPrevImageClicked()
{
	if(m_frameCurrent == m_frames.begin())
		return;

	--m_frameCurrent;
	if(m_subtitle->lastIndex() >= 0)
		m_subtitle->removeLines(RangeList(Range(m_subtitle->lastIndex())), Both);

	ui->progressBar->setValue((*m_frameCurrent)->index + 1);

	m_pieces = (*m_frameCurrent)->pieces;
	m_pieceCurrent = m_pieces.end();

	onPrevSymbolClicked();
}

void
VobSubInputProcessDialog::onNextImageClicked()
{
	if(m_frameCurrent == m_frames.end() - 1)
		return;

	++m_frameCurrent;

	ui->progressBar->setValue((*m_frameCurrent)->index + 1);

	m_pieces = (*m_frameCurrent)->pieces;
	m_pieceCurrent = m_pieces.begin() - 1;

	onNextSymbolClicked();
}

void
VobSubInputProcessDialog::onPrevSymbolClicked()
{
	do {
		if(m_pieceCurrent == m_pieces.begin())
			return onPrevImageClicked();
		--m_pieceCurrent;
	} while((*m_pieceCurrent)->symbolCount == 0);

	updateCurrentPiece();
}

void
VobSubInputProcessDialog::onNextSymbolClicked()
{
	do {
		if(m_pieceCurrent >= m_pieces.end() - 1)
			return onNextImageClicked();
		++m_pieceCurrent;
	} while((*m_pieceCurrent)->symbolCount == 0);

	updateCurrentPiece();
}

void
VobSubInputProcessDialog::updateCurrentPiece()
{
	processCurrentPiece();

	ui->lineEdit->setText((*m_pieceCurrent)->text.string());
	ui->lineEdit->selectAll();

	int style = (*m_pieceCurrent)->text.styleFlagsAt(0);
	ui->styleBold->setChecked((style & RichString::Bold) != 0);
	ui->styleItalic->setChecked((style & RichString::Italic) != 0);
	ui->styleUnderline->setChecked((style & RichString::Underline) != 0);

	ui->symbolCount->setValue((*m_pieceCurrent)->symbolCount);
}
