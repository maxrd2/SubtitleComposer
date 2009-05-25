#!/usr/bin/env python
# -*- coding: utf-8 -*-

import ranges
import subtitle
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

for r in each_range( ranges.newSelectionRangeList(), True ):
	debug.information( str( r.start() ) + ":" + str( r.end() ) )

for l in each_line( ranges.newSelectionRangeList(), False ):
	debug.information( "Plain text: " + l.plainPrimaryText() + "\n\nRich Text: " + l.richPrimaryText() )
