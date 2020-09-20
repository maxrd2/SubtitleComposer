/*
 * Copyright (C) 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
 * Copyright (C) 2010-2019 Mladen Milinkovic <max@smoothware.net>
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

#include "playerwidget.h"
#include "application.h"
#include "actions/useractionnames.h"
#include "helpers/commondefs.h"
#include "core/subtitleiterator.h"
#include "videoplayer/videoplayer.h"
#include "widgets/layeredwidget.h"
#include "widgets/textoverlaywidget.h"
#include "widgets/attachablewidget.h"
#include "widgets/pointingslider.h"
#include "widgets/timeedit.h"

#include <QEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QKeyEvent>

#include <QMenu>
#include <QPushButton>
#include <QDesktopWidget>
#include <QCursor>
#include <QLabel>
#include <QToolButton>
#include <QGroupBox>
#include <QGridLayout>

#include <KConfigGroup>
#include <KMessageBox>
#include <KLocalizedString>

using namespace SubtitleComposer;

// FIXME WTF is this!??
#define MAGIC_NUMBER -1
#define HIDE_MOUSE_MSECS 1000
#define UNKNOWN_LENGTH_STRING (" / " + Time().toString(false) + ' ')

PlayerWidget::PlayerWidget(QWidget *parent) :
	QWidget(parent),
	m_subtitle(0),
	m_translationMode(false),
	m_showTranslation(false),
	m_overlayLine(0),
	m_playingLine(nullptr),
	m_pauseAfterPlayingLine(nullptr),
	m_fullScreenTID(0),
	m_fullScreenMode(false),
	m_player(VideoPlayer::instance()),
	m_lengthString(UNKNOWN_LENGTH_STRING),
	m_updatePositionControls(1),
	m_updateVideoPosition(false),
	m_updateVolumeControls(true),
	m_updatePlayerVolume(false),
	m_showPositionTimeEdit(SCConfig::showPositionTimeEdit())
{
	m_layeredWidget = new LayeredWidget(this);
	m_layeredWidget->setAcceptDrops(true);
	m_layeredWidget->installEventFilter(this);
	m_layeredWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	m_player->init(m_layeredWidget);

	m_seekSlider = new PointingSlider(Qt::Horizontal, this);
	m_seekSlider->setTickPosition(QSlider::NoTicks);
	m_seekSlider->setTracking(false);
	m_seekSlider->setMinimum(0);
	m_seekSlider->setMaximum(1000);
	m_seekSlider->setPageStep(10);
	m_seekSlider->setFocusPolicy(Qt::NoFocus);

	m_infoControlsGroupBox = new QWidget(this);
	m_infoControlsGroupBox->setAcceptDrops(true);
	m_infoControlsGroupBox->installEventFilter(this);

	QLabel *positionTagLabel = new QLabel(m_infoControlsGroupBox);
	positionTagLabel->setText(i18n("<b>Position</b>"));
	positionTagLabel->installEventFilter(this);

	m_positionLabel = new QLabel(m_infoControlsGroupBox);
	m_positionEdit = new TimeEdit(m_infoControlsGroupBox);
	m_positionEdit->setFocusPolicy(Qt::NoFocus);

	QLabel *lengthTagLabel = new QLabel(m_infoControlsGroupBox);
	lengthTagLabel->setText(i18n("<b>Length</b>"));
	lengthTagLabel->installEventFilter(this);
	m_lengthLabel = new QLabel(m_infoControlsGroupBox);

	QLabel *fpsTagLabel = new QLabel(m_infoControlsGroupBox);
	fpsTagLabel->setText(i18n("<b>FPS</b>"));
	fpsTagLabel->installEventFilter(this);
	m_fpsLabel = new QLabel(m_infoControlsGroupBox);
	m_fpsLabel->setMinimumWidth(m_positionEdit->sizeHint().width());        // sets the minimum width for the whole group

	QLabel *rateTagLabel = new QLabel(m_infoControlsGroupBox);
	rateTagLabel->setText(i18n("<b>Playback Rate</b>"));
	rateTagLabel->installEventFilter(this);
	m_rateLabel = new QLabel(m_infoControlsGroupBox);

	m_volumeSlider = new PointingSlider(Qt::Vertical, this);
	m_volumeSlider->setFocusPolicy(Qt::NoFocus);
	m_volumeSlider->setTickPosition(QSlider::NoTicks);
//  m_volumeSlider->setInvertedAppearance( true );
	m_volumeSlider->setTracking(true);
	m_volumeSlider->setPageStep(5);
	m_volumeSlider->setMinimum(0);
	m_volumeSlider->setMaximum(100);
	m_volumeSlider->setFocusPolicy(Qt::NoFocus);

	QGridLayout *videoControlsLayout = new QGridLayout();
	videoControlsLayout->setMargin(0);
	videoControlsLayout->setSpacing(2);
	videoControlsLayout->addWidget(createToolButton(this, ACT_PLAY_PAUSE, 16), 0, 0);
	videoControlsLayout->addWidget(createToolButton(this, ACT_STOP, 16), 0, 1);
	videoControlsLayout->addWidget(createToolButton(this, ACT_SEEK_BACKWARD, 16), 0, 2);
	videoControlsLayout->addWidget(createToolButton(this, ACT_SEEK_FORWARD, 16), 0, 3);
	videoControlsLayout->addItem(new QSpacerItem(2, 2), 0, 4);
	videoControlsLayout->addWidget(createToolButton(this, ACT_SEEK_TO_PREVIOUS_LINE, 16), 0, 5);
	videoControlsLayout->addWidget(createToolButton(this, ACT_SEEK_TO_NEXT_LINE, 16), 0, 6);
	videoControlsLayout->addItem(new QSpacerItem(2, 2), 0, 7);
	videoControlsLayout->addWidget(createToolButton(this, ACT_SET_CURRENT_LINE_SHOW_TIME, 16), 0, 8);
	videoControlsLayout->addWidget(createToolButton(this, ACT_SET_CURRENT_LINE_HIDE_TIME, 16), 0, 9);
	videoControlsLayout->addItem(new QSpacerItem(2, 2), 0, 10);
	videoControlsLayout->addWidget(createToolButton(this, ACT_CURRENT_LINE_FOLLOWS_VIDEO, 16), 0, 11);
	videoControlsLayout->addItem(new QSpacerItem(2, 2), 0, 12);
	videoControlsLayout->addWidget(createToolButton(this, ACT_PLAY_RATE_DECREASE, 16), 0, 13);
	videoControlsLayout->addWidget(createToolButton(this, ACT_PLAY_RATE_INCREASE, 16), 0, 14);
	videoControlsLayout->addWidget(m_seekSlider, 0, 15);

	QGridLayout *audioControlsLayout = new QGridLayout();
	audioControlsLayout->setMargin(0);
	audioControlsLayout->addWidget(createToolButton(this, ACT_TOGGLE_MUTED, 16), 0, 0, Qt::AlignHCenter);
	audioControlsLayout->addWidget(m_volumeSlider, 1, 0, Qt::AlignHCenter);

	QGridLayout *infoControlsLayout = new QGridLayout(m_infoControlsGroupBox);
	infoControlsLayout->setSpacing(5);
	infoControlsLayout->addWidget(fpsTagLabel, 0, 0);
	infoControlsLayout->addWidget(m_fpsLabel, 1, 0);
	infoControlsLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 2, 0);
	infoControlsLayout->addWidget(rateTagLabel, 3, 0);
	infoControlsLayout->addWidget(m_rateLabel, 4, 0);
	infoControlsLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 5, 0);
	infoControlsLayout->addWidget(lengthTagLabel, 6, 0);
	infoControlsLayout->addWidget(m_lengthLabel, 7, 0);
	infoControlsLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 8, 0);
	infoControlsLayout->addWidget(positionTagLabel, 9, 0);
	infoControlsLayout->addWidget(m_positionLabel, 10, 0);
	infoControlsLayout->addWidget(m_positionEdit, 11, 0);

	m_mainLayout = new QGridLayout(this);
	m_mainLayout->setMargin(0);
	m_mainLayout->setSpacing(5);
	m_mainLayout->addWidget(m_infoControlsGroupBox, 0, 0, 2, 1);
	m_mainLayout->addWidget(m_layeredWidget, 0, 1);
	m_mainLayout->addLayout(audioControlsLayout, 0, 2);
	m_mainLayout->addLayout(videoControlsLayout, 1, 1);
	m_mainLayout->addWidget(createToolButton(this, ACT_TOGGLE_FULL_SCREEN, 16), 1, 2);

	m_fullScreenControls = new AttachableWidget(AttachableWidget::Bottom, 4);
	m_fullScreenControls->setAutoFillBackground(true);
	m_layeredWidget->setWidgetMode(m_fullScreenControls, LayeredWidget::IgnoreResize);

	m_fsSeekSlider = new PointingSlider(Qt::Horizontal, m_fullScreenControls);
	m_fsSeekSlider->setTickPosition(QSlider::NoTicks);
	m_fsSeekSlider->setTracking(false);
	m_fsSeekSlider->setMinimum(0);
	m_fsSeekSlider->setMaximum(1000);
	m_fsSeekSlider->setPageStep(10);

	m_fsVolumeSlider = new PointingSlider(Qt::Horizontal, m_fullScreenControls);
	m_fsVolumeSlider->setFocusPolicy(Qt::NoFocus);
	m_fsVolumeSlider->setTickPosition(QSlider::NoTicks);
	m_fsVolumeSlider->setTracking(true);
	m_fsVolumeSlider->setPageStep(5);
	m_fsVolumeSlider->setMinimum(0);
	m_fsVolumeSlider->setMaximum(100);

	m_fsPositionLabel = new QLabel(m_fullScreenControls);
	QPalette fsPositionPalette;
	fsPositionPalette.setColor(m_fsPositionLabel->backgroundRole(), Qt::black);
	fsPositionPalette.setColor(m_fsPositionLabel->foregroundRole(), Qt::white);
	m_fsPositionLabel->setPalette(fsPositionPalette);
	m_fsPositionLabel->setAutoFillBackground(true);
	m_fsPositionLabel->setFrameShape(QFrame::Panel);
	m_fsPositionLabel->setText(Time().toString(false) + " /  " + Time().toString(false));
	m_fsPositionLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	m_fsPositionLabel->adjustSize();
	m_fsPositionLabel->setMinimumWidth(m_fsPositionLabel->width());

	QHBoxLayout *fullScreenControlsLayout = new QHBoxLayout(m_fullScreenControls);
	fullScreenControlsLayout->setMargin(0);
	fullScreenControlsLayout->setSpacing(0);

	const int FS_BUTTON_SIZE = 32;
	fullScreenControlsLayout->addWidget(createToolButton(m_fullScreenControls, ACT_PLAY_PAUSE, FS_BUTTON_SIZE));
	fullScreenControlsLayout->addWidget(createToolButton(m_fullScreenControls, ACT_STOP, FS_BUTTON_SIZE));
	fullScreenControlsLayout->addWidget(createToolButton(m_fullScreenControls, ACT_SEEK_BACKWARD, FS_BUTTON_SIZE));
	fullScreenControlsLayout->addWidget(createToolButton(m_fullScreenControls, ACT_SEEK_FORWARD, FS_BUTTON_SIZE));
	fullScreenControlsLayout->addSpacing(3);
	fullScreenControlsLayout->addWidget(createToolButton(m_fullScreenControls, ACT_SEEK_TO_PREVIOUS_LINE, FS_BUTTON_SIZE));
	fullScreenControlsLayout->addWidget(createToolButton(m_fullScreenControls, ACT_SEEK_TO_NEXT_LINE, FS_BUTTON_SIZE));
	fullScreenControlsLayout->addSpacing(3);
	fullScreenControlsLayout->addWidget(createToolButton(m_fullScreenControls, ACT_PLAY_RATE_DECREASE, FS_BUTTON_SIZE));
	fullScreenControlsLayout->addWidget(createToolButton(m_fullScreenControls, ACT_PLAY_RATE_INCREASE, FS_BUTTON_SIZE));
	fullScreenControlsLayout->addSpacing(3);
	fullScreenControlsLayout->addWidget(m_fsSeekSlider, 9);
//  fullScreenControlsLayout->addSpacing( 1 );
	fullScreenControlsLayout->addWidget(m_fsPositionLabel);
	fullScreenControlsLayout->addWidget(m_fsVolumeSlider, 2);
	fullScreenControlsLayout->addWidget(createToolButton(m_fullScreenControls, ACT_TOGGLE_MUTED, FS_BUTTON_SIZE));
	fullScreenControlsLayout->addWidget(createToolButton(m_fullScreenControls, ACT_TOGGLE_FULL_SCREEN, FS_BUTTON_SIZE));
	m_fullScreenControls->adjustSize();

	connect(m_volumeSlider, &QAbstractSlider::valueChanged, this, &PlayerWidget::onVolumeSliderValueChanged);
	connect(m_fsVolumeSlider, &QAbstractSlider::valueChanged, this, &PlayerWidget::onVolumeSliderValueChanged);

	connect(m_seekSlider, &QAbstractSlider::valueChanged, this, &PlayerWidget::onSeekSliderValueChanged);
	connect(m_seekSlider, &QAbstractSlider::sliderMoved, this, &PlayerWidget::onSeekSliderMoved);
	connect(m_seekSlider, &QAbstractSlider::sliderPressed, this, &PlayerWidget::onSeekSliderPressed);
	connect(m_seekSlider, &QAbstractSlider::sliderReleased, this, &PlayerWidget::onSeekSliderReleased);
	connect(m_fsSeekSlider, &QAbstractSlider::valueChanged, this, &PlayerWidget::onSeekSliderValueChanged);
	connect(m_fsSeekSlider, &QAbstractSlider::sliderMoved, this, &PlayerWidget::onSeekSliderMoved);
	connect(m_fsSeekSlider, &QAbstractSlider::sliderPressed, this, &PlayerWidget::onSeekSliderPressed);
	connect(m_fsSeekSlider, &QAbstractSlider::sliderReleased, this, &PlayerWidget::onSeekSliderReleased);

	connect(m_positionEdit, &TimeEdit::valueChanged, this, &PlayerWidget::onPositionEditValueChanged);
	connect(m_positionEdit, &TimeEdit::valueEntered, this, &PlayerWidget::onPositionEditValueChanged);

	connect(SCConfig::self(), &KCoreConfigSkeleton::configChanged, this, &PlayerWidget::onConfigChanged);

	connect(m_player, &VideoPlayer::fileOpened, this, &PlayerWidget::onPlayerFileOpened);
	connect(m_player, &VideoPlayer::fileOpenError, this, &PlayerWidget::onPlayerFileOpenError);
	connect(m_player, &VideoPlayer::fileClosed, this, &PlayerWidget::onPlayerFileClosed);
	connect(m_player, &VideoPlayer::playbackError, this, &PlayerWidget::onPlayerPlaybackError);
	connect(m_player, &VideoPlayer::playing, this, &PlayerWidget::onPlayerPlaying);
	connect(m_player, &VideoPlayer::stopped, this, &PlayerWidget::onPlayerStopped);
	connect(m_player, &VideoPlayer::positionChanged, this, &PlayerWidget::onPlayerPositionChanged);
	connect(m_player, &VideoPlayer::lengthChanged, this, &PlayerWidget::onPlayerLengthChanged);
	connect(m_player, &VideoPlayer::framesPerSecondChanged, this, &PlayerWidget::onPlayerFramesPerSecondChanged);
	connect(m_player, &VideoPlayer::playbackRateChanged, this, &PlayerWidget::onPlayerPlaybackRateChanged);
	connect(m_player, &VideoPlayer::volumeChanged, this, &PlayerWidget::onPlayerVolumeChanged);
	connect(m_player, &VideoPlayer::muteChanged, m_fsVolumeSlider, &QWidget::setDisabled);
	connect(m_player, &VideoPlayer::muteChanged, m_volumeSlider, &QWidget::setDisabled);

	connect(m_player, &VideoPlayer::leftClicked, this, &PlayerWidget::onPlayerLeftClicked);
	connect(m_player, &VideoPlayer::rightClicked, this, &PlayerWidget::onPlayerRightClicked);
	connect(m_player, &VideoPlayer::doubleClicked, this, &PlayerWidget::onPlayerDoubleClicked);

	setOverlayLine(0);
	onPlayerFileClosed();
	onConfigChanged();    // initializes the font
}

PlayerWidget::~PlayerWidget()
{
	m_player->cleanup();

	m_fullScreenControls->deleteLater();
}

QWidget *
PlayerWidget::infoSidebarWidget()
{
	return m_infoControlsGroupBox;
}

QToolButton *
PlayerWidget::toolButton(QWidget *parent, const char *name)
{
	return parent->findChild<QToolButton *>(name);
}

QToolButton *
PlayerWidget::createToolButton(QWidget *parent, const char *name, int size)
{
	QToolButton *toolButton = new QToolButton(parent);
	toolButton->setObjectName(name);
	toolButton->setMinimumSize(size, size);
	toolButton->setIconSize(size >= 32 ? QSize(size - 6, size - 6) : QSize(size, size));
	toolButton->setAutoRaise(true);
	toolButton->setFocusPolicy(Qt::NoFocus);
	return toolButton;
}

void
PlayerWidget::loadConfig()
{
	onPlayerVolumeChanged(m_player->volume());
}

void
PlayerWidget::saveConfig()
{}

bool
PlayerWidget::fullScreenMode() const
{
	return m_fullScreenMode;
}

void
PlayerWidget::setFullScreenMode(bool fullScreenMode)
{
	if(m_fullScreenMode != fullScreenMode) {
		m_fullScreenMode = fullScreenMode;

		if(m_fullScreenMode) {
			increaseFontSize(18);

			window()->hide();

			// Move m_layeredWidget to a temporary widget which will be
			// displayed in full screen mode.
			// Can not call showFullScreen() on m_layeredWidget directly
			// because restoring the previous state is buggy under
			// some desktop environments / window managers.

			auto *fullScreenWidget = new QWidget;
			auto *fullScreenLayout = new QHBoxLayout;
			fullScreenLayout->setMargin(0);
			fullScreenWidget->setLayout(fullScreenLayout);
			m_layeredWidget->setParent(fullScreenWidget);
			fullScreenLayout->addWidget(m_layeredWidget);
			fullScreenWidget->showFullScreen();

			m_layeredWidget->unsetCursor();
			m_layeredWidget->setMouseTracking(true);
			m_fullScreenControls->attach(m_layeredWidget);

			m_fullScreenTID = startTimer(HIDE_MOUSE_MSECS);
		} else {
			if(m_fullScreenTID) {
				killTimer(m_fullScreenTID);
				m_fullScreenTID = 0;
			}

			decreaseFontSize(18);

			m_fullScreenControls->dettach();
			m_layeredWidget->setMouseTracking(false);
			m_layeredWidget->unsetCursor();

			// delete temporary parent widget later and set this as parent again
			m_layeredWidget->parent()->deleteLater();
			m_layeredWidget->setParent(this);

			m_mainLayout->addWidget(m_layeredWidget, 0, 1);

			window()->show();
		}
	}
}

SubtitleLine *
PlayerWidget::playingLine()
{
	return m_playingLine;
}

SubtitleLine *
PlayerWidget::overlayLine()
{
	return m_overlayLine;
}

void
PlayerWidget::plugActions()
{
	toolButton(this, ACT_STOP)->setDefaultAction(app()->action(ACT_STOP));
	toolButton(this, ACT_PLAY_PAUSE)->setDefaultAction(app()->action(ACT_PLAY_PAUSE));
	toolButton(this, ACT_SEEK_BACKWARD)->setDefaultAction(app()->action(ACT_SEEK_BACKWARD));
	toolButton(this, ACT_SEEK_FORWARD)->setDefaultAction(app()->action(ACT_SEEK_FORWARD));
	toolButton(this, ACT_SEEK_TO_PREVIOUS_LINE)->setDefaultAction(app()->action(ACT_SEEK_TO_PREVIOUS_LINE));
	toolButton(this, ACT_SEEK_TO_NEXT_LINE)->setDefaultAction(app()->action(ACT_SEEK_TO_NEXT_LINE));
	toolButton(this, ACT_SET_CURRENT_LINE_SHOW_TIME)->setDefaultAction(app()->action(ACT_SET_CURRENT_LINE_SHOW_TIME));
	toolButton(this, ACT_SET_CURRENT_LINE_HIDE_TIME)->setDefaultAction(app()->action(ACT_SET_CURRENT_LINE_HIDE_TIME));
	toolButton(this, ACT_CURRENT_LINE_FOLLOWS_VIDEO)->setDefaultAction(app()->action(ACT_CURRENT_LINE_FOLLOWS_VIDEO));
	toolButton(this, ACT_TOGGLE_MUTED)->setDefaultAction(app()->action(ACT_TOGGLE_MUTED));
	toolButton(this, ACT_TOGGLE_FULL_SCREEN)->setDefaultAction(app()->action(ACT_TOGGLE_FULL_SCREEN));
	toolButton(this, ACT_PLAY_RATE_DECREASE)->setDefaultAction(app()->action(ACT_PLAY_RATE_DECREASE));
	toolButton(this, ACT_PLAY_RATE_INCREASE)->setDefaultAction(app()->action(ACT_PLAY_RATE_INCREASE));

	toolButton(m_fullScreenControls, ACT_STOP)->setDefaultAction(app()->action(ACT_STOP));
	toolButton(m_fullScreenControls, ACT_PLAY_PAUSE)->setDefaultAction(app()->action(ACT_PLAY_PAUSE));
	toolButton(m_fullScreenControls, ACT_SEEK_BACKWARD)->setDefaultAction(app()->action(ACT_SEEK_BACKWARD));
	toolButton(m_fullScreenControls, ACT_SEEK_FORWARD)->setDefaultAction(app()->action(ACT_SEEK_FORWARD));
	toolButton(m_fullScreenControls, ACT_SEEK_TO_PREVIOUS_LINE)->setDefaultAction(app()->action(ACT_SEEK_TO_PREVIOUS_LINE));
	toolButton(m_fullScreenControls, ACT_SEEK_TO_NEXT_LINE)->setDefaultAction(app()->action(ACT_SEEK_TO_NEXT_LINE));
	toolButton(m_fullScreenControls, ACT_TOGGLE_MUTED)->setDefaultAction(app()->action(ACT_TOGGLE_MUTED));
	toolButton(m_fullScreenControls, ACT_TOGGLE_FULL_SCREEN)->setDefaultAction(app()->action(ACT_TOGGLE_FULL_SCREEN));
	toolButton(m_fullScreenControls, ACT_PLAY_RATE_DECREASE)->setDefaultAction(app()->action(ACT_PLAY_RATE_DECREASE));
	toolButton(m_fullScreenControls, ACT_PLAY_RATE_INCREASE)->setDefaultAction(app()->action(ACT_PLAY_RATE_INCREASE));
}

void
PlayerWidget::timerEvent(QTimerEvent * /*event */)
{
	if(m_currentCursorPos != m_savedCursorPos) {
		m_savedCursorPos = m_currentCursorPos;
	} else if(!m_fullScreenControls->underMouse()) {
		if(m_layeredWidget->cursor().shape() != Qt::BlankCursor)
			m_layeredWidget->setCursor(QCursor(Qt::BlankCursor));
		if(m_fullScreenControls->isAttached())
			m_fullScreenControls->toggleVisible(false);
	}
}

