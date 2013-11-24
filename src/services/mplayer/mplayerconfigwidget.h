#ifndef MPLAYERCONFIGWIDGET_H
#define MPLAYERCONFIGWIDGET_H

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

#include "mplayerconfig.h"
#include "../../config/appconfiggroupwidget.h"

class QCheckBox;
class KComboBox;
class KIntSpinBox;
class KUrlRequester;

namespace SubtitleComposer {
class MPlayerConfigWidget : public AppConfigGroupWidget
{
	Q_OBJECT

	friend class MPlayerPlayerBackend;

public:
	virtual ~MPlayerConfigWidget();

	virtual void setControlsFromConfig();
	virtual void setConfigFromControls();

private:
	explicit MPlayerConfigWidget(QWidget *parent = 0);

	MPlayerConfig * config() { return static_cast<MPlayerConfig *>(m_config); }

private:
	QCheckBox *m_vdpauDivxCheckBox;
	QCheckBox *m_frameDropCheckBox;
	QCheckBox *m_hardFrameDropCheckBox;
	KComboBox *m_videoOutputComboBox;
	QCheckBox *m_videoOutputCheckBox;
	KComboBox *m_audioOutputComboBox;
	KIntSpinBox *m_avsyncSpinBox;
	QCheckBox *m_audioChannelsCheckBox;
	KIntSpinBox *m_audioChannelsSpinBox;
	QCheckBox *m_volumeAmplificationCheckBox;
	KIntSpinBox *m_volumeAmplificationSpinBox;
	QCheckBox *m_volumeNormalizationCheckBox;
	QCheckBox *m_avsyncCheckBox;
	QCheckBox *m_audioOutputCheckBox;
	KUrlRequester *m_pathUrlRequester;
};
}

#endif
