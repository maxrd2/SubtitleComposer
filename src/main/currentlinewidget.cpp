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

#include "currentlinewidget.h"
#include "application.h"
#include "configs/spellingconfig.h"
#include "actions/useractionnames.h"
#include "../widgets/timeedit.h"
#include "../widgets/simplerichtextedit.h"

#include <QtCore/QTimer>
#include <QtGui/QLabel>
#include <QtGui/QToolButton>
#include <QtGui/QGroupBox>
#include <QtGui/QGridLayout>
#include <QtGui/QKeyEvent>

#include <KDebug>
#include <KLocale>
#include <KIcon>
#include <KConfigGroup>

using namespace SubtitleComposer;

QToolButton *
CurrentLineWidget::createToolButton(const QString &text, const char *icon, QObject *receiver, const char *slot, bool checkable /* = true*/)
{
	QToolButton *toolButton = new QToolButton(this);
	toolButton->setToolTip(text);
	toolButton->setIcon(KIcon(icon));
	toolButton->setMinimumSize(20, 20);
	toolButton->setMaximumSize(20, 20);
	toolButton->setCheckable(checkable);
	toolButton->setAutoRaise(true);
	toolButton->setFocusPolicy(Qt::NoFocus);
	connect(toolButton, SIGNAL(clicked()), receiver, slot);
	return toolButton;
}

