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
Application::buildSubtitleFilesFilter(bool openFileFilter)
{
	static QString filterSave;
	static QString filterOpen;

	if(filterSave.isEmpty()) {
		QString textExtensions;
		QString imageExtensions;
		QStringList formats = FormatManager::instance().inputNames();
		for(const QString &fmt : formats) {
			const InputFormat *format = FormatManager::instance().input(fmt);
			QString extensions;
			for(const QString &ext : format->extensions())
				extensions += $(" *.") % ext;
			const QString formatLine = format->dialogFilter() % QChar::LineFeed;
			filterOpen += formatLine;
			if(format->isBinary()) {
				imageExtensions += extensions;
			} else {
				textExtensions += extensions;
				filterSave += formatLine;
			}
		}
		filterOpen = i18n("All Text Subtitles") % $(" (") % textExtensions.midRef(1) % $(")\n")
			% i18n("All Image Subtitles") % $(" (") % imageExtensions.midRef(1) % $(")\n")
			% filterSave
			% i18n("All Files") % $(" (*)");
		filterSave.truncate(filterSave.size() - 1);
	}

	return openFileFilter ? filterOpen : filterSave;
}

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
	m_subtitleUrl.clear();

	processSubtitleOpened(nullptr, QString());
}

void
Application::openSubtitle()
{
	QFileDialog openDlg(m_mainWindow, i18n("Open Subtitle"), QString(), buildSubtitleFilesFilter());

	openDlg.setModal(true);
	openDlg.selectUrl(m_lastSubtitleUrl);

	if(openDlg.exec() == QDialog::Accepted)
		openSubtitle(openDlg.selectedUrls().first());
}

void
Application::openSubtitle(const QUrl &url, bool warnClashingUrls)
{
	m_lastSubtitleUrl = url;

	if(warnClashingUrls && !acceptClashingUrls(url, m_subtitleTrUrl))
		return;

	if(!closeSubtitle())
		return;

	QTextCodec *codec = codecForEncoding(KRecentFilesActionExt::encodingForUrl(url));

	m_subtitle = new Subtitle();
	FormatManager::Status res = FormatManager::instance().readSubtitle(*m_subtitle, true, url, &codec, &m_subtitleFormat);
	if(res == FormatManager::SUCCESS) {
		m_subtitleUrl = url;
		processSubtitleOpened(codec, m_subtitleFormat);

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
							auxUrl.setScheme($("file"));
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
		m_subtitle = nullptr;

		if(res == FormatManager::ERROR) {
			KMessageBox::sorry(
				m_mainWindow,
				i18n("<qt>Could not parse the subtitle file.<br/>"
					 "This may have been caused by usage of the wrong encoding.</qt>"));
		}
	}
}

void
Application::reopenSubtitleWithCodec(QTextCodec *codec)
{
	if(m_subtitleUrl.isEmpty())
		return;

	Subtitle *subtitle = new Subtitle();
	QString subtitleFormat;

	FormatManager::Status res = FormatManager::instance().readSubtitle(*subtitle, true, m_subtitleUrl, &codec, &subtitleFormat);
	if(res != FormatManager::SUCCESS) {
		if(res == FormatManager::ERROR) {
			KMessageBox::sorry(
				m_mainWindow,
				i18n("<qt>Could not parse the subtitle file.<br/>"
					 "This may have been caused by usage of the wrong encoding.</qt>"));
		}
		delete subtitle;
		return;
	}

	emit subtitleClosed();

	if(m_translationMode)
		subtitle->setSecondaryData(*m_subtitle, false);

	delete m_subtitle;
	m_subtitle = subtitle;

	processSubtitleOpened(codec, subtitleFormat);
}

