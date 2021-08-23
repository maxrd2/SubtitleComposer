#!/usr/bin/env ruby

# SPDX-FileCopyrightText: 2007-2009 Sergio Pistone (sergio_pistone@yahoo.com.ar)
#
# SPDX-License-Identifier: GPL-2.0-or-later
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the
# Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
# Boston, MA 02110-1301, USA.

require "ranges"
require "subtitle"

subtitle = Subtitle.instance()
for line_index in 0..subtitle.linesCount()-1
	line = subtitle.line( line_index )
	text = line.primaryText()
	line.setPrimaryText( text.left( 1 ).toUpper().append( text.mid( 1 ) ) )
end
