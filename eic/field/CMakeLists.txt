
set(INCLUDE_DIRECTORIES
${ROOT_INCLUDE_DIR} 

#${GEANT3_INCLUDE_DIR}
 
${CMAKE_SOURCE_DIR}/base 
#${CMAKE_SOURCE_DIR}/base/sim 
#${CMAKE_SOURCE_DIR}/base/steer 
#${CMAKE_SOURCE_DIR}/base/event  
#${CMAKE_SOURCE_DIR}/dbase/dbInterface 
#${CMAKE_SOURCE_DIR}/dbase/dbValidation
#${CMAKE_SOURCE_DIR}/dbase/dbInput

${CMAKE_SOURCE_DIR}/geobase 
${CMAKE_SOURCE_DIR}/parbase
#${CMAKE_SOURCE_DIR}/trackbase 
${CMAKE_SOURCE_DIR}/fairtools
${CMAKE_SOURCE_DIR}/field
${CMAKE_SOURCE_DIR}/pnddata
#${CMAKE_SOURCE_DIR}/pnddata/TrackData
#${CMAKE_SOURCE_DIR}/GenfitTools/trackrep/GeaneTrackRep
#${CMAKE_SOURCE_DIR}/GenfitTools/recotasks
#${CMAKE_SOURCE_DIR}/PndTools/generalTools
#${CMAKE_SOURCE_DIR}/geane
#${CMAKE_SOURCE_DIR}/genfit

${CMAKE_SOURCE_DIR}/eic/ayk
${CMAKE_SOURCE_DIR}/eic/base
${CMAKE_SOURCE_DIR}/eic/cad
#${CMAKE_SOURCE_DIR}/eic/htc/include
${CMAKE_SOURCE_DIR}/eic/field
)

#include_directories( ${INCLUDE_DIRECTORIES} )

# List of source files;
set( CXXSOURCES 
EicMediaHub.cxx

EicMagneticFieldMap.cxx 
EicBeamLineElementMap.cxx
EicConstantField.cxx

EicMagneticField.cxx

#EicMagneticFieldGrad.cxx
#EicMagneticFieldFromGradients.cxx
EicBeamLineElementGrad.cxx 
)

SET(DEPENDENCIES ayk eicbase ${ROOT_LIBRARIES} -lcrypto)
GENERATE_EICROOT_LIBRARY(field qfieldLinkDef.h)
	 
