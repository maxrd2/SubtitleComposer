#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2018 Safa AlFulaij (safa1996alfulaij@gmail.com)
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

s = subtitle.instance()

# All the lines
for line_index in range(0, s.linesCount()):
	line = s.line( line_index )

	# To store the modefied sublines
	combinedArray = []

	# each line in the subtitle line
	for eachSubLine in line.richPrimaryText().split("\n"):
		combinedArray.append(eachSubLine.decode('utf8')[1:]) # Remove the first character (RLE/LRE)

	line.setRichPrimaryText("\n".join(combinedArray))
