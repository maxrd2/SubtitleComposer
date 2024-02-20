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

#include <klocalizedstring.h>

#include "appglobal.h"
#include "application.h"
#include "core/richtext/richdocument.h"
#include "core/subtitleiterator.h"
#include "scconfig.h"
#include "translate/deeplengine.h"
#include "translate/googlecloudengine.h"
#include "translate/translateengine.h"

using namespace SubtitleComposer;

TranslateDialog::TranslateDialog(QWidget *parent)
	: ActionWithTargetDialog(i18n("Translate"), parent),
	  m_engines({ new GoogleCloudEngine(this), new DeepLEngine(this) }),
	  m_settings(nullptr)
{
	setMinimumWidth(500);

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
	TranslateDialog *dlg = new TranslateDialog();
	connect(dlg, &QDialog::finished, dlg, [=](){ dlg->deleteLater(); });
	dlg->setModal(true);
	dlg->show();
}

void
TranslateDialog::accept()
{
	hide();

	RangeList ranges = app()->linesWidget()->targetRanges(selectedLinesTarget());
	const bool primary = selectedTextsTarget() == Primary;

	QVector<QString> *texts = new QVector<QString>;
	for(SubtitleIterator it(*appSubtitle(), ranges); it.current(); ++it)
		texts->push_back(it.current()->doc(primary)->toHtml());

	connect(m_engine, &TranslateEngine::translated, this, [=](){
		SubtitleCompositeActionExecutor executor(appSubtitle(), i18n("Translate texts"));
		int i = 0;
		for(SubtitleIterator it(*appSubtitle(), ranges); it.current(); ++it)
			it.current()->doc(primary)->setHtml(texts->at(i++));
		delete texts;

		ActionWithTargetDialog::accept();
	});

	SCConfig::setTranslateEngine(m_engine->name());
	m_engine->translate(*texts);
	SCConfig::self()->save();
}
