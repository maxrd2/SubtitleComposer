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

#include "mpvconfigwidget.h"

#include <locale>
#include <mpv/client.h>

#include "scconfig.h"

using namespace SubtitleComposer;

MPVConfigWidget::MPVConfigWidget(QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);

	// FIXME: libmpv requires LC_NUMERIC category to be set to "C".. is there some nicer way to do this?
	std::setlocale(LC_NUMERIC, "C");
	m_mpv = mpv_create();
	mpv_request_log_messages(m_mpv, "info");
	if(mpv_initialize(m_mpv) >= 0) {
		getHelpResponse(); // make sure there are no log messages
		static QStringList bad = {
			QStringLiteral("libmpv"),
			QStringLiteral("null"),
			QStringLiteral("image"),
			QStringLiteral("tct"),
			QStringLiteral("caca"),
			QStringLiteral("pcm"),
		};

		mpv_set_property_string(m_mpv, "vo", "help");
		for(QString row : getHelpResponse()) {
			int pos = row.indexOf(QChar(' '));
			if(pos == -1)
				continue;
			const QString name = row.left(pos);
			if(bad.contains(name))
				continue;
			row.insert(pos, "\t-");
			if(SCConfig::mpvVideoOutput() == name)
				kcfg_mpvVideoOutput->setCurrentIndex(kcfg_mpvHwDecode->count());
			kcfg_mpvVideoOutput->addItem(row, name);
		}
		kcfg_mpvVideoOutput->setProperty("kcfg_property", QByteArray("currentData"));

		mpv_set_property_string(m_mpv, "hwdec", "help");
		kcfg_mpvHwDecode->addItem(QStringLiteral("auto\t- Choose best HW decoder"), QStringLiteral("auto"));
		for(QString row : getHelpResponse()) {
			int pos = row.indexOf(QChar(' '));
			if(pos == -1)
				continue;
			const QString name = row.left(pos);
			const QString lastName = kcfg_mpvHwDecode->itemData(kcfg_mpvHwDecode->count() - 1).toString();
			if(lastName == name || bad.contains(name))
				continue;
			if(SCConfig::mpvHwDecode() == name)
				kcfg_mpvHwDecode->setCurrentIndex(kcfg_mpvHwDecode->count());
			kcfg_mpvHwDecode->addItem(name, name);
		}
		kcfg_mpvHwDecode->setProperty("kcfg_property", QByteArray("currentData"));

		mpv_set_property_string(m_mpv, "ao", "help");
		for(QString row : getHelpResponse()) {
			int pos = row.indexOf(QChar(' '));
			if(pos == -1)
				continue;
			const QString name = row.left(pos);
			if(bad.contains(name))
				continue;
			row.insert(pos, "\t-");
			if(SCConfig::mpvAudioOutput() == name)
				kcfg_mpvAudioOutput->setCurrentIndex(kcfg_mpvHwDecode->count());
			kcfg_mpvAudioOutput->addItem(row, name);
		}
		kcfg_mpvAudioOutput->setProperty("kcfg_property", QByteArray("currentData"));

		mpv_detach_destroy(m_mpv);
	} else {
		kcfg_mpvVideoOutput->addItems(QString("vdpau vaapi opengl opengl-hq sdl xv wayland x11 null").split(' '));
		kcfg_mpvVideoOutput->setProperty("kcfg_property", QByteArray("currentText"));

		kcfg_mpvHwDecode->addItems(QString("auto vdpau vaapi vaapi-copy").split(' '));
		kcfg_mpvHwDecode->setProperty("kcfg_property", QByteArray("currentText"));

		kcfg_mpvAudioOutput->addItems(QString("pulse alsa oss portaudio jack null").split(' '));
		kcfg_mpvAudioOutput->setProperty("kcfg_property", QByteArray("currentText"));
	}
}

const QStringList
MPVConfigWidget::getHelpResponse()
{
	QStringList res;
	while(m_mpv) {
		mpv_event *event = mpv_wait_event(m_mpv, .1);
		if(event->event_id == MPV_EVENT_LOG_MESSAGE) {
			mpv_event_log_message *msg = reinterpret_cast<mpv_event_log_message *>(event->data);
			if(msg->log_level == MPV_LOG_LEVEL_INFO && strcmp(msg->prefix, "cplayer") == 0) {
				QString row = QString::fromUtf8(msg->text).simplified();
				if(row.endsWith(QChar(':')))
					continue;
				res << row;
			}
		} else if(event->event_id == MPV_EVENT_NONE) {
			break;
		}
	}
	return res;
}

MPVConfigWidget::~MPVConfigWidget()
{

}
