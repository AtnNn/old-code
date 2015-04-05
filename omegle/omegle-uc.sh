#!/bin/bash

id=`curl -s -d '' omegle.com/start | sed s/\"//g`

#echo "Connected! ($id)"


pid=$$;

(
  while true; do
      #curl -d id="$id" omegle.com/events; echo
      json=`curl -s -d id="$id" omegle.com/events`
      if [ "$json" == "null" ]; then
          break
      fi
      js -e "j=$json; for(i in j){ e=j[i]; if(e[0] == 'gotMessage'){ print(e[1]) } }" | perl -pe '$_=uc'
  done
  kill $pid
) &

bgp=$!;

while read line; do
    if [ "$line" == "/quit" ]; then
        curl -s -d id="$id" omegle.com/disconnect
        break
    fi
    line=`echo "$line" | perl -pe '$_=uc'`
    msg=`echo 'print(escape(arguments[0]))' | js - "$line"`
    r=`curl -s -d "id=$id&msg=$msg" omegle.com/send`
    if [ "$r" == "fail" ]; then
        true; #echo === Could not send message ===
    fi
done

kill $bgp
