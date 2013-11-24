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

#include "actionwithtargetdialog.h"
#include "../application.h"

#include <QtGui/QGroupBox>
#include <QtGui/QButtonGroup>
#include <QtGui/QRadioButton>
#include <QtGui/QGridLayout>

#include <KDebug>

using namespace SubtitleComposer;

ActionWithTargetDialog::ActionWithTargetDialog(const QString &title, QWidget *parent) :
	ActionDialog(title, parent),
	m_targetGroupBox(0),
	m_targetLayout(0),
	m_lineTargetsButtonGroup(0),
	m_textTargetsButtonGroup(0),
	m_selectionTargetOnlyMode(false),
	m_selectionTargetWasChecked(false),
	m_translationMode(false),
	m_nonTranslationModeTarget(Subtitle::Primary)
{
//  setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Default );
//  setDefaultButton( KDialog::Ok );
//  setButtonIcon( KDialog::Default, KIcon() );
//  setButtonText( KDialog::Default, i18n( "Target <<" ) );
//  setButtonToolTip( KDialog::Default, QString() );
//  connect( this, SIGNAL( defaultClicked() ), this, SLOT( onDefaultButtonClicked() ) );
}

int
ActionWithTargetDialog::exec()
{
	setTranslationMode(app()->translationMode());
	setSelectionTargetOnlyMode(app()->showingLinesContextMenu());

	return ActionDialog::exec();
}

void
ActionWithTargetDialog::show()
{
	setTranslationMode(app()->translationMode());
	setSelectionTargetOnlyMode(app()->showingLinesContextMenu());

	ActionDialog::show();
}

QGroupBox *
ActionWithTargetDialog::createTargetsGroupBox(const QString &title, bool addToLayout)
{
	if(!m_targetGroupBox) {
		m_targetGroupBox = createGroupBox(title, addToLayout);
		m_targetLayout = createLayout(m_targetGroupBox);
	}

	return m_targetGroupBox;
}

void
ActionWithTargetDialog::updateTargetsGroupBoxHiddenState()
{
	bool hidden = true;
	QList<QWidget *> children = m_targetGroupBox->findChildren<QWidget *>();
	for(int index = 0, size = children.size(); index < size; ++index) {
		if(!children.at(index)->isHidden()) {
			hidden = false;
			break;
		}
	}

	if(hidden != m_targetGroupBox->isHidden()) {
		if(hidden)
			m_targetGroupBox->hide();
		else
			m_targetGroupBox->show();
	}
}

void
ActionWithTargetDialog::onDefaultButtonClicked()
{
	if(m_lineTargetsButtonGroup) {
		LinesTarget prevTarget = selectedLinesTarget();
		setSelectionTargetOnlyMode(!m_lineTargetsButtonGroup->button(0)->isHidden());
		setSelectedLinesTarget(prevTarget);     // setSelectionTargetOnlyMode resets the target so we restore it
	}
}

void
ActionWithTargetDialog::setTargetsButtonsHiddenState(QButtonGroup *targetButtonGroup, bool hidden)
{
	if(!targetButtonGroup || hidden == targetButtonGroup->button(0)->isHidden())
		return;

	QList<QAbstractButton *> buttons = targetButtonGroup->buttons();
	if(hidden) {
		for(int index = 0, size = buttons.size(); index < size; ++index)
			buttons.at(index)->hide();
	} else {
		for(int index = 0, size = buttons.size(); index < size; ++index)
			buttons.at(index)->show();
	}

	updateTargetsGroupBoxHiddenState();

	if(targetButtonGroup == m_lineTargetsButtonGroup) {
		setButtonText(KDialog::Default, hidden ? i18n("Target >>") : i18n("Target <<"));

		m_mainWidget->updateGeometry();
		setMinimumSize(minimumSizeHint());
		resize(size().width(), minimumSizeHint().height());
	}
}

/// LINE TARGETS
/// ============

void
ActionWithTargetDialog::createLineTargetsButtonGroup()
{
	setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Default);
	setDefaultButton(KDialog::Ok);
	setButtonIcon(KDialog::Default, KIcon());
	setButtonText(KDialog::Default, i18n("Target <<"));
	setButtonToolTip(KDialog::Default, QString());
	connect(this, SIGNAL(defaultClicked()), this, SLOT(onDefaultButtonClicked()));

	createTargetsGroupBox();

	m_lineTargetsButtonGroup = new QButtonGroup(this);

	for(int index = 0; index < None; ++index) {
		QRadioButton *radioButton = new QRadioButton(m_targetGroupBox);
		m_lineTargetsButtonGroup->addButton(radioButton, index);
		m_targetLayout->addWidget(radioButton, index, 0);
	}

	m_lineTargetsButtonGroup->button(AllLines)->setText(i18n("All lines"));
	m_lineTargetsButtonGroup->button(Selection)->setText(i18n("Selected lines"));
	m_lineTargetsButtonGroup->button(FromSelected)->setText(i18n("All lines from first selected"));
	m_lineTargetsButtonGroup->button(UpToSelected)->setText(i18n("All lines up to last selected"));

	m_lineTargetsButtonGroup->button(AllLines)->setChecked(true);

	_setSelectionTargetOnlyMode(m_selectionTargetOnlyMode, true);
}