bool
PlayerWidget::eventFilter(QObject *object, QEvent *event)
{
	if(object == m_layeredWidget) {
		switch(event->type()) {
		case QEvent::DragEnter:
		case QEvent::Drop:
			foreach(const QUrl &url, static_cast<QDropEvent *>(event)->mimeData()->urls()) {
				if(url.scheme() == QLatin1String("file")) {
					event->accept();
					if(event->type() == QEvent::Drop) {
						app()->openVideo(url);
					}
					return true; // eat event
				}
			}
			event->ignore();
			return true;

		case QEvent::DragMove:
			return true; // eat event

		case QEvent::KeyPress: {
			// NOTE: when on full screen mode, the keyboard input is received but
			// for some reason it doesn't trigger the correct actions automatically
			// so we process the event and handle the issue ourselves.
			QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
			if(m_fullScreenMode && keyEvent->key() == Qt::Key_Escape) {
				app()->action(ACT_TOGGLE_FULL_SCREEN)->trigger();
				return true;
			}
			return app()->triggerAction(QKeySequence((keyEvent->modifiers() & ~Qt::KeypadModifier) + keyEvent->key()));
		}

		case QEvent::MouseMove: {
			QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
			if(mouseEvent->globalPos() != m_currentCursorPos) {
				m_currentCursorPos = mouseEvent->globalPos();
				if(m_layeredWidget->cursor().shape() == Qt::BlankCursor)
					m_layeredWidget->unsetCursor();
				if(m_fullScreenControls->isAttached())
					m_fullScreenControls->toggleVisible(true);
			}
			break;
		}

		default:
			;
		}

	} else if(object == m_infoControlsGroupBox || object->parent() == m_infoControlsGroupBox) {
		if(event->type() != QEvent::MouseButtonRelease)
			return false;

		QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

		if(mouseEvent->button() != Qt::RightButton)
			return false;

		QMenu menu;
		QAction *action = menu.addAction(i18n("Show editable position control"));
		action->setCheckable(true);
		action->setChecked(SCConfig::showPositionTimeEdit());

		if(menu.exec(mouseEvent->globalPos()) == action)
			SCConfig::setShowPositionTimeEdit(!SCConfig::showPositionTimeEdit());

		return true; // eat event
	}

	return false;
}

