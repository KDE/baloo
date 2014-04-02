#!/bin/sh

# This script is mostly used in order to make sure the baloo_file_extractor
# process can cope with Xapian::DatabaseModified exceptions and when
# it cannot obtain the write lock
for i in {1..1000000}
do
    FILE="$HOME/_baloo_test_file$i"
    touch $FILE
    sleep 2
    rm $FILE
    sleep 2
done