ActionWithTargetDialog::LinesTarget
ActionWithTargetDialog::selectedLinesTarget() const
{
	if(!m_lineTargetsButtonGroup) // lines target was not created
		return AllLines;

	int checkedId = m_lineTargetsButtonGroup->checkedId();
	return checkedId == -1 ? None : (LinesTarget)checkedId;
}

void
ActionWithTargetDialog::setSelectedLinesTarget(ActionWithTargetDialog::LinesTarget target)
{
	if(m_lineTargetsButtonGroup && m_lineTargetsButtonGroup->button(target))
		m_lineTargetsButtonGroup->button(target)->setChecked(true);
}

bool
ActionWithTargetDialog::isLinesTargetEnabled(LinesTarget target) const
{
	return m_lineTargetsButtonGroup && m_lineTargetsButtonGroup->button(target) && m_lineTargetsButtonGroup->button(target)->isEnabled();
}

void
ActionWithTargetDialog::setLinesTargetEnabled(LinesTarget target, bool enabled)
{
	if(m_lineTargetsButtonGroup && m_lineTargetsButtonGroup->button(target))
		m_lineTargetsButtonGroup->button(target)->setEnabled(enabled);
}

bool
ActionWithTargetDialog::selectionTargetOnlyMode() const
{
	return m_selectionTargetOnlyMode;
}

void
ActionWithTargetDialog::setSelectionTargetOnlyMode(bool value)
{
	_setSelectionTargetOnlyMode(value, false);
}

void
ActionWithTargetDialog::_setSelectionTargetOnlyMode(bool value, bool force)
{
	if(force || m_selectionTargetOnlyMode != value) {
		m_selectionTargetOnlyMode = value;

		if(!m_targetGroupBox || !m_lineTargetsButtonGroup)
			return;

		if(m_selectionTargetOnlyMode) {
			m_selectionTargetWasChecked = m_lineTargetsButtonGroup->button(Selection)->isChecked();
			m_lineTargetsButtonGroup->button(Selection)->setEnabled(true);
			m_lineTargetsButtonGroup->button(Selection)->setChecked(true);
		} else {
			if(!m_selectionTargetWasChecked) {
				m_lineTargetsButtonGroup->button(AllLines)->setEnabled(true);
				m_lineTargetsButtonGroup->button(AllLines)->setChecked(true);
			}
		}

		setTargetsButtonsHiddenState(m_lineTargetsButtonGroup, m_selectionTargetOnlyMode);
	}
}

/// TEXT TARGETS
/// ============

void
ActionWithTargetDialog::createTextTargetsButtonGroup()
{
	createTargetsGroupBox();

	m_textTargetsButtonGroup = new QButtonGroup(this);

	for(int index = 0; index < Subtitle::TextTargetSIZE; ++index) {
		QRadioButton *radioButton = new QRadioButton(m_targetGroupBox);
		m_textTargetsButtonGroup->addButton(radioButton, index);
	}

	m_textTargetsButtonGroup->button(SubtitleLine::Both)->setText(i18n("Both subtitles"));
	m_targetLayout->addWidget(m_textTargetsButtonGroup->button(SubtitleLine::Both), 0, 1);

	m_textTargetsButtonGroup->button(SubtitleLine::Primary)->setText(i18n("Primary subtitle"));
	m_targetLayout->addWidget(m_textTargetsButtonGroup->button(SubtitleLine::Primary), 1, 1);

	m_textTargetsButtonGroup->button(SubtitleLine::Secondary)->setText(i18n("Translation subtitle"));
	m_targetLayout->addWidget(m_textTargetsButtonGroup->button(SubtitleLine::Secondary), 2, 1);

	_setTranslationMode(m_translationMode, true);
}

Subtitle::TextTarget
ActionWithTargetDialog::nonTranslationModeTarget() const
{
	return m_nonTranslationModeTarget;
}

void
ActionWithTargetDialog::setNonTranslationModeTarget(Subtitle::TextTarget target)
{
	if(m_nonTranslationModeTarget != target) {
		if(!m_translationMode && m_textTargetsButtonGroup) {
			m_textTargetsButtonGroup->button(m_nonTranslationModeTarget)->setChecked(false);
			m_textTargetsButtonGroup->button(target)->setChecked(true);
		}

		m_nonTranslationModeTarget = target;
	}
}

