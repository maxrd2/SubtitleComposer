#! /usr/bin/env bash
$EXTRACTRC `find src -name '*ui.rc' -o -name '*.ui' -o -name '*.kcfg'` >> rc.cpp
$XGETTEXT rc.cpp `find src -name '*.cpp' -o -name '*.h' -o -name '*.c'` -o $podir/subtitlecomposer.pot
