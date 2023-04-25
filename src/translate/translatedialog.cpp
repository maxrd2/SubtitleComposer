/*
    SPDX-FileCopyrightText: 2023 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "translatedialog.h"

#include <QFile>
#include <QLabel>
#include <QComboBox>
#include <QGroupBox>
#include <QGridLayout>
#include <QIcon>

#include "appglobal.h"
#include "application.h"
#include "core/richtext/richdocument.h"
#include "core/subtitleiterator.h"
#include "scconfig.h"
#include "translate/googlecloudengine.h"
#include "translate/translateengine.h"

using namespace SubtitleComposer;

TranslateDialog::TranslateDialog(QWidget *parent)
	: ActionWithTargetDialog(i18n("Translate"), parent),
	  m_engines({ new GoogleCloudEngine(parent) }),
	  m_settings(nullptr)
{
	QComboBox *engineCombo = new QComboBox(m_mainWidget);
	engineCombo->setEditable(false);
	for(const TranslateEngine *e: qAsConst(m_engines))
		engineCombo->addItem(e->name());
	m_mainLayout->addWidget(engineCombo);
	engineCombo->setCurrentText(SCConfig::translateEngine());
	connect(engineCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TranslateDialog::updateEngineUI);
	updateEngineUI(engineCombo->currentIndex());

	createTargetsGroupBox(i18n("Translate texts in"));
	createLineTargetsButtonGroup();
	createTextTargetsButtonGroup();
	setTextsTargetEnabled(Both, false);
}

void
TranslateDialog::updateEngineUI(int index)
{
	m_engine = m_engines.at(index);

	m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
	connect(m_engine, &TranslateEngine::engineReady, this, [&](bool status){
		m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(status);
	});

	if(m_settings)
		m_settings->deleteLater();
	m_settings = new QWidget(m_mainWidget);
	m_engine->settings(m_settings);
	m_settings->layout()->setContentsMargins(0, 0, 0, 0);
	m_mainLayout->insertWidget(1, m_settings);
}

void
TranslateDialog::performTranslation()
{
	static TranslateDialog *dlg = new TranslateDialog(app()->mainWindow());
	if(dlg->exec() != QDialog::Accepted)
		return;

	SubtitleCompositeActionExecutor executor(appSubtitle(), i18n("Translate texts"));

	RangeList ranges = app()->linesWidget()->targetRanges(dlg->selectedLinesTarget());
	const bool primary = dlg->selectedTextsTarget() == Primary;

	QVector<QString> *texts = new QVector<QString>;
	for(SubtitleIterator it(*appSubtitle(), ranges); it.current(); ++it)
		texts->push_back(it.current()->doc(primary)->toHtml());

	SCConfig::setTranslateEngine(dlg->m_engine->name());
	dlg->m_engine->translate(*texts);
	SCConfig::self()->save();

	connect(dlg->m_engine, &TranslateEngine::translated, dlg, [=](){
		int i = 0;
		for(SubtitleIterator it(*appSubtitle(), ranges); it.current(); ++it)
			it.current()->doc(primary)->setHtml(texts->at(i++));
		delete texts;
	});
}
