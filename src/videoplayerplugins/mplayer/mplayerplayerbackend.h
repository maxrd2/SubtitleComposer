#ifndef MPLAYERPLAYERBACKEND_H
#define MPLAYERPLAYERBACKEND_H

/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2018 Mladen Milinkovic <max@smoothware.net>
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

#include "videoplayer/playerbackend.h"

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

	QWidget * newConfigWidget(QWidget *parent) Q_DECL_OVERRIDE;

protected:
	bool initialize(VideoWidget *videoWidget) Q_DECL_OVERRIDE;
	void finalize() Q_DECL_OVERRIDE;
	void _finalize();
	bool reconfigure() Q_DECL_OVERRIDE;

	bool openFile(const QString &filePath, bool &playingAfterCall) Q_DECL_OVERRIDE;
	void closeFile() Q_DECL_OVERRIDE;

	bool play() Q_DECL_OVERRIDE;
	bool pause() Q_DECL_OVERRIDE;
	bool seek(double seconds, bool accurate) Q_DECL_OVERRIDE;
	bool step(int /*frameOffset*/) Q_DECL_OVERRIDE { return false; }
	bool stop() Q_DECL_OVERRIDE;

	void playbackRate(double /*newRate*/) Q_DECL_OVERRIDE {}

	bool setActiveAudioStream(int audioStream) Q_DECL_OVERRIDE;

	bool setVolume(double volume) Q_DECL_OVERRIDE;

protected slots:
	void onMediaDataLoaded();
	void onPlayingReceived();
	void onPausedReceived();
	void onProcessExited();
	void onPositionReceived(double seconds);

protected:
	void setupProcessArgs(const QString &filePath);

private:
	void setSCConfig(SCConfig *scConfig) Q_DECL_OVERRIDE;

protected:
	MPlayerPlayerProcess *m_process;

	double m_position;
	bool m_reportUpdates;
};
}

#endif