CurrentLineWidget::CurrentLineWidget(QWidget *parent) :
	QWidget(parent),
	m_currentLine(0),
	m_translationMode(false),
	m_updateCurrentLine(true),
	m_updateControls(true),
	m_updateShorcutsTimer(new QTimer(this))
{
	QGroupBox *timesControlsGroupBox = new QGroupBox(this);

	QLabel *showTimeLabel = new QLabel(timesControlsGroupBox);
	showTimeLabel->setText(i18n("<b>Show</b>"));
	m_showTimeEdit = new TimeEdit(timesControlsGroupBox);
	m_showTimeEdit->setFocusPolicy(Qt::ClickFocus);

	QLabel *hideTimeLabel = new QLabel(timesControlsGroupBox);
	hideTimeLabel->setText(i18n("<b>Hide</b>"));
	m_hideTimeEdit = new TimeEdit(timesControlsGroupBox);
	m_hideTimeEdit->setFocusPolicy(Qt::ClickFocus);

	QLabel *durationTimeLabel = new QLabel(timesControlsGroupBox);
	durationTimeLabel->setText(i18n("<b>Duration</b>"));
	m_durationTimeEdit = new TimeEdit(timesControlsGroupBox);
	m_durationTimeEdit->setFocusPolicy(Qt::ClickFocus);

	QGridLayout *buttonsLayouts[2];

	for(int index = 0; index < 2; ++index) {
		m_textLabels[index] = new QLabel(this);
		m_textLabels[index]->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		m_textLabels[index]->setTextFormat(Qt::PlainText);
		m_textLabels[index]->setWordWrap(true);

		m_textEdits[index] = new SimpleRichTextEdit(this);
		m_textEdits[index]->setTabChangesFocus(true);
		m_textEdits[index]->installEventFilter(this);

		m_boldButtons[index] = createToolButton(i18n("Toggle Bold"), "format-text-bold", m_textEdits[index], SLOT(toggleFontBold()));
		m_italicButtons[index] = createToolButton(i18n("Toggle Italic"), "format-text-italic", m_textEdits[index], SLOT(toggleFontItalic()));
		m_underlineButtons[index] = createToolButton(i18n("Toggle Underline"), "format-text-underline", m_textEdits[index], SLOT(toggleFontUnderline()));
		m_strikeThroughButtons[index] = createToolButton(i18n("Toggle Strike Through"), "format-text-strikethrough", m_textEdits[index], SLOT(toggleFontStrikeOut()));
		m_textColorButtons[index] = createToolButton(i18n("Change Text Color"), "format-text-color", m_textEdits[index], SLOT(changeTextColor()), false);

		buttonsLayouts[index] = new QGridLayout();
		buttonsLayouts[index]->setContentsMargins(0, 0, 5, 0);
		buttonsLayouts[index]->addWidget(m_boldButtons[index], 0, 0, Qt::AlignBottom);
		buttonsLayouts[index]->addWidget(m_italicButtons[index], 0, 1, Qt::AlignBottom);
		buttonsLayouts[index]->addWidget(m_underlineButtons[index], 0, 2, Qt::AlignBottom);
		buttonsLayouts[index]->addWidget(m_strikeThroughButtons[index], 0, 3, Qt::AlignBottom);
		buttonsLayouts[index]->addWidget(m_textColorButtons[index], 0, 4, Qt::AlignBottom);
	}

	QFont font = m_textLabels[0]->font();
	font.setPointSize(font.pointSize() - 1);
	m_textLabels[0]->setFont(font);
	m_textLabels[1]->setFont(font);

	QGridLayout *timesControlsLayout = new QGridLayout(timesControlsGroupBox);
	timesControlsLayout->addWidget(showTimeLabel, 0, 0);
	timesControlsLayout->addWidget(m_showTimeEdit, 0, 1);
	timesControlsLayout->addWidget(hideTimeLabel, 1, 0);
	timesControlsLayout->addWidget(m_hideTimeEdit, 1, 1);
	timesControlsLayout->addWidget(durationTimeLabel, 2, 0);
	timesControlsLayout->addWidget(m_durationTimeEdit, 2, 1);

	m_mainLayout = new QGridLayout(this);
	m_mainLayout->setMargin(0);
	m_mainLayout->setSpacing(0);
	m_mainLayout->setColumnMinimumWidth(1, 5);
	m_mainLayout->setColumnMinimumWidth(4, 0);

	m_mainLayout->setColumnStretch(2, 1);

	m_mainLayout->addWidget(timesControlsGroupBox, 0, 0, 2, 1);
	m_mainLayout->addWidget(m_textLabels[0], 0, 2);
	m_mainLayout->addLayout(buttonsLayouts[0], 0, 3);
	m_mainLayout->addWidget(m_textEdits[0], 1, 2, 1, 2);
	m_mainLayout->addWidget(m_textLabels[1], 0, 5);
	m_mainLayout->addLayout(buttonsLayouts[1], 0, 6);
	m_mainLayout->addWidget(m_textEdits[1], 1, 5, 1, 2);

	int maxTextHeight = timesControlsGroupBox->minimumSizeHint().height() - m_boldButtons[0]->minimumSizeHint().height() + 4;
	m_textEdits[0]->setMaximumHeight(maxTextHeight);
	m_textEdits[1]->setMaximumHeight(maxTextHeight);

	timesControlsGroupBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	m_textEdits[0]->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
	m_textEdits[1]->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

	connect(m_textEdits[0], SIGNAL(selectionChanged()), this, SLOT(onPrimaryTextEditSelectionChanged()));
	connect(m_textEdits[0], SIGNAL(cursorPositionChanged()), this, SLOT(onPrimaryTextEditSelectionChanged()));
	connect(m_textEdits[0], SIGNAL(textChanged()), this, SLOT(onPrimaryTextEditChanged()));
	connect(m_textEdits[1], SIGNAL(selectionChanged()), this, SLOT(onSecondaryTextEditSelectionChanged()));
	connect(m_textEdits[1], SIGNAL(cursorPositionChanged()), this, SLOT(onSecondaryTextEditSelectionChanged()));
	connect(m_textEdits[1], SIGNAL(textChanged()), this, SLOT(onSecondaryTextEditChanged()));
	connect(m_showTimeEdit, SIGNAL(valueChanged(int)), this, SLOT(onShowTimeEditChanged(int)));
	connect(m_hideTimeEdit, SIGNAL(valueChanged(int)), this, SLOT(onHideTimeEditChanged(int)));
	connect(m_durationTimeEdit, SIGNAL(valueChanged(int)), this, SLOT(onDurationTimeEditChanged(int)));

	setTranslationMode(m_translationMode);

	m_updateShorcutsTimer->setInterval(1000);
	m_updateShorcutsTimer->setSingleShot(true);

	connect(m_updateShorcutsTimer, SIGNAL(timeout()), this, SLOT(updateShortcuts()));

	connect(app()->spellingConfig(), SIGNAL(optionChanged(const QString &, const QString &)), this, SLOT(onSpellingOptionChanged(const QString &, const QString &)));
}

