/*
    SPDX-FileCopyrightText: 2020-2022 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "richdocumentptr.h"

using namespace SubtitleComposer;


RichDocumentPtr::RichDocumentPtr(RichDocument *doc)
	: m_doc(doc)
{
	qRegisterMetaType<RichDocumentPtr>("RichDocumentPtr");
}

RichDocumentPtr::RichDocumentPtr(const RichDocumentPtr &other)
	: m_doc(other.m_doc)
{
}

RichDocumentPtr::~RichDocumentPtr()
{
}
