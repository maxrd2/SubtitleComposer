#ifndef ACTIONDIALOG_H
#define ACTIONDIALOG_H

/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2018 Mladen Milinkovic <max@smoothware.net>
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
	virtual int exec();
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
