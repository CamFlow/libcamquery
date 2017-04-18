#!/bin/bash

count=0

while read -r cpu
do
    if [ "$count" == 10 ]
    then
        echo "$cpu" >> $2
        count=0
    else
        count=$(($count + 1))
    fi
done < $1