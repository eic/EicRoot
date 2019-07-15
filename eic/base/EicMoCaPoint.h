//
// AYK (ayk@bnl.gov), 2013/06/13
//
//  EIC Monte-Carlo point (hit); assume it can be done generic enough 
//  to be the same for all types of detectors; if this becomes too 
//  restrictive at some point, consider to introduce derived classes;
//

#ifndef _EIC_MC_POINT_
#define _EIC_MC_POINT_

#include "FairMCPoint.h"

#include <ostream> 

class EicMoCaPoint : public FairMCPoint {
public:
 EicMoCaPoint(): FairMCPoint(), mStep(0.0), mPointID(-1), mMultiIndex(0), 
    mPrimaryMotherID(-1), mSecondaryMotherID(-1) {};
  EicMoCaPoint(Int_t trackID, Int_t primaryMotherId, Int_t secondaryMotherId, Int_t detID,
	       ULong64_t multiIndex,
	       const TVector3 &PosIn, const TVector3 &PosOut, 
	       const TVector3 &MomIn, const TVector3 &MomOut,
	       Double_t tof, Double_t length, Double_t eLoss, Double_t step);
  virtual ~EicMoCaPoint() {};

  virtual void Print(const Option_t* opt="") const;
 
  void SetStep(Double_t step)          { mStep = step; } 
  Double_t GetStep()             const { return mStep; }

  // Make life a bit easier; yes, this call is not much efficient;
  TVector3 GetPosIn()            const { TVector3 vv; Position(vv); return vv;}
  const TVector3 &GetPosOut()    const { return mPosOut;}
  TVector3 GetPosAvg()           const { return 0.5*(GetPosIn() + GetPosOut()); };
  TVector3 GetMomAvg()           const { 
    return 0.5*(TVector3(fPx, fPy, fPz) + mMomOut); 
  };
  
  Int_t GetPrimaryMotherID()     const { return mPrimaryMotherID; };
  Int_t GetSecondaryMotherID()   const { return mSecondaryMotherID; };

  void SetPointID(int id)          { mPointID = id; };
  Int_t GetPointID()             const { return mPointID; };

  ULong64_t GetMultiIndex()      const { return mMultiIndex; };

private:
  Double32_t mStep;          // step length

  // Yes, want to store exit coordinates and momenta as well;
  TVector3 mPosOut, mMomOut; // "out" position and momentum of this hit

  // Back door to help EicRawHitProducer::Exec() be a bit more generic;
  // perhaps remove this stuff later, depending on how complicated 
  // detector hit producer codes are;
  Int_t mPointID;            // index in fMoCaPointArray array

  ULong64_t mMultiIndex;     // encoded volume copy IDs of this hit in the geometry tree

  Int_t mPrimaryMotherID;    // "true" primary mother ID
  //   If either no "black hole" volumes defined in the setup or neither of the parent
  // particles entered such a volue, this ID is equal to mPrimaryMotherID;
  // Otherwise this will be ID of the very first parent which entered any of the "black
  // hole" volumes; see comments in EicGeoParData.h for further details;
  Int_t mSecondaryMotherID;  // guessed "most useful" parent further in the parent list

  //std::vector<Int_t> mMeaningfulParents;

  ClassDef(EicMoCaPoint,11)
};

#endif
