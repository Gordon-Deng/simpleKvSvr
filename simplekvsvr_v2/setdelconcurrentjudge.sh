#!/bin/bash 
tail -n500 input1 | cut -d" " -f2 | sort -n | md5sum
tail -n500 input2 | cut -d" " -f2 | sort -n | md5sum
tail -n500 input3 | cut -d" " -f2 | sort -n | md5sum

cat output1 | sort -n | md5sum
cat output2 | sort -n | md5sum
cat output3 | sort -n | md5sum
