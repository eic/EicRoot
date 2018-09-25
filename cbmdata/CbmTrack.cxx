// -------------------------------------------------------------------------
// -----                        CbmTrack source file                   -----
// -----                  Created 29/11/07  by V. Friese               -----
// -----                  Modified 26/05/09  by A. Lebedev             -----
// -------------------------------------------------------------------------

#include "CbmTrack.h"

#include "FairMultiLinkedData.h"

#include <iostream>
#include <memory>
using std::cout;
using std::endl;

// -----   Default constructor   -------------------------------------------
CbmTrack::CbmTrack() :
    TObject(),
    fHitIndex(),
    fHitType(),
    fPidHypo(0),
    fParamFirst(),
    fParamLast(),
    fFlag(0),
    fChiSq(0.),
    fNDF(0),
    fPreviousTrackId(-1),
    fLinks(NULL)
{
}
// -------------------------------------------------------------------------

CbmTrack::CbmTrack(const CbmTrack& rhs)
 : TObject(rhs),
    fHitIndex(rhs.fHitIndex),
    fHitType(rhs.fHitType),
    fPidHypo(rhs.fPidHypo),
    fParamFirst(rhs.fParamFirst),
    fParamLast(rhs.fParamLast),
    fFlag(rhs.fFlag),
    fChiSq(rhs.fChiSq),
    fNDF(rhs.fNDF),
   fPreviousTrackId(rhs.fPreviousTrackId),
   fLinks(NULL)
{
   if (NULL != rhs.fLinks) {
     fLinks = new FairMultiLinkedData(*(rhs.fLinks));
   }
}

CbmTrack& CbmTrack::operator=(const CbmTrack& rhs)
{

  if (this != &rhs) {

    TObject::operator=(rhs);
    fHitIndex = rhs.fHitIndex;
    fHitType = rhs.fHitType;
    fPidHypo = rhs.fPidHypo;
    fParamFirst = rhs.fParamFirst;
    fParamLast = rhs.fParamLast;
    fFlag = rhs.fFlag;
    fChiSq = rhs.fChiSq;
    fNDF = rhs.fNDF;
    fPreviousTrackId = rhs.fPreviousTrackId;

    if (NULL != rhs.fLinks) {
      std::auto_ptr<FairMultiLinkedData> tmp(new FairMultiLinkedData(*rhs.fLinks));
      delete fLinks;
      fLinks = tmp.release();
    } else {
      fLinks = NULL;
    }
  }
  return *this;
}



// -----   Destructor   ----------------------------------------------------
CbmTrack::~CbmTrack()
{
}
// -------------------------------------------------------------------------

// -----   Public method AddHit   ------------------------------------------
void CbmTrack::AddHit(
		Int_t index,
		HitType type)
{
	fHitIndex.push_back(index);//std::make_pair(type, index));
	fHitType.push_back(type);
}
// -------------------------------------------------------------------------

// -----   Public method Print   -------------------------------------------
void CbmTrack::Print() const
{
	cout << "CbmTrack: nof hits=" << fHitIndex.size() << ", chiSq=" << fChiSq
		<< ", NDF=" << fNDF << ", pidHypo=" << fPidHypo
		<< ", previousTrackId=" << fPreviousTrackId << ", flag=" << fFlag << endl;
//	fParamFirst.Print();
//	fParamLast.Print();
}
// -------------------------------------------------------------------------

ClassImp(CbmTrack);
