#!/bin/bash
for i in {1..1000}
do
	kv="sed -n "$i"p input1"
	kv=$($kv)
	order="set "$kv
	tem=`echo $order | nc 127.0.0.1 7777`
	if [ $[$i%2] == 0 ]
	then
		k="sed -n $[$i/2]p input1"
		k=$($k)
		k=($k)
		k=${k[0]}
		order="delete $k"
		tem=`echo $order | nc 127.0.0.1 7777`
	fi
done
for i in {501..1000}
do
	k="sed -n $[$i]p input1"
	k=$($k)
	k=($k)
	k=${k[0]}
	order="get $k"
	echo `echo $order | nc 127.0.0.1 7777`
done
