
set(INCLUDE_DIRECTORIES
${ROOT_INCLUDE_DIR} 

#${GEANT3_INCLUDE_DIR}
 
${CMAKE_SOURCE_DIR}/base 

${CMAKE_SOURCE_DIR}/geobase 
${CMAKE_SOURCE_DIR}/parbase
${CMAKE_SOURCE_DIR}/fairtools
${CMAKE_SOURCE_DIR}/field
${CMAKE_SOURCE_DIR}/pnddata

${CMAKE_SOURCE_DIR}/eic/ayk
)

#include_directories( ${INCLUDE_DIRECTORIES} )

# List of source files;
set( SOURCES 
3d.cxx
Keyword.cxx
Splitter.cxx

# Used in mgrid fitting; not really tested after HTC -> EicRoot merge;
ThreeDeePolynomial.cxx
ThreeDeePolySpace.cxx

CoordSystem.cxx
import.cxx
Mgrid.cxx
MgridDirection.cxx
MgridInterpolation.cxx

KfMatrix.cxx
)

SET(DEPENDENCIES ${ROOT_LIBRARIES})
GENERATE_EICROOT_LIBRARY(ayk aykLinkDef.h)