void
PlayerWidget::setSubtitle(Subtitle *subtitle)
{
	if(m_subtitle) {
		disconnect(m_subtitle, &Subtitle::linesInserted, this, &PlayerWidget::invalidateOverlayLine);
		disconnect(m_subtitle, &Subtitle::linesRemoved, this, &PlayerWidget::invalidateOverlayLine);

		m_subtitle = 0;                 // has to be set to 0 for invalidateOverlayLine

		invalidateOverlayLine();
		setPlayingLine(nullptr);
	}

	m_subtitle = subtitle;

	if(m_subtitle) {
		connect(m_subtitle, &Subtitle::linesInserted, this, &PlayerWidget::invalidateOverlayLine);
		connect(m_subtitle, &Subtitle::linesRemoved, this, &PlayerWidget::invalidateOverlayLine);
	}
}

void
PlayerWidget::setTranslationMode(bool enabled)
{
	m_translationMode = enabled;

	if(!m_translationMode)
		setShowTranslation(false);
}

void
PlayerWidget::setShowTranslation(bool showTranslation)
{
	if(m_showTranslation != showTranslation) {
		m_showTranslation = showTranslation;

		invalidateOverlayLine();
		setPlayingLine(nullptr);
	}
}

void
PlayerWidget::increaseFontSize(int points)
{
	SCConfig::setFontPointSize(SCConfig::fontPointSize() + points);
	m_player->subtitleOverlay().setFontSizePt(SCConfig::fontPointSize());
}

