
#include <TGeoMatrix.h>
#include <TRandom.h>

#include <EicProtoGenerator.h>

// ---------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------

TVector3 EicProtoGenerator::GetSimulatedPrimaryVertex( void )
{
  double vtx[3];

  for(unsigned iq=0; iq<3; iq++) {
    vtx[iq] = mCoord[iq] + 
      (mGaussianCoordinateSmearing ? 
       (mCoordSigma[iq] ? gRandom->Gaus(0.0, mCoordSigma[iq]) : 0.0) :
       (mCoordRange[iq] ? gRandom->Uniform(-mCoordRange[iq]/2, mCoordRange[iq]/2) : 0.0));
  } //for iq

  return TVector3(vtx);
} // EicProtoGenerator::GetSimulatedPrimaryVertex()

// ---------------------------------------------------------------------------------------

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
// ---------------------------------------------------------------------------------------

ClassImp(EicProtoGenerator)
