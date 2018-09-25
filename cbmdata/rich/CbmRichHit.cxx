/**
 * \file CbmRichHit.cxx
 * \author B. Polichtchouk
 **/

#include "CbmRichHit.h"
#include <sstream>
using std::stringstream;
using std::endl;

CbmRichHit::CbmRichHit()
  : CbmPixelHit(), 
    fPmtId(0),
    fNPhotons(0),
    fAmplitude(0.)
{
   SetType(kRICHHIT);
}

CbmRichHit::~CbmRichHit()
{
}

string CbmRichHit::ToString() const {
   stringstream ss;
   ss << "CbmRichHit: address=" << GetAddress()
       << " pos=(" << GetX() << "," << GetY() << "," << GetZ()
       << ") err=(" << GetDx() << "," << GetDy() << "," << GetDz()
       << ") dxy=" << GetDxy() << " refId=" << GetRefId()
       << " pmtId=" << GetPmtId() << " nofPhotons=" << GetNPhotons()
       << " amplitude=" << GetAmplitude() << endl;
   return ss.str();
}

ClassImp(CbmRichHit)
