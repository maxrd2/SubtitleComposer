#ifndef TEXTDEMUX_H
#define TEXTDEMUX_H

#include <QObject>

QT_FORWARD_DECLARE_CLASS(QWidget)
QT_FORWARD_DECLARE_CLASS(QProgressBar)

namespace SubtitleComposer {
class Subtitle;
class StreamProcessor;

class TextDemux : public QObject
{
	Q_OBJECT

public:
	explicit TextDemux(QWidget *parent=NULL);

	void demuxFile(Subtitle *subtitle, const QString filename, int textStreamIndex);

	QWidget * progressWidget();

signals:
	void onError(const QString &message);

private slots:
	void onStreamData(const QString &text, quint64 msecStart, quint64 msecDuration);
	void onStreamProgress(quint64 msecPos, quint64 msecLength);
	void onStreamError(int code, const QString &message, const QString &debug);
	void onStreamFinished();

private:
	Subtitle *m_subtitle;
	StreamProcessor *m_streamProcessor;

	QWidget *m_progressWidget;
	QProgressBar *m_progressBar;
};
}

#endif // TEXTDEMUX_H
