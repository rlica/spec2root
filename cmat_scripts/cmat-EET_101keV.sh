#!/bin/bash

#Author: R. Lica, razvan.lica@cern.ch
# This script will perform 1D cuts of 3D matrices using 'cmat - M2D'

########################## EDIT HERE ############################

# 68 keV - 149Ba
# LEFT=(134 124)   #Left limit of each region  
#RIGHT=(139 130)   #Right limit of each region

# 316 keV - 149Ba
# LEFT=(629 640)   #Left limit of each region  
#RIGHT=(636 647)   #Right limit of each region

# 96,168,211,248,294 keV - 149Ba
# To extract halflife of the 68 keV Level
# even [0,2,4..] are peak, odd [1,3,5..] are background
#        0   1   2   3   4   5   6   7   8   9
# LEFT=(190 197 333 346 419 430 494 506 587 593)   #Left limit of each region  
#RIGHT=(196 203 339 352 425 436 498 511 600 607)   #Right limit of each region

# 597,216,380 keV - 150Ba
# To extract halflife of the 101.2 keV Level
# even [0,2,4..] are peak, odd [1,3,5..] are background
#        0   1   2   3   4   5  
 LEFT=(1190 1199 755 765 431 438)   #Left limit of each region  
RIGHT=(1198 1207 762 772 436 443)   #Right limit of each region


 FILE="EET-Ge-La1-Beta-150Cs"
 OUT="${FILE}_101keV.cmat"


 EXTENSION="cmat"
 INDEX_TO_PROJECT="2 3"
 FACTORS="1 -1"

#################################################################



################# USE this for filename as argument #############
#if [ -z $1 ] 
#  then
#    echo "ERROR: File name required as argument."
#   exit 1
#fi

#if [ ! -f $1 ]
#  then 
#	 echo "ERROR: File $1 does not exist"
#	 exit 1
#fi

# FILE=$1
#################################################################


####### Cleaning up existing files


   if [ -f $OUT ]; then
     echo Removing existing file: $OUT
     rm -f $OUT
   fi



####### CMAT

    
cmat -l << echo
o $FILE
m2d
$INDEX_TO_PROJECT
n
${LEFT[0]} ${RIGHT[0]}
${LEFT[2]} ${RIGHT[2]}
${LEFT[4]} ${RIGHT[4]}


temp1
m2d
$INDEX_TO_PROJECT
n
${LEFT[1]} ${RIGHT[1]}
${LEFT[3]} ${RIGHT[3]}
${LEFT[5]} ${RIGHT[5]}


temp2
c
add
2
temp1
temp2
$OUT
$FACTORS

q
echo

rm temp1.cmat temp2.cmat

    