void
PlayerWidget::decreaseFontSize(int points)
{
	SCConfig::setFontPointSize(SCConfig::fontPointSize() - points);
	m_player->subtitleOverlay().setFontSizePt(SCConfig::fontPointSize());
}

void
PlayerWidget::updateOverlayLine(const Time &videoPosition)
{
	if(!m_subtitle)
		return;

	bool seekedBackwards = m_lastCheckedTime > videoPosition;
	m_lastCheckedTime = videoPosition;

	if(m_overlayLine) {
		if(!seekedBackwards && videoPosition <= m_overlayLine->hideTime()) {
			// m_overlayLine is the line to show or the next line to show
			if(videoPosition >= m_overlayLine->showTime()) { // m_overlayLine is the line to show
				const SString &text = m_showTranslation ? m_overlayLine->secondaryText() : m_overlayLine->primaryText();
				m_player->subtitleOverlay().setText(text.richString(SString::Verbose));
			}
			return;
		} else {
			// m_overlayLine is no longer the line to show nor the next line to show
			m_player->subtitleOverlay().setText(QString());

			setOverlayLine(nullptr);
		}
	}

	if(seekedBackwards || m_lastSearchedLineToShowTime > videoPosition) {
		// search the next line to show
		for(SubtitleIterator it(*m_subtitle); it.current(); ++it) {
			if(videoPosition <= it.current()->hideTime()) {
				m_lastSearchedLineToShowTime = videoPosition;

				setOverlayLine(it.current());

				if(m_overlayLine->showTime() <= videoPosition && videoPosition <= m_overlayLine->hideTime()) {
					const SString &text = m_showTranslation ? m_overlayLine->secondaryText() : m_overlayLine->primaryText();
					m_player->subtitleOverlay().setText(text.richString(SString::Verbose));
				}
				return;
			}
		}
	}
}

