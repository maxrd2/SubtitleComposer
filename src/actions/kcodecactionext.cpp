/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2018 Mladen Milinkovic <max@smoothware.net>
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

#include "kcodecactionext.h"

#include <QMenu>
#include <QStringBuilder>
#include <QTextCodec>
#include <QVariant>

#include <KActionCollection>
#include <KCharsets>
#include <KEncodingProber>
#include <KLocalizedString>

#include "application.h"
#include "scconfig.h"

void
KCodecActionExt::init()
{
	m_defaultAction = action(0);
	m_defaultAction->setCheckable(false);
	m_defaultAction->setText(m_defaultAction->text() + ": " + SCConfig::defaultSubtitlesEncoding());

	if(m_showAutoDetect) {
		m_autodetectAction = new QAction(parent());
		m_autodetectAction->setText(i18n("Autodetect"));
		m_autodetectAction->setData(QVariant(KEncodingProber::Universal));
		m_autodetectAction->setActionGroup(selectableActionGroup());
		SubtitleComposer::app()->mainWindow()->actionCollection()->setShortcutsConfigurable(m_autodetectAction, false);
		menu()->insertAction(m_defaultAction, m_autodetectAction);
	}

	menu()->insertSeparator(action(1));

	for(QAction *action: actions()) {
		KSelectAction *groupAction = qobject_cast<KSelectAction *>(action);
		if(groupAction) {
			for(QAction *subAction: groupAction->actions())
				subAction->setText(subAction->text().toUpper());
		}
	}

	if(!m_showDefault)
		menu()->removeAction(m_defaultAction);
}

KCodecActionExt::KCodecActionExt(QObject *parent, bool showAutoDetect, bool showDefault)
	: KCodecAction(parent, false),
	  m_showDefault(showDefault),
	  m_showAutoDetect(showAutoDetect)
{
	init();
}

KCodecActionExt::KCodecActionExt(const QString &text, QObject *parent, bool showAutoDetect, bool showDefault)
	: KCodecAction(text, parent, false),
	  m_showDefault(showDefault),
	  m_showAutoDetect(showAutoDetect)
{
	init();
}

KCodecActionExt::KCodecActionExt(const QIcon &icon, const QString &text, QObject *parent, bool showAutoDetect, bool showDefault)
	: KCodecAction(icon, text, parent, false),
	  m_showDefault(showDefault),
	  m_showAutoDetect(showAutoDetect)
{
	init();
}

void
KCodecActionExt::actionTriggered(QAction *action)
{
	// do not emit signals from top-level action menus
	if(action == m_autodetectAction)
		emit triggered(KEncodingProber::Universal);
	else if(action == m_defaultAction)
		emit defaultItemTriggered();
}


