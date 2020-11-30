#!/bin/bash

#grab and copy dependencies
#get file path
ldd ./lpixgui | cut -f2 -d '>' | cut -f1 -d '(' > test

while read -r line || [[ -n "$line" ]]; 
do
	if [ -f $line ]
	then
		cp $line ./lib/				
		
	fi    
	
done < "test"
