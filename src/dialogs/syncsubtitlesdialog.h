/*
 * SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2010-2018 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SYNCSUBTITLESDIALOG_H
#define SYNCSUBTITLESDIALOG_H

#include "selectablesubtitledialog.h"

QT_FORWARD_DECLARE_CLASS(QRadioButton)

namespace SubtitleComposer {
class SyncSubtitlesDialog : public SelectableSubtitleDialog
{
	Q_OBJECT

public:
	explicit SyncSubtitlesDialog(QWidget *parent = 0);

	bool adjustToReferenceSubtitle() const;
	bool synchronizeToReferenceTimes() const;

private:
	QRadioButton *m_adjustRadioButton;
	QRadioButton *m_synchronizeRadioButton;
};
}
#endif
