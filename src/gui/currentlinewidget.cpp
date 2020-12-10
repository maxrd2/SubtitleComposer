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
#include "core/richdocument.h"
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
#include <QTextDocument>

#include <KConfigGroup>
#include <KLocalizedString>

using namespace SubtitleComposer;

enum { COL_TIME, COL_PRIMARY, COL_SECONDARY };

QTextDocument CurrentLineWidget::m_blankDoc;

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
	textEdit->setDocument(&m_blankDoc);
	textEdit->setTabChangesFocus(true);
	textEdit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
	layout->addWidget(textEdit, 1, 0, 1, 7);
	m_textEdits[index] = textEdit;
	connect(textEdit, &SimpleRichTextEdit::textChanged, [this, textEdit, textLabel](){
		textLabel->setText(buildTextDescription(textEdit->toPlainText()));
	});

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
		disconnect(m_currentLine, &SubtitleLine::showTimeChanged, this, &CurrentLineWidget::onLineShowTimeChanged);
		disconnect(m_currentLine, &SubtitleLine::hideTimeChanged, this, &CurrentLineWidget::onLineHideTimeChanged);
	}

	m_currentLine = line;

	if(m_currentLine) {
		connect(m_currentLine, &SubtitleLine::showTimeChanged, this, &CurrentLineWidget::onLineShowTimeChanged);
		connect(m_currentLine, &SubtitleLine::hideTimeChanged, this, &CurrentLineWidget::onLineHideTimeChanged);

		onLineShowTimeChanged(m_currentLine->showTime());
		onLineHideTimeChanged(m_currentLine->hideTime());

		if(m_textEdits[0]->isReadOnly())
			m_textEdits[0]->setReadOnly(false);
		m_textEdits[0]->setDocument(m_currentLine->primaryDoc());

		if(m_textEdits[1]->isReadOnly())
			m_textEdits[1]->setReadOnly(false);
		m_textEdits[1]->setDocument(m_currentLine->secondaryDoc());

		if(m_subtitle)
			onLineAnchorChanged(m_currentLine, m_subtitle->isLineAnchored(m_currentLine));

		setEnabled(true);
	} else {
		QSignalBlocker s1(m_textEdits[0]), s2(m_textEdits[1]);
		m_textLabels[0]->setText(i18n("No current line"));
		m_textEdits[0]->setDocument(&m_blankDoc);
		m_textEdits[0]->setReadOnly(true);
		m_textLabels[1]->setText(i18n("No current line"));
		m_textEdits[1]->setDocument(&m_blankDoc);
		m_textEdits[1]->setReadOnly(true);
		onLineShowTimeChanged(Time());
		onLineHideTimeChanged(Time());

		setEnabled(false);
	}
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
CurrentLineWidget::onShowTimeEditChanged(int showTime)
{
	m_currentLine->setShowTime(showTime, true);
	m_hideTimeEdit->setValue(m_currentLine->hideTime().toMillis());
	m_durationTimeEdit->setValue(m_hideTimeEdit->value() - showTime);
}

void
CurrentLineWidget::onHideTimeEditChanged(int hideTime)
{
	m_currentLine->setHideTime(hideTime);
	m_durationTimeEdit->setValue(hideTime - m_showTimeEdit->value());
}

void
CurrentLineWidget::onDurationTimeEditChanged(int durationTime)
{
	m_currentLine->setDurationTime(durationTime);
	m_hideTimeEdit->setValue(m_showTimeEdit->value() + durationTime);
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
CurrentLineWidget::onLineShowTimeChanged(const Time &showTime)
{
	QSignalBlocker s1(m_showTimeEdit), s2(m_hideTimeEdit), s3(m_durationTimeEdit);
	m_showTimeEdit->setValue(showTime.toMillis());
	m_hideTimeEdit->setMinimumTime(QTime(0, 0, 0, 0).addMSecs(showTime.toMillis()));
}

void
CurrentLineWidget::onLineHideTimeChanged(const Time &hideTime)
{
	QSignalBlocker s1(m_showTimeEdit), s2(m_hideTimeEdit), s3(m_durationTimeEdit);
	m_hideTimeEdit->setValue(hideTime.toMillis());
	m_durationTimeEdit->setValue(m_hideTimeEdit->value() - m_showTimeEdit->value());
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
CurrentLineWidget::onConfigChanged()
{
	m_textEdits[0]->setSpellCheckingLanguage(SCConfig::defaultLanguage());
	m_textEdits[1]->setSpellCheckingLanguage(SCConfig::defaultLanguage());
}
