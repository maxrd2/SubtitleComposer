#ifndef VOBSUBINPUTFORMAT_H
#define VOBSUBINPUTFORMAT_H

/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2017 Mladen Milinkovic <max@smoothware.net>
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

#include "mplayer/mp_msg.h"
#include "mplayer/vobsub.h"
#include "mplayer/spudec.h"

#include "application.h"
#include "formats/inputformat.h"
#include "vobsubinputinitdialog.h"
#include "vobsubinputprocessdialog.h"

#include <QUrl>

namespace SubtitleComposer {
class VobSubInputFormat : public InputFormat
{
	friend class FormatManager;

public:
	virtual bool readBinary(Subtitle &subtitle, const QUrl &url)
	{
#if defined(VERBOSE) || !defined(NDEBUG)
		qputenv("MPLAYER_VERBOSE", QByteArrayLiteral("1"));
#endif

		mp_msg_init();

		const QString filename = url.toLocalFile();
		const QByteArray filebase = filename.left(filename.lastIndexOf('.')).toLatin1();

		// Open the sub/idx subtitles
		void *spu;
		void *vob = vobsub_open(filebase.constData(), 0, 1, 0, &spu);
		if(!vob || !vobsub_get_indexes_count(vob)) {
			qDebug() << "Couldn't open VobSub files '" << filebase << ".idx/.sub'\n";
			return false;
		}

		VobSubInputInitDialog dlgInit(vob, spu, Application::instance()->mainWindow());
		if(dlgInit.exec() == QDialog::Rejected)
			return true;

		vobsub_id = dlgInit.streamIndex();

		// show subtitle updates in realtime
		LinesWidget *linesWidget = Application::instance()->linesWidget();
		Subtitle *oldSubtitle = linesWidget->model()->subtitle();
		linesWidget->setSubtitle(&subtitle);

		VobSubInputProcessDialog dlgProc(&subtitle, vob, spu, Application::instance()->mainWindow());

		QByteArray symFile(filebase + ".sym");

		dlgProc.symFileOpen(symFile);
		dlgProc.exec();
		dlgProc.symFileSave(symFile);

		// TODO: move all these regexps into separate class that can be reused, make them static and optimize them after init
		quint32 ppFlags = dlgInit.postProcessingFlags();
		const QList<SubtitleLine *> lines = subtitle.allLines();
		foreach(SubtitleLine *line, lines) {
			SString text = line->primaryText();
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
					.replace(QRegularExpression(QStringLiteral("(?<=[?!:])\\.")), QStringLiteral(""))
					// ,, => ...; -- => ...
					.replace(QRegularExpression(QStringLiteral("(?:,{2,}|-{2,})")), QStringLiteral("..."));

			if(ppFlags & VobSubInputInitDialog::SPACE_NUMBERS)
				text
					.replace(QRegularExpression(QStringLiteral("\\d[\\d,.]*\\K +(?=[\\d,.])")), QStringLiteral("")); // remove space between numbers

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
			line->setPrimaryText(text);
		}

		// restore original subtitle
		linesWidget->setSubtitle(oldSubtitle);

		return true;
	}

protected:
	virtual bool parseSubtitles(Subtitle &, const QString &) const
	{
		return false;
	}

	VobSubInputFormat()
		: InputFormat(QStringLiteral("VobSub"),
		  QStringList(QStringLiteral("idx")))
	{}

	QUrl m_url;
};
}

#endif
