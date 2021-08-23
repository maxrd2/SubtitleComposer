#ifndef SUBTITLETARGET_H
#define SUBTITLETARGET_H

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

namespace SubtitleComposer {

enum SubtitleTarget {
	Primary = 0,
	Secondary,
	Both,
	SubtitleTargetSize
};

}

#endif // SUBTITLETARGET_H
