#!/usr/bin/bash
while :
do 
	ps --no-header -o "%cpu %mem" -p $(pidof camflow-provenance)
	# echo $(pidof camflow-provenance)
	sleep 1
done

