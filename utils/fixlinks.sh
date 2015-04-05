#!/bin/bash

# Quick and dirty hack to fix links in mirrored html files
#
# Convert all links to <domain> or / in the html files of <directory>
# into links to the other files in <directory>
# fixlinks.sh <domain> <directory>
#
# Convert all links to <domain> or / in <file> to point to $PWD instead
# fixlinks.sh <domain> <file>

if [ -d "$2" ]; then
	echo Scanning $2 for html files
	find "$2" -type f -name \*.html -exec "$0" "$1" {} \;
elif [ -f "$2" ]; then
	base="$(dirname "$2" | sed -r 's|[^/]+|..|g; s/^..$/./; s/^...//')"
	echo "Processing $2 ($base)"
	sed -i.fixlinks-backup -r 's$(href|src)="/$\1="'"$base"'/$g; s|http://'"$1"'|'"$base"'|g' "$2" 
else
	echo 'Invalid arguments'
	exit 1
fi
