
#include <FairGenerator.h>
#include <FairPrimaryGenerator.h>

#ifndef _EIC_PROTO_GENERATOR_
#define _EIC_PROTO_GENERATOR_

class EicProtoGenerator: public FairGenerator
{
 public:
 EicProtoGenerator(const char* name): FairGenerator(name), mVerticalBeamDivergence(0.0), 
    mHorizontalBeamDivergence(0.0), mHorizontalBeamRotation(0.0) {
    memset(mCoord,      0x00, sizeof(mCoord));
    memset(mCoordSigma, 0x00, sizeof(mCoordSigma));
    memset(mCoordRange, 0x00, sizeof(mCoordRange));

    mGaussianCoordinateSmearing = true;   
  };
  ~EicProtoGenerator() {};

  // This stuff is only good for rough forward proton modeling estimates; 
  // a random 3D rotation with these parameters in [rad] will be applied to the 
  // outgoing particles;
  void SetNaiveVerticalBeamDivergence  (double value) { mVerticalBeamDivergence   = value; };
  void SetNaiveHorizontalBeamDivergence(double value) { mHorizontalBeamDivergence = value; };

  void SetNaiveHorizontalBeamRotation  (double value) { mHorizontalBeamRotation   = value; };

  // Well, do I need more intelligent access methods here?; FIXME: should be moved
  // to EicProtoGenerator at some point;
  void SetVertex(double x, double y, double z = 0.0) {
    mCoord[0] = x; mCoord[1] = y; mCoord[2] = z;
  };
  double GetVx()         const { return mCoord[0]; };
  double GetVy()         const { return mCoord[1]; };
  double GetVz()         const { return mCoord[2]; };
  void SetVertexSmearing(double sx, double sy, double sz = 0.0) {
    mCoordSigma[0] = sx; mCoordSigma[1] = sy; mCoordSigma[2] = sz;
    mGaussianCoordinateSmearing = true;
  };
  void SetVertexRange(double rx, double ry, double rz = 0.0) {
    mCoordRange[0] = rx; mCoordRange[1] = ry; mCoordRange[2] = rz;
    mGaussianCoordinateSmearing = false;
  };

  double GetVxSmearing() const { return mCoordSigma[0]; };
  double GetVySmearing() const { return mCoordSigma[1]; };
  double GetVzSmearing() const { return mCoordSigma[2]; };

  TVector3 GetSimulatedPrimaryVertex( void );

  TVector3 GetModifiedTrack(const TVector3 track);

  double mVerticalBeamDivergence, mHorizontalBeamDivergence, mHorizontalBeamRotation;

  Double_t mCoord[3];       // (average) vertex coordinates in [cm];      default: (0,0,0)
  Double_t mCoordSigma[3];  // gaussian vertex smearing (if any) in [cm]; default: (0,0,0)
  Double_t mCoordRange[3];  // uniform vertex range (if any) in [cm];     default: (0,0,0)
  Bool_t mGaussianCoordinateSmearing; // use gaussian coordinate smearing (default);

  ClassDef(EicProtoGenerator,1);
};

#endif
