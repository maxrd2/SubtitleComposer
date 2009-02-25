#ifndef MEDIADATA_H
#define MEDIADATA_H

/***************************************************************************
 *   smplayer, GUI front-end for mplayer.                                  *
 *   Copyright (C) 2007 Ricardo Villalba (rvm@escomposlinux.org)           *
 *                                                                         *
 *   modified for inclusion in Subtitle Composer                           *
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

#include <QtCore/QString>
#include <QtCore/QMap>

namespace SubtitleComposer
{
	class TrackData
	{
		public:

			QString name;
			QString language;
	};

	class MediaData
	{
		public:

			MediaData() { reset(); };

			void reset()
			{
				duration = 0;

				hasVideo = false;

				videoWidth = 0;
				videoHeight = 0;
				videoDAR = 4.0/3.0;
				videoFPS = 0.0;

				audioTracks.clear();
			}

			double duration;

			int videoWidth;
			int videoHeight;
			double videoDAR;
			double videoFPS;

			bool hasVideo;

			QMap<int,TrackData> audioTracks;
	};
}

#endif
