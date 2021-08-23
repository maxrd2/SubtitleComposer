/*
    SPDX-FileCopyrightText: 2020 Mladen Milinkovic <max@smoothware.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RICHDOCUMENTPTR_H
#define RICHDOCUMENTPTR_H

#include "core/richdocument.h"

namespace SubtitleComposer {

class RichDocumentPtr {
public:
	explicit RichDocumentPtr(RichDocument *doc=nullptr);
	RichDocumentPtr(const RichDocumentPtr &other);
	virtual ~RichDocumentPtr();

	inline RichDocument * operator->() { return m_doc; }
	inline operator RichDocument *() { return m_doc; }
	inline RichDocumentPtr & operator=(const RichDocumentPtr &other) { m_doc = other.m_doc; return *this; }

private:
	RichDocument *m_doc;
};

}

Q_DECLARE_METATYPE(SubtitleComposer::RichDocumentPtr)

#endif // RICHDOCUMENTPTR_H
