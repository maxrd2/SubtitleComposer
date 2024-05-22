/*
    SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
    SPDX-FileCopyrightText: 2010-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "currentlinewidget.h"
#include "core/richtext/richdocument.h"
#include "helpers/common.h"
#include "scconfig.h"
#include "widgets/timeedit.h"
#include "widgets/simplerichtextedit.h"

#include <QComboBox>
#include <QCursor>
#include <QDebug>
#include <QGridLayout>
#include <QGroupBox>
#include <QIcon>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStringBuilder>
#include <QTextDocument>
#include <QTimer>
#include <QToolButton>

#include <KConfigGroup>
#include <KLocalizedString>

#include <forward_list>

#define TOOLBUTTON_SIZE 21

using namespace SubtitleComposer;

enum { COL_TIME, COL_PRIMARY, COL_SECONDARY };

QTextDocument CurrentLineWidget::m_blankDoc;


/// Breadcrumb Widget

namespace SubtitleComposer {

class Breadcrumb : public QWidget {
	using WidgetList = std::forward_list<QWidget *>;

public:
	Breadcrumb(SimpleRichTextEdit *textEdit, CurrentLineWidget *parent);

private:
	void crumbUpdate();
	void scheduleUpdate() { m_timer->start(100); }
	QPushButton *addButton();
	QLabel *addSeparator();
	void startLayout();
	void finishLayout();
	void hideAll();

private:
	QTimer *m_timer;
	WidgetList m_elements;
	WidgetList::const_iterator m_iter;
	WidgetList::const_iterator m_last;
	CurrentLineWidget *m_lineWidget;
	QHBoxLayout *m_layout;
	SimpleRichTextEdit *m_textEdit;
};

}

Breadcrumb::Breadcrumb(SimpleRichTextEdit *textEdit, CurrentLineWidget *parent)
	: QWidget(parent),
	  m_timer(new QTimer(this)),
	  m_lineWidget(parent),
	  m_layout(new QHBoxLayout(this)),
	  m_textEdit(textEdit)
{
	QFont f = font();
	f.setPointSize(f.pointSize() - 1);
	setFont(f);
	setContentsMargins(3, 0, 3, 0);

	m_layout->setContentsMargins(0, 0, 0, 0);
	m_layout->setSpacing(0);

	connect(m_textEdit, &SimpleRichTextEdit::textChanged, this, &Breadcrumb::scheduleUpdate);
	connect(m_textEdit, &SimpleRichTextEdit::cursorPositionChanged, this, &Breadcrumb::scheduleUpdate);
	connect(m_timer, &QTimer::timeout, this, &Breadcrumb::crumbUpdate);
	m_timer->setSingleShot(true);
}

void
Breadcrumb::startLayout()
{
	m_iter = m_elements.cbegin();
	if(m_iter != m_elements.cend()) {
		++m_iter;
		return;
	}
	m_layout->addStretch(1);
	m_elements.push_front(nullptr);
	m_last = m_elements.cbegin();
}

QPushButton *
Breadcrumb::addButton()
{
	QPushButton *b;
	if(m_iter != m_elements.cend()) {
		b = static_cast<QPushButton *>(*m_iter);
		b->show();
		++m_iter;
		return b;
	}

	b = new QPushButton(this);
	connect(b, &QPushButton::clicked, this, [&](){
		m_lineWidget->onBreadcrumbClick(
					static_cast<QPushButton *>(QObject::sender()),
					m_textEdit);
	});
	b->setFocusPolicy(Qt::NoFocus);
	m_layout->insertWidget(0, b);
	m_last = m_elements.insert_after(m_last, b);
	return b;
}

QLabel *
Breadcrumb::addSeparator()
{
	QLabel *l;
	if(m_iter != m_elements.cend()) {
		l = static_cast<QLabel *>(*m_iter);
		l->show();
		++m_iter;
		return l;
	}

	l = new QLabel($(" > "), this);
	m_layout->insertWidget(0, l);
	m_last = m_elements.insert_after(m_last, l);
	return l;
}

void
Breadcrumb::finishLayout()
{
	while(m_iter != m_elements.cend()) {
		(*m_iter)->hide();
		++m_iter;
	}
}

void
Breadcrumb::hideAll()
{
	m_iter = m_elements.cbegin();
	while(m_iter != m_elements.cend()) {
		if(*m_iter)
			(*m_iter)->hide();
		++m_iter;
	}
}

void
Breadcrumb::crumbUpdate()
{
	RichDocument *doc = qobject_cast<RichDocument *>(m_textEdit->document());
	if(!doc)
		return hideAll();
	QFontMetrics fm(font());
	const quint32 pos = m_textEdit->textCursor().position();
	RichDOM::Node *n = doc->nodeAt(pos ? pos - 1 : 0);
	if(!n)
		return hideAll();

	startLayout();
	for(;;) {
		QPushButton *b = addButton();
		b->setText(n->cssSel());
		b->setProperty("start", n->nodeStart);
		b->setProperty("end", n->nodeEnd);
		b->setFixedSize(fm.size(0, b->text()) + QSize(fm.height(), 2));
		n = n->parent;
		if(!n)
			break;
		addSeparator();
	}
	finishLayout();
}


/// CurrentLineWidget

CurrentLineWidget::CurrentLineWidget(QWidget *parent)
	: QWidget(parent),
	  m_subtitle(nullptr),
	  m_currentLine(nullptr),
	  m_translationMode(false)
{
	QGridLayout *mainLayout = new QGridLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);
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
	btn->setMinimumSize(TOOLBUTTON_SIZE, TOOLBUTTON_SIZE);
	btn->setMaximumSize(TOOLBUTTON_SIZE, TOOLBUTTON_SIZE);
	btn->setCheckable(checkable);
	btn->setAutoRaise(true);
	btn->setFocusPolicy(Qt::NoFocus);
	return btn;
}

QWidget *
CurrentLineWidget::createLineWidgetBox(int index)
{
	enum {
		COL_DURATION, COL_SPACER,
		COL_VOICE, COL_CLASS, COL_BOLD, COL_ITALIC, COL_UNDERLINE, COL_STRIKE, COL_COLOR,
		COL_TOTAL
	};

	QGridLayout *layout = new QGridLayout();
	layout->setContentsMargins(2, 3, 1, 0);
	layout->setHorizontalSpacing(4);
	layout->setVerticalSpacing(2);
	layout->setColumnStretch(0, 1);

	QLabel *textLabel = new QLabel(this);
	textLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	textLabel->setTextFormat(Qt::RichText);
	textLabel->setIndent(3);
	textLabel->setWordWrap(true);
	QFont f = textLabel->font();
	f.setPointSize(f.pointSize() - 1);
	textLabel->setFont(f);
	layout->addWidget(textLabel, 0, COL_DURATION);
	m_textLabels[index] = textLabel;

	SimpleRichTextEdit *textEdit = new SimpleRichTextEdit(this);
	textEdit->setDocument(&m_blankDoc);
	textEdit->setTabChangesFocus(true);
	textEdit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
	layout->addWidget(textEdit, 1, 0, 1, COL_TOTAL);
	m_textEdits[index] = textEdit;

	layout->addWidget(new Breadcrumb(textEdit, this), 2, 0, 1, 7);

	QToolButton *btnVoice = createToolButton(i18n("Change Voice Tag"), "actor", false);
	connect(btnVoice, &QToolButton::clicked, textEdit, &SimpleRichTextEdit::changeTextVoice);
	layout->addWidget(btnVoice, 0, COL_VOICE, Qt::AlignBottom);

	QToolButton *btnClass = createToolButton(i18n("Change Class Tag"), "format-text-code", false);
	connect(btnClass, &QToolButton::clicked, textEdit, &SimpleRichTextEdit::changeTextClass);
	layout->addWidget(btnClass, 0, COL_CLASS, Qt::AlignBottom);

	QToolButton *btnBold = createToolButton(i18n("Toggle Bold"), "format-text-bold");
	connect(btnBold, &QToolButton::clicked, textEdit, &SimpleRichTextEdit::toggleFontBold);
	layout->addWidget(btnBold, 0, COL_BOLD, Qt::AlignBottom);

	QToolButton *btnItalic = createToolButton(i18n("Toggle Italic"), "format-text-italic");
	connect(btnItalic, &QToolButton::clicked, textEdit, &SimpleRichTextEdit::toggleFontItalic);
	layout->addWidget(btnItalic, 0, COL_ITALIC, Qt::AlignBottom);

	QToolButton *btnUnderline = createToolButton(i18n("Toggle Underline"), "format-text-underline");
	connect(btnUnderline, &QToolButton::clicked, textEdit, &SimpleRichTextEdit::toggleFontUnderline);
	layout->addWidget(btnUnderline, 0, COL_UNDERLINE, Qt::AlignBottom);

	QToolButton *btnStrike = createToolButton(i18n("Toggle Strike Through"), "format-text-strikethrough");
	connect(btnStrike, &QToolButton::clicked, textEdit, &SimpleRichTextEdit::toggleFontStrikeOut);
	layout->addWidget(btnStrike, 0, COL_STRIKE, Qt::AlignBottom);

	QToolButton *btnColor = createToolButton(i18n("Change Text Color"), "format-text-color", false);
	connect(btnColor, &QToolButton::clicked, textEdit, &SimpleRichTextEdit::changeTextColor);
	layout->addWidget(btnColor, 0, COL_COLOR, Qt::AlignBottom);

	connect(textEdit, &SimpleRichTextEdit::cursorPositionChanged, this, [=](){
		btnBold->setChecked(textEdit->charFormat().fontWeight() == QFont::Bold);
		btnItalic->setChecked(textEdit->charFormat().fontItalic());
		btnUnderline->setChecked(textEdit->charFormat().fontUnderline());
		btnStrike->setChecked(textEdit->charFormat().fontStrikeOut());
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
		disconnect(m_subtitle.constData(), &Subtitle::lineAnchorChanged, this, &CurrentLineWidget::onLineAnchorChanged);

	m_subtitle = subtitle;

	if(subtitle)
		connect(m_subtitle.constData(), &Subtitle::lineAnchorChanged, this, &CurrentLineWidget::onLineAnchorChanged);
	else
		setCurrentLine(nullptr);
}

void
CurrentLineWidget::updateLabels()
{
	if(!m_currentLine)
		return;

	m_textLabels[0]->setText(buildTextDescription(true));
	m_textLabels[1]->setText(buildTextDescription(false));
}

void
CurrentLineWidget::setCurrentLine(SubtitleLine *line)
{
	if(m_currentLine) {
		disconnect(m_currentLine, nullptr, this, nullptr);
	}

	m_currentLine = line;

	if(m_currentLine) {
		connect(m_currentLine, &QObject::destroyed, this, [this](){ setCurrentLine(nullptr); });
		connect(m_currentLine, &SubtitleLine::showTimeChanged, this, &CurrentLineWidget::onLineShowTimeChanged);
		connect(m_currentLine, &SubtitleLine::hideTimeChanged, this, &CurrentLineWidget::onLineHideTimeChanged);

		RichDocument *doc = m_currentLine->primaryDoc();
		if(m_textEdits[0]->isReadOnly())
			m_textEdits[0]->setReadOnly(false);
		doc->setDefaultFont(QFont());
		m_textEdits[0]->setDocument(doc);
		connect(m_currentLine, &SubtitleLine::primaryTextChanged, this, &CurrentLineWidget::updateLabels);

		doc = m_currentLine->secondaryDoc();
		if(m_textEdits[1]->isReadOnly())
			m_textEdits[1]->setReadOnly(false);
		doc->setDefaultFont(QFont());
		m_textEdits[1]->setDocument(doc);
		connect(m_currentLine, &SubtitleLine::secondaryTextChanged, this, &CurrentLineWidget::updateLabels);

		onLineTimesChanged(m_currentLine->showTime(), m_currentLine->hideTime());

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
		onLineTimesChanged(Time(), Time());

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
	m_currentLine->setShowTime(showTime);
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
CurrentLineWidget::buildTextDescription(bool primary)
{
	static const QString blkSep = $(" <b>\u2014</b> ");
	static const QString colorTagBlank = $("<font>");
	static const QString colorTagRed = $("<font color=\"#ff0000\">");
	static const QString colorTagEnd = $("</font>");
	const QString &text = m_textEdits[primary ? 0 : 1]->toPlainText();
	QString res;

	// character count
	bool tagStarted = false;
	QStringList lines = text.split(QLatin1Char('\n'));
	if(text.length() > SCConfig::maxCharacters()) {
		tagStarted = true;
		res += colorTagRed;
	} else {
		res += colorTagBlank;
	}
	for(int i = 0; i < lines.length(); i++) {
		if(!tagStarted && i >= SCConfig::maxLines())
			res += colorTagRed;
		if(i)
			res += $(" + ");
		const bool wrap = !tagStarted && lines[i].length() > SCConfig::maxCharactersPerLine();
		if(wrap)
			res += colorTagRed;
		res += QString::number(lines.at(i).length());
		if(wrap)
			res += colorTagEnd;
	}
	if(lines.length() > 1)
		res += $(" = ") % QString::number(text.length() - lines.length() + 1);
	if(tagStarted)
		res += colorTagEnd;
	res += QLatin1Char(' ') % i18n("chars");

	res += blkSep;

	// characters/duration
	const QColor textColor = m_currentLine->durationColor(m_textLabels[0]->palette().color(QPalette::WindowText), primary);
	const QString colorDur = $("<font color=\"#%1\">").arg(textColor.rgb(), 6, 16, QLatin1Char(' '));
	const double chrDur = m_currentLine->durationTime().toMillis() / text.length();
	res += colorDur % QString::number(chrDur, 'f', 1) % colorTagEnd % QLatin1Char(' ') % i18n("ms/char")
		% blkSep
		% colorDur % QString::number(1000. / chrDur, 'f', 1) % colorTagEnd % QLatin1Char(' ') % i18n("chars/sec");

	return res;
}

void
CurrentLineWidget::onLineAnchorChanged(const SubtitleLine *line, bool anchored)
{
	if(m_subtitle && line == m_currentLine)
		m_showTimeEdit->setEnabled(!m_subtitle->hasAnchors() || anchored);
}

void
CurrentLineWidget::onLineTimesChanged(const Time &showTime, const Time &hideTime)
{
	QSignalBlocker s1(m_showTimeEdit), s2(m_hideTimeEdit), s3(m_durationTimeEdit);
	const int showMillis = showTime.toMillis();
	m_showTimeEdit->setValue(showMillis);
	m_hideTimeEdit->setMinimumTime(QTime::fromMSecsSinceStartOfDay(showMillis));
	m_hideTimeEdit->setValue(hideTime.toMillis());
	m_durationTimeEdit->setValue(m_hideTimeEdit->value() - m_showTimeEdit->value());
	updateLabels();
}

void
CurrentLineWidget::onLineShowTimeChanged(const Time &showTime)
{
	QSignalBlocker s1(m_showTimeEdit), s2(m_hideTimeEdit), s3(m_durationTimeEdit);
	m_showTimeEdit->setValue(showTime.toMillis());
	m_hideTimeEdit->setMinimumTime(QTime::fromMSecsSinceStartOfDay(showTime.toMillis()));
	updateLabels();
}

void
CurrentLineWidget::onLineHideTimeChanged(const Time &hideTime)
{
	QSignalBlocker s1(m_showTimeEdit), s2(m_hideTimeEdit), s3(m_durationTimeEdit);
	m_hideTimeEdit->setValue(hideTime.toMillis());
	m_durationTimeEdit->setValue(m_hideTimeEdit->value() - m_showTimeEdit->value());
	updateLabels();
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
CurrentLineWidget::onBreadcrumbClick(QPushButton *btn, SimpleRichTextEdit *textEdit)
{
	const int start = btn->property("start").toInt();
	const int end = btn->property("end").toInt();
	textEdit->setSelection(start, end - 1);
}

void
CurrentLineWidget::onConfigChanged()
{
	m_textEdits[0]->setSpellCheckingLanguage(SCConfig::defaultLanguage());
	m_textEdits[1]->setSpellCheckingLanguage(SCConfig::defaultLanguage());
}
