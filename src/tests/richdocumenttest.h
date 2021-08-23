#ifndef RICHDOCUMENTTEST_H
#define RICHDOCUMENTTEST_H
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

class RichDocumentTest : public QObject
{
	Q_OBJECT

	SubtitleComposer::RichDocument doc;

private slots:
	void testCursor();

	void testHtml_data();
	void testHtml();

	void testRegExpReplace_data();
	void testRegExpReplace();

	void testIndexReplace_data();
	void testIndexReplace();

	void testCleanupSpaces_data();
	void testCleanupSpaces();

	void testUpperLower();

	void testSentence_data();
	void testSentence();

	void testTitle_data();
	void testTitle();
};

#endif // RICHDOCUMENTTEST_H