CurrentLineWidget::~CurrentLineWidget()
{}

QString
CurrentLineWidget::focusedText() const
{
	for(int index = 0; index < 2; ++index)
		if(m_textEdits[index]->hasFocus() && m_textEdits[index]->hasSelection())
			return m_textEdits[index]->selectedText();
	return QString();
}

void
CurrentLineWidget::loadConfig()
{
	KConfigGroup group(KGlobal::config()->group("Current Line Settings"));

	m_textEdits[0]->setCheckSpellingEnabled(group.readEntry<bool>("PrimaryCheckSpelling", false));
	m_textEdits[1]->setCheckSpellingEnabled(group.readEntry<bool>("TranslationCheckSpelling", false));
}

void
CurrentLineWidget::saveConfig()
{
	KConfigGroup group(KGlobal::config()->group("Current Line Settings"));

	group.writeEntry("PrimaryCheckSpelling", m_textEdits[0]->checkSpellingEnabled());
	group.writeEntry("TranslationCheckSpelling", m_textEdits[1]->checkSpellingEnabled());
}

void
CurrentLineWidget::setSubtitle(Subtitle *subtitle)
{
	if(!subtitle)
		setCurrentLine(0);
}

void
CurrentLineWidget::setCurrentLine(SubtitleLine *line)
{
	if(m_currentLine) {
		disconnect(m_currentLine, SIGNAL(primaryTextChanged(const SString &)), this, SLOT(onLinePrimaryTextChanged(const SString &)));
		disconnect(m_currentLine, SIGNAL(secondaryTextChanged(const SString &)), this, SLOT(onLineSecondaryTextChanged(const SString &)));
		disconnect(m_currentLine, SIGNAL(showTimeChanged(const Time &)), this, SLOT(onLineShowTimeChanged(const Time &)));
		disconnect(m_currentLine, SIGNAL(hideTimeChanged(const Time &)), this, SLOT(onLineHideTimeChanged(const Time &)));
	}

	m_currentLine = line;

	if(m_currentLine) {
		connect(m_currentLine, SIGNAL(primaryTextChanged(const SString &)), this, SLOT(onLinePrimaryTextChanged(const SString &)));
		connect(m_currentLine, SIGNAL(secondaryTextChanged(const SString &)), this, SLOT(onLineSecondaryTextChanged(const SString &)));
		connect(m_currentLine, SIGNAL(showTimeChanged(const Time &)), this, SLOT(onLineShowTimeChanged(const Time &)));
		connect(m_currentLine, SIGNAL(hideTimeChanged(const Time &)), this, SLOT(onLineHideTimeChanged(const Time &)));
	}

	onLineShowTimeChanged(m_currentLine ? m_currentLine->showTime() : 0);
	onLineHideTimeChanged(m_currentLine ? m_currentLine->hideTime() : 0);

	if(m_currentLine) {
		onLinePrimaryTextChanged(m_currentLine->primaryText());
		onLineSecondaryTextChanged(m_currentLine->secondaryText());
	} else {
		m_updateCurrentLine = false;

		m_textLabels[0]->setText(i18n("No current line"));
		m_textEdits[0]->setRichText(SString());
		onPrimaryTextEditSelectionChanged();

		if(m_translationMode) {
			m_textLabels[1]->setText(i18n("No current line"));
			m_textEdits[1]->setRichText(SString());
			onSecondaryTextEditSelectionChanged();
		}

		m_updateCurrentLine = true;
	}

	setEnabled(m_currentLine != 0);
}

