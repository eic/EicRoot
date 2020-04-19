
set(INCLUDE_DIRECTORIES
${BASE_INCLUDE_DIRECTORIES}

${CMAKE_SOURCE_DIR}/field
${CMAKE_SOURCE_DIR}/pnddata

${CMAKE_SOURCE_DIR}/eic/ayk
${CMAKE_SOURCE_DIR}/eic/base
${CMAKE_SOURCE_DIR}/eic/cad
#${CMAKE_SOURCE_DIR}/eic/htc/include
${CMAKE_SOURCE_DIR}/eic/field
)

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
GENERATE_EICROOT_LIBRARY(qfield qfieldLinkDef.h)
	 
