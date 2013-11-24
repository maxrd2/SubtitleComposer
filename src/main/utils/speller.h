#ifndef SPELLER_H
#define SPELLER_H

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

#include "../../core/subtitle.h"
#include "../../core/subtitleline.h"

#include <QtCore/QObject>

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

	void spellCheck(int currentIndex);

signals:
	void misspelled(SubtitleLine *line, bool primary, int startIndex, int endIndex);

private:
	void invalidate();
	bool advance();

private slots:
	void onBufferDone();
	void onMisspelling(const QString &before, int pos);
	void onCorrected(const QString &before, int pos, const QString &after);

	void onSpellingOptionChanged(const QString &option, const QString &value);

private:
	Subtitle *m_subtitle;
	bool m_translationMode;
	bool m_feedPrimaryNext;

	Sonnet::Dialog *m_sonnetDialog;
	SubtitleIterator *m_iterator;
	int m_firstIndex;
};
}
#endif
