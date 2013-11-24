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

#include "statusbar.h"
#include "application.h"
#include "actions/useractionnames.h"

#include <QtGui/QProgressBar>
#include <QtGui/QToolButton>

using namespace SubtitleComposer;

StatusBar2::StatusBar2(QWidget *parent) :
	KStatusBar(parent)
{
	m_decodingProgressBar = new QProgressBar(this);
	m_decodingProgressBar->hide();
	m_decodingProgressBar->setFormat(i18n("Extracting audio %p%"));
	QFont font = m_decodingProgressBar->font();
	font.setPointSize(font.pointSize() - 1);
	m_decodingProgressBar->setFont(font);
	m_decodingProgressBar->setTextVisible(true);

	m_cancelDecodingButton = new QToolButton(this);
	m_cancelDecodingButton->hide();
	m_cancelDecodingButton->setAutoRaise(true);
	m_cancelDecodingButton->setFocusPolicy(Qt::NoFocus);
}

StatusBar2::~StatusBar2()
{}

void
StatusBar2::loadConfig()
{}

void
StatusBar2::saveConfig()
{}

void
StatusBar2::plugActions()
{
	m_cancelDecodingButton->setDefaultAction(app()->action(ACT_CANCEL_AUDIO_EXTRACTION));
}

void
StatusBar2::setSubtitle(Subtitle * /*subtitle */)
{}

void
StatusBar2::initDecoding()
{
	addPermanentWidget(m_decodingProgressBar);
	m_decodingProgressBar->show();
	addPermanentWidget(m_cancelDecodingButton);
	m_cancelDecodingButton->show();
	m_decodingProgressBar->setValue(0);
}

void
StatusBar2::setDecodingPosition(double position)
{
	if(position > 0.0)
		m_decodingProgressBar->setValue((int)position);
}

void
StatusBar2::setDecodingLength(double length)
{
	if(length > 0.0)
		m_decodingProgressBar->setMaximum((int)length);
}

void
StatusBar2::endDecoding()
{
	removeWidget(m_decodingProgressBar);
	m_decodingProgressBar->hide();
	removeWidget(m_cancelDecodingButton);
	m_cancelDecodingButton->hide();
}

void
StatusBar2::showEvent(QShowEvent *event)
{
	QWidget::showEvent(event);

	const int statusBarHeight = height();
	m_decodingProgressBar->setMaximumHeight(statusBarHeight - 5);
	m_decodingProgressBar->setMaximumWidth(350);
	m_cancelDecodingButton->setMaximumHeight(statusBarHeight - 4);
	m_cancelDecodingButton->setMaximumWidth(statusBarHeight - 4);
}

#include "statusbar.moc"
