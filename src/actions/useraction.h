#ifndef USERACTION_H
#define USERACTION_H

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

#include <QList>

#include <QAction>

namespace SubtitleComposer {
class Subtitle;
class LinesWidget;
class CurrentLineWidget;

class UserAction : public QObject
{
	Q_OBJECT

public:
	typedef enum {
		SubClosed        = 0x1,	// Subtitle is not opened
		SubOpened        = 0x2,	// Subtitle is opened
		SubTrClosed      = 0x4,	// Subtitle is not opened
		SubTrOpened      = 0x8,	// Subtitle is opened
		SubHasLine       = 0x10,	// Subtitle opened with, at least, one line
		SubHasLines      = 0x20,	// Subtitle opened with, at least, two lines
		SubPDirty        = 0x40,	// Subtitle opened and has unsaved changes
		SubPClean        = 0x80,	// Subtitle opened or closed without unsaved changes
		SubSDirty        = 0x100,	// Subtitle opened and has unsaved changes
		SubSClean        = 0x200,	// Subtitle opened or closed without unsaved changes
		HasSelection     = 0x400,	// Subtitle opened with, at least, one line selected
		VideoClosed      = 0x800,
		VideoOpened      = 0x1000,
		VideoStopped     = 0x2000,
		VideoPlaying     = 0x4000,
		FullScreenOn     = 0x8000,
		FullScreenOff    = 0x10000,
		AnchorsNone      = 0x20000,	// None of the subtitles is anchored
		AnchorsSome      = 0x40000,	// At least one subtitle is anchored
		EditableShowTime = 0x80000,	// Selected line's show time is editable

		AnchorsMask = AnchorsNone | AnchorsSome | EditableShowTime,
		SubtitleMask = SubClosed | SubOpened | SubTrClosed | SubTrOpened | SubPDirty | SubPClean | SubSDirty | SubSClean | SubHasLine | SubHasLines | AnchorsMask,
		SelectionMask = HasSelection,
		VideoMask = VideoClosed | VideoOpened | VideoStopped | VideoPlaying,
		FullScreenMask = FullScreenOn | FullScreenOff,
		AllMask = AnchorsMask | SubtitleMask | SelectionMask | VideoMask | FullScreenMask
	} EnableFlag;

	explicit UserAction(QAction *action, int enableFlags = SubOpened);

	QAction * action();
	int enableFlags();

	bool isEnabled();
	void setActionEnabled(bool enabled);
	void setContextFlags(int contextFlags);

private:
	void updateEnabledState();

private slots:
	void onActionChanged();

private:
	QAction *m_action;
	int m_enableFlags;

	bool m_actionEnabled;
	bool m_contextEnabled;

	bool m_ignoreActionEnabledSignal;
};

class VideoPlayer;

class UserActionManager : public QObject
{
	Q_OBJECT

public:
	static UserActionManager * instance();

	void addAction(QAction *action, int enableFlags = UserAction::SubOpened);
	void addAction(UserAction *actionSpec);
	void removeAction(UserAction *actionSpec);

public slots:
	void setSubtitle(Subtitle *subtitle = 0);
	void setLinesWidget(LinesWidget *linesWidget = 0);
	void setPlayer(VideoPlayer *player = 0);
	void setTranslationMode(bool translationMode);
	void setFullScreenMode(bool fullScreenMode);

private:
	UserActionManager();

	void updateActionsContext(int contextFlags);

private slots:
	void onSubtitleLinesChanged();
	void onPrimaryDirtyStateChanged(bool dirty);
	void onSecondaryDirtyStateChanged(bool dirty);
	void onLinesWidgetSelectionChanged();
	void onPlayerStateChanged();
	void onSubtitleAnchorsChanged();

private:
	QList<UserAction *> m_actionSpecs;

	const Subtitle *m_subtitle;
	const LinesWidget *m_linesWidget;
	const VideoPlayer *m_player;
	bool m_translationMode;

	int m_contextFlags;
};
}
#endif
