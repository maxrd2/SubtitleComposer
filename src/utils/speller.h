/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SPELLER_H
#define SPELLER_H

#include "core/subtitle.h"
#include "core/subtitleline.h"

#include <QExplicitlySharedDataPointer>
#include <QObject>

namespace Sonnet {
class Dialog;
}
namespace SubtitleComposer {
class SubtitleIterator;

class Speller : public QObject
{
	Q_OBJECT

public:
	explicit Speller(QWidget *parent = 0);
	virtual ~Speller();

	QWidget * parentWidget();

public slots:
	void setSubtitle(Subtitle *subtitle = 0);
	void setTranslationMode(bool enabled);
	void setUseTranslation(bool useTranslation);

	void spellCheck(int currentIndex);

signals:
	void misspelled(SubtitleLine *line, bool primary, int startIndex, int endIndex);

private:
	void invalidate();
	bool advance();
	void updateBuffer();

private slots:
	void onBufferDone();
	void onMisspelling(const QString &before, int pos);
	void onCorrected(const QString &before, int pos, const QString &after);

	void onConfigChanged();

private:
	QExplicitlySharedDataPointer<const Subtitle> m_subtitle;
	bool m_translationMode;
	bool m_useTranslation;

	Sonnet::Dialog *m_sonnetDialog;
	SubtitleIterator *m_iterator;
	int m_firstIndex;
};
}
#endif