void
PlayerWidget::updatePlayingLine(const Time &videoPosition)
{
	if(!m_subtitle)
		return;

	// playing line is already correct
	if(m_playingLine && videoPosition <= m_playingLine->hideTime() && videoPosition >= m_playingLine->showTime())
		return;

	// overlay line is playing line
	if(m_overlayLine && videoPosition <= m_overlayLine->hideTime() && videoPosition >= m_overlayLine->showTime()) {
		setPlayingLine(m_overlayLine);
		return;
	}

	// iterate through all lines and find the playing line
	SubtitleIterator it(*m_subtitle);
	while(it.current()) {
		if(videoPosition <= it.current()->hideTime() && videoPosition >= it.current()->showTime()) {
			setPlayingLine(it.current());
			return;
		}
		++it;
	}

	setPlayingLine(nullptr);
}

void
PlayerWidget::pauseAfterPlayingLine(const SubtitleLine *line)
{
	m_pauseAfterPlayingLine = line;
}

void
PlayerWidget::invalidateOverlayLine()
{
	m_player->subtitleOverlay().setText(QString());

	setOverlayLine(0);

	if(m_player->position() >= 0.0)
		updateOverlayLine((long)(m_player->position() * 1000));
}

void
PlayerWidget::setOverlayLine(SubtitleLine *line)
{
	if(m_overlayLine) {
		disconnect(m_overlayLine, &SubtitleLine::showTimeChanged, this, &PlayerWidget::invalidateOverlayLine);
		disconnect(m_overlayLine, &SubtitleLine::hideTimeChanged, this, &PlayerWidget::invalidateOverlayLine);
	}

	m_overlayLine = line;

	if(m_overlayLine) {
		connect(m_overlayLine, &SubtitleLine::showTimeChanged, this, &PlayerWidget::invalidateOverlayLine);
		connect(m_overlayLine, &SubtitleLine::hideTimeChanged, this, &PlayerWidget::invalidateOverlayLine);
	} else
		m_lastSearchedLineToShowTime = Time::MaxMseconds;
}

