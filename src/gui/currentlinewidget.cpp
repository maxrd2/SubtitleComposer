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

#include "currentlinewidget.h"
#include "application.h"
#include "actions/useractionnames.h"
#include "widgets/timeedit.h"
#include "widgets/simplerichtextedit.h"

#include <QTimer>
#include <QLabel>
#include <QToolButton>
#include <QGroupBox>
#include <QGridLayout>
#include <QKeyEvent>
#include <QDebug>
#include <QIcon>

#include <KConfigGroup>
#include <KLocalizedString>

using namespace SubtitleComposer;

enum { COL_TIME, COL_PRIMARY, COL_SECONDARY };

CurrentLineWidget::CurrentLineWidget(QWidget *parent)
	: QWidget(parent),
	  m_subtitle(nullptr),
	  m_currentLine(nullptr),
	  m_translationMode(false)
{
	QGridLayout *mainLayout = new QGridLayout(this);
	mainLayout->setMargin(0);
	mainLayout->setSpacing(0);

	QGroupBox *timesControlsGroupBox = new QGroupBox(this);
	{
		QGridLayout *timesControlsLayout = new QGridLayout(timesControlsGroupBox);
		timesControlsLayout->setContentsMargins(5, 4, 4, 4);
		timesControlsLayout->setHorizontalSpacing(5);
		timesControlsLayout->setVerticalSpacing(2);

		QLabel *showTimeLabel = new QLabel(timesControlsGroupBox);
		showTimeLabel->setText(i18n("<b>Show</b>"));
		timesControlsLayout->addWidget(showTimeLabel, 0, 0);

		m_showTimeEdit = new TimeEdit(timesControlsGroupBox);
		m_showTimeEdit->setFocusPolicy(Qt::ClickFocus);
		timesControlsLayout->addWidget(m_showTimeEdit, 0, 1);

		QLabel *hideTimeLabel = new QLabel(timesControlsGroupBox);
		hideTimeLabel->setText(i18n("<b>Hide</b>"));
		timesControlsLayout->addWidget(hideTimeLabel, 1, 0);

		m_hideTimeEdit = new TimeEdit(timesControlsGroupBox);
		m_hideTimeEdit->setFocusPolicy(Qt::ClickFocus);
		timesControlsLayout->addWidget(m_hideTimeEdit, 1, 1);

		QLabel *durationTimeLabel = new QLabel(timesControlsGroupBox);
		durationTimeLabel->setText(i18n("<b>Duration</b>"));
		timesControlsLayout->addWidget(durationTimeLabel, 2, 0);

		m_durationTimeEdit = new TimeEdit(timesControlsGroupBox);
		m_durationTimeEdit->setFocusPolicy(Qt::ClickFocus);
		timesControlsLayout->addWidget(m_durationTimeEdit, 2, 1);
	}
	timesControlsGroupBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	mainLayout->addWidget(timesControlsGroupBox, 0, COL_TIME);

	m_boxPrimary = createLineWidgetBox(0);
	mainLayout->setColumnStretch(COL_PRIMARY, 1);
	mainLayout->addWidget(m_boxPrimary, 0, COL_PRIMARY);

	m_boxTranslation = createLineWidgetBox(1);
	mainLayout->addWidget(m_boxTranslation, 0, COL_SECONDARY);

	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

	connect(m_textEdits[0], &QTextEdit::textChanged, this, &CurrentLineWidget::onPrimaryTextEditChanged);
	connect(m_textEdits[1], &QTextEdit::textChanged, this, &CurrentLineWidget::onSecondaryTextEditChanged);
	connect(m_showTimeEdit, &TimeEdit::valueChanged, this, &CurrentLineWidget::onShowTimeEditChanged);
	connect(m_hideTimeEdit, &TimeEdit::valueChanged, this, &CurrentLineWidget::onHideTimeEditChanged);
	connect(m_durationTimeEdit, &TimeEdit::valueChanged, this, &CurrentLineWidget::onDurationTimeEditChanged);

	setTranslationMode(m_translationMode);

	connect(SCConfig::self(), &KCoreConfigSkeleton::configChanged, this, &CurrentLineWidget::onConfigChanged);
}

CurrentLineWidget::~CurrentLineWidget()
{
}

QToolButton *
CurrentLineWidget::createToolButton(const QString &text, const char *icon, bool checkable)
{
	QToolButton *btn = new QToolButton(this);
	btn->setToolTip(text);
	btn->setIcon(QIcon::fromTheme(icon));
	btn->setMinimumSize(20, 20);
	btn->setMaximumSize(20, 20);
	btn->setCheckable(checkable);
	btn->setAutoRaise(true);
	btn->setFocusPolicy(Qt::NoFocus);
	return btn;
}

