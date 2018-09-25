// $Id: ZdcLinkDef+;,v 1.1.1.1
//2005/06/23 07:14:26 dbertini Exp $

#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ class PndStack+;
#pragma link C++ class PndMCTrack+;

#pragma link C++ class PndTrackCand+;
#pragma link C++ class PndTrack+;
#pragma link C++ class PndTrackID+;
#pragma link C++ class PndTrackCandHit+;
#pragma link C++ class PndVertex+;

#pragma link C++ class PndPidCandidate+;
#pragma link C++ class PndPidProbability+;

#pragma link C++ class FairRecoCandidate;
#pragma link C++ function operator << ( ostream &, const FairRecoCandidate & );

#endif

