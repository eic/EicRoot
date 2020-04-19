
set(INCLUDE_DIRECTORIES
${BASE_INCLUDE_DIRECTORIES}

${CMAKE_SOURCE_DIR}/pnddata

${CMAKE_SOURCE_DIR}/eic/base
${CMAKE_SOURCE_DIR}/eic/field

${CMAKE_SOURCE_DIR}/eic/cad/elmer
${CMAKE_SOURCE_DIR}/eic/cad/tetgen
${CMAKE_SOURCE_DIR}/eic/cad
)

set( CXXSOURCES
EicStlVertex.cxx
EicStlFacetEdge.cxx
EicStlFacet.cxx
EicStlMediaGroup.cxx

EicCompositeShape.cxx

EicCadFile.cxx
EicCadFileConfig.cxx
EicStlFactory.cxx
)

# Elmer is needed to import .mphtxt files only; prefer to include a dozen
# of the respective source and header files directly into the EicRoot distribution;
# hope there are no legal issues about this?;
# NB: assume that SHOWMEM=0, EXE_MODE=0, LIB_MODE=1 in the ElmerGUI/Application/plugins;
#
# Tetgen is needed to import .stl files; same story: include source files directly;
set( EXTRA_SOURCES
  elmer/src/meshtype.cpp 
  elmer/plugins/egconvert.cpp 
  elmer/plugins/egmain.cpp
  elmer/plugins/egutils.cpp 
  elmer/plugins/egmesh.cpp 
  elmer/plugins/egnative.cpp

  tetgen/predicates.cxx
  tetgen/tetgen.cxx
)

# FIXME: until recently Elmer support was optional -> need to define this flag;
# NB: TETLIBRARY is a preprocessor flag known to TetGen internally -> do not change!;
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_ELMER_ -DTETLIBRARY")

if(DEFINED OPENCASCADE)
  set(INCLUDE_DIRECTORIES ${INCLUDE_DIRECTORIES} ${OPENCASCADE}/include/oce) 
  SET(DEPENDENCIES wizard )
endif()
SET(EXTRA_HEADERS EicStlKey.h)

GENERATE_EICROOT_LIBRARY(cad eiccadLinkDef.h)

