#!/usr/bin/env python
# -*- coding: utf-8 -*-

# SPDX-FileCopyrightText: 2018 Safa AlFulaij <safa1996alfulaij@gmail.com>
#
# SPDX-License-Identifier: GPL-2.0-or-later

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
