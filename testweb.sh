#!/bin/bash

c=$1

u=$2

e=/tmp/testweb-$BASHPID.errors

rm $e 2>/dev/null || true
touch $e

time (

p=

for i in `seq 1 $c`; do
	(
		curl -s $u >/dev/null || echo -n . >> $e
	) &
	p="$p $!"
done

for n in $p; do
	wait $p 2>/dev/null
done

)

echo Number of errors: `wc -c $e | awk '{print $1}'`
rm $e 2>/dev/null || true
