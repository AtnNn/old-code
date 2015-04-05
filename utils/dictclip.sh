#!/bin/bash

# (C) 2010 Etienne Laurin

# dictclip: search for words pasted to the primary selection with dict
# uses uclip because xsel and xclip fail on some UTF-8 input (like the xterm selection)

# copy $END to quit (xsel ignores ^C)

# do not run this script continuously, it steals the data you copy
# and might corrupt it

END="enddictclip"

#CMD="dict -P -"
#CMD="gtranslate.sh ru en"
CMD="yatr ru-en"

if [ "$1" != "" ]; then
    CMD="$1"
fi

while true; do
   xsel -o | xsel -i -n;
   CONTENTS="`xsel -o`"
   while [[ "a$CONTENTS" == "a" ]]; do
   	echo '' | xsel -i -n
	CONTENTS="`xsel -o`"
   done
   if [[ "$CONTENTS" == "$END" ]]; then exit; fi
   #INDEX=`expr index "$CONTENTS" " /"`
   #if [[ $INDEX == 0 ]]; then
       #dict `/usr/bin/printf '%b' "$CONTENTS"` # xsel returns utf8 asa \uHHHH
                                                # and the builtin printf doesn't expand \u
       echo -n "$END> "
       $CMD "$CONTENTS"
	
   #fi
done
