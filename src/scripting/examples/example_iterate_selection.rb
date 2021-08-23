#!/usr/bin/env ruby

# SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
#
# SPDX-License-Identifier: GPL-2.0-or-later

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
