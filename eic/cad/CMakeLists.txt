
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
)

#set(LINK_DIRECTORIES
#${ROOT_LIBRARY_DIR}
#)
 
#link_directories( ${LINK_DIRECTORIES})

set( CXXSOURCES
EicStlVertex.cxx
EicStlFacetEdge.cxx
EicStlFacet.cxx
EicStlMediaGroup.cxx

EicCompositeShape.cxx

EicCadFile.cxx
EicCadFileConfig.cxx
EicStlFactory.cxx

#EicCadWizard.cxx
#EicCadWizardPlane.cxx
#EicCadWizardCylinder.cxx
#EicCadWizardCone.cxx
#EicCadWizardSphere.cxx
#EicCadWizardTorus.cxx
)

# Elmer support is optional; needed to import .mphtxt files though; NB: compile few source 
# codes in directly into the 'cad' library, no need to link against anything else of the 
# Elmer package;
IF(DEFINED ELMER)
  # NB: assume that SHOWMEM=0, EXE_MODE=0, LIB_MODE=1 in the ElmerGUI/Application/plugins;
  set(ELMER_SRCS
    ${ELMER}/ElmerGUI/Application/src/meshtype.cpp 
    ${ELMER}/ElmerGUI/Application/plugins/egconvert.cpp 
    ${ELMER}/ElmerGUI/Application/plugins/egmain.cpp
    ${ELMER}/ElmerGUI/Application/plugins/egutils.cpp 
    ${ELMER}/ElmerGUI/Application/plugins/egmesh.cpp 
    ${ELMER}/ElmerGUI/Application/plugins/egnative.cpp
  )

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_ELMER_")

  # NB: INCLUDE_DIRECTORIES was used already for ROOT dictionary creation; this is fine;
  set(INCLUDE_DIRECTORIES ${INCLUDE_DIRECTORIES} ${ELMER}/ElmerGUI/Application)

  SET(EXTRA_SOURCES ${ELMER_SRCS})
ENDIF()

SET(EXTRA_HEADERS EicStlKey.h)

#SET(DEPENDENCIES Base )
GENERATE_EICROOT_LIBRARY(cad eiccadLinkDef.h)

# -------------------------------

#@X@CHANGE_FILE_EXTENSION(*.cxx *.h EICCAD_HEADERS "${EICCAD_SRCS}")

#@X@set(EICCAD_LINKDEF eiccadLinkDef.h)
#@X@set(EICCAD_DICTIONARY ${CMAKE_CURRENT_BINARY_DIR}/eiccadDict.cxx) 

#@X@ROOT_GENERATE_DICTIONARY("${EICCAD_HEADERS}" "${EICCAD_LINKDEF}" "${EICCAD_DICTIONARY}" "${INCLUDE_DIRECTORIES}")

#@X@SET(EICCAD_SRCS ${EICCAD_SRCS} ${EICCAD_DICTIONARY})

# Well, OpenCascade is not needed to import either STL or COMSOL .mphtxt files;
#@X@IF(DEFINED OPENCASCADE)
#@X@  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_OPENCASCADE_")

#@X@  set(INCLUDE_DIRECTORIES ${INCLUDE_DIRECTORIES} ${OPENCASCADE}/include/oce) 

#@X@  SET(OCCLIB "-L${OPENCASCADE}/lib -lTKSTEP -lTKIGES -lTKXDEIGES -lTKXDESTEP -lTKSTL -lTKPrim")
#@X@ENDIF()


# Technically Tetgen support is optional, since both OCC (in case of clean object 
# decomposition into elementary shapes) and Elmer-facilitated import can live without it;
IF(DEFINED TETGEN)
  set(INCLUDE_DIRECTORIES ${INCLUDE_DIRECTORIES} ${TETGEN})

  # Simplify things here and for StepCode (single library) -> include by name;
  # otherwise would have to change config.sh.in (LD_LIBRARY_PATH appendix);
  SET(TETLIB ${TETGEN}/libtet.so)
  # NB: TETLIBRARY is a preprocessor flag known to TetGen internally -> do not change!;
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTETLIBRARY")
ENDIF()

# Optional auxiliary STEP file material (and color attribute) parser; 
IF(DEFINED STEPCODE)
  set(INCLUDE_DIRECTORIES ${INCLUDE_DIRECTORIES} 

  ${STEPCODE}/include
  ${STEPCODE}/build/include
  ${STEPCODE}/src/clstepcore
  ${STEPCODE}/src/clutils
  ${STEPCODE}/src/cldai
  ${STEPCODE}/src/cleditor
  )

  set(EICSTEP_LINKDEF eicstepLinkDef.h)
  set(EICSTEP_DICTIONARY ${CMAKE_CURRENT_BINARY_DIR}/eicstepDict.cxx) 

  ROOT_GENERATE_DICTIONARY("EicStepMaterialReader.h" "${EICSTEP_LINKDEF}" "${EICSTEP_DICTIONARY}" "${INCLUDE_DIRECTORIES}")

  SET(EICCAD_SRCS ${EICCAD_SRCS} EicStepMaterialReader.cxx ${EICSTEP_DICTIONARY})

  SET(STEPLIB ${STEPCODE}/build/lib/libsdai_AP214E3_2010.so)

  # Otherwise respective parts of code will not be compiled;
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_STEPCODE_")
ENDIF()

#@X@include_directories( ${INCLUDE_DIRECTORIES})

#@X@add_library(cad SHARED ${EICCAD_SRCS})
# NB: TETLIB & STEPLIB will stay NULL strings and do not hurt here if respective support is not compiled in;
#@X@target_link_libraries(cad ${ROOT_LIBRARIES} ${TETLIB} ${OCCLIB} ${STEPLIB} )
#@X@set_target_properties(cad PROPERTIES VERSION 0.0.1 )

#@X@install(TARGETS cad DESTINATION ${CMAKE_BINARY_DIR}/lib)

# ---------------------------------------

# export LD_LIBRARY_PATH=/home/ayk/FairRoot/CAD/oce/build/lib:${LD_LIBRARY_PATH}
# export LD_LIBRARY_PATH=/home/ayk/FairRoot/stepcode/build/lib:${LD_LIBRARY_PATH}

#/DATA00/ayk/netgen-5.3.1/nglib
#/DATA00/ayk/netgen-5.3.1/libsrc/include

#/home/ayk/FairRoot/fairsoft/apr13/transport/vgm/packages/VGM/include
#/home/ayk/FairRoot/fairsoft/apr13/transport/vgm/packages/BaseVGM/include
#/home/ayk/FairRoot/fairsoft/apr13/transport/vgm/packages/RootGM/include
#/home/ayk/FairRoot/fairsoft/apr13/transport/vgm/packages/Geant4GM/include
#/home/ayk/FairRoot/fairsoft/apr13/transport/vgm/packages/XmlVGM/include

#target_link_libraries(cad ${ROOT_LIBRARIES} ${TETLIB} -L/DATA00/ayk/netgen-5.3.1/libsrc/interface/.libs -linterface -L/DATA00/ayk/netgen-5.3.1/nglib/.libs -lnglib -L/DATA00/ayk/netgen-5.3.1/libsrc/stlgeom/.libs -lstl -L/home/ayk/FairRoot/CAD/oce/build/lib -lTKSTEP -lTKIGES -lTKXDEIGES -lTKXDESTEP -lTKSTL -lTKPrim -L/home/ayk/FairRoot/stepcode/build/lib -lsdai_AP214E3_2010 -L/home/ayk/FairRoot/fairsoft/apr13-build/lib -lRootGM -lGeant4GM -lXmlVGM)
