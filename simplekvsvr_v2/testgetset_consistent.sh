#!/bin/bash
./testset.sh > /dev/null 2>&1
./testget.sh | sort | md5sum
cat ranstr | awk '{print $2}' | sort | md5sum
