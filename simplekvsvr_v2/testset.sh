#!/bin/bash
time cat ranstr | awk '{printf "-set %s %s",$1,$2}' | xargs -P3 -tn3 ./connect.sh 
