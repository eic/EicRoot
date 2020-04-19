
set(INCLUDE_DIRECTORIES
${BASE_INCLUDE_DIRECTORIES}

${CMAKE_SOURCE_DIR}/field
${CMAKE_SOURCE_DIR}/pnddata

${CMAKE_SOURCE_DIR}/eic/ayk
)

# List of source files;
set( CXXSOURCES 
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
