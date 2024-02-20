/*
    SPDX-FileCopyrightText: 2023 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TRANSLATEDIALOG_H
#define TRANSLATEDIALOG_H

#include "dialogs/actionwithtargetdialog.h"

#include <QVector>

QT_FORWARD_DECLARE_CLASS(QWidget)

namespace SubtitleComposer {

class TranslateEngine;

class TranslateDialog : public ActionWithTargetDialog
{
public:
	TranslateDialog(QWidget *parent = nullptr);

	static void performTranslation();

private:
	void updateEngineUI(int index);

public Q_SLOTS:
    void accept() override;

private:
	QVector<TranslateEngine *> m_engines;
	QWidget *m_settings;
	TranslateEngine *m_engine;
};
} // namespace SubtitleComposer

#endif
