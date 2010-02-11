#!/usr/bin/env python
# -*- coding: utf-8 -*-

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

import ranges
import subtitle
import strings
import debug

def each_range( range_list, forward=True ):
	r = range( 0, range_list.rangesCount() )
	if not forward: r.reverse()
	for range_index in r:
		yield range_list.range( range_index )

def each_line( range_list, forward=True ):
	for r in each_range( range_list, forward ):
		r2 = range( r.start(), r.end() + 1 )
		if not forward: r2.reverse()
		for line_index in r2:
			yield subtitle.instance().line( line_index )

after = strings.newString( "\\1-" )
for line in each_line( ranges.newSelectionRangeList(), False ):
	line.setPrimaryText( line.primaryText().replaceAll( "(^|\n)- *", after, True, True ) )
