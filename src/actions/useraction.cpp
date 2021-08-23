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

#include "useraction.h"
#include "application.h"
#include "core/subtitle.h"
#include "videoplayer/videoplayer.h"
#include "gui/treeview/lineswidget.h"
#include "gui/currentlinewidget.h"

#include <QApplication>

using namespace SubtitleComposer;

UserAction::UserAction(QAction *action, int enableFlags) :
	m_action(action),
	m_enableFlags(enableFlags),
	m_actionEnabled(action->isEnabled()),
	m_contextEnabled(false),
	m_ignoreActionEnabledSignal(false)
{
	updateEnabledState();

	connect(action, &QAction::changed, this, &UserAction::onActionChanged);
}

void
UserAction::onActionChanged()
{
	if(!m_ignoreActionEnabledSignal) {
		if(m_action->isEnabled() != isEnabled()) {
			// qDebug() << "enabled state externally changed for" << m_action->objectName();

			m_actionEnabled = m_action->isEnabled();
			updateEnabledState();
		}
	}
}

int
UserAction::enableFlags()
{
	return m_enableFlags;
}

QAction *
UserAction::action()
{
	return m_action;
}

bool
UserAction::isEnabled()
{
	return m_actionEnabled && m_contextEnabled;
}

void
UserAction::setActionEnabled(bool enabled)
{
	if(m_actionEnabled != enabled) {
		m_actionEnabled = enabled;
		updateEnabledState();
	}
}

void
UserAction::setContextFlags(int contextFlags)
{
	bool contextEnabled = (contextFlags & m_enableFlags) == m_enableFlags;
	if(m_contextEnabled != contextEnabled) {
		m_contextEnabled = contextEnabled;
		updateEnabledState();
	}
}

void
UserAction::updateEnabledState()
{
	bool enabled = m_actionEnabled && m_contextEnabled;

	if(m_action->isEnabled() != enabled) {
		m_ignoreActionEnabledSignal = true;

		m_action->setEnabled(enabled);

//		QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

//		qDebug() << "setting action" << m_action->text() << (m_action->isEnabled() ? "ENABLED" : "DISABLED");

		m_ignoreActionEnabledSignal = false;
	}
}

/// USER ACTION MANAGER

UserActionManager::UserActionManager()
	: m_actionSpecs(),
	  m_subtitle(nullptr),
	  m_linesWidget(nullptr),
	  m_translationMode(false),
	  m_contextFlags(UserAction::SubClosed | UserAction::SubTrClosed | UserAction::VideoClosed | UserAction::FullScreenOff | UserAction::AnchorsNone)
{
	VideoPlayer *videoPlayer = VideoPlayer::instance();
	connect(videoPlayer, &VideoPlayer::fileOpened, this, &UserActionManager::onPlayerStateChanged);
	connect(videoPlayer, &VideoPlayer::fileClosed, this, &UserActionManager::onPlayerStateChanged);
	connect(videoPlayer, &VideoPlayer::playing, this, &UserActionManager::onPlayerStateChanged);
	connect(videoPlayer, &VideoPlayer::paused, this, &UserActionManager::onPlayerStateChanged);
	connect(videoPlayer, &VideoPlayer::stopped, this, &UserActionManager::onPlayerStateChanged);
	onPlayerStateChanged();
}

UserActionManager *
UserActionManager::instance()
{
	static UserActionManager *actionManager = nullptr;
	if(!actionManager) {
		actionManager = new UserActionManager();
		actionManager->setParent(QApplication::instance());
	}
	return actionManager;
}

void
UserActionManager::addAction(QAction *action, int enableFlags)
{
	UserAction *a = new UserAction(action, enableFlags);
	addAction(a);
	a->setParent(this); // UserActionManager will delete *a
}

void
UserActionManager::addAction(UserAction *actionSpec)
{
	m_actionSpecs.append(actionSpec);

	actionSpec->setContextFlags(m_contextFlags);
}

