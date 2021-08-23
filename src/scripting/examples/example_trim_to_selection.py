#!/usr/bin/env python
# -*- coding: utf-8 -*-

# SPDX-FileCopyrightText: 2007-2009 Sergio Pistone <sergio_pistone@yahoo.com.ar>
#
# SPDX-License-Identifier: GPL-2.0-or-later

import ranges
import subtitle

subtitle.instance().removeLines( ranges.newSelectionRangeList().complement() )
