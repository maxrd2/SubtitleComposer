/***************************************************************************
 *   Copyright (C) 2007-2009 Sergio Pistone (sergio_pistone@yahoo.com.ar)  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/

#include "opensubtitledialog.h"
#include "../application.h"
#include "../../formats/formatmanager.h"

#include <KDebug>
#include <KLocale>
#include <KComboBox>
#include <kabstractfilewidget.h>

using namespace SubtitleComposer;

OpenSubtitleDialog::OpenSubtitleDialog(bool primary, const QString &startDir, const QString &encoding, QWidget *parent) :
	KFileDialog(startDir, inputFormatsFilter(), parent)
{
	setCaption(primary ? i18n("Open Subtitle") : i18n("Open Translation Subtitle"));
	setOperationMode(KFileDialog::Opening);

	setModal(true);
	setMode(KFile::File | KFile::ExistingOnly);

	m_encodingComboBox = new KComboBox(this);
	m_encodingComboBox->addItem(i18n("Autodetect"));
	m_encodingComboBox->addItems(app()->availableEncodingNames());
	m_encodingComboBox->setCurrentItem(encoding.isEmpty() ? i18n("Autodetect") : encoding);
	m_encodingComboBox->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

	// FIXME set "encoding" label buddy to m_encodingComboBox (how do we get the "encoding" label widget?)
	fileWidget()->setCustomWidget(i18n("Encoding:"), m_encodingComboBox);
}

QString
OpenSubtitleDialog::selectedEncoding() const
{
	return m_encodingComboBox->currentIndex() == 0 ? QString() : m_encodingComboBox->currentText();
}

QString
OpenSubtitleDialog::inputFormatsFilter()
{
	static QString filter;

	if(filter.isEmpty()) {
		QString allSupported;
		QStringList formats = FormatManager::instance().inputNames();
		for(QStringList::ConstIterator it = formats.begin(), end = formats.end(); it != end; ++it) {
			const InputFormat *format = FormatManager::instance().input(*it);
			const QStringList &formatExtensions = format->extensions();
			QString extensions;
			for(QStringList::ConstIterator extIt = formatExtensions.begin(), extEnd = formatExtensions.end(); extIt != extEnd; ++extIt)
				extensions += "*." + *extIt + " *." + (*extIt).toUpper();
			allSupported += ' ' + extensions;
			filter += '\n' + extensions + '|' + format->name();
		}

		filter = allSupported + '|' + i18n("All Supported") + filter;
		filter = filter.trimmed();
	}

	return filter;
}
