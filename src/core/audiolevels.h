#ifndef AUDIOLEVELS_H
#define AUDIOLEVELS_H

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

#include "time.h"

#include <QtCore/QObject>

#include <KUrl>

namespace SubtitleComposer
{
	class AudioLevels : public QObject
	{
		Q_OBJECT

		public:

			AudioLevels();
			virtual ~AudioLevels();

			bool load( const KUrl& fileUrl );
			bool loadFromMedia( const QString& mediaPath, unsigned streamIndex );
			bool save( const KUrl& fileUrl, bool overwrite ) const;

			void reset();

			inline unsigned channelsCount() const
			{
				return m_channelsCount;
			};

			inline unsigned samplesCount() const // in milliseconds
			{
				return m_samplesCount;
			};

			inline unsigned samplesInterval() const // in milliseconds
			{
				return m_samplesInterval;
			};

			inline Time length() const // in milliseconds
			{
				return m_samplesCount*m_samplesInterval;
			};

			/// warning!! no checks performed!
			inline double dataBySample( unsigned channel, unsigned sample ) const
			{
				//return sample >= m_samplesCount ? 0 : m_samplesData[m_samplesCount*channel+sample];
				return m_samplesData[m_samplesCount*channel+sample];
			};

			inline double dataByPosition( unsigned channel, const Time& position ) const
			{
				int dataIndex = m_samplesCount*channel+position.toMillis()/m_samplesInterval;
				return dataIndex > m_lastDataIndex ? 0 : m_samplesData[dataIndex];
			};

			inline unsigned sampleForPosition( const Time& position ) const
			{
				if ( position > length() )
					return m_samplesCount - 1;

				return position.toMillis()/m_samplesInterval;
			};

		signals:

			void loadProgress( int percentage );

		private:

			unsigned m_channelsCount;
			unsigned m_samplesInterval;
			unsigned m_samplesCount;
			double* m_samplesData;
			int m_lastDataIndex;
			KUrl m_mediaUrl;
	};
}

#endif

