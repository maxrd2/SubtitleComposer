#!/usr/bin/env ruby

# SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
#
# SPDX-License-Identifier: GPL-2.0-or-later

require "ranges"
require "subtitle"

subtitle = Subtitle.instance()
for line_index in 0..subtitle.linesCount()-1
	line = subtitle.line( line_index )
	text = line.primaryText()
	line.setPrimaryText( text.left( 1 ).toUpper().append( text.mid( 1 ) ) )
end
