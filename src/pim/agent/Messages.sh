#! /bin/sh
$XGETTEXT `find . -name '*.cpp' | grep -v '/test/'` -o $podir/akonadi_baloo_indexer.pot
