#ifndef MPLAYERPLAYERBACKEND_H
#define MPLAYERPLAYERBACKEND_H

/**
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2015 Mladen Milinkovic <max@smoothware.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "../../videoplayer/playerbackend.h"

#include <QWidget>

QT_FORWARD_DECLARE_CLASS(QString)

namespace SubtitleComposer {
class MPlayerPlayerProcess;

class MPlayerPlayerBackend : public PlayerBackend
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID PlayerBackend_iid)
	Q_INTERFACES(SubtitleComposer::PlayerBackend)

public:
	MPlayerPlayerBackend();
	virtual ~MPlayerPlayerBackend();

	virtual QWidget * newConfigWidget(QWidget *parent);

protected:
	virtual bool initialize(VideoWidget *videoWidget);
	virtual void finalize();
	void _finalize();
	virtual bool reconfigure();

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

private:
	virtual void setSCConfig(SCConfig *scConfig);

protected:
	MPlayerPlayerProcess *m_process;

	double m_position;
	bool m_reportUpdates;
};
}

#endif
