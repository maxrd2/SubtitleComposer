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

#include "gstreamer.h"

#include <QtCore/QStringList>

#include <KDebug>

//#define VERBOSE

using namespace SubtitleComposer;

int GStreamer::s_inited = 0;

bool
GStreamer::init()
{
	if(!s_inited) {
		if(!gst_init_check(NULL, NULL, NULL)) {
			kError() << "GStreamer initialization failed!";
			return false;
		}
	}
	s_inited++;
	return true;
}

void
GStreamer::deinit()
{
	if(s_inited) {
		s_inited--;
		if(!s_inited)
			gst_deinit();
	}
}

GstElement *
GStreamer::createElement(const QString &types, const char *name)
{
	return createElement(types.split(" "), name);
}

GstElement *
GStreamer::createElement(const QStringList &types, const char *name)
{
	GstElement *element = 0;
	for(QStringList::ConstIterator it = types.begin(), end = types.end(); it != end; ++it)
		if(!(*it).isEmpty() && (element = gst_element_factory_make((*it).toAscii(), name)))
			break;
	return element;
}

GstStateChangeReturn
GStreamer::setElementState(GstElement *element, int state, unsigned timeout)
{
	GstStateChangeReturn ret = gst_element_set_state(element, (GstState)state);
	if(ret == GST_STATE_CHANGE_SUCCESS)
		return ret;

	if(ret != GST_STATE_CHANGE_ASYNC) {
#if defined(VERBOSE) || !defined(NDEBUG)
		if(ret == GST_STATE_CHANGE_FAILURE)
			qDebug() << "error setting element" << gst_element_get_name(element) << "state to" << state;
#endif
		return ret;
	}

	if(!timeout)
		return GST_STATE_CHANGE_ASYNC;

	// wait for state change or timeout
	if(!gst_element_get_state(element, NULL, NULL, timeout * GST_MSECOND) == GST_STATE_CHANGE_SUCCESS) {
#if defined(VERBOSE) || !defined(NDEBUG)
		qDebug() << "error setting element" << gst_element_get_name(element) << "state to" << state;
#endif
		return GST_STATE_CHANGE_FAILURE;
	} else
		return GST_STATE_CHANGE_SUCCESS;
}

static int
intValue(const GValue *gvalue, bool maximum, const QList<int> suggested)
{
	if(G_VALUE_HOLDS(gvalue, G_TYPE_INT)) {
		return g_value_get_int(gvalue);
	} else if(GST_VALUE_HOLDS_LIST(gvalue)) {
		for(int index = 0, size = suggested.size(); index < size; ++index) {
			const int suggestedValue = suggested.at(index);
			for(guint index2 = 0, size2 = gst_value_list_get_size(gvalue); index2 < size2; ++index2) {
				if(suggestedValue == g_value_get_int(gst_value_list_get_value(gvalue, index2)))
					return suggestedValue;
			}
		}

		int value;
		if(maximum) {
			value = INT_MIN;
			for(guint index = 0, size = gst_value_list_get_size(gvalue); index < size; ++index) {
				const int newValue = g_value_get_int(gst_value_list_get_value(gvalue, index));
				if(newValue > value)
					value = newValue;
			}
		} else {
			value = INT_MAX;
			for(guint index = 0, size = gst_value_list_get_size(gvalue); index < size; ++index) {
				const int newValue = g_value_get_int(gst_value_list_get_value(gvalue, index));
				if(newValue < value)
					value = newValue;
			}
		}
		return value;
	} else if(GST_VALUE_HOLDS_INT_RANGE(gvalue)) {
		const int minValue = gst_value_get_int_range_min(gvalue);
		const int maxValue = gst_value_get_int_range_max(gvalue);
		for(int index = 0, size = suggested.size(); index < size; ++index) {
			const int value = suggested.at(index);
			if(minValue <= value && value <= maxValue)
				return value;
		}
		return maximum ? maxValue : minValue;
	}

	return 0;
}

WaveFormat
GStreamer::formatFromAudioCaps(GstCaps *caps)
{
	WaveFormat format(0, 0, 0);

	const GstStructure *capsStruct = gst_caps_get_structure(caps, 0);
	if(capsStruct) {
		const gchar *name = gst_structure_get_name(capsStruct);
		format.setInteger(strcmp(name, "audio/x-raw-int") == 0);

		QList<int> suggestedValues;

		if(gst_structure_has_field(capsStruct, "channels"))
			format.setChannels(intValue(gst_structure_get_value(capsStruct, "channels"), true, suggestedValues));

		if(gst_structure_has_field(capsStruct, "width"))
			format.setBitsPerSample(intValue(gst_structure_get_value(capsStruct, "width"), true, suggestedValues));

		suggestedValues << 48000 << 44100 << 24000 << 22050 << 12000 << 11025 << 8000;
		if(gst_structure_has_field(capsStruct, "rate"))
			format.setSampleRate(intValue(gst_structure_get_value(capsStruct, "rate"), true, suggestedValues));
	}

	return format;
}

