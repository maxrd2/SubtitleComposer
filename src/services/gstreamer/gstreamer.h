#ifndef GSTREAMER_H
#define GSTREAMER_H

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

#include "../waveformat.h"

#include <QtCore/QString>
#include <QtCore/QStringList>

#include <gst/gst.h>

namespace SubtitleComposer {
class GStreamer
{
	friend class GStreamerPlayerBackend;
	friend class GStreamerDecoderBackend;

private:
	static bool init();
	static void deinit();

	static GstElement * createElement(const QString &types, const char *name);
	static GstElement * createElement(const QStringList &types, const char *name);

	static GstStateChangeReturn setElementState(GstElement *element, int state, unsigned timeout = 0);

	static WaveFormat formatFromAudioCaps(GstCaps *caps);
	static GstCaps * audioCapsFromFormat(const WaveFormat &format, bool addSampleRate = true);

	static GstPadLinkReturn link(GstBin *bin, const char *srcElement, const char *dstElement, GstCaps *filter = 0);
	static GstPadLinkReturn link(GstBin *bin, const char *srcElement, const char *srcPad, const char *dstElement, const char *dstPad, GstCaps *filter = 0);

	static void freePipeline(GstPipeline **pipeline, GstBus **bus);

	static void inspectTags(GstTagList *tags, const QString &prefix = QString());
	static void inspectPad(GstPad *pad, const QString &prefix = QString());
	static void inspectCaps(GstCaps *caps, const QString &prefix = QString());
	static void inspectMessage(GstMessage *message);
	static void inspectObject(GObject *object);

private:
	static int s_inited;
};
}
#endif
