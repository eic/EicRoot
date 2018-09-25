/** CbmStsTrackMatch.cxx
 *@author V.Friese <v.friese@gsi.de>
 *@since 07.05.2009
 **/


#include "CbmTrackMatch.h"



// -----   Default constructor   -------------------------------------------
CbmTrackMatch::CbmTrackMatch() :
  fMCTrackId(-1),
  fNofTrueHits(0),
  fNofWrongHits(0),
  fNofFakeHits(0),
  fNofMCTracks(0)
{ }
// -------------------------------------------------------------------------



// -----   Standard constructor   ------------------------------------------
CbmTrackMatch::CbmTrackMatch(Int_t mcTrackId, 
			     Int_t nTrue,
			     Int_t nWrong, 
			     Int_t nFake, 
			     Int_t nTracks) :
  fMCTrackId(mcTrackId),
  fNofTrueHits(nTrue),
  fNofWrongHits(nWrong),
  fNofFakeHits(nFake),
  fNofMCTracks(nTracks)
{ }
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmTrackMatch::~CbmTrackMatch() {}
// -------------------------------------------------------------------------


ClassImp(CbmTrackMatch)
