#! /usr/bin/env bash
$XGETTEXT `find . -name "*.cpp" | grep -v "/kcm/" | grep -v "/autotests/" | grep -v "/tests/" | grep -v "/extractor/" | grep -v "/search/"` -o $podir/baloo_file.pot