QWidget *
CurrentLineWidget::createLineWidgetBox(int index)
{
	QGridLayout *layout = new QGridLayout();
	layout->setContentsMargins(2, 3, 1, 0);
	layout->setHorizontalSpacing(4);
	layout->setVerticalSpacing(2);

	QLabel *textLabel = new QLabel(this);
	textLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	textLabel->setTextFormat(Qt::PlainText);
	textLabel->setIndent(3);
	textLabel->setWordWrap(true);
	QFont f = textLabel->font();
	f.setPointSize(f.pointSize() - 1);
	textLabel->setFont(f);
	layout->addWidget(textLabel, 0, 0);
	m_textLabels[index] = textLabel;

	SimpleRichTextEdit *textEdit = new SimpleRichTextEdit(this);
	textEdit->setTabChangesFocus(true);
	textEdit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
	layout->addWidget(textEdit, 1, 0, 1, 7);
	m_textEdits[index] = textEdit;

	QToolButton *btnBold = createToolButton(i18n("Toggle Bold"), "format-text-bold");
	connect(btnBold, &QToolButton::clicked, textEdit, &SimpleRichTextEdit::toggleFontBold);
	layout->addWidget(btnBold, 0, 2, Qt::AlignBottom);

	QToolButton *btnItalic = createToolButton(i18n("Toggle Italic"), "format-text-italic");
	connect(btnItalic, &QToolButton::clicked, textEdit, &SimpleRichTextEdit::toggleFontItalic);
	layout->addWidget(btnItalic, 0, 3, Qt::AlignBottom);

	QToolButton *btnUnderline = createToolButton(i18n("Toggle Underline"), "format-text-underline");
	connect(btnUnderline, &QToolButton::clicked, textEdit, &SimpleRichTextEdit::toggleFontUnderline);
	layout->addWidget(btnUnderline, 0, 4, Qt::AlignBottom);

	QToolButton *btnStrike = createToolButton(i18n("Toggle Strike Through"), "format-text-strikethrough");
	connect(btnStrike, &QToolButton::clicked, textEdit, &SimpleRichTextEdit::toggleFontStrikeOut);
	layout->addWidget(btnStrike, 0, 5, Qt::AlignBottom);

	QToolButton *btnColor = createToolButton(i18n("Change Text Color"), "format-text-color", false);
	connect(btnColor, &QToolButton::clicked, textEdit, &SimpleRichTextEdit::changeTextColor);
	layout->addWidget(btnColor, 0, 6, Qt::AlignBottom);

	connect(textEdit, &SimpleRichTextEdit::cursorPositionChanged, [=](){
		btnBold->setChecked(textEdit->fontBold());
		btnItalic->setChecked(textEdit->fontItalic());
		btnUnderline->setChecked(textEdit->fontUnderline());
		btnStrike->setChecked(textEdit->fontStrikeOut());
	});

	QWidget *cont = new QWidget(this);
	cont->setLayout(layout);
	return cont;
}

QString
CurrentLineWidget::focusedText() const
{
	for(int index = 0; index < 2; ++index) {
		if(m_textEdits[index]->hasFocus() && m_textEdits[index]->hasSelection())
			return m_textEdits[index]->selectedText();
	}
	return QString();
}

void
CurrentLineWidget::loadConfig()
{
	KConfigGroup group(KSharedConfig::openConfig()->group("Current Line Settings"));
	m_textEdits[0]->setCheckSpellingEnabled(group.readEntry<bool>("PrimaryCheckSpelling", false));
	m_textEdits[1]->setCheckSpellingEnabled(group.readEntry<bool>("TranslationCheckSpelling", false));
}

void
CurrentLineWidget::saveConfig()
{
	KConfigGroup group(KSharedConfig::openConfig()->group("Current Line Settings"));
	group.writeEntry("PrimaryCheckSpelling", m_textEdits[0]->checkSpellingEnabled());
	group.writeEntry("TranslationCheckSpelling", m_textEdits[1]->checkSpellingEnabled());
}

void
CurrentLineWidget::setSubtitle(Subtitle *subtitle)
{
	if(m_subtitle)
		disconnect(m_subtitle, &Subtitle::lineAnchorChanged, this, &CurrentLineWidget::onLineAnchorChanged);

	m_subtitle = subtitle;

	if(subtitle)
		connect(m_subtitle, &Subtitle::lineAnchorChanged, this, &CurrentLineWidget::onLineAnchorChanged);
	else
		setCurrentLine(nullptr);
}

