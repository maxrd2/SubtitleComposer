#!/usr/bin/env ruby

require "ranges"
require "subtitle"

def show( text )
	text = "\"" + text.gsub( "\\", "\\\\\\" ).gsub( "\"", "\\\"" ).gsub( "`", "\\\\`" ) + "\""
	`kdialog --msgbox #{shell_quote( text.to_s )}`
end

subtitle = Subtitle.instance
selection = Ranges.selection
selection.rangesCount.times do |rangeIdx|
	range = selection.range( rangeIdx )
	range.length.times do |lineIdx|
		lineIdx += range.start
		line = subtitle.line( lineIdx )
		line.primaryText = "-----------"
	end
end
