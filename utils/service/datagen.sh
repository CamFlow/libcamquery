#!/usr/bin/bash
awk 'NR % 3 == 0' $1 > "ps.txt"
awk 'NR % 3 == 2' $1 > "top.txt"
while read -r cpu mem
do
	echo "$cpu" >> "ps_cpu.txt"
	echo "$mem" >> "ps_mem.txt"
done < "ps.txt"

while read -r cpu mem
do
	echo "$cpu" >> "top_cpu.txt"
	echo "$mem" >> "top_mem.txt"
done < "top.txt"

rm "ps.txt"
rm "top.txt"
