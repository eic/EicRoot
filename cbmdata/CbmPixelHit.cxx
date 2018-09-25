/**
 * \file CbmPixelHit.cxx
 * \author Andrey Lebedev <andrey.lebedev@gsi.de>
 * \date 2009
 **/
#include "CbmPixelHit.h"

#include <sstream>
using std::stringstream;
using std::endl;

CbmPixelHit::CbmPixelHit():
	CbmBaseHit(),
	fX(0.),
	fY(0.),
	fDx(0.),
	fDy(0.),
	fDxy(0.)
{
	SetType(kPIXELHIT);
}

CbmPixelHit::CbmPixelHit(
		Int_t address,
		Double_t x,
		Double_t y,
		Double_t z,
		Double_t dx,
		Double_t dy,
		Double_t dz,
		Double_t dxy,
		Int_t refId):
	CbmBaseHit(),
	fX(x),
	fY(y),
	fDx(dx),
	fDy(dy),
	fDxy(dxy)
{
	SetType(kPIXELHIT);
   SetAddress(address);
   SetZ(z);
   SetDz(dz);
   SetRefId(refId);
}

CbmPixelHit::CbmPixelHit(
		Int_t address,
		const TVector3& pos,
		const TVector3& err,
		Double_t dxy,
		Int_t refId):
	CbmBaseHit(),
	fX(pos.X()),
	fY(pos.Y()),
	fDx(err.X()),
	fDy(err.Y()),
	fDxy(dxy)
{
	SetType(kPIXELHIT);
	SetAddress(address);
	SetZ(pos.Z());
	SetDz(err.Z());
	SetRefId(refId);
}

CbmPixelHit::~CbmPixelHit()
{
}

string CbmPixelHit::ToString() const
{
   stringstream ss;
   ss << "CbmPixelHit: address=" << GetAddress()
       << " pos=(" << GetX() << "," << GetY() << "," << GetZ()
       << ") err=(" << GetDx() << "," << GetDy() << "," << GetDz()
       << ") dxy=" << GetDxy() << " refId=" << GetRefId() << endl;
   return ss.str();
}

void CbmPixelHit::Position(TVector3& pos) const
{
   pos.SetXYZ(GetX(), GetY(), GetZ());
}

void CbmPixelHit::PositionError(TVector3& dpos) const
{
   dpos.SetXYZ(GetDx(), GetDy(), GetDz());
}

void CbmPixelHit::SetPosition(const TVector3& pos)
{
	SetX(pos.X());
	SetY(pos.Y());
	SetZ(pos.Z());
}

void CbmPixelHit::SetPositionError(const TVector3& dpos)
{
	SetDx(dpos.X());
	SetDy(dpos.Y());
	SetDz(dpos.Z());
}

ClassImp(CbmPixelHit);