void
PlayerWidget::setPlayingLine(SubtitleLine *line)
{
	if(!line || m_playingLine != line) {
		m_playingLine = line;
		emit playingLineChanged(m_playingLine);
	}
}

void
PlayerWidget::updatePositionEditVisibility()
{
	if(m_showPositionTimeEdit && m_player->state() >= VideoPlayer::Playing)
		m_positionEdit->show();
	else
		m_positionEdit->hide();
}

void
PlayerWidget::onVolumeSliderValueChanged(int value)
{
	if(m_updatePlayerVolume) {
		m_updatePlayerVolume = false;
		m_updateVolumeControls = false;

		if(sender() == m_fsVolumeSlider)
			m_volumeSlider->setValue(value);
		else
			m_fsVolumeSlider->setValue(value);

		m_player->setVolume(value);

		m_updateVolumeControls = true;
		m_updatePlayerVolume = true;
	}
}

void
PlayerWidget::onSeekSliderPressed()
{
	m_updatePositionControls = 0;
}

void
PlayerWidget::onSeekSliderReleased()
{
	m_updatePositionControls = MAGIC_NUMBER;
}

void
PlayerWidget::onSeekSliderValueChanged(int value)
{
	if(m_updateVideoPosition) {
		m_updatePositionControls = MAGIC_NUMBER;
		pauseAfterPlayingLine(nullptr);
		m_player->seek(m_player->duration() * value / 1000.0);
	}
}

