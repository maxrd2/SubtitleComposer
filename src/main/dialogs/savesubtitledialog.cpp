/**
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2015 Mladen Milinkovic <max@smoothware.net>
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

#include "savesubtitledialog.h"
#include "../application.h"
#include "../../common/filesavehelper.h"
#include "../../formats/formatmanager.h"

#include <QLabel>
#include <QGridLayout>
#include <QComboBox>
#include <QVBoxLayout>
#include <QPushButton>

#include <KLocalizedString>
#include <KFileFilterCombo>

//#include <QDebug>

using namespace SubtitleComposer;

SaveSubtitleDialog::SaveSubtitleDialog(bool primary, const QUrl &startDir, const QString &encoding, Format::NewLine newLine, const QString &format, QWidget *parent)
	: QDialog(parent),
	  m_fileWidget(new KFileWidget(startDir, this))
{
	setWindowTitle(primary ? i18n("Save Subtitle") : i18n("Save Translation Subtitle"));
	setModal(true);
	m_fileWidget->setOperationMode(KFileWidget::Saving);
	m_fileWidget->setMode(KFile::File);
	m_fileWidget->setFilter(outputFormatsFilter());
	m_fileWidget->setConfirmOverwrite(true);

	m_fileWidget->filterWidget()->setEditable(false);

	if(FormatManager::instance().output(format))
		setCurrentFilter(format);
	else
		setCurrentFilter(FormatManager::instance().outputNames().first());

	// setting the current filter will force the first valid extension for the format which
	// may not be the one of the file (even when the file's extension is perfectly valid)
//	m_fileWidget->setUrl(startDir); // FIXME?

	QWidget *customWidget = new QWidget(this);

	m_encodingComboBox = new QComboBox(customWidget);
	m_encodingComboBox->addItems(app()->availableEncodingNames());
	m_encodingComboBox->setCurrentText(encoding.toUpper());
	m_encodingComboBox->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

	m_newLineComboBox = new QComboBox(customWidget);
	m_newLineComboBox->addItems(QStringList() << "UNIX" << "Windows" << "Macintosh");
	m_newLineComboBox->setCurrentIndex(newLine);
	m_newLineComboBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

	QLabel *newLineLabel = new QLabel(customWidget);
	newLineLabel->setText(i18n("EOL:"));
	newLineLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	newLineLabel->setBuddy(m_newLineComboBox);

	QGridLayout *grid = new QGridLayout(customWidget);
	grid->setContentsMargins(0, 0, 0, 0);
	grid->addWidget(m_encodingComboBox, 0, 0);
	grid->addWidget(newLineLabel, 0, 1);
	grid->addWidget(m_newLineComboBox, 0, 2);

	// FIXME set "encoding" label buddy to m_encodingComboBox (how do we get the "encoding" label widget?)
	m_fileWidget->setCustomWidget(i18n("Encoding:"), customWidget);

	setLayout(new QVBoxLayout);
	connect(m_fileWidget, SIGNAL(filterChanged(QString)), this, SIGNAL(filterSelected(QString)));
	layout()->addWidget(m_fileWidget);

	m_buttons = new QDialogButtonBox(this);
	m_buttons->addButton(m_fileWidget->okButton(), QDialogButtonBox::AcceptRole);
	m_buttons->addButton(m_fileWidget->cancelButton(), QDialogButtonBox::RejectRole);
	connect(m_buttons, SIGNAL(rejected()), m_fileWidget, SLOT(slotCancel()));
	connect(m_fileWidget->okButton(), SIGNAL(clicked(bool)), m_fileWidget, SLOT(slotOk()));
	connect(m_fileWidget, SIGNAL(accepted()), m_fileWidget, SLOT(accept()));
	connect(m_fileWidget, SIGNAL(accepted()), this, SLOT(accept()));
	connect(m_fileWidget->cancelButton(), SIGNAL(clicked(bool)), this, SLOT(reject()));
	layout()->addWidget(m_buttons);

	resize(m_fileWidget->dialogSizeHint());
}

QString
SaveSubtitleDialog::selectedEncoding() const
{
	return m_encodingComboBox->currentText();
}

void
SaveSubtitleDialog::setCurrentFilter(const QString &formatName)
{
	const OutputFormat *format = FormatManager::instance().output(formatName);
	if(format) {
		QStringList extensions(format->extensions());
		QString filter;
		for(QStringList::ConstIterator it = extensions.begin(), end = extensions.end(); it != end; ++it)
			filter += "*." + *it + " *." + (*it).toUpper();
		filter += '|' + formatName;

		m_fileWidget->filterWidget()->setCurrentFilter(filter);
	}
}

QString
SaveSubtitleDialog::selectedFormat() const
{
	return m_fileWidget->filterWidget()->currentText();
}

Format::NewLine
SaveSubtitleDialog::selectedNewLine() const
{
	return m_newLineComboBox ? (Format::NewLine)m_newLineComboBox->currentIndex() : Format::CurrentOS;
}

QString
SaveSubtitleDialog::outputFormatsFilter()
{
	static QString filter;

	if(filter.isEmpty()) {
		QStringList formats = FormatManager::instance().outputNames();
		for(QStringList::ConstIterator it = formats.begin(), end = formats.end(); it != end; ++it) {
			const OutputFormat *format = FormatManager::instance().output(*it);
			const QStringList &formatExtensions = format->extensions();
			QString extensions;
			for(QStringList::ConstIterator extIt = formatExtensions.begin(), extEnd = formatExtensions.end(); extIt != extEnd; ++extIt)
				extensions += "*." + *extIt + " *." + (*extIt).toUpper();
			filter += '\n' + extensions + '|' + format->name();
		}

		filter = filter.trimmed();
	}

	return filter;
}

/*virtual*/ void
SaveSubtitleDialog::showEvent(QShowEvent */*event*/)
{
	KConfigGroup group = KSharedConfig::openConfig()->group("FileDialogSize");
	restoreGeometry(group.readEntry("WindowSize", QByteArray()));
	m_fileWidget->restoreGeometry(group.readEntry("FileWidget", QByteArray()));
}

/*virtual*/ void
SaveSubtitleDialog::hideEvent(QHideEvent */*event*/)
{
	KConfigGroup group = KSharedConfig::openConfig()->group("FileDialogSize");
	group.writeEntry("WindowSize", saveGeometry());
	group.writeEntry("FileWidget", m_fileWidget->saveGeometry());
}

QUrl
SaveSubtitleDialog::selectedUrl() const
{
	return QUrl::fromLocalFile(m_fileWidget->selectedFile());
}
