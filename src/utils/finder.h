/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FINDER_H
#define FINDER_H

#include "core/subtitle.h"
#include "core/subtitleline.h"

#include <QExplicitlySharedDataPointer>
#include <QObject>
#include <QPointer>

QT_FORWARD_DECLARE_CLASS(QGroupBox)
QT_FORWARD_DECLARE_CLASS(QRadioButton)
class KFind;
class KFindDialog;

namespace SubtitleComposer {
class SubtitleIterator;

class Finder : public QObject
{
	Q_OBJECT

public:
	explicit Finder(QWidget *parent = 0);
	virtual ~Finder();

	QWidget * parentWidget();

public slots:
	void setSubtitle(Subtitle *subtitle = 0);
	void setTranslationMode(bool enabled);

	void find(const RangeList &selectionRanges, int currentIndex, const QString &text = QString(), bool findBackwards = false);
	bool findNext();
	bool findPrevious();

signals:
	void found(SubtitleLine *line, bool primary, int startIndex, int endIndex);

private slots:
	void invalidate();

	void onLinePrimaryTextChanged();
	void onLineSecondaryTextChanged();

	void onHighlight(const QString &text, int matchingIndex, int matchedLength);

	void onIteratorSynchronized(int firstIndex, int lastIndex, bool inserted);

private:
	void advance();

private:
	QExplicitlySharedDataPointer<const Subtitle> m_subtitle;
	bool m_translationMode;
	bool m_feedingPrimary;

	KFind *m_find;
	KFindDialog *m_dialog;
	QGroupBox *m_targetGroupBox;
	QRadioButton *m_targetRadioButtons[SubtitleTargetSize];
	SubtitleIterator *m_iterator;
	QPointer<SubtitleLine> m_dataLine;
	bool m_instancesFound;
	int m_allSearchedIndex;
};
}
#endif
