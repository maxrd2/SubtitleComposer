#ifndef FINDERRORSDIALOG_H
#define FINDERRORSDIALOG_H

/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2020 Mladen Milinkovic <max@smoothware.net>
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

#include "dialogs/actionwithtargetdialog.h"

#include <QCheckBox>

QT_FORWARD_DECLARE_CLASS(QGroupBox)
QT_FORWARD_DECLARE_CLASS(QGridLayout)

namespace SubtitleComposer {
class FindErrorsDialog : public ActionWithTargetDialog
{
	Q_OBJECT

public:
	FindErrorsDialog(QWidget *parent);
	virtual ~FindErrorsDialog();

	inline bool clearOtherErrors() const { return m_clearOtherErrorsCheckBox->isChecked(); }
	inline bool clearMarks() const { return m_clearMarksCheckBox->isChecked(); }

	int selectedErrorFlags() const;

protected:
	void setTranslationMode(bool value) override;
	QGroupBox * createErrorsGroupBox(const QString &title);
	void createErrorsButtons(bool showUserMarks, bool showMissingTranslation);

private slots:
	void selectAllErrorFlags();
	void deselectAllErrorFlags();

private:
	QGroupBox *m_errorsGroupBox;
	QCheckBox **m_errorsCheckBox;
	QGridLayout *m_errorsLayout;
	QCheckBox *m_clearOtherErrorsCheckBox;
	QCheckBox *m_clearMarksCheckBox;
};
}

#endif // FINDERRORSDIALOG_H
