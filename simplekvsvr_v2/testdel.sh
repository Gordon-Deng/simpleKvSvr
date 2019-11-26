#!/bin/bash
time cat ranstr | awk '{printf "-delete %s\n",$1}' | xargs -P3 -n2 ./connect.sh 
