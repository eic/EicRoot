
set(INCLUDE_DIRECTORIES
${ROOT_INCLUDE_DIR}

${CMAKE_SOURCE_DIR}/geobase 
${CMAKE_SOURCE_DIR}/parbase 
${CMAKE_SOURCE_DIR}/base 

${CMAKE_SOURCE_DIR}/pnddata

${CMAKE_SOURCE_DIR}/fairtools

${CMAKE_SOURCE_DIR}/eic/base
${CMAKE_SOURCE_DIR}/eic/field

${CMAKE_SOURCE_DIR}/eic/cad
${CMAKE_SOURCE_DIR}/eic/cad/wizard
${OPENCASCADE}/include/oce
)

set( CXXSOURCES 
  EicCadWizard.cxx 
  EicCadWizardPlane.cxx
  EicCadWizardCylinder.cxx
  EicCadWizardCone.cxx
  EicCadWizardSphere.cxx
  EicCadWizardTorus.cxx
)

SET(OCCLIB "-L${OPENCASCADE}/lib -lTKSTEP -lTKIGES -lTKXDEIGES -lTKXDESTEP -lTKSTL -lTKPrim")
SET(DEPENDENCIES ${OCCLIB} )
GENERATE_EICROOT_LIBRARY(wizard eicwizardLinkDef.h)
