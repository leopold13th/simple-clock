#!/bin/bash

./clock &
sleep 0.3
WID=$(wmctrl -l | grep 'Desktop Clock' | cut -f 1 -d ' ')
wmctrl -i -r $WID -e 0,3285,847,-1,-1
wmctrl -i -r $WID -b add,sticky
