#!/bin/bash

#Author: R. Lica, razvan.lica@cern.ch

ARGV=("$@")
ARGC=$#

for (( i=0; i < $ARGC; i++ ));
    do 
 
spec2root ${ARGV[i]} << echo
4                                    
n
t
y
1900 2100
echo

    done
    
#Nr. of Channels
#Display in root
#Time or energy
#Fit Gaussian
#Channels
