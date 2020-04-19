
set(INCLUDE_DIRECTORIES
${ROOT_INCLUDE_DIR} 

${GEANT3_INCLUDE_DIR}

${CMAKE_SOURCE_DIR}/geobase 
${CMAKE_SOURCE_DIR}/parbase 
${CMAKE_SOURCE_DIR}/base 

${CMAKE_SOURCE_DIR}/base/sim 
${CMAKE_SOURCE_DIR}/base/steer 
${CMAKE_SOURCE_DIR}/base/event 
${CMAKE_SOURCE_DIR}/dbase/dbInterface 
${CMAKE_SOURCE_DIR}/dbase/dbValidation
${CMAKE_SOURCE_DIR}/dbase/dbInput

${CMAKE_SOURCE_DIR}/generators 

${CMAKE_SOURCE_DIR}/mcstack
${CMAKE_SOURCE_DIR}/passive
${CMAKE_SOURCE_DIR}/pnddata
${CMAKE_SOURCE_DIR}/field
${CMAKE_SOURCE_DIR}/trackbase 
${CMAKE_SOURCE_DIR}/pnddata/TrackData
${CMAKE_SOURCE_DIR}/pnddata/PidData
${CMAKE_SOURCE_DIR}/rho
${CMAKE_SOURCE_DIR}/pid/PidData
${CMAKE_SOURCE_DIR}/fairtools
${CMAKE_SOURCE_DIR}/PndTools/generalTools
${CMAKE_SOURCE_DIR}/eventdisplay

# FIXME: do these assignments conditional later;
${EICSMEAR}/include
#@${PROMC}/include
#@${PROMC}/src
#@${PROTOBUF}/include/google/protobuf/io
#@${EICMC}/include

${CMAKE_SOURCE_DIR}/eic/base
${CMAKE_SOURCE_DIR}/eic/event
${CMAKE_SOURCE_DIR}/eic/calorimetry
)

#include_directories( ${INCLUDE_DIRECTORIES} )

# List of source files;
set( CXXSOURCES 
    EicSmearTask.cxx 
    EicEventGenerator.cxx

    EicRcEvent.cxx
    EicRcParticle.cxx
    EicRcVertex.cxx
    EicEventAssembler.cxx
    EicRootManager.cxx
)

SET(DEPENDENCIES Base ${EICSMEAR}/build/libeicsmear.so)
GENERATE_EICROOT_LIBRARY(event eicsmearLinkDef.h)

