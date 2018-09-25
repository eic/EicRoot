/* $Id: CbmRichRing.cxx,v 1.8 2006/09/13 14:51:28 hoehne Exp $*/

/* History of CVS commits:
 *
 * $Log: CbmRichRing.cxx,v $
 * Revision 1.8  2006/09/13 14:51:28  hoehne
 * two variables (Selection2D, SelectionNN) added in which values between 0 and 1 are stored for fake rejection
 * ReconstructedFlag removed
 *
 * Revision 1.7  2006/08/11 14:03:57  hoehne
 * move SetUncertainty and GetUncertainty to SetChi2 and GetChi2
 *
 * Revision 1.6  2006/07/12 06:27:54  hoehne
 * new functions: SetDistance and GetDistance added which set/ give the distance between ring center and track attached to this
 * ring
 *
 * Revision 1.5  2006/02/23 11:24:10  hoehne
 * RecFlag added (Simeon Lebedev)
 *
 * Revision 1.4  2006/01/23 11:40:13  hoehne
 * MCMotherID added
 *
 * Revision 1.3  2006/01/19 10:40:06  hoehne
 * restructured according to new RinfFinder Class:
 * array of hits belonging to a certain ring added
 *
 *
 *
 */


// -------------------------------------------------------------------------
// -----                      CbmRichRing source file                  -----
// -----   Created 05/07/04 by A. Soloviev <solovjev@cv.jinr.ru>       -----
// -------------------------------------------------------------------------


#include "CbmRichRing.h"
//#include "CbmRichHit.h"

# include "TMath.h"

# include <iostream>
# include <cmath>

using std::cout;
using std::sqrt;
using std::fabs;
using std::atan;

// -----   Default constructor   -------------------------------------------
CbmRichRing::CbmRichRing()
  : TObject(),
    fHitCollection(),
    fAPar(0.),
    fBPar(0.),
    fCPar(0.),
    fDPar(0.),
    fEPar(0.),
    fFPar(0.),
    fCenterX(0.), 
    fCenterY(0.), 
    fRadius(0.),
    fAaxis(0.),
    fBaxis(0.),
    fAaxisCor(0.),
    fBaxisCor(0.),
    fPhi(0.),
    fTrackID(-1),
    fDistance(99.),
    fChi2(0.),
    fAngle(0.),
    fNofHitsOnRing(-1),
    fSelectionNN(-1.),
    fRecFlag(0)
{
    fHitCollection.reserve(40);
}
// -------------------------------------------------------------------------


// -----   Standard constructor   ------------------------------------------
CbmRichRing::CbmRichRing ( Float_t x,
			   Float_t y,
			   Float_t r )
  : TObject(),
    fHitCollection(),
    fAPar(0.),
    fBPar(0.),
    fCPar(0.),
    fDPar(0.),
    fEPar(0.),
    fFPar(0.),
    fCenterX(x), 
    fCenterY(y), 
    fRadius(r),
    fAaxis(0.),
    fBaxis(0.),
    fAaxisCor(0.),
    fBaxisCor(0.),
    fPhi(0.),
    fTrackID(-1),
    fDistance(99.),
    fChi2(0.),
    fAngle(0.),
    fNofHitsOnRing(-1),
    fSelectionNN(-1.),
    fRecFlag(0)
{
    fHitCollection.reserve(40);
}
// -------------------------------------------------------------------------


// -----   Destructor   ----------------------------------------------------
CbmRichRing::~CbmRichRing()
{
	fHitCollection.clear();
}
// -------------------------------------------------------------------------

void CbmRichRing::SetXYABPhi(Double_t x, Double_t y,
                Double_t a, Double_t b,
                Double_t phi)
{
    fCenterX = x;
    fCenterY = y;
    fAaxis = a;
    fBaxis = b;
    fPhi = phi;
}

Bool_t CbmRichRing::RemoveHit(UShort_t hitId)
{
	//Int_t nofHits = fHitCollection.size();
	std::vector<UShort_t>::iterator it;
	for (it = fHitCollection.begin(); it!=fHitCollection.end(); it++){
		if (hitId == *it){
			fHitCollection.erase(it);
			return true;
		}
	}
	return false;
}

