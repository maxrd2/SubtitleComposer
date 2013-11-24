#ifndef MPLAYERPLAYERBACKEND_H
#define MPLAYERPLAYERBACKEND_H

/***************************************************************************
 *   Copyright (C) 2007-2009 Sergio Pistone (sergio_pistone@yahoo.com.ar)  *
 *   based on smplayer by Ricardo Villalba                                 *
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
#include "../playerbackend.h"

#include <QtGui/QWidget>
#include <QtCore/QString>

namespace SubtitleComposer {
class MPlayerPlayerProcess;

class MPlayerPlayerBackend : public PlayerBackend
{
	Q_OBJECT

public:
	MPlayerPlayerBackend(Player *player);
	virtual ~MPlayerPlayerBackend();

	const MPlayerConfig * config() { return static_cast<const MPlayerConfig *const>(PlayerBackend::config()); }

	virtual AppConfigGroupWidget * newAppConfigGroupWidget(QWidget *parent);

protected:
	virtual VideoWidget * initialize(QWidget *videoWidgetParent);
	virtual void finalize();
	void _finalize();

	virtual bool openFile(const QString &filePath, bool &playingAfterCall);
	virtual void closeFile();

	virtual bool play();
	virtual bool pause();
	virtual bool seek(double seconds, bool accurate);
	virtual bool stop();

	virtual bool setActiveAudioStream(int audioStream);

	virtual bool setVolume(double volume);

protected slots:
	void onMediaDataLoaded();
	void onPlayingReceived();
	void onPausedReceived();
	void onProcessExited();
	void onPositionReceived(double seconds);

protected:
	void setupProcessArgs(const QString &filePath);

protected:
	MPlayerPlayerProcess *m_process;

	double m_position;
	bool m_reportUpdates;
};
}

#endif
