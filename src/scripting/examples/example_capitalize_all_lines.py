#!/usr/bin/env python
# -*- coding: utf-8 -*-

# SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
#
# SPDX-License-Identifier: GPL-2.0-or-later

import ranges
import subtitle

s = subtitle.instance()
for line_index in range( 0, s.linesCount() ):
	line = s.line( line_index )
	text = line.primaryText()
	line.setPrimaryText( text.left( 1 ).toUpper().append( text.mid( 1 ) ) )
