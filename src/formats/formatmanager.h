/*
 * SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2010-2019 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef FORMATMANAGER_H
#define FORMATMANAGER_H

#include "format.h"

#include <QString>
#include <QStringList>
#include <QMap>

#include <QUrl>
#include <KEncodingProber>

namespace SubtitleComposer {
class InputFormat;
class OutputFormat;
class Subtitle;
class FormatManager
{
public:
	enum Status {
		SUCCESS = 1,
		CANCEL = 0,
		ERROR = -1
	};
	static FormatManager & instance();

	bool hasInput(const QString &name) const;
	const InputFormat * input(const QString &name) const;
	QStringList inputNames() const;

	Status readSubtitle(Subtitle &subtitle, bool primary, const QUrl &url,
						QTextCodec **codec, QString *format = nullptr) const;

	bool hasOutput(const QString &name) const;
	const OutputFormat * output(const QString &name) const;
	const OutputFormat * defaultOutput() const;
	QStringList outputNames() const;

	bool writeSubtitle(const Subtitle &subtitle, bool primary, const QUrl &url,
					   QTextCodec *codec, const QString &format, bool overwrite) const;

protected:
	FormatManager();
	~FormatManager();

	Status readBinary(Subtitle &subtitle, const QUrl &url, bool primary,
					  QTextCodec **codec, QString *format) const;
	Status readText(Subtitle &subtitle, const QUrl &url, bool primary,
					QTextCodec **codec, QString *formatName) const;

	QMap<QString, InputFormat *> m_inputFormats;
	QMap<QString, OutputFormat *> m_outputFormats;
};
}
#endif
