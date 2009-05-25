#!/usr/bin/env python
# -*- coding: utf-8 -*-

import ranges
import subtitle

subtitle.instance().removeLines( ranges.newSelectionRangeList().complement() )
