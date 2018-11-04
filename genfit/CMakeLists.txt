
set(INCLUDE_DIRECTORIES
${ROOT_INCLUDE_DIR} 
${CLHEP_INCLUDE_DIR}
${CMAKE_SOURCE_DIR}/genfit
${CMAKE_SOURCE_DIR}/genfit/test

${CMAKE_SOURCE_DIR}/trackbase
${CMAKE_SOURCE_DIR}/GenfitTools/trackrep/GeaneTrackRep
)

#include_directories( ${INCLUDE_DIRECTORIES} )

# List of source files;
set( SOURCES GFAbsRecoHit.cxx GFAbsTrackRep.cxx GFBookkeeping.cxx
GFDetPlane.cxx GFException.cxx GFKalman.cxx GFPlanarHitPolicy.cxx GFPseudoSpacepointWireHitPolicy.cxx
GFRecoHitFactory.cxx GFRecoHitProducer.cxx GFSpacepointHitPolicy.cxx
GFTools.cxx GFTrackCand.cxx GFTrack.cxx GFWireHitPolicy.cxx
GFWirepointHitPolicy.cxx GFConstField.cxx GFFieldManager.cxx
GFAbsFinitePlane.cxx GFRectFinitePlane.cxx 
GFDaf.cxx GFDafHit.cxx GFTrackProximity.cxx)

#@@SET(DEPENDENCIES Geom VMC EG)
SET(DEPENDENCIES Geom VMC EG trackrep)
SET(EXTRA_HEADERS GFRecoHitIfc.h GFAbsRecoHitComparator.h)
GENERATE_EICROOT_LIBRARY(genfit genfitLinkDef.h)
	 
