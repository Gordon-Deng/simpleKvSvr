#!/bin/bash

function gen()
{
    hexdump -e '"%x"' -n 5 /dev/urandom 
}

for ((i = 0; i < 1000; i++)); do
    key=$(gen)
    val=$(gen)
    echo "set ${key} ${val}"
    echo "get ${key}"
    echo "get ${key}"
    echo "delete ${key}"
    echo "quit"
    echo "stats"
done | shuf > cmd.csv

cat cmd.csv | xargs -P 20 -d '\n' -n1 ./connect.sh
