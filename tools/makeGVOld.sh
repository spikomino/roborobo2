#! /bin/bash
# Draws a series of graphs on dot (GraphViz) format
# starting from $(1)0.dot, up to $(1)$($(2)-1).dot
#This was originally created for drawing Neural Networks
#SKIPSIZE (3rd parameter) represents the index difference 
#between two consecutive graphs

if [ ! $# -eq 4 ]
then
	echo "USAGE: ./makeGV PREFIX NUMBER_GRAPHS SKIPSIZE NUMBER_AGENTS"
else
    index=$(($2-1))
    for cnt in `seq 0 $index`
    do
	idxAgent=$(($4-1))
	for idA in `seq 0 $idxAgent`
	do
	    name=$1$idA-$(($cnt*$3-1))
	
	    echo Plot $name of $(($2*$4))
	    file=$name.dot
	    echo 'In file' $file
	    echo =-=-=-=-=-=
	    dot -Tsvg $file -o $name.svg
	    echo -
	done
    done
fi