Subtitle::TextTarget
ActionWithTargetDialog::selectedTextsTarget() const
{
	if(!m_textTargetsButtonGroup) // texts target was not created
		return Subtitle::TextTargetSIZE;

	int checkedId = m_textTargetsButtonGroup->checkedId();
	return checkedId == -1 ? Subtitle::TextTargetSIZE : (Subtitle::TextTarget)checkedId;
}

void
ActionWithTargetDialog::setSelectedTextsTarget(Subtitle::TextTarget target)
{
	if(m_textTargetsButtonGroup && m_textTargetsButtonGroup->button(target))
		m_textTargetsButtonGroup->button(target)->setChecked(true);
}

bool
ActionWithTargetDialog::isTextsTargetEnabled(Subtitle::TextTarget target) const
{
	return m_textTargetsButtonGroup && m_textTargetsButtonGroup->button(target) && m_textTargetsButtonGroup->button(target)->isEnabled();
}

void
ActionWithTargetDialog::setTextsTargetEnabled(Subtitle::TextTarget target, bool enabled)
{
	if(m_textTargetsButtonGroup && m_textTargetsButtonGroup->button(target))
		m_textTargetsButtonGroup->button(target)->setEnabled(enabled);
}

bool
ActionWithTargetDialog::translationMode() const
{
	return m_translationMode;
}

void
ActionWithTargetDialog::setTranslationMode(bool enabled)
{
	_setTranslationMode(enabled, false);
}

void
ActionWithTargetDialog::_setTranslationMode(bool enabled, bool force)
{
	if(force || m_translationMode != enabled) {
		m_translationMode = enabled;

		if(!m_targetGroupBox || !m_textTargetsButtonGroup)
			return;

		if(!m_translationMode)
			m_textTargetsButtonGroup->button(m_nonTranslationModeTarget)->setChecked(true);

		setTargetsButtonsHiddenState(m_textTargetsButtonGroup, !m_translationMode);
	}
}

/// ACTION WITH LINES TARGET DIALOG
/// ===============================

ActionWithLinesTargetDialog::ActionWithLinesTargetDialog(const QString &title, QWidget *parent) : ActionWithTargetDialog(title, parent)
{
	createLineTargetsButtonGroup();
}

ActionWithLinesTargetDialog::ActionWithLinesTargetDialog(const QString &title, const QString &desc, QWidget *parent) : ActionWithTargetDialog(title, parent)
{
	createTargetsGroupBox(desc);
	createLineTargetsButtonGroup();
}

int
ActionWithLinesTargetDialog::exec()
{
	setTranslationMode(app()->translationMode());
	setSelectionTargetOnlyMode(app()->showingLinesContextMenu());

	return m_targetGroupBox->isHidden() ? QDialog::Accepted : ActionDialog::exec();
}

/// ACTION WITH TEXTS TARGET DIALOG
/// ===============================

ActionWithTextsTargetDialog::ActionWithTextsTargetDialog(const QString &title, QWidget *parent) :
	ActionWithTargetDialog(title, parent)
{
	createTextTargetsButtonGroup();
}

ActionWithTextsTargetDialog::ActionWithTextsTargetDialog(const QString &title, const QString &desc, QWidget *parent) : ActionWithTargetDialog(title, parent)
{
	createTargetsGroupBox(desc);
	createTextTargetsButtonGroup();
}

int
ActionWithTextsTargetDialog::exec()
{
	setTranslationMode(app()->translationMode());
	setSelectionTargetOnlyMode(app()->showingLinesContextMenu());

	return m_targetGroupBox->isHidden() ? QDialog::Accepted : ActionDialog::exec();
}

/// ACTION WITH LINES AND TEXTS TARGET DIALOG
/// =========================================

ActionWithLinesAndTextsTargetDialog::ActionWithLinesAndTextsTargetDialog(const QString &title, QWidget *parent) :
	ActionWithTargetDialog(title, parent)
{
	createLineTargetsButtonGroup();
	createTextTargetsButtonGroup();
}

ActionWithLinesAndTextsTargetDialog::ActionWithLinesAndTextsTargetDialog(const QString &title, const QString &desc, QWidget *parent) : ActionWithTargetDialog(title, parent)
{
	createTargetsGroupBox(desc);
	createLineTargetsButtonGroup();
	createTextTargetsButtonGroup();
}

int
ActionWithLinesAndTextsTargetDialog::exec()
{
	setTranslationMode(app()->translationMode());
	setSelectionTargetOnlyMode(app()->showingLinesContextMenu());

	return m_targetGroupBox->isHidden() ? QDialog::Accepted : ActionDialog::exec();
}

#include "actionwithtargetdialog.moc"
