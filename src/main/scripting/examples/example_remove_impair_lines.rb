#!/usr/bin/env ruby

require "subtitle"

subtitle = Subtitle.instance()
for line_index in (0..subtitle.linesCount()-1).to_a().reverse()
	subtitle.removeLine( line_index ) if ( line_index % 2 == 0 )
end
