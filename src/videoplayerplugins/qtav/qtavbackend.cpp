/*
 * Copyright (C) 2010-2020 Mladen Milinkovic <max@smoothware.net>
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

#include "qtavbackend.h"
#include "qtavconfig.h"

#include <QtAVWidgets>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>

using namespace SubtitleComposer;

QtAVBackend::QtAVBackend()
	: PlayerBackend(),
	  m_state(STOPPED),
	  m_renderer(new QtAV::VideoOutput(QtAV::VideoRendererId_Widget/*VideoRendererId_GLWidget2*/, this)),
	  m_player(new QtAV::AVPlayer(this))
{
	m_name = QStringLiteral("QtAV");

	m_player->setRenderer(m_renderer);
	m_player->setNotifyInterval(10);

	connect(m_renderer, &QtAV::VideoOutput::videoFrameSizeChanged, this, [&](){
		QSize fs = m_renderer->videoFrameSize();
		emit resolutionChanged(fs.width(), fs.height(), m_renderer->sourceAspectRatio());
	});
	connect(m_player, &QtAV::AVPlayer::loaded, this, [&](){
		emit fpsChanged(m_player->statistics().video.frame_rate);
	});
	connect(m_player->audio(), &QtAV::AudioOutput::volumeChanged, this, [&](qreal volume){
		emit volumeChanged(volume * 100.);
	});
	connect(m_player->audio(), &QtAV::AudioOutput::muteChanged, this, [&](bool muted){
		emit muteChanged(muted);
	});
	connect(m_player, &QtAV::AVPlayer::positionChanged, this, [&](qint64 pos){
		emit positionChanged(pos / 1000.);
	});
	connect(m_player, &QtAV::AVPlayer::durationChanged, this, [&](qint64 duration){
		emit lengthChanged(duration / 1000.);
	});
	connect(m_player, &QtAV::AVPlayer::speedChanged, this, [&](qreal speed){
		emit speedChanged(speed);
	});

	connect(m_player, &QtAV::AVPlayer::stateChanged, this, [&](QtAV::AVPlayer::State state){
		switch(state) {
		case QtAV::AVPlayer::StoppedState:
			setState(STOPPED);
			emit speedChanged(0.);
			break;
		case QtAV::AVPlayer::PlayingState:
			setState(PLAYING);
			emit speedChanged(m_player->speed());
			break;
		case QtAV::AVPlayer::PausedState:
			setState(PAUSED);
			break;
		}
	});

	setState(STOPPED);
}

QtAVBackend::~QtAVBackend()
{
	cleanup();
}

bool
QtAVBackend::init(QWidget *videoWidget)
{
	if(!m_renderer->widget()) {
		qWarning() << "Cannot create QtAV video renderer";
		return false;
	}
	m_renderer->widget()->setAttribute(Qt::WA_DontCreateNativeAncestors);
	m_renderer->widget()->setAttribute(Qt::WA_NativeWindow);
	m_renderer->widget()->setParent(videoWidget);
	return true;
}

void
QtAVBackend::cleanup()
{
	m_renderer->widget()->setParent(nullptr);
	setState(STOPPED);
}

void
QtAVBackend::setState(PlayState state)
{
	static VideoPlayer::State vpState[] = { VideoPlayer::Ready, VideoPlayer::Paused, VideoPlayer::Playing };
	if(m_state == state)
		return;
	m_state = state;
	emit stateChanged(vpState[state]);
}

bool
QtAVBackend::openFile(const QString &path)
{
	m_player->setFile(path);
	return m_player->load();
}

bool
QtAVBackend::closeFile()
{
	stop();
	m_currentFilePath.clear();
	return true;
}

bool
QtAVBackend::stop()
{
	m_player->stop();
	return true;
}

bool
QtAVBackend::play()
{
	if(m_state == PAUSED)
		m_player->togglePause();
	else
		m_player->play();
	return true;
}

