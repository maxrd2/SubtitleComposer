#ifndef FORMAT_H
#define FORMAT_H

/*
 * SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * SPDX-FileCopyrightText: 2010-2019 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "core/formatdata.h"
#include "core/subtitle.h"
#include "core/subtitleline.h"

#include <QString>
#include <QStringBuilder>
#include <QStringList>

namespace SubtitleComposer {
class Format
{
public:
	friend class FormatManager;

	typedef enum {
		UNIX = 0,
		Windows,
		Macintosh,
#ifdef Q_OS_UNIX                                // krazy:exclude=c++/cpp
		CurrentOS = UNIX
#else
#ifdef Q_OS_WIN                                 // krazy:exclude=c++/cpp
		CurrentOS = Windows
#else
		CurrentOS = Macintosh
#endif
#endif
	} NewLine;

	Format(const QString &name, const QStringList &extensions) :
		m_name(name),
		m_extensions(extensions)
	{}

	virtual ~Format() {}

	const QString & name() const { return m_name; }
	const QStringList & extensions() const { return m_extensions; }
	const QString dialogFilter() const
	{
		QString extensions;
		for(const QString &ext : m_extensions)
			extensions += QStringLiteral(" *.") % ext;
		return m_name % QStringLiteral(" (") % extensions.midRef(1) % QChar(')');
	}

	bool knowsExtension(const QString &extension) const
	{
		for(const QString &knownExtension : m_extensions) {
			if(knownExtension != QChar('*') && knownExtension.compare(extension, Qt::CaseInsensitive) == 0) {
				return true;
			}
		}
		return false;
	}

protected:
	FormatData createFormatData() const
	{
		return FormatData(m_name);
	}

	FormatData * formatData(const Subtitle &subtitle) const
	{
		FormatData *formatData = subtitle.formatData();
		return formatData && formatData->formatName() == m_name ? formatData : nullptr;
	}

	void setFormatData(Subtitle &subtitle, FormatData *formatData) const
	{
		subtitle.setFormatData(formatData);
	}

	void setFormatData(Subtitle &subtitle, FormatData &formatData) const
	{
		subtitle.setFormatData(&formatData);
	}

	FormatData * formatData(const SubtitleLine *line) const
	{
		FormatData *formatData = line->formatData();
		return formatData && formatData->formatName() == m_name ? formatData : nullptr;
	}

	void setFormatData(SubtitleLine *line, FormatData *formatData) const
	{
		line->setFormatData(formatData);
	}

	void setFormatData(SubtitleLine *line, FormatData &formatData) const
	{
		line->setFormatData(&formatData);
	}

	QString m_name;
	QStringList m_extensions;
};
}

#endif
