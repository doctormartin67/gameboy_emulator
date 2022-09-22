#!/bin/bash

roms="$HOME/Projects/LLD_gbemu/roms/*"
prog_name="main"
MAX_WAIT_TIME=30

function rm_s()
{
	if [ -f $1 ]
	then
		rm $1
	fi
}

for rom in $roms
do
	if [[ $rom =~ roms/[0-9] ]]
	then
		rm_s nohup.out
		nohup ./$prog_name "$rom" 2>/dev/null &
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
		if ps -a | grep -q $prog_name
		then
			kill -9 $!
			wait $! 2>/dev/null
		fi
	fi
done

rm_s nohup.out
