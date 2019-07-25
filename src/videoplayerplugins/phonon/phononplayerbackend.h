#ifndef PHONONPLAYERBACKEND_H
#define PHONONPLAYERBACKEND_H

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

#include <QString>
#include <QTextStream>   // NOTE this is only here because Qt complains otherwise when including Phonon/Global...
#include <QWidget>
#include <KDE/Phonon/Global>

QT_FORWARD_DECLARE_CLASS(QTimer)

namespace Phonon {
	class MediaObject;
	class MediaController;
	class AudioOutput;
	class VideoWidget;
}

namespace SubtitleComposer {
class PhononPlayerBackend : public PlayerBackend
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID PlayerBackend_iid)
	Q_INTERFACES(SubtitleComposer::PlayerBackend)

public:
	PhononPlayerBackend();
	virtual ~PhononPlayerBackend();

	QWidget * newConfigWidget(QWidget */*parent*/) override { return nullptr; }
	KCoreConfigSkeleton * config() const override { return nullptr; }

protected:
	bool init(QWidget *videoWidget) override;
	void cleanup() override;

	bool openFile(const QString &filePath) override;
	bool closeFile() override;

	bool play() override;
	bool pause() override;
	bool seek(double seconds) override;
	bool step(int /*frameOffset*/) override { return false; }
	bool stop() override;

	bool playbackRate(double /*newRate*/) override { return false; }

	bool selectAudioStream(int streamIndex) override;

	bool setVolume(double volume) override;

protected slots:
	void onHasVideoChanged(bool hasVideo);
	void onFinished();
	void onTick(qint64 currentTime);
	void onTotalTimeChanged(qint64 newTotalTime);
	void onAvailableAudioChannelsChanged();
	void onAvailableSubtitlesChanged();
	void onStateChanged(Phonon::State newState, Phonon::State oldState);

private:
	void initMediaObject();
	enum PlayState { STOPPED, PAUSED, PLAYING } m_state;
	void setState(PlayState state);

	Phonon::MediaObject *m_mediaObject;
	Phonon::MediaController *m_mediaController;
	Phonon::AudioOutput *m_audioOutput;
	Phonon::VideoWidget *m_videoOutput;
};
}

#endif
