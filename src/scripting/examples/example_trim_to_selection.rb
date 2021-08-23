#!/usr/bin/env ruby

# SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
#
# SPDX-License-Identifier: GPL-2.0-or-later

require "ranges"
require "subtitle"

Subtitle.instance().removeLines( Ranges.newSelectionRangeList().complement() )
