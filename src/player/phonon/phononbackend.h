#ifndef PHONONBACKEND_H
#define PHONONBACKEND_H

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

#include "phononconfig.h"
#include "../playerbackend.h"

#include <QtCore/QString>
#include <QtCore/QTextStream> // NOTE this is only here because Qt complains otherwise when including Phonon/Global...
#include <QtGui/QWidget>
#include <Phonon/Global>

class QTimer;

namespace Phonon
{
	class MediaObject;
	class MediaController;
	class AudioOutput;
	class VideoWidget;
}

namespace SubtitleComposer
{
	class PhononBackend : public PlayerBackend
	{
		Q_OBJECT

		public:

			PhononBackend( Player* player );
			virtual ~PhononBackend();

			const PhononConfig* const config() { return static_cast<const PhononConfig* const>( PlayerBackend::config() ); }

			virtual AppConfigGroupWidget* newAppConfigGroupWidget( QWidget* parent );

		protected:

			virtual bool doesVolumeCorrection() const;
			virtual bool supportsChangingAudioStream( bool* onTheFly ) const;

			virtual VideoWidget* initialize( QWidget* videoWidgetParent );
			virtual void finalize();
			void _finalize();

			virtual bool openFile( const QString& filePath, bool& playingAfterCall );
			virtual void closeFile();

			virtual bool play();
			virtual bool pause();
			virtual bool seek( double seconds, bool accurate );
			virtual bool stop();

			virtual bool setActiveAudioStream( int audioStream );

			virtual bool setVolume( double volume );

		protected:

			void initMediaObject();

		protected slots:

			void onHasVideoChanged( bool hasVideo );
			void onFinished();
			void onTick( qint64 currentTime );
			void onTotalTimeChanged( qint64 newTotalTime );
			void onAvailableAudioChannelsChanged();
			void onAvailableSubtitlesChanged();
			void onStateChanged( Phonon::State newState, Phonon::State oldState );

		protected:

			Phonon::MediaObject* m_mediaObject;
			Phonon::MediaController* m_mediaController;
			Phonon::AudioOutput* m_audioOutput;
			Phonon::VideoWidget* m_videoOutput;
	};
}

#endif

