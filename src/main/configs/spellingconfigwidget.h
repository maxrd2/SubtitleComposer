#ifndef SPELLINGCONFIGWIDGET_H
#define SPELLINGCONFIGWIDGET_H

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

namespace Sonnet {
class ConfigWidget;
}

namespace SubtitleComposer {
class SpellingConfigWidget : public QWidget
{
	Q_OBJECT

	friend class ConfigDialog;

public:
	explicit SpellingConfigWidget(QWidget *parent=NULL);
	virtual ~SpellingConfigWidget();

private:
	Sonnet::ConfigWidget *m_sonnetConfigWidget;
};
}

#endif
