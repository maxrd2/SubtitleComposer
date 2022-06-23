/*
    SPDX-FileCopyrightText: 2017-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef VOBSUBINPUTFORMAT_H
#define VOBSUBINPUTFORMAT_H

#include "application.h"
#include "core/richdocument.h"
#include "formats/inputformat.h"
#include "vobsubinputinitdialog.h"
#include "vobsubinputprocessdialog.h"
#include "streamprocessor/streamprocessor.h"

#include <QUrl>
#include <QFile>

namespace SubtitleComposer {
class VobSubInputFormat : public InputFormat
{
	friend class FormatManager;

public:
	bool isBinary() const override { return true; }

	FormatManager::Status readBinary(Subtitle &subtitle, const QUrl &url) override
	{
		QString filename = url.toLocalFile();
		const int extension = filename.lastIndexOf('.');
		const QByteArray filebase = filename.left(extension).toUtf8();

		if(filename.midRef(extension + 1) == QStringLiteral("sub")) {
			const QString filenameIdx = filebase + ".idx";
			if(QFile(filenameIdx).exists())
				filename = filenameIdx;
		}

		// open the sub/idx subtitles
		StreamProcessor proc;
		if(!proc.open(filename))
			return FormatManager::ERROR;

		QStringList streamList = proc.listImage();
		if(streamList.empty())
			return FormatManager::ERROR;

		// show init dialog
		VobSubInputInitDialog dlgInit(app()->mainWindow());
		dlgInit.streamListSet(streamList);
		if(dlgInit.exec() == QDialog::Rejected)
			return FormatManager::CANCEL;

		if(!proc.initImage(dlgInit.streamIndex()))
			return FormatManager::ERROR;

		// subtitle updates will show in realtime
		LinesWidget *linesWidget = app()->linesWidget();
		Subtitle *oldSubtitle = linesWidget->model()->subtitle();
		linesWidget->setSubtitle(&subtitle);

		// show process dialog
		VobSubInputProcessDialog dlgProc(&subtitle, app()->mainWindow());

		dlgProc.processFrames(&proc);

		QByteArray symFile(filebase + ".sym");

		dlgProc.symFileOpen(symFile);
		const int dlgRes = dlgProc.exec();
		dlgProc.symFileSave(symFile);
		if(dlgRes == QDialog::Rejected) {
			// restore original subtitle
			linesWidget->setSubtitle(oldSubtitle);
			return FormatManager::CANCEL;
		}

		// TODO: move all these regexps into separate class that can be reused, make them static and optimize them after init
		quint32 ppFlags = dlgInit.postProcessingFlags();
		for(int i = 0, n = subtitle.count(); i < n; i++) {
			SubtitleLine *line = subtitle.at(i);
			RichString text = line->primaryDoc()->toRichText();
			if(ppFlags & VobSubInputInitDialog::APOSTROPHE_TO_QUOTES)
				text
					.replace(QRegularExpression(QStringLiteral("(?:"
						"' *'" // double apostrophes ' ' => "
						"|"
						"\" *\"" // double quotes "" => "
						")")), QStringLiteral("\""));

			if(ppFlags & VobSubInputInitDialog::SPACE_PARENTHESES)
				text
					.replace(QRegularExpression(QStringLiteral("(?:"
						" *['`]" // normalize apostrophes and remove leading space
						"|"
						"(?<=[A-ZÁ-Úa-zá-ú]) *['`] *(?=(ll|ve|s|m|d|t)\\b)" // remove space around apostrophe in: I'd, It's, He'll, ..
						")")), QStringLiteral("'"));

			if(ppFlags & VobSubInputInitDialog::SPACE_PUNCTUATION)
				text
					// remove space before/between, add it after punctuation
					.replace(QRegularExpression(QStringLiteral(" *(?:([\\.,?!;:]) *)+")), QStringLiteral("\\1 "))
					// ?. => ?, !. => !, :. => :
					.replace(QRegularExpression(QStringLiteral("(?<=[?!:])\\.")), QString())
					// ,, => ...; -- => ...
					.replace(QRegularExpression(QStringLiteral("(?:,{2,}|-{2,})")), QStringLiteral("..."));

			if(ppFlags & VobSubInputInitDialog::SPACE_NUMBERS)
				text
					.replace(QRegularExpression(QStringLiteral("\\d[\\d,.]*\\K +(?=[\\d,.])")), QString()); // remove space between numbers

			if(ppFlags & VobSubInputInitDialog::CHARS_OCR)
				text
					.replace(QRegularExpression(QStringLiteral("\\d[,.]?\\KO")), QStringLiteral("0")) // uppercase O => zero 0
					.replace(QRegularExpression(QStringLiteral("(?:[A-Z]\\K0|\\b0(?=A-Za-z))")), QStringLiteral("O")); // zero 0 => uppercase O

			if(ppFlags & VobSubInputInitDialog::SPACE_PARENTHESES)
				text
					// remove space inside parentheses
					.replace(QRegularExpression(QStringLiteral("([\\(\\[\\{]\\K +| +(?=[\\]\\}\\)]))")), QStringLiteral(""))
					// add space around parentheses
					.replace(QRegularExpression(QStringLiteral("((?<!^|[ \n])(?=[\\(\\[\\{])|(?<=[\\]\\}\\)])(?!$|[ \\n]))")), QStringLiteral(" "))
					// add space around, remove it from inside parentheses
					.replace(QRegularExpression(QStringLiteral(" *\" *([^\"]+?) *\" *")), QStringLiteral(" \"\\1\" "));

			if(ppFlags & VobSubInputInitDialog::CHARS_OCR)
				text
					// fix roman numerals
					.replace(QRegularExpression(QStringLiteral("\\b[VXLCDM]*\\K[lI]{3}\\b")), QStringLiteral("III"))
					.replace(QRegularExpression(QStringLiteral("\\b[VXLCDM]*\\K[lI]{2}\\b")), QStringLiteral("II"))
					.replace(QRegularExpression(QStringLiteral("\\b[VXLCDM]*\\Kl(?=[VXLCDM]*\\b)")), QStringLiteral("I"))
					// replace II => ll
					.replace(QRegularExpression(QStringLiteral("(?:[a-zá-ú]\\KII|II(?=[a-zá-ú]))")), QStringLiteral("ll"))
					// replace I => l
					.replace(QRegularExpression(QStringLiteral("(?:[a-zá-ú]\\KI(?=[a-zá-ú]|\\b)|\\bI(?=[oaeiuyá-ú])|\\b[A-ZÁ-Ú]\\KI(?=[a-zá-ú]))")), QStringLiteral("l"))
					// replace l => I
					.replace(QRegularExpression(QStringLiteral("(?:[A-ZÁ-Ú]{2,}\\Kl\\b|[A-ZÁ-Ú]\\Kl(?=[A-ZÁ-Ú])|\\bl\\b|\\bl(?=[^aeiouyàá-úl]))")), QStringLiteral("I"))
					// replace 'II => 'll
					.replace(QRegularExpression(QStringLiteral("[A-ZÁ-Úa-zá-ú]\\K *' *II\\b")), QStringLiteral("'ll"))
					// exceptions l => I: Ian, Iowa, Ion, Iodine
					.replace(QRegularExpression(QStringLiteral("\\bKl(?=(?:an|owa|ll)\\b|oni|odi|odo)")), QStringLiteral("I"))
					// word Ill
					.replace(QRegularExpression(QStringLiteral("(^|[.?!-\"] *)\\KIII\\b")), QStringLiteral("Ill"));

			// cleanup whitespace
			text.replace(QRegularExpression(QStringLiteral("(?: *(?=\\n)|(?<=\\n) *|^ *| *$| *(?= )|(?<= ) *)")), QStringLiteral(""));
			line->primaryDoc()->setRichText(text, true);
		}

		// restore original subtitle
		linesWidget->setSubtitle(oldSubtitle);

		return FormatManager::SUCCESS;
	}

protected:
	bool parseSubtitles(Subtitle &, const QString &) const override
	{
		return false;
	}

	VobSubInputFormat()
		: InputFormat(QStringLiteral("DVD/BluRay Subpicture"), QStringList{
					  QStringLiteral("idx"),
					  QStringLiteral("sup"),
					  QStringLiteral("m2ts"),
					  QStringLiteral("vob"),
					  QStringLiteral("mkv"),
					  QStringLiteral("mp4")})
	{}

	QUrl m_url;
};
}

#endif
