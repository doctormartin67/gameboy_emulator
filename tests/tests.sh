#!/bin/bash

roms="$HOME/Projects/LLD_gbemu/roms/*"

MAX_WAIT_TIME=30

for rom in $roms
do
	if [[ $rom =~ roms/[0-9] ]]
	then
		if [ -f nohup.out ]
		then
			rm nohup.out
		fi
		nohup ./main "$rom" 2>/dev/null &
		sleep 1
		time=0
		while ! tail nohup.out | grep -q Passed
		do
			sleep 1
			((time++))
			if [[ $time == $MAX_WAIT_TIME ]]
			then
				echo "$rom Failed"
			fi
		done
		kill -9 $!
		wait $! 2>/dev/null
	fi
done

if [ -f nohup.out ]
then
	rm nohup.out
fi