void
CurrentLineWidget::setTranslationMode(bool enabled)
{
	m_translationMode = enabled;

	if(m_translationMode) {
		m_textLabels[1]->show();
		m_textEdits[1]->show();

		m_boldButtons[1]->show();
		m_italicButtons[1]->show();
		m_underlineButtons[1]->show();
		m_strikeThroughButtons[1]->show();
		m_textColorButtons[1]->show();

		m_mainLayout->setColumnMinimumWidth(4, 5);
		m_mainLayout->setColumnStretch(5, 1);
	} else {
		m_textLabels[1]->hide();
		m_textEdits[1]->hide();

		m_boldButtons[1]->hide();
		m_italicButtons[1]->hide();
		m_underlineButtons[1]->hide();
		m_strikeThroughButtons[1]->hide();
		m_textColorButtons[1]->hide();

		m_mainLayout->setColumnMinimumWidth(4, 0);
		m_mainLayout->setColumnStretch(5, 0);
	}

	setCurrentLine(m_currentLine);
}

void
CurrentLineWidget::onPrimaryTextEditSelectionChanged()
{
	m_boldButtons[0]->setChecked(m_textEdits[0]->fontBold());
	m_italicButtons[0]->setChecked(m_textEdits[0]->fontItalic());
	m_underlineButtons[0]->setChecked(m_textEdits[0]->fontUnderline());
	m_strikeThroughButtons[0]->setChecked(m_textEdits[0]->fontStrikeOut());
}

void
CurrentLineWidget::onSecondaryTextEditSelectionChanged()
{
	m_boldButtons[1]->setChecked(m_textEdits[1]->fontBold());
	m_italicButtons[1]->setChecked(m_textEdits[1]->fontItalic());
	m_underlineButtons[1]->setChecked(m_textEdits[1]->fontUnderline());
	m_strikeThroughButtons[1]->setChecked(m_textEdits[1]->fontStrikeOut());
}

void
CurrentLineWidget::onPrimaryTextEditChanged()
{
	if(m_updateCurrentLine) {
		m_updateControls = false;

		SString text(m_textEdits[0]->richText());
		m_currentLine->setPrimaryText(text);
		m_textLabels[0]->setText(buildTextDescription(text.string()));

		m_updateControls = true;
	}
}

void
CurrentLineWidget::onSecondaryTextEditChanged()
{
	if(m_updateCurrentLine) {
		m_updateControls = false;

		SString text(m_textEdits[1]->richText());
		m_currentLine->setSecondaryText(text);
		m_textLabels[1]->setText(buildTextDescription(text.string()));

		m_updateControls = true;
	}
}

void
CurrentLineWidget::onShowTimeEditChanged(int showTime)
{
	if(m_updateCurrentLine) {
		m_updateControls = false;
		m_currentLine->setShowTime(showTime);
		m_durationTimeEdit->setValue(m_hideTimeEdit->value() - showTime);
		m_updateControls = true;
	}
}

void
CurrentLineWidget::onHideTimeEditChanged(int hideTime)
{
	if(m_updateCurrentLine) {
		m_updateControls = false;
		m_currentLine->setHideTime(hideTime);
		m_durationTimeEdit->setValue(hideTime - m_showTimeEdit->value());
		m_updateControls = true;
	}
}

void
CurrentLineWidget::onDurationTimeEditChanged(int durationTime)
{
	if(m_updateCurrentLine) {
		m_updateControls = false;
		m_currentLine->setDurationTime(durationTime);
		m_hideTimeEdit->setValue(m_showTimeEdit->value() + durationTime);
		m_updateControls = true;
	}
}

