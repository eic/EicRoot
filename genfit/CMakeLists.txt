
set(INCLUDE_DIRECTORIES
${BASE_INCLUDE_DIRECTORIES}

${CMAKE_SOURCE_DIR}/genfit
${CMAKE_SOURCE_DIR}/genfit/test

${CMAKE_SOURCE_DIR}/trackbase
${CMAKE_SOURCE_DIR}/GenfitTools/trackrep/GeaneTrackRep
)

# List of source files;
set( CXXSOURCES GFAbsRecoHit.cxx GFAbsTrackRep.cxx GFBookkeeping.cxx
GFDetPlane.cxx GFException.cxx GFKalman.cxx GFPlanarHitPolicy.cxx GFPseudoSpacepointWireHitPolicy.cxx
GFRecoHitFactory.cxx GFRecoHitProducer.cxx GFSpacepointHitPolicy.cxx
GFTools.cxx GFTrackCand.cxx GFTrack.cxx GFWireHitPolicy.cxx
GFWirepointHitPolicy.cxx GFConstField.cxx GFFieldManager.cxx
GFAbsFinitePlane.cxx GFRectFinitePlane.cxx 
GFDaf.cxx GFDafHit.cxx GFTrackProximity.cxx)

SET(DEPENDENCIES Geom VMC EG trackrep)
SET(EXTRA_HEADERS GFRecoHitIfc.h GFAbsRecoHitComparator.h)
GENERATE_EICROOT_LIBRARY(genfit genfitLinkDef.h)
	 
