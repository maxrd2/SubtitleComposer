/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ERRORFINDER_H
#define ERRORFINDER_H

#include "core/subtitle.h"

#include <QObject>

namespace SubtitleComposer {
class FindErrorsDialog;
class SubtitleIterator;

class ErrorFinder : public QObject
{
	Q_OBJECT

public:
	explicit ErrorFinder(QWidget *parent = 0);
	virtual ~ErrorFinder();

	QWidget * parentWidget();

public slots:
	void setSubtitle(Subtitle *subtitle = 0);
	void setTranslationMode(bool enabled);

	void find(int searchFromIndex, bool findBackwards = false);
	bool findNext(int fromIndex = -1);
	bool findPrevious(int fromIndex = -1);

signals:
	void found(SubtitleLine *line);

private:
	void advance(bool advanceIteratorOnFirstStep);

private slots:
	void invalidate();

private:
	QExplicitlySharedDataPointer<Subtitle> m_subtitle;
	FindErrorsDialog *m_dialog;
	bool m_translationMode;

	int m_targetErrorFlags;
	bool m_findBackwards;
	SubtitleIterator *m_iterator;
};
}
#endif