QString
CurrentLineWidget::buildTextDescription(const QString &text)
{
	static const QString currentLineText(i18n("Current line:"));
	QString expressionText(' ');

	int characters = 0;

	QStringList lines = text.split('\n');
	if(!lines.empty()) {
		characters += lines.first().length();

		if(lines.size() > 1) {
			static const QString plus(" + ");
			static const QString equals(" = ");

			expressionText += QString::number(lines.first().length());
			for(int index = 1, count = lines.size(); index < count; ++index) {
				characters += lines[index].length();
				expressionText += plus + QString::number(lines[index].length());
			}
			expressionText += equals;
		}
	}

	return currentLineText + expressionText + i18np("1 character", "%1 characters", characters);
}

void
CurrentLineWidget::onLinePrimaryTextChanged(const SString &primaryText)
{
	if(m_updateControls) {
		m_updateCurrentLine = false;

		m_textLabels[0]->setText(buildTextDescription(primaryText.string()));
		m_textEdits[0]->setRichText(primaryText);

		onPrimaryTextEditSelectionChanged();

		m_updateCurrentLine = true;
	}
}

void
CurrentLineWidget::onLineSecondaryTextChanged(const SString &secondaryText)
{
	if(m_updateControls && m_translationMode) {
		m_updateCurrentLine = false;

		m_textLabels[1]->setText(buildTextDescription(secondaryText.string()));
		m_textEdits[1]->setRichText(secondaryText);

		onSecondaryTextEditSelectionChanged();

		m_updateCurrentLine = true;
	}
}

void
CurrentLineWidget::onLineShowTimeChanged(const Time &showTime)
{
	if(m_updateControls) {
		m_updateCurrentLine = false;
		m_showTimeEdit->setValue(showTime.toMillis());
		m_updateCurrentLine = true;
	}
}

void
CurrentLineWidget::onLineHideTimeChanged(const Time &hideTime)
{
	if(m_updateControls) {
		m_updateCurrentLine = false;
		m_hideTimeEdit->setValue(hideTime.toMillis());
		m_durationTimeEdit->setValue(m_hideTimeEdit->value() - m_showTimeEdit->value());
		m_updateCurrentLine = true;
	}
}

void
CurrentLineWidget::highlightPrimary(int startIndex, int endIndex)
{
	m_textEdits[1]->clearSelection();
	m_textEdits[0]->setSelection(startIndex, endIndex);
	m_textEdits[0]->setFocus();
}

void
CurrentLineWidget::highlightSecondary(int startIndex, int endIndex)
{
	m_textEdits[0]->clearSelection();
	m_textEdits[1]->setSelection(startIndex, endIndex);
	m_textEdits[1]->setFocus();
}

void
CurrentLineWidget::setupActions()
{
	for(int index = 0; index < 2; ++index) {
		KAction *spellingAction = m_textEdits[index]->action(SimpleRichTextEdit::CheckSpelling);
		spellingAction->disconnect();
		connect(spellingAction, SIGNAL(triggered()), app(), SLOT(spellCheck()));
	}

	connect(app()->action(ACT_UNDO), SIGNAL(changed()), this, SLOT(markUpdateShortcuts()));
	connect(app()->action(ACT_REDO), SIGNAL(changed()), this, SLOT(markUpdateShortcuts()));
	connect(app()->action(ACT_SELECT_ALL_LINES), SIGNAL(changed()), this, SLOT(markUpdateShortcuts()));
	connect(app()->action(ACT_TOGGLE_SELECTED_LINES_BOLD), SIGNAL(changed()), this, SLOT(markUpdateShortcuts()));
	connect(app()->action(ACT_TOGGLE_SELECTED_LINES_ITALIC), SIGNAL(changed()), this, SLOT(markUpdateShortcuts()));
	connect(app()->action(ACT_TOGGLE_SELECTED_LINES_UNDERLINE), SIGNAL(changed()), this, SLOT(markUpdateShortcuts()));
	connect(app()->action(ACT_TOGGLE_SELECTED_LINES_STRIKETHROUGH), SIGNAL(changed()), this, SLOT(markUpdateShortcuts()));
	connect(app()->action(ACT_CHANGE_SELECTED_LINES_TEXT_COLOR), SIGNAL(changed()), this, SLOT(markUpdateShortcuts()));
}

