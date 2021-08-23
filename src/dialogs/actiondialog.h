/*
 * SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2010-2019 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef ACTIONDIALOG_H
#define ACTIONDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>

QT_FORWARD_DECLARE_CLASS(QGridLayout)
QT_FORWARD_DECLARE_CLASS(QVBoxLayout)
QT_FORWARD_DECLARE_CLASS(QGroupBox)

namespace SubtitleComposer {
class ActionDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ActionDialog(const QString &title, QWidget *parent = 0);

public slots:
	virtual int exec() override;
	virtual void show();

protected:
	QGroupBox * createGroupBox(const QString &title = QString(), bool addToLayout = true);
	QGridLayout * createLayout(QGroupBox *groupBox);

protected:
	QWidget *m_mainWidget;
	QVBoxLayout *m_mainLayout;
	QDialogButtonBox *m_buttonBox;
};
}
#endif
