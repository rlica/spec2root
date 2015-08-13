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
e
35 120
echo

    done
    
#Nr. of Channels
#Display in root
#Time or energy
#Fit Expo
#Channels
