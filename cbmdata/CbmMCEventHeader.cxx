/** CbmMCEvent.cxx
 *@author V.Friese <v.friese@gsi.de>
 ** Data class (level MC) containing information about the input event.
 ** 15.05.2008 change the event time to ns (M. Al-Turany)
 ** 11.05.2009 New CBM class derived from FairMCEventHeader
 **/


#include "CbmMCEventHeader.h"
#include "FairRootManager.h"

#include <iostream>
using std::cout;
using std::endl;

// -----   Default constructor   ------------------------------------------
CbmMCEventHeader::CbmMCEventHeader() 
  : FairMCEventHeader(),
    fPhi (0.)
{
}
// ------------------------------------------------------------------------



// -----   Constructor with run identifier   ------------------------------
CbmMCEventHeader::CbmMCEventHeader(UInt_t runId) 
  : FairMCEventHeader(runId),
    fPhi (0.)
{
}
// ------------------------------------------------------------------------



// -----   Standard constructor   -----------------------------------------
CbmMCEventHeader::CbmMCEventHeader(UInt_t runId, Int_t iEvent, 
		       Double_t x, Double_t y,  Double_t z, Double_t t,
				   Double_t b, Double_t phi, Int_t nPrim)
  : FairMCEventHeader(iEvent, x, y, z, t, b, nPrim),
    fPhi(phi)
{
}
// ------------------------------------------------------------------------



// -----   Destructor   ---------------------------------------------------
CbmMCEventHeader::~CbmMCEventHeader() { }
// ------------------------------------------------------------------------



// -----   Public method Reset   ------------------------------------------
void CbmMCEventHeader::Reset() {
  FairMCEventHeader::Reset();
  fPhi = 0.;
}
// ------------------------------------------------------------------------

void CbmMCEventHeader::Register()
{ 
  cout <<"Register CbmMCEventHeader."<<endl; 
  // For derived classes there has to be no dot at the end of the
  // branch name which is registered. Don't know why. 
  FairRootManager::Instance()->Register("MCEventHeader.", "Event", this , kTRUE);
}
  
ClassImp(CbmMCEventHeader)
