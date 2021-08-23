#!/usr/bin/env ruby

# SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
#
# SPDX-License-Identifier: GPL-2.0-or-later

require "subtitle"

subtitle = Subtitle.instance()
for line_index in (0..subtitle.linesCount()-1).to_a().reverse()
	subtitle.removeLine( line_index ) if ( line_index % 2 == 0 )
end
