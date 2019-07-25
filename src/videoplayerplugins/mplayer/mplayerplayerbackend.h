#ifndef MPLAYERBACKEND_H
#define MPLAYERBACKEND_H

/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2019 Mladen Milinkovic <max@smoothware.net>
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
#include <QString>

namespace SubtitleComposer {
class MPlayerProcess;

class MPlayerBackend : public PlayerBackend
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID PlayerBackend_iid)
	Q_INTERFACES(SubtitleComposer::PlayerBackend)

public:
	MPlayerBackend();
	virtual ~MPlayerBackend();

	QWidget * newConfigWidget(QWidget *parent) override;
	KCoreConfigSkeleton * config() const override;

protected:
	bool init(QWidget *videoWidget) override;
	void cleanup() override;

	bool openFile(const QString &path) override;
	bool closeFile() override;

	bool play() override;
	bool pause() override;
	bool seek(double seconds) override;
	bool step(int frameOffset) override;
	bool stop() override;

	bool playbackRate(double newRate) override;

	bool selectAudioStream(int streamIndex) override;

	bool setVolume(double volume) override;

protected slots:
	void onMediaDataLoaded();
	void onPlayingReceived();
	void onPausedReceived();
	void onProcessExited();
	void onPositionReceived(double seconds);

protected:
	void setupProcessArgs(const QString &filePath);

private:
	bool reconfigure();

	enum PlayState { STOPPED, PAUSED, PLAYING } m_state;
	void setState(PlayState state);

	bool m_initialized;
	bool m_muted;
	MPlayerProcess *m_process;
	QWidget *m_nativeWindow;
	double m_position;
	bool m_reportUpdates;
	QString m_currentFilePath;
};
}

#endif // MPLAYERBACKEND_H
