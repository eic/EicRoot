#!/bin/bash

# Assume, that job index, original working directory and the seed are provided;
[ $# -eq 3 ] || { echo "Syntax!"; exit 0; }
ID=${1}
WRK=${2}
SEED=${3}

# 'cd' to scratch disk;
cd ${_CONDOR_SCRATCH_DIR} || exit 1

# Make it easy: just copy over all the *.C files;
cp ${WRK}/*.C .

#root -l sandbox-t1018.C
root -l simulation.C\($SEED,$ID\)
root -l digitization.C\($SEED\)
root -l reconstruction.C\($SEED\)

# Move all the *.root files back;
for f in *.root; do 
  mv $f ${WRK}/${ID}
done

exit 0
