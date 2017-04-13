#!/usr/bin/bash
echo "%CPU %MEM"
while :
do 
	ps --no-header -o "%cpu %mem" -p $(pidof camflow-provenance)
	top -bn1 -p $(pidof camflow-provenance) | grep "^ " | awk '{ printf("%-8s %-8s\n", $9, $10); }'
	# echo $(pidof camflow-provenance)
	sleep 1
done

