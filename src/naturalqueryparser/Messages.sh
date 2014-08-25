#! /usr/bin/env bash
$XGETTEXT `find . -name "*.cpp" | grep -v "/autotests/"` -o $podir/baloo_naturalqueryparser.pot