Double_t CbmRichRing::GetXF1() const
{
	Double_t c = sqrt(fAaxis*fAaxis - fBaxis*fBaxis);
	Double_t xc = c*cos(fabs(fPhi));

	return fCenterX+xc;
}

Double_t CbmRichRing::GetYF1() const
{
	Double_t c = sqrt(fAaxis*fAaxis - fBaxis*fBaxis);
	Double_t yc = c*sin(fabs(fPhi));
	if (fPhi >=0){
		return fCenterY+yc;
	}else{
		return fCenterY-yc;
	}
}

Double_t CbmRichRing::GetXF2() const
{
	Double_t c = sqrt(fAaxis*fAaxis - fBaxis*fBaxis);
	Double_t xc = c*cos(fabs(fPhi));

	return fCenterX-xc;
}

Double_t CbmRichRing::GetYF2() const
{
	Double_t c = sqrt(fAaxis*fAaxis - fBaxis*fBaxis);
	Double_t yc = c*sin(fabs(fPhi));
	if (fPhi >=0){
		return fCenterY-yc;
	}else{
		return fCenterY+yc;
	}
}

void CbmRichRing::Print(){
	cout << " Ring parameters: " <<
			" Aaxis = " << GetAaxis() <<
			", Baxis = " << GetBaxis() <<
			", Phi = " << GetPhi() <<
			", CenterX = " << GetCenterX() <<
			", CenterY = " << GetCenterY() <<
			", Radius = " << GetRadius() <<
			", NofHits = " << GetNofHits() <<
			", RadialPosition = " << GetRadialPosition() <<
			", Chi2 = " << GetChi2() <<
			", RingTrackDistance = "<< GetDistance() <<
			", Angle() = " << GetAngle() <<
			", NofHitsOnRing = " << GetNofHitsOnRing() << std::endl;
}

Float_t CbmRichRing::GetRadialPosition() const
{
	if (fCenterY > 0.f) {
		return sqrt(fCenterX*fCenterX + (fCenterY-110.f)*(fCenterY-110.f));
	} else {
		return sqrt(fCenterX*fCenterX + (fCenterY+110.f)*(fCenterY+110.f));
	}
}

Double_t CbmRichRing::GetRadialAngle() const{
  /*  if (fCenterY > 0){
            return  atan((100 - fCenterY) / (0 - fCenterX));
    } else {
            return atan((-100 - fCenterY) / (0 - fCenterX));
    }*/

    if( fCenterX > 0 && fCenterY > 0 ){
        return atan(fabs((100 - fCenterY) / (0 - fCenterX)));
    }
    if( fCenterX < 0 && fCenterY > 0 ){
        return TMath::Pi() - atan(fabs((100 - fCenterY) / (0 - fCenterX)));
    }
    if( fCenterX < 0 && fCenterY < 0 ){
        return TMath::Pi() + atan(fabs((-100 - fCenterY) / (0 - fCenterX)));
    }
    if( fCenterX > 0 && fCenterY < 0 ){
        return 2*TMath::Pi() - atan(fabs((-100 - fCenterY) / (0 - fCenterX)));
    }

    return 999.;
}
//
//CbmRichRingLight* CbmRichRing::toLightRing()
//{
//	CbmRichRingLight* rl = new CbmRichRingLight();
//
//	for (int i = 0; i < this->GetNofHits(); i ++){
//		rl->AddHit(this->GetHit(i));
//	}
//	rl->SetCenterX(this->GetCenterX());
//	rl->SetCenterY(this->GetCenterY());
//	rl->SetRadius(this->GetRadius());
//	rl->SetAngle(this->GetAngle());
//	rl->SetChi2(this->GetChi2());
//	rl->SetNofHitsOnRing(this->GetNofHitsOnRing());
//	rl->SetSelectionNN(this->GetSelectionNN());
//	rl->SetRecFlag(this->GetRecFlag());
//
//	return rl;
//}

ClassImp(CbmRichRing)

