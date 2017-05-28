#!/bin/bash

#Author: R. Lica, razvan.lica@cern.ch

ARGV=("$@")
ARGC=$#

for (( i=0; i < $ARGC; i++ ));
    do 
 
spec2root_osx_int ${ARGV[i]} << echo
4                                    
y
t
e
40 300
echo

    done
    
#Nr. of Channels
#Display in root
#Time or energy
#Fit Expo
#Channels