void
CurrentLineWidget::markUpdateShortcuts()
{
	if(!m_updateShorcutsTimer->isActive())
		m_updateShorcutsTimer->start();
}

static void
mapShortcuts(SimpleRichTextEdit *textEdit, int textEditActionID, const char *appActionID)
{
	KAction *textEditAction = textEdit->action(textEditActionID);
	KAction *appAction = static_cast<KAction *>(app()->action(appActionID));
	if(textEditAction && appAction)
		textEditAction->setShortcut(appAction->shortcut());
}

void
CurrentLineWidget::onSpellingOptionChanged(const QString &option, const QString &value)
{
	if(option == SpellingConfig::keyDefaultLanguage()) {
		m_textEdits[0]->setSpellCheckingLanguage(value);
		m_textEdits[1]->setSpellCheckingLanguage(value);
	}
}

#include <KStandardShortcut>

void
CurrentLineWidget::updateShortcuts()
{
	for(int index = 0; index < 2; ++index) {
		mapShortcuts(m_textEdits[index], SimpleRichTextEdit::Undo, ACT_UNDO);
		mapShortcuts(m_textEdits[index], SimpleRichTextEdit::Redo, ACT_REDO);
		mapShortcuts(m_textEdits[index], SimpleRichTextEdit::SelectAll, ACT_SELECT_ALL_LINES);
//      m_textEdits[index]->action( SimpleRichTextEdit::SelectAll )->setShortcut( KStandardShortcut::selectAll() );
		mapShortcuts(m_textEdits[index], SimpleRichTextEdit::ToggleBold, ACT_TOGGLE_SELECTED_LINES_BOLD);
		mapShortcuts(m_textEdits[index], SimpleRichTextEdit::ToggleItalic, ACT_TOGGLE_SELECTED_LINES_ITALIC);
		mapShortcuts(m_textEdits[index], SimpleRichTextEdit::ToggleUnderline, ACT_TOGGLE_SELECTED_LINES_UNDERLINE);
		mapShortcuts(m_textEdits[index], SimpleRichTextEdit::ToggleStrikeOut, ACT_TOGGLE_SELECTED_LINES_STRIKETHROUGH);
		mapShortcuts(m_textEdits[index], SimpleRichTextEdit::ChangeTextColor, ACT_CHANGE_SELECTED_LINES_TEXT_COLOR);
	}
}

bool
CurrentLineWidget::eventFilter(QObject *object, QEvent *event)
{
	if(object == m_textEdits[0] || object == m_textEdits[1]) {
		if(event->type() == QEvent::KeyPress) {
			// NOTE: for some reason, application actions are not triggered when the text edits
			// have the focus so we have to intercept the event and handle issue ourselves.
			// When doing so, we must take care of not triggering application actions for key
			// sequences that have a special meaning in the text edit context.

			QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

			if(!keyEvent->modifiers() || keyEvent->modifiers() & (Qt::ShiftModifier | Qt::KeypadModifier)) {
				if(!keyEvent->text().isEmpty())
					return QWidget::eventFilter(object, event);

				switch(keyEvent->key()) {
				case Qt::Key_Left:
				case Qt::Key_Right:
				case Qt::Key_Up:
				case Qt::Key_Down:

				case Qt::Key_Home:
				case Qt::Key_End:
				case Qt::Key_PageUp:
				case Qt::Key_PageDown:
				case Qt::Key_Insert:
				case Qt::Key_Delete:

					return QWidget::eventFilter(object, event);
				}
			}

			QKeySequence keySequence((keyEvent->modifiers() & ~Qt::KeypadModifier) + keyEvent->key());

			KAction *action;
			foreach(action, m_textEdits[0]->actions()) {
				if(action->shortcut().primary() == keySequence || action->shortcut().alternate() == keySequence)
					return QWidget::eventFilter(object, event);
			}

			return app()->triggerAction(keySequence);
		}
	}

	return QWidget::eventFilter(object, event);
}

#include "currentlinewidget.moc"
