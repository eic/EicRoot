// $Id: RichLinkDef.h,v 1.16 2006/09/13 14:56:13 hoehne Exp $

#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ class CbmRich+;
#pragma link C++ class CbmRichHitProducer+;
#pragma link C++ class CbmGeoRich+;
#pragma link C++ class CbmRichMatchRings+;
#pragma link C++ class CbmRichElectronIdAnn+;
#pragma link C++ class CbmRichContFact;
#pragma link C++ class CbmGeoRichPar;

//fitter
#pragma link C++ class CbmRichRingFitterCircle+;
#pragma link C++ class CbmRichRingFitterRobustCOP+;
#pragma link C++ class CbmRichRingFitterTAU+;
#pragma link C++ class CbmRichFitRings+;
#pragma link C++ class CbmRichRingFitterImpl+;
#pragma link C++ class CbmRichRingFitterEllipseTau+;
#pragma link C++ class CbmRichRingFitterQa+;
#pragma link C++ class CbmRichRingFitterEllipseBase+;
#pragma link C++ class CbmRichRingFitterEllipse+;

//finder
//#pragma link C++ class CbmRichRingFinderHough+;
#pragma link C++ class CbmRichRingFinderTrack+;
#pragma link C++ class CbmRichRingFinderIdeal+;
#pragma link C++ class CbmRichFindRings+;
#pragma link C++ class CbmRichRingFinderHoughParallel;

//qa
#pragma link C++ class  CbmRichTestSim+;
#pragma link C++ class  CbmRichTestHits+;
#pragma link C++ class  CbmRichRingQa+;
#pragma link C++ class CbmRichGeoTest+;
#pragma link C++ class CbmRichElectronsQa+;
//#pragma link C++ class CbmRichParallelQa+;
//selection

//tracks
#pragma link C++ class  CbmRichProjectionProducer+;
#pragma link C++ class  CbmRichTrackExtrapolationIdeal+;
#pragma link C++ class  CbmRichTrackExtrapolationMirrorIdeal+;
#pragma link C++ class  CbmRichExtrapolateTracks+;
#pragma link C++ class  CbmRichRingTrackAssignClosestD+;
#pragma link C++ class  CbmRichRingTrackAssignIdeal+;
#pragma link C++ class  CbmRichAssignTrack+;

//#pragma link C++ class LhcbRingFinder;

#endif

