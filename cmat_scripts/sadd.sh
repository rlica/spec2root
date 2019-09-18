#!/bin/bash

#Author: R. Lica, razvan.lica@cern.ch
# This script will add and align 2 spectra using 'sadd - M2D'

########################## EDIT HERE ############################

 
 FACTORS1="-819.29, 1"
 FACTORS2="-882.27, 1"
 REBIN="20"

#################################################################



################# USE this for filename as argument #############
if [ -z $1 ] 
  then
    echo "ERROR: File name 1 required as argument."
   exit 1
fi

if [ ! -f $1 ]
  then 
	 echo "ERROR: File $1 does not exist"
	 exit 1
fi

if [ -z $2 ] 
  then
    echo "ERROR: File name 2 required as argument."
   exit 1
fi

if [ ! -f $2 ]
  then 
	 echo "ERROR: File $2 does not exist"
	 exit 1
fi

 FILE1=$1
 FILE2=$2
 OUT="sum_${FILE1}"
 
#################################################################


####### Cleaning up existing files


   if [ -f $OUT ]; then
     echo Removing existing file: $OUT
     rm -f $OUT
   fi



####### SADD

    
sadd -l << echo
i $FILE1
SHIFT $FACTORS1
o temp1.l8
i $FILE2
SHIFT $FACTORS2
o temp2.l8
i temp1.l8
add temp2.l8
pack $REBIN
o $OUT
q
echo


mv SADD.LOG ${OUT}_SADD.LOG
rm temp1.l8 temp2.l8

    
