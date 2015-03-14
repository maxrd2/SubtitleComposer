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

#include "opensubtitledialog.h"
#include "../application.h"
#include "../../formats/formatmanager.h"

#include <QComboBox>
#include <QVBoxLayout>
#include <QPushButton>

#include <KLocalizedString>

#include <QDebug>

using namespace SubtitleComposer;

OpenSubtitleDialog::OpenSubtitleDialog(bool primary, const QUrl &startDir, const QString &encoding, QWidget *parent)
	: QDialog(parent),
	  m_fileWidget(new KFileWidget(startDir, this))
{
	setWindowTitle(primary ? i18n("Open Subtitle") : i18n("Open Translation Subtitle"));
	setModal(true);
	m_fileWidget->setOperationMode(KFileWidget::Opening);
	m_fileWidget->setMode(KFile::File | KFile::ExistingOnly);
	m_fileWidget->setFilter(inputFormatsFilter());

	m_encodingComboBox = new QComboBox(this);
	m_encodingComboBox->addItem(i18n("Autodetect"));
	m_encodingComboBox->addItems(app()->availableEncodingNames());
	m_encodingComboBox->setCurrentText(encoding.isEmpty() ? i18n("Autodetect") : encoding);
	m_encodingComboBox->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
	// FIXME set "encoding" label buddy to m_encodingComboBox (how do we get the "encoding" label widget?)
	m_fileWidget->setCustomWidget(i18n("Encoding:"), m_encodingComboBox);

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

/*virtual*/ void
OpenSubtitleDialog::showEvent(QShowEvent */*event*/)
{
	KConfigGroup group = KSharedConfig::openConfig()->group("FileDialogSize");
	restoreGeometry(group.readEntry("WindowSize", QByteArray()));
	m_fileWidget->restoreGeometry(group.readEntry("FileWidget", QByteArray()));
}

/*virtual*/ void
OpenSubtitleDialog::hideEvent(QHideEvent */*event*/)
{
	KConfigGroup group = KSharedConfig::openConfig()->group("FileDialogSize");
	group.writeEntry("WindowSize", saveGeometry());
	group.writeEntry("FileWidget", m_fileWidget->saveGeometry());
}

QUrl
OpenSubtitleDialog::selectedUrl() const
{
	return QUrl::fromLocalFile(m_fileWidget->selectedFile());
}
