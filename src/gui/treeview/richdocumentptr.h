#ifndef RICHDOCUMENTPTR_H
#define RICHDOCUMENTPTR_H

/*
 * SPDX-FileCopyrightText: 2020 Mladen Milinkovic <max@smoothware.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