void
CurrentLineWidget::setCurrentLine(SubtitleLine *line)
{
	if(m_currentLine) {
		disconnect(m_currentLine, &SubtitleLine::primaryTextChanged, this, &CurrentLineWidget::onLinePrimaryTextChanged);
		disconnect(m_currentLine, &SubtitleLine::secondaryTextChanged, this, &CurrentLineWidget::onLineSecondaryTextChanged);
		disconnect(m_currentLine, &SubtitleLine::showTimeChanged, this, &CurrentLineWidget::onLineShowTimeChanged);
		disconnect(m_currentLine, &SubtitleLine::hideTimeChanged, this, &CurrentLineWidget::onLineHideTimeChanged);
	}

	m_currentLine = line;

	if(m_currentLine) {
		connect(m_currentLine, &SubtitleLine::primaryTextChanged, this, &CurrentLineWidget::onLinePrimaryTextChanged);
		connect(m_currentLine, &SubtitleLine::secondaryTextChanged, this, &CurrentLineWidget::onLineSecondaryTextChanged);
		connect(m_currentLine, &SubtitleLine::showTimeChanged, this, &CurrentLineWidget::onLineShowTimeChanged);
		connect(m_currentLine, &SubtitleLine::hideTimeChanged, this, &CurrentLineWidget::onLineHideTimeChanged);
	}

	onLineShowTimeChanged(m_currentLine ? m_currentLine->showTime() : Time());
	onLineHideTimeChanged(m_currentLine ? m_currentLine->hideTime() : Time());

	if(m_currentLine) {
		onLinePrimaryTextChanged(m_currentLine->primaryText());
		onLineSecondaryTextChanged(m_currentLine->secondaryText());
		if(m_subtitle)
			onLineAnchorChanged(m_currentLine, m_subtitle->isLineAnchored(m_currentLine));
	} else {
		m_userChangingText++;

		m_textLabels[0]->setText(i18n("No current line"));
		m_textEdits[0]->setRichText(SString());

		if(m_translationMode) {
			m_textLabels[1]->setText(i18n("No current line"));
			m_textEdits[1]->setRichText(SString());
		}

		m_userChangingText--;
	}

	setEnabled(m_currentLine != nullptr);
}

void
CurrentLineWidget::setTranslationMode(bool enabled)
{
	m_translationMode = enabled;

	QGridLayout *mainLayout = static_cast<QGridLayout *>(layout());
	if(m_translationMode) {
		mainLayout->setColumnStretch(COL_SECONDARY, 1);
		m_boxTranslation->show();
	} else {
		mainLayout->setColumnStretch(COL_SECONDARY, 0);
		m_boxTranslation->hide();
	}

	setCurrentLine(m_currentLine);
}

void
CurrentLineWidget::onPrimaryTextEditChanged()
{
	if(m_userChangingText)
		return;

	m_userChangingTime++;

	SString text(m_textEdits[0]->richText());
	if(m_currentLine)
		m_currentLine->setPrimaryText(text);
	m_textLabels[0]->setText(buildTextDescription(text.string()));

	m_userChangingTime--;
}

void
CurrentLineWidget::onSecondaryTextEditChanged()
{
	if(m_userChangingText)
		return;

	m_userChangingTime++;

	SString text(m_textEdits[1]->richText());
	if(m_currentLine)
		m_currentLine->setSecondaryText(text);
	m_textLabels[1]->setText(buildTextDescription(text.string()));

	m_userChangingTime--;
}

void
CurrentLineWidget::onShowTimeEditChanged(int showTime)
{
	if(m_userChangingText)
		return;

	m_userChangingTime++;

	m_currentLine->setShowTime(showTime, true);
	m_hideTimeEdit->setValue(m_currentLine->hideTime().toMillis());
	m_durationTimeEdit->setValue(m_hideTimeEdit->value() - showTime);

	m_userChangingTime--;
}

void
CurrentLineWidget::onHideTimeEditChanged(int hideTime)
{
	if(m_userChangingText)
		return;

	m_userChangingTime++;

	m_currentLine->setHideTime(hideTime);
	m_durationTimeEdit->setValue(hideTime - m_showTimeEdit->value());

	m_userChangingTime--;
}

