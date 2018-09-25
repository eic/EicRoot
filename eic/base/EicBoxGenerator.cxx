//
// AYK (ayk@bnl.gov), 2015/07/15
//
//  A complete re-write; inherit from FairGenerator directly; only minimum 
//  functionality is retained with reasonable default values;
//

#include <cmath>

#include <TRandom.h>
#include <TMath.h>

#include <EicBoxGenerator.h>

// ---------------------------------------------------------------------------------------

Bool_t EicBoxGenerator::ReadEvent(FairPrimaryGenerator *primGen)
{
  // Well, momentum and PDG should be defined;
  if (!mPDGs.size() || !mPmax) {
    printf("EicBoxGenerator::ReadEvent(): either PDG or momentum range undefined!\n");
    return kFALSE;
  } //if

    // Vertex; as of 2015/11/06 the same for all mMult tracks; after all, what is the 
  // purpose to generate N tracks?; they should originate from the same primary vertex, or?; 
  double vtx[3];
  for(unsigned iq=0; iq<3; iq++) {
    vtx[iq] = mCoord[iq] + 
      (mGaussianCoordinateSmearing ? 
       (mCoordSigma[iq] ? gRandom->Gaus(0.0, mCoordSigma[iq]) : 0.0) :
       (mCoordRange[iq] ? gRandom->Uniform(-mCoordRange[iq]/2, mCoordRange[iq]/2) : 0.0));
  } //for iq

  // At some point may want to introduce parameter range check and respective 
  // flag (like 'done') in order to perform this only once;

  for (unsigned pt=0; pt<mMult; pt++) {
    double phi = (/*mPhiMin == mPhiMax ? mPhiMin :*/ gRandom->Uniform(mPhiMin, mPhiMax))* 
      TMath::DegToRad();
  
    //printf("%f %f\n", cos(mThetaMin* TMath::DegToRad()), cos(mThetaMax* TMath::DegToRad()));
    double theta = //mThetaMin == mThetaMax ? mThetaMin*TMath::DegToRad() : 
      mUniformTheta ? gRandom->Uniform(mThetaMin, mThetaMax) * TMath::DegToRad() :
      acos(gRandom->Uniform(cos(mThetaMax * TMath::DegToRad()),
			    cos(mThetaMin * TMath::DegToRad())));
    double nx = TMath::Sin(theta)*TMath::Cos(phi);
    double ny = TMath::Sin(theta)*TMath::Sin(phi);
    double nz = TMath::Cos(theta);
    double pp = /*mPmin == mPmax ? mPmin :*/ gRandom->Uniform(mPmin, mPmax);

    {
      TVector3 pvect = GetModifiedTrack(pp*TVector3(nx, ny, nz));
      printf("%7.3f %7.3f %7.3f\n", pvect[0], pvect[1], pvect[2]);
      //primGen->AddTrack(mPDGs[mFlipCounter], pp*nx, pp*ny, pp*nz, vtx[0], vtx[1], vtx[2]);
      primGen->AddTrack(mPDGs[mFlipCounter], pvect[0], pvect[1], pvect[2], vtx[0], vtx[1], vtx[2]);
    }
    mFlipCounter = (mFlipCounter+1)%mPDGs.size();
    //printf("%d\n", mFlipCounter);
  } //for iq

  return kTRUE;
} // EicBoxGenerator::ReadEvent()

// ---------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------

#include <TGeoMatrix.h>

TVector3 EicProtoGenerator::GetModifiedTrack(const TVector3 track) {
  if (!mVerticalBeamDivergence && !mHorizontalBeamDivergence && !mHorizontalBeamRotation) 
    return track;

  {
    TVector3 trout = track;
    TGeoRotation *rw = new TGeoRotation();
    
    // FIXME: admittedly these codes are not the most efficient (and also ugly);  
    if (mVerticalBeamDivergence) {
      double in[3] = {trout.x(), trout.y(), trout.z()}, out[3];
      double angle = gRandom->Gaus(0.0, mVerticalBeamDivergence * TMath::RadToDeg());
      
      rw->RotateX(angle);
      
      rw->LocalToMasterVect(in, out);
      trout = TVector3(out);
    } //if
    // Do not mind to apply in sequence;
    if (mHorizontalBeamDivergence || mHorizontalBeamRotation) {
      double in[3] = {trout.x(), trout.y(), trout.z()}, out[3];
      double angle = TMath::RadToDeg() * gRandom->Gaus(mHorizontalBeamRotation, mHorizontalBeamDivergence);
      
      rw->RotateY(angle);
      
      rw->LocalToMasterVect(in, out);
      trout = TVector3(out); 
    } //if
     
    delete rw;
    return trout;
  } 
} // EicProtoGenerator::GetModifiedTrack() 

// ---------------------------------------------------------------------------------------

ClassImp(EicProtoGenerator)
ClassImp(EicBoxGenerator)
