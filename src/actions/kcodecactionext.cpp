/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2019 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
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
	setToolBarMode(MenuMode);

	if(m_mode == Save) {
		m_defaultCodecAction = addAction(i18nc("Encodings menu", "Default: %1", SCConfig::defaultSubtitlesEncoding()));
		m_defaultCodecAction->setCheckable(false);
		connect(m_defaultCodecAction, &QAction::triggered, this, [&](bool){
			emit triggered(KCharsets::charsets()->codecForName(SCConfig::defaultSubtitlesEncoding()));
		});

		m_currentCodecAction = addAction(i18nc("Encodings menu", "Current: %1", SCConfig::defaultSubtitlesEncoding()));
		m_currentCodecAction->setCheckable(false);
		connect(m_currentCodecAction, &QAction::triggered, this, [&](bool){
			emit triggered(nullptr);
		});
	} else {
		m_autodetectAction = addAction(i18nc("Encodings menu", "Autodetect"));
		m_autodetectAction->setCheckable(false);
		connect(m_autodetectAction, &QAction::triggered, this, [&](bool){
			emit triggered(nullptr);
		});
	}

	menu()->addSeparator();

	const auto encodings = KCharsets::charsets()->encodingsByScript();
	for(const QStringList &encodingsForScript: encodings) {
		KSelectAction *group = new KSelectAction(encodingsForScript.at(0), this);
		for(int i = 1; i < encodingsForScript.size(); ++i)
			group->addAction(encodingsForScript.at(i).toUpper())->setCheckable(m_mode == Open);
		connect(group, QOverload<QAction *>::of(&KSelectAction::triggered), this, [=](QAction *a){
			emit triggered(KCharsets::charsets()->codecForName(a->text()));
		});
		group->setCheckable(m_mode == Open);
		addAction(group);
	}
}

bool
KCodecActionExt::setCurrentCodec(QTextCodec *codec)
{
	if(!codec)
		codec = KCharsets::charsets()->codecForName(SCConfig::defaultSubtitlesEncoding());

	for(int i = 0, m = actions().size(); i < m; i++) {
		KSelectAction *menuAction = qobject_cast<KSelectAction *>(actions().at(i));
		if(!menuAction)
			continue;
		const QMenu *menu = menuAction->menu();
		for(int j = 0, n = menu->actions().size(); j < n; j++) {
			QAction *action = menu->actions().at(j);
			if(codec == KCharsets::charsets()->codecForName(action->text())) {
				if(m_mode == Save)
					m_currentCodecAction->setText(i18nc("Encodings menu", "Current: %1", action->text()));
				if(action->isCheckable()) {
					setCurrentAction(menuAction);
					menuAction->setCurrentAction(action);
				}
				return true;
			}
		}
	}
	return false;
}

KCodecActionExt::KCodecActionExt(QObject *parent, Mode mode)
	: KSelectAction(parent),
	  m_mode(mode)
{
	init();
}

KCodecActionExt::KCodecActionExt(const QString &text, QObject *parent, Mode mode)
	: KSelectAction(text, parent),
	  m_mode(mode)
{
	init();
}

KCodecActionExt::KCodecActionExt(const QIcon &icon, const QString &text, QObject *parent, Mode mode)
	: KSelectAction(icon, text, parent),
	  m_mode(mode)
{
	init();
}