void
CurrentLineWidget::onDurationTimeEditChanged(int durationTime)
{
	if(m_userChangingText)
		return;

	m_userChangingTime++;

	m_currentLine->setDurationTime(durationTime);
	m_hideTimeEdit->setValue(m_showTimeEdit->value() + durationTime);

	m_userChangingTime--;
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
CurrentLineWidget::onLineAnchorChanged(const SubtitleLine *line, bool anchored)
{
	if(m_subtitle && line == m_currentLine)
		m_showTimeEdit->setEnabled(!m_subtitle->hasAnchors() || anchored);
}

void
CurrentLineWidget::onLinePrimaryTextChanged(const SString &primaryText)
{
	if(m_userChangingTime)
		return;

	m_userChangingText++;

	m_textLabels[0]->setText(buildTextDescription(primaryText.string()));
	m_textEdits[0]->setRichText(primaryText);

	m_userChangingText--;
}

void
CurrentLineWidget::onLineSecondaryTextChanged(const SString &secondaryText)
{
	if(m_userChangingTime || !m_translationMode)
		return;

	m_userChangingText++;

	m_textLabels[1]->setText(buildTextDescription(secondaryText.string()));
	m_textEdits[1]->setRichText(secondaryText);

	m_userChangingText--;
}

void
CurrentLineWidget::onLineShowTimeChanged(const Time &showTime)
{
	if(m_userChangingTime)
		return;

	m_userChangingText++;

	m_showTimeEdit->setValue(showTime.toMillis());
	m_hideTimeEdit->setMinimumTime(QTime(0, 0, 0, 0).addMSecs(showTime.toMillis()));

	m_userChangingText--;
}

void
CurrentLineWidget::onLineHideTimeChanged(const Time &hideTime)
{
	if(m_userChangingTime)
		return;

	m_userChangingText++;

	m_hideTimeEdit->setValue(hideTime.toMillis());
	m_durationTimeEdit->setValue(m_hideTimeEdit->value() - m_showTimeEdit->value());

	m_userChangingText--;
}

void
CurrentLineWidget::selectPrimaryText(int startIndex, int endIndex)
{
	m_textEdits[1]->clearSelection();
	m_textEdits[0]->setSelection(startIndex, endIndex);
	m_textEdits[0]->setFocus();
}

void
CurrentLineWidget::selectTranslationText(int startIndex, int endIndex)
{
	m_textEdits[0]->clearSelection();
	m_textEdits[1]->setSelection(startIndex, endIndex);
	m_textEdits[1]->setFocus();
}

void
CurrentLineWidget::setupShortcut(int teActionId, const char *appActionId)
{
	QAction *appAction = qobject_cast<QAction *>(app()->action(appActionId));
	QAction *teAction1 = m_textEdits[0]->action(teActionId);
	QAction *teAction2 = m_textEdits[1]->action(teActionId);
	connect(appAction, &QAction::changed, [teAction1, teAction2, appAction](){
		teAction1->setShortcut(appAction->shortcut());
		teAction2->setShortcut(appAction->shortcut());
	});
	teAction1->setShortcut(appAction->shortcut());
	teAction2->setShortcut(appAction->shortcut());
}

void
CurrentLineWidget::setupActions()
{
	for(int index = 0; index < 2; ++index) {
		QAction *spellingAction = m_textEdits[index]->action(SimpleRichTextEdit::CheckSpelling);
		spellingAction->disconnect();
		connect(spellingAction, &QAction::triggered, app(), &Application::spellCheck);
	}

	setupShortcut(SimpleRichTextEdit::Undo, ACT_UNDO);
	setupShortcut(SimpleRichTextEdit::Redo, ACT_REDO);
	setupShortcut(SimpleRichTextEdit::SelectAll, ACT_SELECT_ALL_LINES);
	setupShortcut(SimpleRichTextEdit::ToggleBold, ACT_TOGGLE_SELECTED_LINES_BOLD);
	setupShortcut(SimpleRichTextEdit::ToggleItalic, ACT_TOGGLE_SELECTED_LINES_ITALIC);
	setupShortcut(SimpleRichTextEdit::ToggleUnderline, ACT_TOGGLE_SELECTED_LINES_UNDERLINE);
	setupShortcut(SimpleRichTextEdit::ToggleStrikeOut, ACT_TOGGLE_SELECTED_LINES_STRIKETHROUGH);
	setupShortcut(SimpleRichTextEdit::ChangeTextColor, ACT_CHANGE_SELECTED_LINES_TEXT_COLOR);
}

void
CurrentLineWidget::onConfigChanged()
{
	m_textEdits[0]->setSpellCheckingLanguage(SCConfig::defaultLanguage());
	m_textEdits[1]->setSpellCheckingLanguage(SCConfig::defaultLanguage());
}