GstCaps *
GStreamer::audioCapsFromFormat(const WaveFormat &format, bool addSampleRate)
{
	GstCaps *caps = gst_caps_new_simple(format.isInteger() ? "audio/x-raw-int" : "audio/x-raw-float",
										"endianness", G_TYPE_INT, (gint)1234,
										"channels", G_TYPE_INT, (gint)format.channels(),
										"width", G_TYPE_INT, (gint)format.bitsPerSample(),
										"depth", G_TYPE_INT, (gint)format.bitsPerSample(),
										NULL);
	GstStructure *structure = gst_caps_get_structure(caps, 0);
	if(format.isInteger())
		gst_structure_set(structure, "signed", G_TYPE_BOOLEAN, (gboolean)format.isSigned(), NULL);
	if(addSampleRate)
		gst_structure_set(structure, "rate", G_TYPE_INT, (gint)format.sampleRate(), NULL);
	return caps;
}

GstPadLinkReturn
GStreamer::link(GstBin *bin, const char *srcElement, const char *dstElement, GstCaps *filter)
{
	return link(bin, srcElement, "src", dstElement, "sink", filter);
}

GstPadLinkReturn
GStreamer::link(GstBin *bin, const char *srcElement, const char *srcPad, const char *sinkElement, const char *sinkPad, GstCaps *filter)
{
	GstElement *source = gst_bin_get_by_name(GST_BIN(bin), srcElement);
	GstElement *sink = gst_bin_get_by_name(GST_BIN(bin), sinkElement);

	GstPad *srcpad = gst_element_get_static_pad(GST_ELEMENT(source), srcPad);
	GstPad *sinkpad = gst_element_get_static_pad(GST_ELEMENT(sink), sinkPad);

	GstPadLinkReturn result;
	if(filter)
		result = gst_element_link_pads_filtered(source, srcPad, sink, sinkPad, filter) ? GST_PAD_LINK_OK : GST_PAD_LINK_REFUSED;
	else
		result = gst_pad_link(srcpad, sinkpad);

#if defined(VERBOSE) || !defined(NDEBUG)
	inspectPad(srcpad, srcElement);
	inspectPad(sinkpad, sinkElement);

	if(filter)
		inspectCaps(filter, "FILTER ");

	if(result == GST_PAD_LINK_OK)
		qDebug() << "successfully linked" << srcElement << srcPad << "to" << sinkElement << sinkPad;
	else
		qDebug() << "error" << result << "linking" << srcElement << srcPad << "to" << sinkElement << sinkPad;
#endif

	gst_object_unref(srcpad);
	gst_object_unref(sinkpad);

	return result;
}

void
GStreamer::freePipeline(GstPipeline **pipeline, GstBus **bus)
{
	if(*bus) {
		gst_object_unref(GST_OBJECT(*bus));
		*bus = NULL;
	}

	if(*pipeline) {
#if defined(VERBOSE) || !defined(NDEBUG)
		qDebug() << "disposing pipeline" << gst_element_get_name(GST_ELEMENT(*pipeline));
#endif

		gst_object_unref(GST_OBJECT(*pipeline));
		*pipeline = NULL;
	}
}

static void
writeTag(const GstTagList *list, const gchar *tag, gpointer userData)
{
	QString &string = *(QString *)userData;

	string += "\n - " + QString(tag) + ": ";
	for(int index = 0, size = /*gst_tag_list_get_tag_size( list, tag ) */ 10; index < size; ++index) {
		const GValue *value = gst_tag_list_get_value_index(list, tag, index);
		if(value) {
			char *strValue = g_strdup_value_contents(value);
			string += strValue;
			string += "; ";
		}
	}
}

void
GStreamer::inspectTags(GstTagList *tags, const QString &prefix)
{
	QString debugString = QString(prefix + "TAGS (%1empty)").arg(gst_tag_list_is_empty(tags) ? "" : "not ");

	gst_tag_list_foreach(tags, writeTag, &debugString);

	qDebug() << debugString;
}

