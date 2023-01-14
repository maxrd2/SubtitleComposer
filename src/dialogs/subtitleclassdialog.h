/*
    SPDX-FileCopyrightText: 2023 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef SUBTITLECOMPOSER_SUBTITLECLASSDIALOG_H
#define SUBTITLECOMPOSER_SUBTITLECLASSDIALOG_H

#include <QDialog>

namespace Ui {
class SubtitleClassDialog;
}

namespace SubtitleComposer {

class SubtitleClassDialog : public QDialog
{
	Q_OBJECT

	Q_PROPERTY(QString currentClass READ currentClass WRITE setCurrentClass
	           NOTIFY currentClassChanged)

public:
	SubtitleClassDialog(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
	SubtitleClassDialog(const QString &initial, QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
	virtual ~SubtitleClassDialog();

	static QString getClass(const QStringList &classes,
				const QString &initial = QString(),
				QWidget *parent = nullptr,
				const QString &title = QString());

	void setCurrentClass(const QString &cssClass);
	QString currentClass() const;

	inline QString selectedClass() const { return currentClass(); }

signals:
	void currentClassChanged(const QString &cssClass);
	void classSelected(const QString &cssClass);

private:
	Ui::SubtitleClassDialog *ui;
};
} // namespace SubtitleComposer

#endif // SUBTITLECOMPOSER_SUBTITLECLASSDIALOG_H
