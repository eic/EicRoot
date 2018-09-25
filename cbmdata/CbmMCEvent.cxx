/** CbmMCEvent.cxx
 *@author V.Friese <v.friese@gsi.de>
 ** Data class (level MC) containing information about the input event.
 ** 15.05.2008 change the event time to ns (M. Al-Turany)
 ** 11.05.2009 New CBM class derived from FairMCEventHeader
 **/


#include "CbmMCEvent.h"



// -----   Default constructor   ------------------------------------------
CbmMCEvent::CbmMCEvent() 
: TNamed("MC Event", "CBM MC Event"), 
  fRunId(0),
  fEventId(0), 
  fX (0.), 
  fY (0.), 
  fZ (0.), 
  fT (0.), 
  fPhi (0.),
  fB (0.),
  fNPrim(0),
  fIsSet(kFALSE)

{
}
// ------------------------------------------------------------------------



// -----   Constructor with run identifier   ------------------------------
CbmMCEvent::CbmMCEvent(UInt_t runId) 
  : TNamed("MC Event", "CBM MC Event"), 
    fRunId(runId),
    fEventId(0), 
    fX (0.), 
    fY (0.), 
    fZ (0.), 
    fT (0.), 
    fB (0.),
    fPhi (0.),
    fNPrim(0),
    fIsSet(kFALSE)
  
{
}
// ------------------------------------------------------------------------



// -----   Standard constructor   -----------------------------------------
CbmMCEvent::CbmMCEvent(UInt_t runId, Int_t iEvent, 
		       Double_t x, Double_t y,  Double_t z, Double_t t,
		       Double_t b, Double_t phi, Int_t nPrim)
  : TNamed("MCEvent", "MC"), 
    fRunId(0),
    fEventId(iEvent), 
    fX (x), 
    fY (y), 
    fZ (z), 
    fT (t), 
    fB (b),
    fPhi(phi),
    fNPrim(nPrim),
    fIsSet(kFALSE)

{
}
// ------------------------------------------------------------------------



// -----   Destructor   ---------------------------------------------------
CbmMCEvent::~CbmMCEvent() { }
// ------------------------------------------------------------------------



// -----   Public method Reset   ------------------------------------------
void CbmMCEvent::Reset() {
  fEventId = fNPrim = 0;
  fX = fY = fZ = fT = fB = fPhi = 0.;
  fIsSet = kFALSE;
}
// ------------------------------------------------------------------------

  

ClassImp(CbmMCEvent)
