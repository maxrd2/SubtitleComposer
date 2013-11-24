#ifndef SELECTABLESUBTITLEDIALOG_H
#define SELECTABLESUBTITLEDIALOG_H

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

#include "actionwithtargetdialog.h"

#include <KLocale>
#include <KUrl>

class QGridLayout;
class QGroupBox;
class KLineEdit;
class KComboBox;

namespace SubtitleComposer {
class SelectableSubtitleDialog : public ActionWithTargetDialog
{
	Q_OBJECT

public:
	SelectableSubtitleDialog(const QString &title, QWidget *parent = 0);

	KUrl subtitleUrl() const;
	QString subtitleEncoding() const;

protected:
	QGroupBox * createSubtitleGroupBox(const QString &title = i18n("Subtitle"), bool addToLayout = true);

private slots:
	void selectSubtitle();

protected:
	QGroupBox *m_subtitleGroupBox;
	QGridLayout *m_subtitleLayout;

	KLineEdit *m_subtitleUrlLineEdit;
	KComboBox *m_subtitleEncodingComboBox;
};
}
#endif
