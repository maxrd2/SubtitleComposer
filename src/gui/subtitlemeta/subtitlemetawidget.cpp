/*
    SPDX-FileCopyrightText: 2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "subtitlemetawidget.h"

#include "core/subtitle.h"
#include "core/richtext/richcss.h"
#include "gui/subtitlemeta/csshighlighter.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QStackedLayout>
#include <QTabBar>
#include <QTabWidget>
#include <QTextEdit>

#include <QDebug>

#include <KLocalizedString>
#include <KTextEdit>

using namespace SubtitleComposer;

SubtitleMetaWidget::SubtitleMetaWidget(QWidget *parent)
	: QWidget(parent),
	  m_subtitle(nullptr),
	  m_dockTitleWidget(new QWidget(parent)),
	  m_tabBar(new QTabBar(m_dockTitleWidget)),
	  m_cssEdit(new QTextEdit(this)),
	  m_commentIntroEdit(new KTextEdit(this)),
	  m_commentTopEdit(new KTextEdit(this)),
	  m_commentBottomEdit(new KTextEdit(this))
{
	m_tabBar->setExpanding(false);
	m_tabBar->addTab(i18n("&Stylesheet"));
	m_tabBar->addTab(i18n("&Comments"));
	m_tabBar->installEventFilter(this);

	// parent title widget otherwise QTabBar has strange margins
	QHBoxLayout *titleLayout = new QHBoxLayout();
	titleLayout->setContentsMargins(0, 0, 0, 0);
	titleLayout->addWidget(m_tabBar);
	m_dockTitleWidget->setLayout(titleLayout);
	m_dockTitleWidget->setContentsMargins(1, 0, 0, 0);

#if QT_VERSION < QT_VERSION_CHECK(5, 11, 0)
	textEdit->setTabStopWidth(4 * QFontMetrics(textEdit->currentFont()).width(QChar::Space));
#else
	m_cssEdit->setTabStopDistance(4 * QFontMetrics(m_cssEdit->currentFont()).horizontalAdvance(QChar::Space));
#endif
	new CSSHighlighter(m_cssEdit->document());
#if 0
	m_cssEdit->setPlainText(R"END(/* example stylesheet */

::cue {
	background-url: url("base64;something ; something");
	background-image: linear-gradient(to bottom, dimgray, lightgray);
	color: papayawhip;
}
/* Style blocks cannot use blank lines nor "dash dash greater than" */
::cue(b) {
	color: peachpuff;
}

::cue(#\31) {
	color: lime;
}
::cue(#crédit\ de\ transcription) {
	color: red;
}
)END");
#endif

	QTabWidget *comments = new QTabWidget();
	comments->setTabPosition(QTabWidget::East);
	comments->addTab(m_commentIntroEdit, i18n("Intro"));
	comments->addTab(m_commentTopEdit, i18n("Header"));
	comments->addTab(m_commentBottomEdit, i18n("Footer"));

	QStackedLayout *mainLayout = new QStackedLayout();
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->addWidget(m_cssEdit);
	mainLayout->addWidget(comments);
	setLayout(mainLayout);

	connect(m_tabBar, &QTabBar::currentChanged, mainLayout, &QStackedLayout::setCurrentIndex);
}

SubtitleMetaWidget::~SubtitleMetaWidget()
{
}

static QString
mergedComments(const Subtitle *subtitle, const QByteArray &prefix)
{
	QString comments;
	for(int i = 0;; i++) {
		const QByteArray key = prefix + QByteArray::number(i);
		if(!subtitle->metaExists(key))
			break;
		comments.append(subtitle->meta(key));
	}
	return comments;
}

static void
setComments(Subtitle *subtitle, const QByteArray &prefix, const QString &comments)
{
	subtitle->meta(prefix + QByteArray("0"), comments);
	for(int i = 1; subtitle->metaRemove(prefix + QByteArray::number(i)); i++);
}

void
SubtitleMetaWidget::setSubtitle(Subtitle *subtitle)
{
	if(m_subtitle) {
		disconnect(m_commentIntroEdit, &KTextEdit::textChanged, this, nullptr);
		disconnect(m_commentTopEdit, &KTextEdit::textChanged, this, nullptr);
		disconnect(m_commentBottomEdit, &KTextEdit::textChanged, this, nullptr);
		disconnect(m_cssEdit, &QTextEdit::textChanged, this, nullptr);
	}

	m_subtitle = subtitle;

	if(subtitle) {
		m_commentIntroEdit->setPlainText(mergedComments(m_subtitle.constData(), "comment.intro."));
		m_commentTopEdit->setPlainText(mergedComments(m_subtitle.constData(), "comment.top."));
		m_commentBottomEdit->setPlainText(mergedComments(m_subtitle.constData(), "comment.bottom."));
		m_cssEdit->setPlainText(m_subtitle->stylesheet()->unformattedCSS());

		connect(m_commentIntroEdit, &KTextEdit::textChanged, this, [&](){
			setComments(m_subtitle.data(), "comment.intro.", m_commentIntroEdit->toPlainText());
		});
		connect(m_commentTopEdit, &KTextEdit::textChanged, this, [&](){
			setComments(m_subtitle.data(), "comment.top.", m_commentTopEdit->toPlainText());
		});
		connect(m_commentBottomEdit, &KTextEdit::textChanged, this, [&](){
			setComments(m_subtitle.data(), "comment.bottom.", m_commentBottomEdit->toPlainText());
		});
		connect(m_cssEdit, &QTextEdit::textChanged, this, [&](){
			QString text = m_cssEdit->toPlainText();
			m_subtitle->stylesheetSet(&text);
		});
	} else {
		m_cssEdit->clear();
		m_commentIntroEdit->clear();
		m_commentTopEdit->clear();
		m_commentBottomEdit->clear();
	}
}

bool
SubtitleMetaWidget::eventFilter(QObject *obj, QEvent *event)
{
	// workaround to forward (mouse) events to parent QDockWidget, otherwise it's impossible to drag the dock
	if(obj == m_tabBar) {
		// grab the mouse so move events are sent
		if(event->type() == QEvent::MouseButtonPress)
			m_tabBar->grabMouse();
		else if(event->type() == QEvent::MouseButtonRelease && reinterpret_cast<QMouseEvent *>(event)->buttons() == 0)
			m_tabBar->releaseMouse();
		// make sure m_tabBar handles the event
		static_cast<QObject *>(m_tabBar)->event(event);
		// flag it as ignored so QDockWidget can handle it
		event->ignore();
		return true;
	}
	return QObject::eventFilter(obj, event);
}
