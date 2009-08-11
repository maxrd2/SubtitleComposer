#ifndef TRANSKODE_GSTLAUNCHDECODERPLUGIN_H
#define TRANSKODE_GSTLAUNCHDECODERPLUGIN_H

/***************************************************************************
 *   Copyright (C) 2005 by Sergio Pistone                                  *
 *   sergio_pistone@yahoo.com.ar                                           *
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
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
	#include <config.h>
#endif

#include <decoderplugin.h>

namespace TransKode
{
	class GstLaunchDecoderPlugin : public DecoderPlugin
	{
		friend class PluginsManager;

		public:

			virtual void setupArgs( ProcessHandler& proc, const QString& src, const QString& dst ) const
			{
				proc.clearArguments();
				bool pMode = proc.getShellParsingMode();
				proc.setShellParsingMode( false );
				proc << programPath() << "filesrc" << "location="+src << "!" << "decodebin" << "!" << "audioconvert" << "!" << "wavenc" << "!" << "filesink" << "location="+dst;
				proc.setShellParsingMode( pMode );
			}

			virtual ProgressParser* newProgressParser() const
			{
				return 0;
			}

		protected:

			GstLaunchDecoderPlugin():
				DecoderPlugin( "GStreamer", QString::null, "gst-launch-0.10", QStringList::split( ":", "*:-ram:-asx" ) ) {}

			~GstLaunchDecoderPlugin() {}
	};
}

#endif

