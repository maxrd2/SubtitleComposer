#ifndef ENCODINGDETECTDIALOG_H
#define ENCODINGDETECTDIALOG_H

#include <QDialog>

namespace Ui {
class EncodingDetectDialog;
}

namespace SubtitleComposer {
class EncodingDetectDialog : public QDialog
{
	Q_OBJECT

public:
	explicit EncodingDetectDialog(const QByteArray &text, QWidget *parent = nullptr);
	~EncodingDetectDialog();

	void addEncoding(const QString &name, int confidence);
	inline const QString & selectedEncoding() { return m_selectedEncoding; }

private:
	Ui::EncodingDetectDialog *ui;
	QByteArray m_text;
	QString m_selectedEncoding;
};
}

#endif // ENCODINGDETECTDIALOG_H
