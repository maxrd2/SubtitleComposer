/*
    SPDX-FileCopyrightText: 2010-2019 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "application.h"
#include "actions/kcodecactionext.h"
#include "actions/krecentfilesactionext.h"
#include "actions/useractionnames.h"
#include "core/undo/undostack.h"
#include "dialogs/joinsubtitlesdialog.h"
#include "dialogs/splitsubtitledialog.h"
#include "dialogs/syncsubtitlesdialog.h"
#include "formats/inputformat.h"
#include "formats/formatmanager.h"
#include "formats/textdemux/textdemux.h"
#include "formats/outputformat.h"
#include "helpers/commondefs.h"
#include "helpers/common.h"
#include "gui/treeview/lineswidget.h"
#include "speechprocessor/speechprocessor.h"
#include "videoplayer/videoplayer.h"

#include <QFileDialog>
#include <QLabel>
#include <QProcess>
#include <QStringBuilder>
#include <QTextCodec>

#include <KCharsets>
#include <KCodecAction>
#include <KLocalizedString>
#include <KMessageBox>

#undef ERROR

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
		const QStringList formats = FormatManager::instance().inputNames();
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
		openSubtitle(openDlg.selectedUrls().constFirst());
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
			QString videoFileName = QFileInfo(VideoPlayer::instance()->filePath()).completeBaseName().toLower();

			if(videoFileName.isEmpty() || subtitleFileName.indexOf(videoFileName) != 0) {
				QStringList subtitleDirFiles = subtitleFileInfo.dir().entryList(QDir::Files | QDir::Readable);
				for(QStringList::ConstIterator it = subtitleDirFiles.constBegin(), end = subtitleDirFiles.constEnd(); it != end; ++it) {
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
		m_subtitle.reset();

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

	emit subtitleOpened(m_subtitle.data());

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

	connect(m_subtitle.constData(), &Subtitle::primaryDirtyStateChanged, this, &Application::updateTitle);
	connect(m_subtitle.constData(), &Subtitle::secondaryDirtyStateChanged, this, &Application::updateTitle);
	updateTitle();

	m_labSubFormat->setText(i18n("Format: %1", m_subtitleFormat));
	m_labSubEncoding->setText(i18n("Encoding: %1", m_subtitleEncoding));
}

void
Application::demuxTextStream(int textStreamIndex)
{
	if(!closeSubtitle())
		return;

	newSubtitle();

	m_textDemux->demuxFile(m_subtitle.data(), VideoPlayer::instance()->filePath(), textStreamIndex);
}

void
Application::speechImportAudioStream(int audioStreamIndex)
{
	if(!closeSubtitle())
		return;

	newSubtitle();

	m_speechProcessor->setSubtitle(m_subtitle.data());
	m_speechProcessor->setAudioStream(VideoPlayer::instance()->filePath(), audioStreamIndex);
}

bool
Application::saveSubtitle(QTextCodec *codec)
{
	if(m_subtitleUrl.isEmpty() || !FormatManager::instance().hasOutput(m_subtitleFormat))
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
		m_subtitleEncoding = codec->name();
		m_labSubFormat->setText(i18n("Format: %1", m_subtitleFormat));
		m_labSubEncoding->setText(i18n("Encoding: %1", m_subtitleEncoding));

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
		const QUrl url = saveDlg.selectedUrls().constFirst();
		if(!acceptClashingUrls(url, m_subtitleTrUrl))
			return false;

		m_subtitleUrl = url;
		m_subtitleFileName = QFileInfo(m_subtitleUrl.path()).fileName();
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

		emit subtitleClosed();

		m_undoStack->clear();

		m_subtitle.reset();

		m_labSubFormat->setText(QString());
		m_labSubEncoding->setText(QString());

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
		openSubtitleTr(openDlg.selectedUrls().constFirst());
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

	QExplicitlySharedDataPointer<Subtitle> subtitleTr(new Subtitle());
	QTextCodec *codec = codecForEncoding(KRecentFilesActionExt::encodingForUrl(url));

	FormatManager::Status res = FormatManager::instance().readSubtitle(*subtitleTr, false, url, &codec, &m_subtitleTrFormat);
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
	m_subtitle->setSecondaryData(*subtitleTr, false);
	processTranslationOpened(codec, m_subtitleTrFormat);
}

void
Application::reopenSubtitleTrWithCodec(QTextCodec *codec)
{
	if(m_subtitleTrUrl.isEmpty())
		return;

	QExplicitlySharedDataPointer<Subtitle> subtitleTr(new Subtitle());
	QString subtitleTrFormat;

	FormatManager::Status res = FormatManager::instance().readSubtitle(*subtitleTr, false, m_subtitleTrUrl, &codec, &subtitleTrFormat);
	if(res != FormatManager::SUCCESS) {
		if(res == FormatManager::ERROR) {
			KMessageBox::sorry(
				m_mainWindow,
				i18n("<qt>Could not parse the subtitle file.<br/>"
					 "This may have been caused by usage of the wrong encoding.</qt>"));
		}
		return;
	}

	m_subtitle->setSecondaryData(*subtitleTr, false);
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
	if(m_subtitleTrUrl.isEmpty() || !FormatManager::instance().hasOutput(m_subtitleTrFormat))
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
		m_subtitleTrEncoding = codec->name();

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
		const OutputFormat *fmt = FormatManager::instance().output(m_subtitleTrFormat);
		if(fmt)
			saveDlg.selectNameFilter(fmt->dialogFilter());
	}

	if(saveDlg.exec() == QDialog::Accepted) {
		const QUrl url = saveDlg.selectedUrls().constFirst();
		if(!acceptClashingUrls(m_subtitleUrl, url))
			return false;

		m_subtitleTrUrl = url;
		m_subtitleTrFileName = QFileInfo(m_subtitleTrUrl.path()).fileName();
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
			format = FormatManager::instance().defaultOutput()->name();

		QFileInfo dstFileInfo;
		if(srcUrl.isEmpty() || !srcUrl.isLocalFile()) {
			QString baseName = primary ? i18n("Untitled") : i18n("Untitled Translation");
			dstFileInfo = QFileInfo(QDir(System::tempDir()), baseName + FormatManager::instance().output(format)->extensions().first());
		} else {
			dstFileInfo = QFileInfo(srcUrl.toLocalFile());
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

void
Application::joinSubtitles()
{
	static JoinSubtitlesDialog *dlg = new JoinSubtitlesDialog(m_mainWindow);

	if(dlg->exec() == QDialog::Accepted) {
		QExplicitlySharedDataPointer<Subtitle> secondSubtitle(new Subtitle());

		const QUrl url = dlg->subtitleUrl();
		QTextCodec *codec = codecForEncoding(KRecentFilesActionExt::encodingForUrl(url));
		const bool primary = dlg->selectedTextsTarget() != Secondary;

		FormatManager::Status res = FormatManager::instance().readSubtitle(*secondSubtitle, primary, url, &codec, nullptr);
		if(res == FormatManager::SUCCESS) {
			if(dlg->selectedTextsTarget() == Both)
				secondSubtitle->setSecondaryData(*secondSubtitle, true);

			m_subtitle->appendSubtitle(*secondSubtitle, dlg->shiftTime().toMillis());
		} else {
			KMessageBox::sorry(m_mainWindow, i18n("Could not read the subtitle file to append."));
		}
	}
}

void
Application::splitSubtitle()
{
	static SplitSubtitleDialog *dlg = new SplitSubtitleDialog(m_mainWindow);

	if(dlg->exec() != QDialog::Accepted)
		return;

	QExplicitlySharedDataPointer<Subtitle> newSubtitle(new Subtitle());
	m_subtitle->splitSubtitle(*newSubtitle, dlg->splitTime().toMillis(), dlg->shiftNewSubtitle());
	if(!newSubtitle->linesCount()) {
		KMessageBox::information(m_mainWindow, i18n("The specified time does not split the subtitles."));
		return;
	}

	QUrl splitUrl = saveSplitSubtitle(
		*newSubtitle,
		m_subtitleUrl,
		m_subtitleEncoding,
		m_subtitleFormat,
		true);

	if(splitUrl.path().isEmpty()) {
		// there was an error saving the split part, undo the splitting of m_subtitle
		m_undoStack->undo();
		return;
	}

	QUrl splitTrUrl;
	if(m_translationMode) {
		splitTrUrl = saveSplitSubtitle(*newSubtitle, m_subtitleTrUrl, m_subtitleTrEncoding, m_subtitleTrFormat, false);

		if(splitTrUrl.path().isEmpty()) {
			// there was an error saving the split part, undo the splitting of m_subtitle
			m_undoStack->undo();
			return;
		}
	}

	QStringList args;
	args << splitUrl.toString(QUrl::PreferLocalFile);
	if(m_translationMode)
		args << splitTrUrl.toString(QUrl::PreferLocalFile);

	if(!QProcess::startDetached(applicationName(), args)) {
		KMessageBox::sorry(m_mainWindow, m_translationMode
			? i18n("Could not open a new Subtitle Composer window.\n" "The split part was saved as %1.", splitUrl.path())
			: i18n("Could not open a new Subtitle Composer window.\n" "The split parts were saved as %1 and %2.", splitUrl.path(), splitTrUrl.path()));
	}
}

void
Application::syncWithSubtitle()
{
	static SyncSubtitlesDialog *dlg = new SyncSubtitlesDialog(m_mainWindow);

	if(dlg->exec() == QDialog::Accepted) {
		QExplicitlySharedDataPointer<Subtitle> referenceSubtitle(new Subtitle());

		const QUrl url = dlg->subtitleUrl();

		FormatManager::Status res = FormatManager::instance().readSubtitle(*referenceSubtitle, true, url, nullptr, nullptr);
		if(res == FormatManager::SUCCESS) {
			if(dlg->adjustToReferenceSubtitle()) {
				if(referenceSubtitle->linesCount() <= 1)
					KMessageBox::sorry(m_mainWindow, i18n("The reference subtitle must have more than one line to proceed."));
				else
					m_subtitle->adjustLines(Range::full(),
											referenceSubtitle->firstLine()->showTime().toMillis(),
											referenceSubtitle->lastLine()->showTime().toMillis());
			} else /*if(dlg->synchronizeToReferenceTimes())*/ {
				m_subtitle->syncWithSubtitle(*referenceSubtitle);
			}
		} else {
			KMessageBox::sorry(m_mainWindow, i18n("Could not parse the reference subtitle file."));
		}
	}
}
