/*
    SPDX-FileCopyrightText: 2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "subtitlepositionwidget.h"
#include "ui_subtitlepositionwidget.h"

#include "application.h"
#include "core/subtitleline.h"
#include "gui/playerwidget.h"
#include "videoplayer/videoplayer.h"


using namespace SubtitleComposer;

SubtitlePositionWidget::SubtitlePositionWidget(QWidget *parent)
	: QWidget(parent),
	  ui(new Ui::SubtitlePositionWidget),
	  m_currentLine(nullptr)
{
	ui->setupUi(this);
	setEnabled(false);

	connect(ui->posTop, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SubtitlePositionWidget::onPosTop);
	connect(ui->posRight, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SubtitlePositionWidget::onPosRight);
	connect(ui->posBottom, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SubtitlePositionWidget::onPosBottom);
	connect(ui->posLeft, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SubtitlePositionWidget::onPosLeft);

	connect(ui->haLeft, &QPushButton::toggled, this, &SubtitlePositionWidget::onHAlignLeft);
	connect(ui->haCenter, &QPushButton::toggled, this, &SubtitlePositionWidget::onHAlignCenter);
	connect(ui->haRight, &QPushButton::toggled, this, &SubtitlePositionWidget::onHAlignRight);
	connect(ui->vaTop, &QPushButton::toggled, this, &SubtitlePositionWidget::onVAlignTop);
	connect(ui->vaBottom, &QPushButton::toggled, this, &SubtitlePositionWidget::onVAlignBottom);
}

SubtitlePositionWidget::~SubtitlePositionWidget()
{
	delete ui;
}


void
SubtitlePositionWidget::setCurrentLine(SubtitleLine *line)
{
	if(m_currentLine == line)
		return;
	m_currentLine = line;

	if(line) {
		setEnabled(true);
		updatePosition(line->pos());
	} else {
		setEnabled(false);
		updatePosition(SubtitleRect());
	}
}

void
SubtitlePositionWidget::updatePosition(const SubtitleRect &pos)
{
	ui->haLeft->setChecked(pos.hAlign == SubtitleRect::START);
	ui->haCenter->setChecked(pos.hAlign == SubtitleRect::CENTER);
	ui->haRight->setChecked(pos.hAlign == SubtitleRect::END);
	ui->vaTop->setChecked(pos.vAlign == SubtitleRect::TOP);
	ui->vaBottom->setChecked(pos.vAlign == SubtitleRect::BOTTOM);
	ui->posTop->setValue(pos.top);
	ui->posLeft->setValue(pos.left);
	ui->posBottom->setValue(100. - pos.bottom);
	ui->posRight->setValue(100. - pos.right);
}

void
SubtitlePositionWidget::onPosTop(double value)
{
	if(!m_currentLine || m_currentLine->pos().top == value)
		return;
	auto p = m_currentLine->pos();
	p.top = value;
	m_currentLine->setPosition(p);
}

void
SubtitlePositionWidget::onPosBottom(double value)
{
	if(!m_currentLine || m_currentLine->pos().bottom == 100. - value)
		return;
	auto p = m_currentLine->pos();
	p.bottom = 100. - value;
	m_currentLine->setPosition(p);
}

void
SubtitlePositionWidget::onPosLeft(double value)
{
	if(!m_currentLine || m_currentLine->pos().left == value)
		return;
	auto p = m_currentLine->pos();
	p.left = value;
	m_currentLine->setPosition(p);
}

void
SubtitlePositionWidget::onPosRight(double value)
{
	if(!m_currentLine || m_currentLine->pos().right == 100. - value)
		return;
	auto p = m_currentLine->pos();
	p.right = 100. - value;
	m_currentLine->setPosition(p);
}

void
SubtitlePositionWidget::onHAlignLeft(bool checked)
{
	if(!checked || !m_currentLine || m_currentLine->pos().hAlign == SubtitleRect::START)
		return;
	auto p = m_currentLine->pos();
	p.hAlign = SubtitleRect::START;
	m_currentLine->setPosition(p);
}

void
SubtitlePositionWidget::onHAlignCenter(bool checked)
{
	if(!checked || !m_currentLine || m_currentLine->pos().hAlign == SubtitleRect::CENTER)
		return;
	auto p = m_currentLine->pos();
	p.hAlign = SubtitleRect::CENTER;
	m_currentLine->setPosition(p);
}

void
SubtitlePositionWidget::onHAlignRight(bool checked)
{
	if(!checked || !m_currentLine || m_currentLine->pos().hAlign == SubtitleRect::END)
		return;
	auto p = m_currentLine->pos();
	p.hAlign = SubtitleRect::END;
	m_currentLine->setPosition(p);
}

void
SubtitlePositionWidget::onVAlignTop(bool checked)
{
	if(!checked || !m_currentLine || m_currentLine->pos().vAlign == SubtitleRect::TOP)
		return;
	auto p = m_currentLine->pos();
	p.vAlign = SubtitleRect::TOP;
	m_currentLine->setPosition(p);
}

void
SubtitlePositionWidget::onVAlignBottom(bool checked)
{
	if(!checked || !m_currentLine || m_currentLine->pos().vAlign == SubtitleRect::BOTTOM)
		return;
	auto p = m_currentLine->pos();
	p.vAlign = SubtitleRect::BOTTOM;
	m_currentLine->setPosition(p);
}
