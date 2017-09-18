#!/usr/bin/env ruby

# Copyright (C) 2007-2009 Sergio Pistone (sergio_pistone@yahoo.com.ar)
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
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
require "debug"

def each_range( range_list, forward=true )
	range_indexes = 0..range_list.rangesCount()-1
	range_indexes = range_indexes.to_a().reverse() if ! forward
	for range_index in range_indexes
		yield range_list.range( range_index )
	end
end

def each_line( range_list, forward=true )
	subtitle = Subtitle.instance()
	each_range( range_list, forward ) do |range|
		line_indexes = range.start()..range.end()
		line_indexes = line_indexes.to_a().reverse() if ! forward
		for line_index in line_indexes
			yield subtitle.line( line_index )
		end
	end
end

each_range( Ranges.newSelectionRangeList(), true ) do |range|
	Debug.information( "#{range.start}:#{range.end}")
end

each_line( Ranges.newSelectionRangeList(), false ) do |line|
	Debug.information( "Plain text: " + line.plainPrimaryText() + "\n\nRich Text: " + line.richPrimaryText() )
end
