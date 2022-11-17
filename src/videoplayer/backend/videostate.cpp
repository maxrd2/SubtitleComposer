/*
    SPDX-FileCopyrightText: 2003 Fabrice Bellard
    SPDX-FileCopyrightText: 2020-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "videostate.h"

#include <KLocalizedString>
#include "helpers/languagecode.h"
#include "videoplayer/backend/ffplayer.h"

using namespace SubtitleComposer;

VideoState::VideoState()
	: audDec(this),
#ifdef AUDIO_VISUALIZATION
	  sample_array(8 * 65536),
#endif
	  vidDec(this)
{
}

int
VideoState::masterSyncType() {
	if(av_sync_type == AV_SYNC_VIDEO_MASTER)
		return vidStream ? AV_SYNC_VIDEO_MASTER : AV_SYNC_AUDIO_MASTER;
	if(av_sync_type == AV_SYNC_AUDIO_MASTER)
		return audStream ? AV_SYNC_AUDIO_MASTER : AV_SYNC_EXTERNAL_CLOCK;
	return AV_SYNC_EXTERNAL_CLOCK;
}

Clock *
VideoState::masterClock()
{
	switch(masterSyncType()) {
	case AV_SYNC_VIDEO_MASTER: return &vidClk;
	case AV_SYNC_AUDIO_MASTER: return &audClk;
	default: return &extClk;
	}
}


double
VideoState::masterTime()
{
	switch(masterSyncType()) {
	case AV_SYNC_VIDEO_MASTER: return vidClk.get();
	case AV_SYNC_AUDIO_MASTER: return audClk.get();
	default: return extClk.get();
	}
}

void
VideoState::checkExternalClockSpeed()
{
	if((vidStreamIdx >= 0 && vidPQ.nbPackets() <= EXTERNAL_CLOCK_MIN_FRAMES) || (audStreamIdx >= 0 && audPQ.nbPackets() <= EXTERNAL_CLOCK_MIN_FRAMES)) {
		extClk.setSpeed(FFMAX(EXTERNAL_CLOCK_SPEED_MIN, extClk.speed() - EXTERNAL_CLOCK_SPEED_STEP));
	} else if((vidStreamIdx < 0 || vidPQ.nbPackets() > EXTERNAL_CLOCK_MAX_FRAMES) && (audStreamIdx < 0 || audPQ.nbPackets() > EXTERNAL_CLOCK_MAX_FRAMES)) {
		extClk.setSpeed(FFMIN(EXTERNAL_CLOCK_SPEED_MAX, extClk.speed() + EXTERNAL_CLOCK_SPEED_STEP));
	} else {
		const double speed = extClk.speed();
		if(speed != 1.0)
			extClk.setSpeed(speed + EXTERNAL_CLOCK_SPEED_STEP * (1.0 - speed) / fabs(1.0 - speed));
	}
}

void
VideoState::notifyState()
{
	if(abortRequested)
		emit player->stateChanged(FFPlayer::Stopped);
	else if(paused || step)
		emit player->stateChanged(FFPlayer::Paused);
	else
		emit player->stateChanged(FFPlayer::Playing);
}

void
VideoState::notifySpeed()
{
	const double speed = audDec.pitch();
	masterClock()->setSpeed(speed);
	emit player->speedChanged(speed);
}

void
VideoState::notifyLoaded()
{
	emit player->mediaLoaded();

	const AVStream *st = audStream ? audStream : vidStream;
	const double streamDuration = double(st->duration) * st->time_base.num / st->time_base.den;
	const double containerDuration = double(fmtContext->duration) / AV_TIME_BASE;
	emit player->durationChanged(qMax(streamDuration, containerDuration));

	notifySpeed();
	emit player->volumeChanged(player->volume());
	emit player->muteChanged(player->muted());

	QStringList audioStreams, videoStreams, subtitleStreams;
	const AVCodecDescriptor *desc;
	for(unsigned int i = 0; i < fmtContext->nb_streams; i++) {
		AVStream *stream = fmtContext->streams[i];
		QString *streamName;

		switch(stream->codecpar->codec_type) {
		case AVMEDIA_TYPE_VIDEO:
			videoStreams.append(i18n("Video Stream #%1", videoStreams.size() + 1));
			streamName = &videoStreams.last();
			break;
		case AVMEDIA_TYPE_AUDIO:
			audioStreams.append(i18n("Audio Stream #%1", audioStreams.size() + 1));
			streamName = &audioStreams.last();
			break;
		case AVMEDIA_TYPE_SUBTITLE:
			desc = avcodec_descriptor_get(stream->codecpar->codec_id);
			if(desc && (desc->props & AV_CODEC_PROP_TEXT_SUB)) {
				subtitleStreams.append(i18n("Subtitle Stream #%1", subtitleStreams.size() + 1));
				streamName = &subtitleStreams.last();
			} else {
				streamName = nullptr;
			}
			break;
		default:
			streamName = nullptr;
			break;
		}
		if(!streamName)
			continue;

		*streamName += QStringLiteral(": ");
		AVDictionaryEntry *tag = av_dict_get(stream->metadata, "lang", nullptr, AV_DICT_IGNORE_SUFFIX);
		*streamName += tag ? QString("%2 (%3)").arg(LanguageCode::nameFromIso(tag->value)).arg(tag->value) : QStringLiteral("Unknown");

		if((tag = av_dict_get(stream->metadata, "title", nullptr, 0)) != nullptr)
			*streamName += QStringLiteral(": ") + QString::fromUtf8(tag->value);

		*streamName += QStringLiteral(" [") + QString::fromUtf8(avcodec_descriptor_get(stream->codecpar->codec_id)->name) + QStringLiteral("]");
//		if(stream->codecpar->extradata)
//			QByteArray((const char*)stream->codecpar->extradata, stream->codecpar->extradata_size);
	}
	emit player->videoStreamsChanged(videoStreams);
	emit player->audioStreamsChanged(audioStreams);
	emit player->subtitleStreamsChanged(subtitleStreams);

	notifyState();
}