void
UserActionManager::setSubtitle(const Subtitle *subtitle)
{
	if(m_subtitle) {
		disconnect(m_subtitle.constData(), &Subtitle::linesRemoved, this, &UserActionManager::onSubtitleLinesChanged);
		disconnect(m_subtitle.constData(), &Subtitle::linesInserted, this, &UserActionManager::onSubtitleLinesChanged);

		disconnect(m_subtitle.constData(), &Subtitle::primaryDirtyStateChanged, this, &UserActionManager::onPrimaryDirtyStateChanged);
		disconnect(m_subtitle.constData(), &Subtitle::secondaryDirtyStateChanged, this, &UserActionManager::onSecondaryDirtyStateChanged);

		disconnect(m_subtitle.constData(), &Subtitle::lineAnchorChanged, this, &UserActionManager::onSubtitleAnchorsChanged);
	}

	m_subtitle = subtitle;

	int newContextFlags = m_contextFlags & ~UserAction::SubtitleMask;

	if(m_subtitle) {
		connect(m_subtitle.constData(), &Subtitle::linesRemoved, this, &UserActionManager::onSubtitleLinesChanged);
		connect(m_subtitle.constData(), &Subtitle::linesInserted, this, &UserActionManager::onSubtitleLinesChanged);

		connect(m_subtitle.constData(), &Subtitle::primaryDirtyStateChanged, this, &UserActionManager::onPrimaryDirtyStateChanged);
		connect(m_subtitle.constData(), &Subtitle::secondaryDirtyStateChanged, this, &UserActionManager::onSecondaryDirtyStateChanged);

		connect(m_subtitle.constData(), &Subtitle::lineAnchorChanged, this, &UserActionManager::onSubtitleAnchorsChanged);

		newContextFlags |= UserAction::SubOpened;

		if(m_subtitle->isPrimaryDirty())
			newContextFlags |= UserAction::SubPDirty;
		else
			newContextFlags |= UserAction::SubPClean;

		if(m_translationMode) {
			newContextFlags |= UserAction::SubTrOpened;

			if(m_subtitle->isSecondaryDirty())
				newContextFlags |= UserAction::SubSDirty;
			else
				newContextFlags |= UserAction::SubSClean;
		} else {
			newContextFlags |= (UserAction::SubTrClosed | UserAction::SubSClean);
		}

		if(m_subtitle->linesCount() > 0)
			newContextFlags |= UserAction::SubHasLine;
		if(m_subtitle->linesCount() > 1)
			newContextFlags |= UserAction::SubHasLines;

		if(!m_subtitle->hasAnchors()) {
			newContextFlags |= UserAction::AnchorsNone;
			newContextFlags |= UserAction::EditableShowTime;
		} else {
			newContextFlags |= UserAction::AnchorsSome;
			const SubtitleLine *selected = m_subtitle->line(m_linesWidget->firstSelectedIndex());
			if(m_subtitle->isLineAnchored(selected))
				newContextFlags |= UserAction::EditableShowTime;
		}
	} else {
		newContextFlags |= (UserAction::SubClosed | UserAction::SubPClean | UserAction::SubTrClosed | UserAction::SubSClean);
	}

	updateActionsContext(newContextFlags);
}

void
UserActionManager::onSubtitleLinesChanged()
{
	int newContextFlags = m_contextFlags & ~(UserAction::SubHasLine | UserAction::SubHasLines);

	if(m_subtitle->linesCount() > 0)
		newContextFlags |= UserAction::SubHasLine;
	if(m_subtitle->linesCount() > 1)
		newContextFlags |= UserAction::SubHasLines;

	updateActionsContext(newContextFlags);
}

void
UserActionManager::onPrimaryDirtyStateChanged(bool dirty)
{
	int newContextFlags = m_contextFlags & ~(UserAction::SubPDirty | UserAction::SubPClean);

	if(dirty)
		newContextFlags |= UserAction::SubPDirty;
	else
		newContextFlags |= UserAction::SubPClean;

	updateActionsContext(newContextFlags);
}

void
UserActionManager::onSecondaryDirtyStateChanged(bool dirty)
{
	int newContextFlags = m_contextFlags & ~(UserAction::SubSDirty | UserAction::SubSClean);

	if(m_translationMode) {
		if(dirty)
			newContextFlags |= UserAction::SubSDirty;
		else
			newContextFlags |= UserAction::SubSClean;
	} else {
		newContextFlags |= UserAction::SubSClean;
	}

	updateActionsContext(newContextFlags);
}

void
UserActionManager::setLinesWidget(LinesWidget *linesWidget)
{
	if(m_linesWidget)
		disconnect(m_linesWidget->selectionModel(), &QItemSelectionModel::selectionChanged, this, &UserActionManager::onLinesWidgetSelectionChanged);

	m_linesWidget = linesWidget;

	if(m_linesWidget) {
		connect(m_linesWidget->selectionModel(), &QItemSelectionModel::selectionChanged, this, &UserActionManager::onLinesWidgetSelectionChanged);
		onLinesWidgetSelectionChanged();
	}
}

