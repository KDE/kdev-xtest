#!/bin/sh
$EXTRACTRC `find . -name \*.rc` `find . -name \*.ui` >>rc.cpp
$XGETTEXT `find . -name \*.cc -o -name \*.cpp -o -name \*.h | grep -v '/tests/'` -o $podir/kdevxtest.pot
rm -f rc.cpp
