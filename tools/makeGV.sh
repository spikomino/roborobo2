#! /bin/bash
# Draws a series of graphs on dot (GraphViz) format
# name format: (prefix)(generation)r(nbAgent).dot

if [ ! $# -eq 4 ]
then
	echo "USAGE: ./makeGV PREFIX START_GEN END_GEN NUMBER_AGENTS"
else
    start=$2
    end=$3
    for cnt in `seq $2 $3`
    do
	idxAgent=$(($4-1))
	for idA in `seq 0 $(($4-1))`
	do
	    name=$1$cnt\r$idA
	
	    echo Plot $name of $(($(($3 - $2 + 1))*$4))
	    file=$name.dot
	    echo 'In file' $file
	    echo =-=-=-=-=-=
	    dot -Tsvg $file -o $name.svg
	    echo -
	done
    done
fi