void
UserActionManager::onLinesWidgetSelectionChanged()
{
	int newContextFlags = m_contextFlags & ~(UserAction::SelectionMask | UserAction::EditableShowTime);

	int selectedIndex = m_linesWidget->firstSelectedIndex();

	if(selectedIndex >= 0)
		newContextFlags |= UserAction::HasSelection;

	if(m_subtitle) {
		if(!m_subtitle->hasAnchors()) {
			newContextFlags |= UserAction::EditableShowTime;
		} else {
			const SubtitleLine *line = m_subtitle->line(selectedIndex);
			if(m_subtitle->isLineAnchored(line))
				newContextFlags |= UserAction::EditableShowTime;
		}
	}

	updateActionsContext(newContextFlags);
}

void
UserActionManager::onPlayerStateChanged()
{
	int newContextFlags = m_contextFlags & ~UserAction::VideoMask;

	const int state = VideoPlayer::instance()->state();
	if(state > VideoPlayer::Opening) {
		newContextFlags |= UserAction::VideoOpened;
		if(state < VideoPlayer::Playing)
			newContextFlags |= UserAction::VideoStopped;
		else
			newContextFlags |= UserAction::VideoPlaying;
	} else {
		newContextFlags |= UserAction::VideoClosed;
	}

	updateActionsContext(newContextFlags);
}

void
UserActionManager::onSubtitleAnchorsChanged()
{
	int newContextFlags = m_contextFlags & ~UserAction::AnchorsMask;

	if(!m_subtitle->hasAnchors()) {
		newContextFlags |= UserAction::AnchorsNone;
		newContextFlags |= UserAction::EditableShowTime;
	} else {
		newContextFlags |= UserAction::AnchorsSome;
		const SubtitleLine *selected = m_subtitle->line(m_linesWidget->firstSelectedIndex());
		if(m_subtitle->isLineAnchored(selected))
			newContextFlags |= UserAction::EditableShowTime;
	}

	updateActionsContext(newContextFlags);
}

void
UserActionManager::setTranslationMode(bool translationMode)
{
	m_translationMode = translationMode;

	int newContextFlags = m_contextFlags & ~(UserAction::SubTrClosed | UserAction::SubTrOpened | UserAction::SubSDirty | UserAction::SubSClean);

	if(m_subtitle && m_translationMode) {
		newContextFlags |= UserAction::SubTrOpened;

		if(m_subtitle->isSecondaryDirty())
			newContextFlags |= UserAction::SubSDirty;
		else
			newContextFlags |= UserAction::SubSClean;
	} else {
		newContextFlags |= (UserAction::SubTrClosed | UserAction::SubSClean);
	}

	updateActionsContext(newContextFlags);
}

void
UserActionManager::setFullScreenMode(bool fullScreenMode)
{
	int newContextFlags = m_contextFlags & ~UserAction::FullScreenMask;

	if(fullScreenMode)
		newContextFlags |= UserAction::FullScreenOn;
	else
		newContextFlags |= UserAction::FullScreenOff;

	updateActionsContext(newContextFlags);
}

void
UserActionManager::updateActionsContext(int contextFlags)
{
//  if ( (m_contextFlags & UserAction::SubHasLine) != (contextFlags & UserAction::SubHasLine)  )
//      qDebug() << "has line:" << ((contextFlags & UserAction::SubHasLine) != 0);
//  if ( (m_contextFlags & UserAction::SubHasLines) != (contextFlags & UserAction::SubHasLines)  )
//      qDebug() << "has lines:" << ((contextFlags & UserAction::SubHasLines) != 0);
//  if ( (m_contextFlags & UserAction::HasSelection) != (contextFlags & UserAction::HasSelection)  )
//      qDebug() << "has selection:" << ((contextFlags & UserAction::HasSelection) != 0);

	if(m_contextFlags != contextFlags) {
		m_contextFlags = contextFlags;
		for(QList<UserAction *>::ConstIterator it = m_actionSpecs.constBegin(), end = m_actionSpecs.constEnd(); it != end; ++it)
			(*it)->setContextFlags(m_contextFlags);
	}
}


