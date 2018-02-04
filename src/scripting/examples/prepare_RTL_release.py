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

# First add a “working” line
s.insertNewLine(s.linesCount(), False)

# Keep track. 2 lines = line 0 line 1
tempLine = s.line(s.linesCount()-1)

# All the lines except the last one
for line_index in range(0, s.linesCount()-1):
    line = s.line( line_index )

    # To store the modefied sublines
    combinedArray = []

    # each line in the subtitle line
    for eachSubLine in line.richPrimaryText().split("\n"):
        tempLine.setRichPrimaryText(eachSubLine)
        if tempLine.isRightToLeft():
            combinedArray.append(u"\u202B%s"%eachSubLine.decode('utf8')) # Add RLE for RTL lines
        else:
            combinedArray.append(u"\u202A%s"%eachSubLine.decode('utf8')) # Add LRE for LTR lines

    line.setRichPrimaryText("\n".join(combinedArray))

# Delete the temp line
s.removeLine(s.linesCount()-1)
