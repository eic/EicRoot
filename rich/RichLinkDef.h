// $Id: RichLinkDef.h,v 1.16 2006/09/13 14:56:13 hoehne Exp $

#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ class CbmRich+;
#pragma link C++ class CbmRichHitProducer+;
#pragma link C++ class CbmGeoRich+;
#pragma link C++ class CbmRichMatchRings+;
#pragma link C++ class CbmRichContFact;
#pragma link C++ class CbmGeoRichPar;
#pragma link C++ class CbmRichTrainAnnSelect;
#pragma link C++ class CbmRichTrainAnnElectrons;
#pragma link C++ class CbmRichEventDisplay+;

//reconstruction
#pragma link C++ class CbmRichReconstruction+;

//qa
#pragma link C++ class CbmRichTestSim+;
#pragma link C++ class CbmRichTestHits+;
#pragma link C++ class CbmRichGeoTest+;
#pragma link C++ class CbmRichUrqmdTest+;
#pragma link C++ class CbmRichRingFitterQa+;

#endif

