#!/bin/bash

# At least two arguments expected (script name and job count);
[ $# -lt 1 ] && { echo "Syntax!"; exit 0; }

# Re-assign command-line arguments to something readable;
SCRIPT=${1}
JOBNUM=1
[ $# -ge 2 ] && JOBNUM=${2}
# Let it be the default one;
SEED=12345678
# Override from the command line;
[ $# -eq 3 ] && SEED=${3}

# Current directory;
WRK=`pwd`

# Temporary file;
TMP=/tmp/`whoami`.$$.in

# Populate Condor input file template with the required lines;
echo "Universe        = vanilla"               > ${TMP}
echo "Executable      = ${SCRIPT}"            >> ${TMP}
echo "GetEnv          = True"                 >> ${TMP}
# Really needed?;
echo "+Experiment     = \"eic\""              >> ${TMP}
#+Job_Type       = "cas"

# Provide current directory and the seed;
echo "Arguments       = \"@id@ $WRK @seed@\"" >> ${TMP}
echo "Output          = @id@/condor.out"      >> ${TMP}

echo "Queue 1"                                >> ${TMP}

# Submit jobs one by one;
counter=${JOBNUM}
while [ ${counter} -gt 0 ]; do
  counter=`expr "$counter" - 1`
  # Zero-pad the counter and create respective directory; 
  ID=`printf "%05d" ${counter}`
  rm -rf ${ID} && mkdir ${ID}

  # Modify template and feed it to Condor;
  sed -e "s:@seed@:$SEED:g" -e "s:@id@:$ID:g" $TMP | condor_submit

  SEED=`expr "$SEED" + 1`
done

exit 0