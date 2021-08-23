#ifndef FINDERRORSDIALOG_H
#define FINDERRORSDIALOG_H

/*
 * SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2010-2020 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
