#!/bin/bash
time cat ranstr | awk '{printf "-get %s\n",$1}' | xargs -P1 -n2 ./connect.sh 