void
GStreamer::inspectPad(GstPad *pad, const QString &prefix)
{
	gchar *padname = gst_pad_get_name(pad);

	QString message = prefix + QString("PAD %1 (%2)")
					   .arg(padname)
					   .arg(gst_pad_get_direction(pad) == GST_PAD_SRC ? "SOURCE" : "SINK");

	qDebug() << message;

	GstCaps *caps = gst_pad_get_current_caps(pad);
	if(caps) {
		inspectCaps(caps, "CURRENT ");
		gst_caps_unref(caps);
		return;
	}

	caps = gst_pad_get_pad_template_caps(pad);
	if(caps) {
		inspectCaps(caps, "TEMPLATE ");
		gst_caps_unref(caps);
	}

	caps = gst_pad_get_allowed_caps(pad);
	if(caps) {
		inspectCaps(caps, "ALLOWED ");
		gst_caps_unref(caps);
	}
}

void
GStreamer::inspectCaps(GstCaps *caps, const QString &prefix)
{
	QString message = prefix + QString("CAPS (%1)")
					   .arg(gst_caps_is_fixed(caps) ? "FIXED" : "NON FIXED");

	gchar *debug = gst_caps_to_string(caps);
	QString token;
	foreach(token, QString(debug).split(';'))
	message += "\n - " + token.trimmed();
	g_free(debug);

	qDebug() << message.trimmed();
}

static QString
state(GstState state)
{
	switch(state) {
	case GST_STATE_VOID_PENDING:
		return "STATE_VOID_PENDING";
	case GST_STATE_NULL:
		return "STATE_NULL";
	case GST_STATE_READY:
		return "STATE_READY";
	case GST_STATE_PAUSED:
		return "STATE_PAUSED";
	case GST_STATE_PLAYING:
		return "STATE_PLAYING";
	default:
		return "STATE_UNKNOWN";
	}
}

void
GStreamer::inspectMessage(GstMessage *msg)
{
	QString data;
	switch(GST_MESSAGE_TYPE(msg)) {
	case GST_MESSAGE_STATE_CHANGED: {
		GstState old, cur, pending;
		gst_message_parse_state_changed(msg, &old, &cur, &pending);
		data = "old:" + state(old) + " | current:" + state(cur) + " | target:" + state(pending);
		break;
	}
	case GST_MESSAGE_ERROR: {
		gchar *debug = NULL;
		GError *error = NULL;
		gst_message_parse_error(msg, &error, &debug);
		data = QString(error ? error->message : "") + " " + QString(debug);
		g_error_free(error);
		g_free(debug);
		break;
	}
	default:
		break;
	}

	QString message = QString("message %1 from %2").arg(GST_MESSAGE_TYPE_NAME(msg)).arg(gst_element_get_name(GST_MESSAGE_SRC(msg)));
	if(!data.isEmpty())
		message += ": " + data;

	qDebug() << message;
}

void
GStreamer::inspectObject(GObject *object)
{
	QString string;
	QTextStream stream(&string);

	guint length;
	GParamSpec **params;

	if(object == NULL)
		return;

	params = g_object_class_list_properties(G_OBJECT_GET_CLASS(G_OBJECT(object)), &length);
	for(guint index = 0; index < length; ++index) {
		gchar *strValue = 0;

		if(params[index]->flags & G_PARAM_READABLE) {
			if(params[index]->value_type != G_TYPE_INVALID) {
				GValue *value = g_new0(GValue, 1);
				g_value_init(value, params[index]->value_type);
				g_object_get_property(G_OBJECT(object), params[index]->name, value);
				strValue = g_strdup_value_contents(value);
				g_value_unset(value);
				g_free(value);
			}
		}

		stream << '\n' << "NAME " << params[index]->name << " | NICK " << g_param_spec_get_nick(params[index])
			   << " | BLURB " << g_param_spec_get_blurb(params[index])
			   << " | TYPE " << g_type_name(params[index]->value_type)
			   << " | FLAGS " << ((params[index]->flags & (G_PARAM_READABLE | G_PARAM_WRITABLE)) == (G_PARAM_READABLE | G_PARAM_WRITABLE) ? "RW" : (params[index]->flags & G_PARAM_READABLE ? "R" : params[index]->flags & G_PARAM_WRITABLE ? "W" : "U")
						   )
			   << " | VALUE " << strValue;

		if(params[index]->flags & G_PARAM_READABLE && strValue)
			g_free(strValue);
	}

	qDebug() << string << '\n';

	g_free(params);
}