void
PlayerWidget::onSeekSliderMoved(int value)
{
	pauseAfterPlayingLine(nullptr);
	m_player->seek(m_player->duration() * value / 1000.0);

	Time time((long)(m_player->duration() * value));

	m_positionLabel->setText(time.toString());
	m_fsPositionLabel->setText(time.toString(false) + m_lengthString);

	if(m_showPositionTimeEdit)
		m_positionEdit->setValue(time.toMillis());
}

void
PlayerWidget::onPositionEditValueChanged(int position)
{
	if(m_positionEdit->hasFocus()) {
		m_updatePositionControls = MAGIC_NUMBER;
		pauseAfterPlayingLine(nullptr);
		m_player->seek(position / 1000.0);
	}
}

void
PlayerWidget::onConfigChanged()
{
	if(m_showPositionTimeEdit != SCConfig::showPositionTimeEdit()) {
		m_showPositionTimeEdit = SCConfig::showPositionTimeEdit();
		updatePositionEditVisibility();
	}

	m_player->subtitleOverlay().setTextColor(SCConfig::fontColor());
	m_player->subtitleOverlay().setFontFamily(SCConfig::fontFamily());
	m_player->subtitleOverlay().setFontSizePt(SCConfig::fontPointSize());
	m_player->subtitleOverlay().setOutlineColor(SCConfig::outlineColor());
	m_player->subtitleOverlay().setOutlineWidth(SCConfig::outlineWidth());
}

void
PlayerWidget::onPlayerFileOpened(const QString & /*filePath */)
{
	m_infoControlsGroupBox->setEnabled(true);

	updatePositionEditVisibility();
}

void
PlayerWidget::onPlayerFileOpenError(const QString &filePath, const QString &reason)
{
	QString message = i18n("<qt>There was an error opening media file %1.</qt>", filePath);
	if(!reason.isEmpty())
		message += "\n" + reason;
	KMessageBox::sorry(this, message);
}

void
PlayerWidget::onPlayerFileClosed()
{
	m_lastCheckedTime = 0;

	m_infoControlsGroupBox->setEnabled(false);

	updatePositionEditVisibility();
	m_positionEdit->setValue(0);

	m_positionLabel->setText(i18n("<i>Unknown</i>"));
	m_lengthLabel->setText(i18n("<i>Unknown</i>"));
	m_fpsLabel->setText(i18n("<i>Unknown</i>"));
	m_rateLabel->setText(i18n("<i>Unknown</i>"));

	m_lengthString = UNKNOWN_LENGTH_STRING;
	m_fsPositionLabel->setText(Time().toString(false) + m_lengthString);

	m_seekSlider->setEnabled(false);
	m_fsSeekSlider->setEnabled(false);
}

void
PlayerWidget::onPlayerPlaybackError(const QString &errorMessage)
{
	if(errorMessage.isEmpty())
		KMessageBox::error(this, i18n("Unexpected error when playing file."), i18n("Error Playing File"));
	else
		KMessageBox::detailedError(this, i18n("Unexpected error when playing file."), errorMessage, i18n("Error Playing File"));
	// onPlayerFileClosed();
}

void
PlayerWidget::onPlayerPlaying()
{
	m_seekSlider->setEnabled(true);
	m_fsSeekSlider->setEnabled(true);

	updatePositionEditVisibility();
}

