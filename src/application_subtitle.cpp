/*
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

#include "application.h"
#include "actions/kcodecactionext.h"
#include "actions/krecentfilesactionext.h"
#include "actions/useractionnames.h"
#include "dialogs/opensubtitledialog.h"
#include "dialogs/savesubtitledialog.h"
#include "formats/inputformat.h"
#include "formats/formatmanager.h"
#include "formats/textdemux/textdemux.h"
#include "formats/outputformat.h"
#include "helpers/commondefs.h"
#include "lineswidget.h"
#include "speechprocessor/speechprocessor.h"
#include "videoplayer/videoplayer.h"

#include <QFileDialog>
#include <QStringBuilder>
#include <QTextCodec>

#include <KCharsets>
#include <KCodecAction>
#include <KLocalizedString>
#include <KMessageBox>

#define $(x) QStringLiteral(x)

using namespace SubtitleComposer;

static const QStringList videoExtensionList = {
	$("avi"), $("divx"), $("flv"), $("m2ts"), $("mkv"), $("mov"), $("mp4"), $("mpeg"),
	$("mpg"), $("ogm"), $("ogv"), $("rmvb"), $("ts"), $("vob"), $("webm"), $("wmv"),
};

static const QStringList audioExtensionList = {
	$("aac"), $("ac3"), $("ape"), $("flac"), $("la"), $("m4a"), $("mac"), $("mp+"),
	$("mp2"), $("mp3"), $("mp4"), $("mpc"), $("mpp"), $("ofr"), $("oga"), $("ogg"),
	$("pac"), $("ra"), $("spx"), $("tta"), $("wav"), $("wma"), $("wv"),
};


const QString &
Application::buildMediaFilesFilter()
{
	static QString filter;

	if(filter.isEmpty()) {
		QString videoExtensions;
		foreach(const QString ext, videoExtensionList)
			videoExtensions += $(" *.") % ext;

		QString audioExtensions;
		foreach(const QString ext, audioExtensionList)
			audioExtensions += $(" *.") % ext;

		filter = i18n("Media Files") % $(" (") % videoExtensions.midRef(1) % audioExtensions % $(")\n")
			% i18n("Video Files") % $(" (") % videoExtensions.midRef(1) % $(")\n")
			% i18n("Audio Files") % $(" (") % audioExtensions.midRef(1) % $(")\n")
			% i18n("All Files") % $(" (*)");
	}

	return filter;
}

QTextCodec *
Application::codecForEncoding(const QString &encoding)
{
	if(encoding.isEmpty())
		return nullptr;

	bool codecFound = false;
	QTextCodec *codec = KCharsets::charsets()->codecForName(encoding, codecFound);
	if(!codecFound)
		return nullptr;

	return codec;
}

bool
Application::acceptClashingUrls(const QUrl &subtitleUrl, const QUrl &subtitleTrUrl)
{
	if(subtitleUrl != subtitleTrUrl || subtitleUrl.isEmpty() || subtitleTrUrl.isEmpty())
		return true;

	return KMessageBox::Continue == KMessageBox::warningContinueCancel(
		m_mainWindow,
		i18n("The requested action would make the subtitle and its translation share the same file, "
			 "possibly resulting in loss of information when saving.\n"
			 "Are you sure you want to continue?"),
		i18n("Conflicting Subtitle Files"));
}

void
Application::newSubtitle()
{
	if(!closeSubtitle())
		return;

	m_subtitle = new Subtitle();

	emit subtitleOpened(m_subtitle);

	connect(m_subtitle, SIGNAL(primaryDirtyStateChanged(bool)), this, SLOT(updateTitle()));
	connect(m_subtitle, SIGNAL(secondaryDirtyStateChanged(bool)), this, SLOT(updateTitle()));

	updateTitle();
}

void
Application::openSubtitle()
{
	OpenSubtitleDialog openDlg(true, m_lastSubtitleUrl, QString(), m_mainWindow);

	if(openDlg.exec() == QDialog::Accepted) {
		if(!acceptClashingUrls(openDlg.selectedUrl(), m_subtitleTrUrl))
			return;

		if(!closeSubtitle())
			return;

		m_lastSubtitleUrl = openDlg.selectedUrl();
		m_recentSubtitlesAction->addUrl(m_lastSubtitleUrl, openDlg.selectedEncoding());
		openSubtitle(m_lastSubtitleUrl);
	}
}

void
Application::openSubtitle(const QUrl &url, bool warnClashingUrls)
{
	if(warnClashingUrls && !acceptClashingUrls(url, m_subtitleTrUrl))
		return;

	if(!closeSubtitle())
		return;

	QTextCodec *codec = codecForEncoding(KRecentFilesActionExt::encodingForUrl(url));

	m_subtitle = new Subtitle();

	if(FormatManager::instance().readSubtitle(*m_subtitle, true, url, &codec, &m_subtitleFormat)) {
		// The loading of the subtitle shouldn't be an undoable action as there's no state before it
		m_undoStack->clear();
		m_subtitle->clearPrimaryDirty();
		m_subtitle->clearSecondaryDirty();

		emit subtitleOpened(m_subtitle);

		m_subtitleUrl = url;
		m_subtitleFileName = QFileInfo(m_subtitleUrl.path()).fileName();

		if(codec) {
			m_subtitleEncoding = codec->name();
			m_reopenSubtitleAsAction->setCurrentCodec(codec);
		} else {
			m_subtitleEncoding = SCConfig::defaultSubtitlesEncoding();
		}

		m_recentSubtitlesAction->addUrl(url, m_subtitleEncoding);

		connect(m_subtitle, SIGNAL(primaryDirtyStateChanged(bool)), this, SLOT(updateTitle()));
		connect(m_subtitle, SIGNAL(secondaryDirtyStateChanged(bool)), this, SLOT(updateTitle()));

		updateTitle();

		if(m_subtitleUrl.isLocalFile() && SCConfig::automaticVideoLoad()) {
			QFileInfo subtitleFileInfo(m_subtitleUrl.toLocalFile());

			QString subtitleFileName = m_subtitleFileName.toLower();
			QString videoFileName = QFileInfo(m_player->filePath()).completeBaseName().toLower();

			if(videoFileName.isEmpty() || subtitleFileName.indexOf(videoFileName) != 0) {
				QStringList subtitleDirFiles = subtitleFileInfo.dir().entryList(QDir::Files | QDir::Readable);
				for(QStringList::ConstIterator it = subtitleDirFiles.begin(), end = subtitleDirFiles.end(); it != end; ++it) {
					QFileInfo fileInfo(*it);
					if(videoExtensionList.contains(fileInfo.suffix().toLower())) {
						if(subtitleFileName.indexOf(fileInfo.completeBaseName().toLower()) == 0) {
							QUrl auxUrl;
							auxUrl.setScheme("file");
							auxUrl.setPath(subtitleFileInfo.dir().filePath(*it));
							openVideo(auxUrl);
							break;
						}
					}
				}
			}
		}
	} else {
		delete m_subtitle;
		m_subtitle = 0;

		KMessageBox::sorry(m_mainWindow, i18n("<qt>Could not parse the subtitle file.<br/>This may have been caused by usage of the wrong encoding.</qt>"));
	}
}

void
Application::reopenSubtitleWithCodecOrDetectScript(QTextCodec *codec)
{
	if(m_subtitleUrl.isEmpty())
		return;

	Subtitle *subtitle = new Subtitle();
	QString subtitleFormat;

	if(!FormatManager::instance().readSubtitle(*subtitle, true, m_subtitleUrl, &codec, &subtitleFormat)) {
		delete subtitle;
		KMessageBox::sorry(m_mainWindow, i18n("<qt>Could not parse the subtitle file.<br/>This may have been caused by usage of the wrong encoding.</qt>"));
		return;
	}

	emit subtitleClosed();

	if(m_translationMode) {
		subtitle->setSecondaryData(*m_subtitle, false);
	}

	delete m_subtitle;
	m_subtitle = subtitle;

	// The loading of the subtitle shouldn't be an undoable action as there's no state before it
	m_undoStack->clear();
	m_subtitle->clearPrimaryDirty();
	m_subtitle->clearSecondaryDirty();

	emit subtitleOpened(m_subtitle);

	m_subtitleEncoding = codec->name();
	m_subtitleFormat = subtitleFormat;

	m_reopenSubtitleAsAction->setCurrentCodec(codec);

	m_recentSubtitlesAction->addUrl(m_subtitleUrl, m_subtitleEncoding);

	connect(m_subtitle, SIGNAL(primaryDirtyStateChanged(bool)), this, SLOT(updateTitle()));
	connect(m_subtitle, SIGNAL(secondaryDirtyStateChanged(bool)), this, SLOT(updateTitle()));
}

void
Application::demuxTextStream(int textStreamIndex)
{
	if(!closeSubtitle())
		return;

	newSubtitle();

	m_textDemux->demuxFile(m_subtitle, m_player->filePath(), textStreamIndex);
}

void
Application::speechImportAudioStream(int audioStreamIndex)
{
	if(!closeSubtitle())
		return;

	newSubtitle();

	m_speechProcessor->setSubtitle(m_subtitle);
	m_speechProcessor->setAudioStream(m_player->filePath(), audioStreamIndex);
}

bool
Application::saveSubtitle()
{
	if(m_subtitleUrl.isEmpty() || m_subtitleEncoding.isEmpty() || !FormatManager::instance().hasOutput(m_subtitleFormat))
		return saveSubtitleAs();

	bool codecFound = true;
	QTextCodec *codec = KCharsets::charsets()->codecForName(m_subtitleEncoding, codecFound);
	if(!codecFound)
		codec = QTextCodec::codecForLocale();

	if(FormatManager::instance().writeSubtitle(*m_subtitle, true, m_subtitleUrl, codec, m_subtitleFormat, true)) {
		m_subtitle->clearPrimaryDirty();

		m_reopenSubtitleAsAction->setCurrentCodec(codec);
		m_recentSubtitlesAction->addUrl(m_subtitleUrl, codec->name());

		updateTitle();

		return true;
	} else {
		KMessageBox::sorry(m_mainWindow, i18n("There was an error saving the subtitle."));
		return false;
	}
}

bool
Application::saveSubtitleAs()
{
	SaveSubtitleDialog saveDlg(
		true,
		m_subtitleUrl,
		m_subtitleEncoding.isEmpty() ? SCConfig::defaultSubtitlesEncoding() : m_subtitleEncoding,
		Format::CurrentOS,
		m_subtitleFormat,
		m_mainWindow);

	if(saveDlg.exec() == QDialog::Accepted) {
		if(!acceptClashingUrls(saveDlg.selectedUrl(), m_subtitleTrUrl))
			return false;

		m_subtitleUrl = saveDlg.selectedUrl();
		m_subtitleFileName = QFileInfo(m_subtitleUrl.path()).completeBaseName();
		m_subtitleEncoding = saveDlg.selectedEncoding();
		m_subtitleFormat = saveDlg.selectedFormat();

		return saveSubtitle();
	}

	return false;
}

bool
Application::closeSubtitle()
{
	if(m_subtitle) {
		if(m_translationMode && m_subtitle->isSecondaryDirty()) {
			int result = KMessageBox::warningYesNoCancel(nullptr,
							i18n("Currently opened translation subtitle has unsaved changes.\nDo you want to save them?"),
							i18n("Close Translation Subtitle") + " - SubtitleComposer");
			if(result == KMessageBox::Cancel)
				return false;
			else if(result == KMessageBox::Yes)
				if(!saveSubtitleTr())
					return false;
		}

		if(m_subtitle->isPrimaryDirty()) {
			int result = KMessageBox::warningYesNoCancel(nullptr,
							i18n("Currently opened subtitle has unsaved changes.\nDo you want to save them?"),
							i18n("Close Subtitle") + " - SubtitleComposer");
			if(result == KMessageBox::Cancel)
				return false;
			else if(result == KMessageBox::Yes)
				if(!saveSubtitle())
					return false;
		}

		if(m_translationMode) {
			m_translationMode = false;
			emit translationModeChanged(false);
		}

		m_undoStack->clear();

		emit subtitleClosed();

		delete m_subtitle;
		m_subtitle = 0;

		m_subtitleUrl = QUrl();
		m_subtitleFileName.clear();
		m_subtitleEncoding.clear();
		m_subtitleFormat.clear();

		m_subtitleTrUrl = QUrl();
		m_subtitleTrFileName.clear();
		m_subtitleTrEncoding.clear();
		m_subtitleTrFormat.clear();

		updateTitle();
	}

	return true;
}

void
Application::newSubtitleTr()
{
	if(!closeSubtitleTr())
		return;

	m_translationMode = true;
	emit translationModeChanged(true);

	updateTitle();
}

void
Application::openSubtitleTr()
{
	if(!m_subtitle)
		return;

	OpenSubtitleDialog openDlg(false, m_lastSubtitleUrl, QString(), m_mainWindow);

	if(openDlg.exec() == QDialog::Accepted) {
		if(!acceptClashingUrls(m_subtitleUrl, openDlg.selectedUrl()))
			return;

		if(!closeSubtitleTr())
			return;

		m_lastSubtitleUrl = openDlg.selectedUrl();
		m_recentSubtitlesTrAction->addUrl(m_lastSubtitleUrl, openDlg.selectedEncoding());
		openSubtitleTr(m_lastSubtitleUrl);
	}
}

void
Application::openSubtitleTr(const QUrl &url, bool warnClashingUrls)
{
	if(!m_subtitle)
		return;

	if(warnClashingUrls && !acceptClashingUrls(m_subtitleUrl, url))
		return;

	if(!closeSubtitleTr())
		return;

	QTextCodec *codec = codecForEncoding(KRecentFilesActionExt::encodingForUrl(url));

	if(FormatManager::instance().readSubtitle(*m_subtitle, false, url, &codec, &m_subtitleTrFormat)) {
		m_subtitleTrUrl = url;
		m_subtitleTrFileName = QFileInfo(m_subtitleTrUrl.path()).fileName();
		m_subtitleTrEncoding = codec->name();

		m_recentSubtitlesTrAction->addUrl(m_subtitleTrUrl, m_subtitleTrEncoding);

		QStringList subtitleStreams;
		subtitleStreams.append(i18nc("@item:inmenu Display primary text in video player", "Primary"));
		subtitleStreams.append(i18nc("@item:inmenu Display translation text in video player", "Translation"));
		KSelectAction *activeSubtitleStreamAction = (KSelectAction *)action(ACT_SET_ACTIVE_SUBTITLE_STREAM);
		activeSubtitleStreamAction->setItems(subtitleStreams);
		activeSubtitleStreamAction->setCurrentItem(0);

		m_translationMode = true;

		updateTitle();
	} else
		KMessageBox::sorry(m_mainWindow, i18n("<qt>Could not parse the subtitle file.<br/>This may have been caused by usage of the wrong encoding.</qt>"));

	// After the loading of the translation subtitle we must clear the history or (from
	// a user POV) it would be possible to execute (undo) actions which would result in an
	// unexpected state.
	m_undoStack->clear();
	// We don't clear the primary dirty state because the loading of the translation
	// only changes it when actually needed (i.e., when the translation had more lines)
	m_subtitle->clearSecondaryDirty();

	if(m_translationMode)
		emit translationModeChanged(true);
}

void
Application::reopenSubtitleTrWithCodecOrDetectScript(QTextCodec *codec)
{
	if(m_subtitleTrUrl.isEmpty())
		return;

	Subtitle *subtitleTr = new Subtitle();
	QString subtitleTrFormat;

	if(!FormatManager::instance().readSubtitle(*subtitleTr, false, m_subtitleTrUrl, &codec, &subtitleTrFormat)) {
		delete subtitleTr;
		KMessageBox::sorry(m_mainWindow, i18n("<qt>Could not parse the subtitle file.<br/>This may have been caused by usage of the wrong encoding.</qt>"));
		return;
	}

	emit subtitleClosed();

	subtitleTr->setPrimaryData(*m_subtitle, true);

	delete m_subtitle;
	m_subtitle = subtitleTr;

	// After the loading of the translation subtitle we must clear the history or (from
	// a user POV) it would be possible to execute (undo) actions which would result in an
	// unexpected state.
	m_undoStack->clear();
	// We don't clear the primary dirty state because the loading of the translation
	// only changes it when actually needed (i.e., when the translation had more lines)
	m_subtitle->clearPrimaryDirty();
	m_subtitle->clearSecondaryDirty();

	emit subtitleOpened(m_subtitle);

	m_subtitleTrEncoding = codec->name();
	m_subtitleTrFormat = subtitleTrFormat;

	m_reopenSubtitleTrAsAction->setCurrentCodec(codec);

	m_recentSubtitlesTrAction->addUrl(m_subtitleTrUrl, m_subtitleTrEncoding);

	connect(m_subtitle, SIGNAL(primaryDirtyStateChanged(bool)), this, SLOT(updateTitle()));
	connect(m_subtitle, SIGNAL(secondaryDirtyStateChanged(bool)), this, SLOT(updateTitle()));
}

bool
Application::saveSubtitleTr()
{
	if(m_subtitleTrUrl.isEmpty() || m_subtitleTrEncoding.isEmpty() || !FormatManager::instance().hasOutput(m_subtitleTrFormat))
		return saveSubtitleTrAs();

	bool codecFound = true;
	QTextCodec *codec = KCharsets::charsets()->codecForName(m_subtitleTrEncoding, codecFound);
	if(!codecFound)
		codec = QTextCodec::codecForLocale();

	if(FormatManager::instance().writeSubtitle(*m_subtitle, false, m_subtitleTrUrl, codec, m_subtitleTrFormat, true)) {
		m_subtitle->clearSecondaryDirty();

		m_recentSubtitlesTrAction->addUrl(m_subtitleTrUrl, codec->name());

		updateTitle();

		return true;
	} else {
		KMessageBox::sorry(m_mainWindow, i18n("There was an error saving the translation subtitle."));
		return false;
	}
}

bool
Application::saveSubtitleTrAs()
{
	SaveSubtitleDialog saveDlg(
		false,
		m_subtitleTrUrl,
		m_subtitleTrEncoding.isEmpty() ? SCConfig::defaultSubtitlesEncoding() : m_subtitleTrEncoding,
		Format::CurrentOS,
		m_subtitleTrFormat,
		m_mainWindow);

	if(saveDlg.exec() == QDialog::Accepted) {
		if(!acceptClashingUrls(m_subtitleUrl, saveDlg.selectedUrl()))
			return false;

		m_subtitleTrUrl = saveDlg.selectedUrl();
		m_subtitleTrFileName = QFileInfo(m_subtitleTrUrl.path()).completeBaseName();
		m_subtitleTrEncoding = saveDlg.selectedEncoding();
		m_subtitleTrFormat = saveDlg.selectedFormat();

		return saveSubtitleTr();
	}

	return false;
}

bool
Application::closeSubtitleTr()
{
	if(m_subtitle && m_translationMode) {
		if(m_translationMode && m_subtitle->isSecondaryDirty()) {
			int result = KMessageBox::warningYesNoCancel(nullptr,
					i18n("Currently opened translation subtitle has unsaved changes.\nDo you want to save them?"),
					i18n("Close Translation Subtitle") + " - SubtitleComposer");
			if(result == KMessageBox::Cancel)
				return false;
			else if(result == KMessageBox::Yes)
				if(!saveSubtitleTr())
					return false;
		}

		m_translationMode = false;
		emit translationModeChanged(false);

		updateTitle();

		m_linesWidget->setUpdatesEnabled(false);

		// The cleaning of the translations texts shouldn't be an undoable action
//		QUndoStack *savedStack = m_undoStack;
//		m_undoStack = new QUndoStack();
		m_subtitle->clearSecondaryTextData();
//		delete m_undoStack;
//		m_undoStack = savedStack;

		m_linesWidget->setUpdatesEnabled(true);
	}

	return true;
}

QUrl
Application::saveSplitSubtitle(const Subtitle &subtitle, const QUrl &srcUrl, QString encoding, QString format, bool primary)
{
	QUrl dstUrl;

	if(subtitle.linesCount()) {
		if(encoding.isEmpty())
			encoding = "UTF-8";

		if(format.isEmpty())
			format = "SubRip";

		QFileInfo dstFileInfo(srcUrl.path());
		if(srcUrl.isEmpty()) {
			QString baseName = primary ? i18n("Untitled") : i18n("Untitled Translation");
			QFileInfo(QDir(System::tempDir()), baseName + FormatManager::instance().defaultOutput()->extensions().first());
		}

		dstUrl = srcUrl;
		dstUrl.setPath(dstFileInfo.path());
		dstUrl = System::newUrl(dstUrl, dstFileInfo.completeBaseName() + " - " + i18nc("Suffix added to split subtitles", "split"), dstFileInfo.suffix());

		bool codecFound;
		QTextCodec *codec = KCharsets::charsets()->codecForName(encoding, codecFound);
		if(!codecFound)
			codec = QTextCodec::codecForLocale();

		if(FormatManager::instance().writeSubtitle(subtitle, primary, dstUrl, codec, format, false)) {
			if(primary)
				m_recentSubtitlesAction->addUrl(dstUrl, codec->name());
			else
				m_recentSubtitlesTrAction->addUrl(dstUrl, codec->name());
		} else {
			dstUrl.clear();
		}
	}

	if(dstUrl.path().isEmpty()) {
		KMessageBox::sorry(m_mainWindow, primary ? i18n("Could not write the split subtitle file.") : i18n("Could not write the split subtitle translation file."));
	}

	return dstUrl;
}

