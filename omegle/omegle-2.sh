#!/bin/bash

ttywrap ./omegle.sh | (
	while read l; do
		echo "$l"
		echo "= $l" >&2
	done
) | ttywrap ./omegle.sh