void
PlayerWidget::onPlayerPositionChanged(double seconds)
{
	if(m_updatePositionControls > 0) {
		if(seconds >= 0) {
			const Time videoPosition(seconds * 1000.);

			// pause if requested
			if(m_pauseAfterPlayingLine) {
				const Time &pauseTime = m_pauseAfterPlayingLine->hideTime();
				if(videoPosition >= pauseTime) {
					m_pauseAfterPlayingLine = nullptr;
					m_player->pause();
					m_player->seek(pauseTime.toSeconds());
					return;
				}
			}

			m_positionLabel->setText(videoPosition.toString());
			m_fsPositionLabel->setText(videoPosition.toString(false) + m_lengthString);

			if(m_showPositionTimeEdit && !m_positionEdit->hasFocus())
				m_positionEdit->setValue(videoPosition.toMillis());

			updateOverlayLine(videoPosition);
			updatePlayingLine(videoPosition);

			int sliderValue = int((seconds / m_player->duration()) * 1000);

			m_updateVideoPosition = false;
			m_seekSlider->setValue(sliderValue);
			m_fsSeekSlider->setValue(sliderValue);
			m_updateVideoPosition = true;
		} else {
			m_positionLabel->setText(i18n("<i>Unknown</i>"));
			m_fsPositionLabel->setText(Time().toString(false) + m_lengthString);
		}
	} else if(m_updatePositionControls < 0) {
		m_updatePositionControls += 2;
	}
}

void
PlayerWidget::onPlayerLengthChanged(double seconds)
{
	if(seconds > 0) {
		m_lengthLabel->setText(Time((long)(seconds * 1000)).toString());
		m_lengthString = " / " + m_lengthLabel->text().left(8) + ' ';
	} else {
		m_lengthLabel->setText(i18n("<i>Unknown</i>"));
		m_lengthString = UNKNOWN_LENGTH_STRING;
	}
}

void
PlayerWidget::onPlayerFramesPerSecondChanged(double fps)
{
	m_fpsLabel->setText(fps > 0 ? QString::number(fps, 'f', 3) : i18n("<i>Unknown</i>"));
}

void
PlayerWidget::onPlayerPlaybackRateChanged(double rate)
{
	m_rateLabel->setText(rate > .0 ? QStringLiteral("%1x").arg(rate, 0, 'g', 3) : i18n("<i>Unknown</i>"));
}

void
PlayerWidget::onPlayerStopped()
{
	onPlayerPositionChanged(0);

	m_seekSlider->setEnabled(false);
	m_fsSeekSlider->setEnabled(false);

	setPlayingLine(nullptr);

	updatePositionEditVisibility();
}

void
PlayerWidget::onPlayerVolumeChanged(double volume)
{
	if(m_updateVolumeControls) {
		m_updatePlayerVolume = false;
		m_volumeSlider->setValue((int)(volume + 0.5));
		m_fsVolumeSlider->setValue((int)(volume + 0.5));
		m_updatePlayerVolume = true;
	}
}

void
PlayerWidget::onPlayerLeftClicked(const QPoint & /*point */)
{
	m_player->togglePlayPaused();
}

void
PlayerWidget::onPlayerRightClicked(const QPoint &point)
{
	static QMenu *menu = new QMenu(this);

	menu->clear();

	menu->addAction(app()->action(ACT_OPEN_VIDEO));
	menu->addAction(app()->action(ACT_CLOSE_VIDEO));

	menu->addSeparator();

	menu->addAction(app()->action(ACT_TOGGLE_FULL_SCREEN));

	menu->addSeparator();

	menu->addAction(app()->action(ACT_STOP));
	menu->addAction(app()->action(ACT_PLAY_PAUSE));
	menu->addAction(app()->action(ACT_SEEK_BACKWARD));
	menu->addAction(app()->action(ACT_SEEK_FORWARD));

	menu->addSeparator();

	menu->addAction(app()->action(ACT_SEEK_TO_PREVIOUS_LINE));
	menu->addAction(app()->action(ACT_SEEK_TO_NEXT_LINE));

	menu->addSeparator();

	menu->addAction(app()->action(ACT_PLAY_RATE_DECREASE));
	menu->addAction(app()->action(ACT_PLAY_RATE_INCREASE));

	menu->addSeparator();

	menu->addAction(app()->action(ACT_SET_ACTIVE_AUDIO_STREAM));
	menu->addAction(app()->action(ACT_INCREASE_VOLUME));
	menu->addAction(app()->action(ACT_DECREASE_VOLUME));
	menu->addAction(app()->action(ACT_TOGGLE_MUTED));

	menu->addSeparator();

	if(m_translationMode)
		menu->addAction(app()->action(ACT_SET_ACTIVE_SUBTITLE_STREAM));

	menu->addAction(app()->action(ACT_INCREASE_SUBTITLE_FONT));
	menu->addAction(app()->action(ACT_DECREASE_SUBTITLE_FONT));

	// NOTE do not use popup->exec() here!!! it freezes the application
	// when using the mplayer backend. i think it's related to the fact
	// that exec() creates a different event loop and the mplayer backend
	// depends on the main loop for catching synchronization signals
	menu->popup(point);
}

void
PlayerWidget::onPlayerDoubleClicked(const QPoint & /*point */)
{
	app()->toggleFullScreenMode();
}
