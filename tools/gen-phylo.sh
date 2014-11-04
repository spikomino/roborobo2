#!/bin/sh

if [ "$#" -lt 1 ] ; then
    echo "Usage: `basename $0` logfile";
    exit 1;
fi


for i in `seq 0 249` ; do
    grep "^$i " logs/evolution.log | awk '{print $2" "$5}' | sort -n | awk '{print $2}' | tr '\n' ' '
    echo 
done

    
    

