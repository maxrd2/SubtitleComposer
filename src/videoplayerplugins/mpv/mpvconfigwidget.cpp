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
#include "mpvconfig.h"

#include <locale>
#include <mpv/client.h>

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

		mpv_set_property_string(m_mpv, "hwdec", "help");
		kcfg_hwDecode->addItem(QStringLiteral("auto\t- Choose best HW decoder"), QStringLiteral("auto"));
		for(QString row : getHelpResponse()) {
			int pos = row.indexOf(QChar(' '));
			if(pos == -1)
				continue;
			const QString name = row.left(pos);
			const QString lastName = kcfg_hwDecode->itemData(kcfg_hwDecode->count() - 1).toString();
			if(lastName == name || bad.contains(name))
				continue;
			kcfg_hwDecode->addItem(name, name);
			if(MPVConfig::hwDecode() == name)
				kcfg_hwDecode->setCurrentIndex(kcfg_hwDecode->count() - 1);
		}
		kcfg_hwDecode->setProperty("kcfg_property", QByteArray("currentData"));

		mpv_set_property_string(m_mpv, "ao", "help");
		for(QString row: getHelpResponse()) {
			int pos = row.indexOf(QChar(' '));
			if(pos == -1)
				continue;
			const QString name = row.left(pos);
			if(bad.contains(name))
				continue;
			row.insert(pos, "\t-");
			kcfg_audioOutput->addItem(row, name);
			if(MPVConfig::audioOutput() == name)
				kcfg_audioOutput->setCurrentIndex(kcfg_audioOutput->count() - 1);
		}
		kcfg_audioOutput->setProperty("kcfg_property", QByteArray("currentData"));

		mpv_detach_destroy(m_mpv);
	} else {
		kcfg_hwDecode->addItems(QString("auto vdpau vaapi vaapi-copy").split(' '));
		kcfg_hwDecode->setProperty("kcfg_property", QByteArray("currentText"));

		kcfg_audioOutput->addItems(QString("pulse alsa oss portaudio jack null").split(' '));
		kcfg_audioOutput->setProperty("kcfg_property", QByteArray("currentText"));
	}

#ifndef MPV_HAS_RENDER_API
	kcfg_renderMethod->setCurrentIndex(0);
	kcfg_renderMethod->setEnabled(false);
#endif
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