void
Application::processSubtitleOpened(QTextCodec *codec, const QString &subtitleFormat)
{
	// The loading of the subtitle shouldn't be an undoable action as there's no state before it
	m_undoStack->clear();
	m_subtitle->clearPrimaryDirty();
	m_subtitle->clearSecondaryDirty();

	emit subtitleOpened(m_subtitle);

	m_subtitleFormat = subtitleFormat;

	if(m_subtitleUrl.isEmpty()) {
		m_subtitleFileName.clear();
		m_subtitleEncoding = SCConfig::defaultSubtitlesEncoding();
		codec = KCharsets::charsets()->codecForName(m_subtitleEncoding);
	} else {
		m_subtitleFileName = QFileInfo(m_subtitleUrl.path()).fileName();
		Q_ASSERT(codec != nullptr);
		m_subtitleEncoding = codec->name();
		m_recentSubtitlesAction->addUrl(m_subtitleUrl, m_subtitleEncoding);
	}
	m_reopenSubtitleAsAction->setCurrentCodec(codec);
	m_saveSubtitleAsAction->setCurrentCodec(codec);

	connect(m_subtitle, SIGNAL(primaryDirtyStateChanged(bool)), this, SLOT(updateTitle()));
	connect(m_subtitle, SIGNAL(secondaryDirtyStateChanged(bool)), this, SLOT(updateTitle()));
	updateTitle();
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
Application::saveSubtitle(QTextCodec *codec)
{
	if(m_subtitleUrl.isEmpty() || m_subtitleEncoding.isEmpty() || !FormatManager::instance().hasOutput(m_subtitleFormat))
		return saveSubtitleAs(codec);

	bool codecFound = true;
	if(!codec)
		codec = KCharsets::charsets()->codecForName(m_subtitleEncoding, codecFound);
	if(!codecFound)
		codec = KCharsets::charsets()->codecForName(SCConfig::defaultSubtitlesEncoding(), codecFound);
	if(!codecFound)
		codec = QTextCodec::codecForLocale();

	if(FormatManager::instance().writeSubtitle(*m_subtitle, true, m_subtitleUrl, codec, m_subtitleFormat, true)) {
		m_subtitle->clearPrimaryDirty();

		m_reopenSubtitleAsAction->setCurrentCodec(codec);
		m_saveSubtitleAsAction->setCurrentCodec(codec);
		m_recentSubtitlesAction->addUrl(m_subtitleUrl, codec->name());

		updateTitle();

		return true;
	} else {
		KMessageBox::sorry(m_mainWindow, i18n("There was an error saving the subtitle."));
		return false;
	}
}

bool
Application::saveSubtitleAs(QTextCodec *codec)
{
	QFileDialog saveDlg(m_mainWindow, i18n("Save Subtitle"), QString(), buildSubtitleFilesFilter(false));

	saveDlg.setModal(true);
	saveDlg.setAcceptMode(QFileDialog::AcceptSave);
	saveDlg.setDirectory(QFileInfo(m_lastSubtitleUrl.toLocalFile()).absoluteDir());
	saveDlg.selectNameFilter(FormatManager::instance().defaultOutput()->dialogFilter());
	if(!m_subtitleUrl.isEmpty()) {
		saveDlg.selectUrl(m_subtitleUrl);
		const OutputFormat *fmt = FormatManager::instance().output(m_subtitleFormat);
		if(fmt)
			saveDlg.selectNameFilter(fmt->dialogFilter());
	}

	if(saveDlg.exec() == QDialog::Accepted) {
		const QUrl url = saveDlg.selectedUrls().first();
		if(!acceptClashingUrls(url, m_subtitleTrUrl))
			return false;

		m_subtitleUrl = url;
		m_subtitleFileName = QFileInfo(m_subtitleUrl.path()).completeBaseName();
		m_subtitleFormat = saveDlg.selectedNameFilter();
		if(!m_subtitleFormat.isEmpty())
			m_subtitleFormat.truncate(m_subtitleFormat.indexOf(QStringLiteral(" (")));
		return saveSubtitle(codec);
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
		m_subtitle = nullptr;

		m_subtitleUrl.clear();
		m_subtitleFileName.clear();
		m_subtitleEncoding.clear();
		m_subtitleFormat.clear();

		m_subtitleTrUrl.clear();
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

	QFileDialog openDlg(m_mainWindow, i18n("Open Translation Subtitle"),  QString(), buildSubtitleFilesFilter());

	openDlg.setModal(true);
	openDlg.selectUrl(m_lastSubtitleUrl);

	if(openDlg.exec() == QDialog::Accepted)
		openSubtitleTr(openDlg.selectedUrls().first());
}

void
Application::openSubtitleTr(const QUrl &url, bool warnClashingUrls)
{
	m_lastSubtitleUrl = url;

	if(!m_subtitle)
		return;

	if(warnClashingUrls && !acceptClashingUrls(m_subtitleUrl, url))
		return;

	if(!closeSubtitleTr())
		return;

	Subtitle subtitleTr;
	QTextCodec *codec = codecForEncoding(KRecentFilesActionExt::encodingForUrl(url));

	FormatManager::Status res = FormatManager::instance().readSubtitle(subtitleTr, false, url, &codec, &m_subtitleTrFormat);
	if(res != FormatManager::SUCCESS) {
		if(res == FormatManager::ERROR) {
			KMessageBox::sorry(
				m_mainWindow,
				i18n("<qt>Could not parse the subtitle file.<br/>"
					 "This may have been caused by usage of the wrong encoding.</qt>"));
		}
		return;
	}

	m_subtitleTrUrl = url;
	m_subtitle->setSecondaryData(subtitleTr, false);
	processTranslationOpened(codec, m_subtitleTrFormat);
}

void
Application::reopenSubtitleTrWithCodec(QTextCodec *codec)
{
	if(m_subtitleTrUrl.isEmpty())
		return;

	Subtitle subtitleTr;
	QString subtitleTrFormat;

	FormatManager::Status res = FormatManager::instance().readSubtitle(subtitleTr, false, m_subtitleTrUrl, &codec, &subtitleTrFormat);
	if(res != FormatManager::SUCCESS) {
		if(res == FormatManager::ERROR) {
			KMessageBox::sorry(
				m_mainWindow,
				i18n("<qt>Could not parse the subtitle file.<br/>"
					 "This may have been caused by usage of the wrong encoding.</qt>"));
		}
		return;
	}

	m_subtitle->setSecondaryData(subtitleTr, false);
	processTranslationOpened(codec, m_subtitleTrFormat);
}

void
Application::processTranslationOpened(QTextCodec *codec, const QString &subtitleFormat)
{
	// We don't clear the primary dirty state because the loading of the translation
	// only changes it when actually needed (i.e., when the translation had more lines)
	m_subtitle->clearSecondaryDirty();

	m_subtitleFormat = subtitleFormat;

	if(!m_subtitleTrUrl.isEmpty()) {
		m_subtitleTrFileName = QFileInfo(m_subtitleTrUrl.path()).fileName();
		Q_ASSERT(codec != nullptr);
		m_subtitleTrEncoding = codec->name();
		m_recentSubtitlesTrAction->addUrl(m_subtitleTrUrl, codec->name());
		m_reopenSubtitleTrAsAction->setCurrentCodec(codec);
		m_saveSubtitleTrAsAction->setCurrentCodec(codec);
	}

	const QStringList subtitleStreams = {
		i18nc("@item:inmenu Display primary text in video player", "Primary"),
		i18nc("@item:inmenu Display translation text in video player", "Translation"),
	};
	KSelectAction *activeSubtitleStreamAction = (KSelectAction *)action(ACT_SET_ACTIVE_SUBTITLE_STREAM);
	activeSubtitleStreamAction->setItems(subtitleStreams);
	if(activeSubtitleStreamAction->currentItem() == -1)
		activeSubtitleStreamAction->setCurrentItem(0);

	if(!m_translationMode) {
		m_translationMode = true;
		updateTitle();
		emit translationModeChanged(true);
	}
}

bool
Application::saveSubtitleTr(QTextCodec *codec)
{
	if(m_subtitleTrUrl.isEmpty() || m_subtitleTrEncoding.isEmpty() || !FormatManager::instance().hasOutput(m_subtitleTrFormat))
		return saveSubtitleTrAs(codec);

	bool codecFound = true;
	if(!codec)
		codec = KCharsets::charsets()->codecForName(m_subtitleTrEncoding, codecFound);
	if(!codecFound)
		codec = KCharsets::charsets()->codecForName(SCConfig::defaultSubtitlesEncoding(), codecFound);
	if(!codecFound)
		codec = QTextCodec::codecForLocale();

	if(FormatManager::instance().writeSubtitle(*m_subtitle, false, m_subtitleTrUrl, codec, m_subtitleTrFormat, true)) {
		m_subtitle->clearSecondaryDirty();

		m_reopenSubtitleTrAsAction->setCurrentCodec(codec);
		m_saveSubtitleTrAsAction->setCurrentCodec(codec);
		m_recentSubtitlesTrAction->addUrl(m_subtitleTrUrl, codec->name());

		updateTitle();

		return true;
	} else {
		KMessageBox::sorry(m_mainWindow, i18n("There was an error saving the translation subtitle."));
		return false;
	}
}

bool
Application::saveSubtitleTrAs(QTextCodec *codec)
{
	QFileDialog saveDlg(m_mainWindow, i18n("Save Translation Subtitle"), QString(), buildSubtitleFilesFilter(false));

	saveDlg.setModal(true);
	saveDlg.setAcceptMode(QFileDialog::AcceptSave);
	saveDlg.setDirectory(QFileInfo(m_lastSubtitleUrl.toLocalFile()).absoluteDir());
	saveDlg.selectNameFilter(FormatManager::instance().defaultOutput()->dialogFilter());
	if(!m_subtitleUrl.isEmpty()) {
		saveDlg.selectUrl(m_subtitleTrUrl);
		saveDlg.selectNameFilter(FormatManager::instance().output(m_subtitleTrFormat)->dialogFilter());
	}

	if(saveDlg.exec() == QDialog::Accepted) {
		const QUrl url = saveDlg.selectedUrls().first();
		if(!acceptClashingUrls(m_subtitleUrl, url))
			return false;

		m_subtitleTrUrl = url;
		m_subtitleTrFileName = QFileInfo(m_subtitleTrUrl.path()).completeBaseName();
		m_subtitleTrFormat = saveDlg.selectedNameFilter();
		if(!m_subtitleTrFormat.isEmpty())
			m_subtitleTrFormat.truncate(m_subtitleTrFormat.indexOf(QStringLiteral(" (")));

		return saveSubtitleTr(codec);
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
