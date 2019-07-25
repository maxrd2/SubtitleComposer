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

	QWidget * newConfigWidget(QWidget *parent) override;

protected:
	bool doesVolumeCorrection() const override;

	bool initialize(VideoWidget *videoWidget) override;
	void finalize() override;
	void _finalize();
	bool reconfigure() override;

	bool openFile(const QString &filePath, bool &playingAfterCall) override;
	void closeFile() override;

	bool play() override;
	bool pause() override;
	bool seek(double seconds, bool accurate) override;
	bool step(int /*frameOffset*/) override { return false; }
	bool stop() override;

	void playbackRate(double /*newRate*/) override {}

	bool setActiveAudioStream(int audioStream) override;

	bool setVolume(double volume) override;

protected:
	void initMediaObject();

protected slots:
	void onHasVideoChanged(bool hasVideo);
	void onFinished();
	void onTick(qint64 currentTime);
	void onTotalTimeChanged(qint64 newTotalTime);
	void onAvailableAudioChannelsChanged();
	void onAvailableSubtitlesChanged();
	void onStateChanged(Phonon::State newState, Phonon::State oldState);

private:
	void setSCConfig(SCConfig *scConfig) override;

protected:
	Phonon::MediaObject *m_mediaObject;
	Phonon::MediaController *m_mediaController;
	Phonon::AudioOutput *m_audioOutput;
	Phonon::VideoWidget *m_videoOutput;
};
}

#endif