bool
QtAVBackend::pause()
{
	m_player->togglePause();
	return true;
}

bool
QtAVBackend::seek(double seconds)
{
	m_player->setPosition(seconds * 1000);
	return true;
}

bool
QtAVBackend::step(int frameOffset)
{
	for(int i = 0, n = qAbs(frameOffset); i < n; i++) {
		if(frameOffset > 0)
			m_player->stepForward();
		else
			m_player->stepBackward();
	}
	return true;
}

bool
QtAVBackend::playbackRate(double newRate)
{
	m_player->setSpeed(newRate);
	return true;
}

bool
QtAVBackend::selectAudioStream(int streamIndex)
{
	m_player->setAudioStream(QString(), streamIndex);
	return true;
}

bool
QtAVBackend::setVolume(double volume)
{
	m_player->audio()->setVolume(volume / 100.);
	return true;
}

bool
QtAVBackend::reconfigure()
{
	return true;
}

/*
void
QTAVBackend::notifyTextStreams(const qtav_event_property *prop)
{
	QStringList textStreams;
	if(prop->format == QTAV_FORMAT_NODE) {
		const qtav_node *node = (qtav_node *)prop->data;
		if(node->format == QTAV_FORMAT_NODE_ARRAY) {
			for(int i = 0; i < node->u.list->num; i++) {
				const qtav_node &val = node->u.list->values[i];
				if(val.format != QTAV_FORMAT_NODE_MAP)
					continue;

				const QMap<QString, QVariant> &map = node_to_variant(&val).toMap();

				if(map[QStringLiteral("type")].toString() != QStringLiteral("sub")
				|| map[QStringLiteral("external")].toBool() == true)
					continue;

				const QString &codec = map[QStringLiteral("codec")].toString();
				if(codec != QStringLiteral("mov_text") && codec != QStringLiteral("subrip"))
					continue;

				const int &id = map[QStringLiteral("id")].toInt();
				const QString &lang = map[QStringLiteral("lang")].toString();
				const QString &title = map[QStringLiteral("title")].toString();

				QString textStreamName = i18n("Text Stream #%1", id);
				if(!lang.isEmpty() && lang != QStringLiteral("und"))
					textStreamName += QStringLiteral(": ") + lang;
				if(!title.isEmpty())
					textStreamName += QStringLiteral(": ") + title;

				textStreams << textStreamName;
			}
		}
	}
	emit textStreamsChanged(textStreams);
}

void
QTAVBackend::notifyAudioStreams(const qtav_event_property *prop)
{
	QStringList audioStreams;
	if(prop->format == QTAV_FORMAT_NODE) {
		const qtav_node *node = (qtav_node *)prop->data;
		if(node->format == QTAV_FORMAT_NODE_ARRAY) {
			for(int i = 0; i < node->u.list->num; i++) {
				const qtav_node &val = node->u.list->values[i];
				if(val.format != QTAV_FORMAT_NODE_MAP)
					continue;

				const QMap<QString, QVariant> &map = node_to_variant(&val).toMap();

				if(map[QStringLiteral("type")].toString() != QStringLiteral("audio"))
					continue;

				const int &id = map[QStringLiteral("id")].toInt();
				const QString &lang = map[QStringLiteral("lang")].toString();
				const QString &title = map[QStringLiteral("title")].toString();
				const QString &codec = map[QStringLiteral("codec")].toString();

				QString audioStreamName = i18n("Audio Stream #%1", id);
				if(!lang.isEmpty() && lang != QStringLiteral("und"))
					audioStreamName += QStringLiteral(": ") + lang;
				if(!title.isEmpty())
					audioStreamName += QStringLiteral(": ") + title;
				if(!codec.isEmpty())
					audioStreamName += QStringLiteral(" [") + codec + QStringLiteral("]");

				audioStreams << audioStreamName;
			}
		}
	}
	emit audioStreamsChanged(audioStreams, audioStreams.isEmpty() ? -1 : 0);
}
*/
