#!/usr/bin/env python
# -*- coding: utf-8 -*-

import subtitle

s = subtitle.instance()
reversed_indexes = range( 0, s.linesCount() );
reversed_indexes.reverse()
for line_index in reversed_indexes:
	if ( line_index % 2 == 0 ):
		s.removeLine( line_index )
