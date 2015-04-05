#!/bin/bash

id=`curl -s -d '' omegle.com/start | sed s/\"//g`

echo "Connected! ($id)"

pid=$$

(
    ( while true; do
        json=`curl -s -d id="$id" omegle.com/events`
        #echo === "$json" === >&3
        if [ "$json" == "null" ]; then
            #echo quit
            #echo yes
            #echo
            break
        fi
        msg=`js -e "j=$json; for(i in j){ e=j[i]; if(e[0] == 'gotMessage'){ print(e[1]) } }"`
        if [ "$msg" != "" ]; then
            echo "$msg"
            echo "$msg" | sed 's/^/> /' >&3
        fi
    done; kill $pid ) | ttywrap "$@" | ( while read line; do
        if [ "$line" != "" ]; then
            echo "< $line"
            msg=`echo 'print(escape(arguments[0]))' | js - "$line"`
            r=`curl -s -d "id=$id&msg=$msg" omegle.com/send`
        fi
        if [ "$r" == "fail" ]; then
            break
        fi
    done )
) 3>&1
curl -s -d id="$id" omegle.com/disconnect
