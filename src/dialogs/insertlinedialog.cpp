/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "insertlinedialog.h"

using namespace SubtitleComposer;

InsertLineDialog::InsertLineDialog(bool insertBefore, QWidget *parent)
	: ActionWithTextsTargetDialog(insertBefore ? i18n("Insert Line Before Current") : i18n("Insert Line After Current"), i18n("Insert Into"), parent)
{
	setNonTranslationModeTarget(Both);
}
