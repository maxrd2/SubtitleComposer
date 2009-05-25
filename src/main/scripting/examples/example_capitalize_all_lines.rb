#!/usr/bin/env ruby

require "ranges"
require "subtitle"

subtitle = Subtitle.instance()
for line_index in 0..subtitle.linesCount()-1
	line = subtitle.line( line_index )
	text = line.primaryText()
	line.setPrimaryText( text.left( 1 ).toUpper().append( text.mid( 1 ) ) )
end
