#!/usr/bin/env python
# -*- coding: utf-8 -*-

# SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
#
# SPDX-License-Identifier: GPL-2.0-or-later

import subtitle

s = subtitle.instance()
reversed_indexes = range( 0, s.linesCount() );
reversed_indexes.reverse()
for line_index in reversed_indexes:
	if ( line_index % 2 == 0 ):
		s.removeLine( line_index )
