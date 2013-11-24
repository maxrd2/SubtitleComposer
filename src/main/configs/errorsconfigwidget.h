#ifndef ERRORSCONFIGWIDGET_H
#define ERRORSCONFIGWIDGET_H

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

#include "errorsconfig.h"
#include "../../config/appconfiggroupwidget.h"

class KIntNumInput;
class QCheckBox;

namespace SubtitleComposer {
class ErrorsConfigWidget : public AppConfigGroupWidget
{
	Q_OBJECT

	friend class ConfigDialog;

public:
	virtual ~ErrorsConfigWidget();

	virtual void setControlsFromConfig();
	virtual void setConfigFromControls();

private:
	explicit ErrorsConfigWidget(QWidget *parent = 0);

	ErrorsConfig * config() { return static_cast<ErrorsConfig *>(m_config); }

	KIntNumInput *m_minDurationSpinBox;
	KIntNumInput *m_maxDurationSpinBox;
	KIntNumInput *m_maxCharactersSpinBox;
	KIntNumInput *m_maxLinesSpinBox;
	KIntNumInput *m_minDurationPerCharSpinBox;
	KIntNumInput *m_maxDurationPerCharSpinBox;
	QCheckBox *m_autoClearFixedCheckBox;
};
}

#endif
