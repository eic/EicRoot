//
// AYK (ayk@bnl.gov), 2013/06/12
//
//  Tracking digi hit class;
//

#ifndef _EIC_TRACKING_DIGI_HIT_
#define _EIC_TRACKING_DIGI_HIT_

#include <math.h>

#include "TVector3.h"
#include "FairHit.h"

#include <EicGeoParData.h>
#include <EicMoCaPoint.h>

class KfMatrix;
class MatchCandidate;

class EicTrackingDigiHit : public FairHit
{
  // FIXME: clean up here, eventually;
  friend class EicTrackingRecoHit;
  friend class EicPlanarRecoHit;
  friend class EicSpaceRecoHit;
  friend class EicHtcTask;
  friend class SensitiveVolume;
  friend class EicKfNodeTemplate1D;
  friend class EicKfNodeTemplateOrth2D;
  friend class EicKfNodeTemplateOrth3D;
  friend class EicKfNodeTemplate;

 public:   
  EicTrackingDigiHit() { Clear(); ResetVars(); mXYmode = true; };

  /** Standard constructor
  *@param cname      Detector name
  *@param odim       Detector measurement vector dimension
  *@param volumePath GEANT path to the sensitive volume (need to be done better)
  *@param detID      Detector unique volume ID
  *@param pointID    Index of corresponding MCPoint
  *@param local      Hit position coordinates in local detector system [cm]
  *@param global     Hit position coordinates in MARS system [cm]
  *@param dpos       Errors in position coordinates [cm]
  **/
  EicTrackingDigiHit(TString cname, const EicMoCaPoint *point, unsigned kfNodeID,
		     // NB: 'global' is passed further down to the FairHit constructor
		     // and 'const' does not work there;
		     TVector3& global, const TVector3& local, bool xy_mode /*= true*/);

#if _NEED_TO_BE_FIXED_
  void Clear() { fTrackID = -1;}; 
#endif
  virtual ~EicTrackingDigiHit() {};   

  virtual void Print(const Option_t* opt = 0) const {;};

  ULong64_t GetMultiIndex() const { return mMultiIndex; };

  //TVector3 GetLocalCoordinates();
  TVector3 GetLocalCoordinates() const { return mLocalCoord; };
  //bool IsXY( void )      const { return true; };

  unsigned GetKfNodeID()    const { return mKfNodeID; };

  virtual double _GetCoord(unsigned id)                  const = 0;
  virtual unsigned GetMdim()                             const = 0;


  protected:
  virtual double GetCovariance(unsigned ip, unsigned iq) const = 0;

  TVector3 mLocalCoord;
  bool mXYmode;

  // FIXME: pack into multi-index later;
  UInt_t mKfNodeID;

  ULong64_t mMultiIndex;

 private:
  void ResetVars() { 
    mKfNodeID = 0;
    mMultiIndex = _LOGICAL_INDEX_INVALID_;
  };

 ClassDef(EicTrackingDigiHit,9);
};

class EicTrackingDigiHit1D : public EicTrackingDigiHit
{
 public:   
 EicTrackingDigiHit1D(): EicTrackingDigiHit() { ResetVars(); };

  EicTrackingDigiHit1D(TString cname, const EicMoCaPoint *point, unsigned kfNodeID,
		       TVector3& global, const TVector3& local, 
		       //double local,
		       double sigma):
  EicTrackingDigiHit(cname, point, kfNodeID, global, local, true), /*mLocalCoord(local),*/ mSigma(sigma) {};
  virtual ~EicTrackingDigiHit1D() {}; 

  unsigned GetMdim()                             const { return 1; };
  //double GetCoord(unsigned id)                   const { return (id ? 0.0 : mLocalCoord); };
  double _GetCoord(unsigned id)                  const { return (id ? 0.0 : mLocalCoord[0]); };
  double GetCovariance(unsigned ip, unsigned iq) const { return (ip || iq ? 0.0 : mSigma*mSigma); };

  protected:

 private:
  void ResetVars() { /*mLocalCoord =*/ mSigma = 0.0; };

  //Double_t mLocalCoord; // local 1D coordinate
  Double_t mSigma;      // gaussian sigma in all cases

 ClassDef(EicTrackingDigiHit1D,6);
};

class EicTrackingDigiHitOrth2D : public EicTrackingDigiHit
{
 public:   
 EicTrackingDigiHitOrth2D(): EicTrackingDigiHit() { ResetVars(); };

  EicTrackingDigiHitOrth2D(TString cname, const EicMoCaPoint *point, unsigned kfNodeID,
			   TVector3& global,  const TVector3& local, bool xy_mode,
			   //double local[2],
			   double sigma[2]):
  EicTrackingDigiHit(cname, point, kfNodeID, global, local, xy_mode) {
    for(unsigned xy=0; xy<2; xy++) {
      //mLocalCoord[xy] = local[xy];
      mSigma     [xy] = sigma[xy];
    } //for xy
  };
  virtual ~EicTrackingDigiHitOrth2D() {}; 

  unsigned GetMdim()                             const { return 2; };
  double _GetCoord(unsigned id)                   const { assert(0); return (id < 2 ? mLocalCoord[id] : 0.0); };
  // check! double _GetCoord(unsigned id)                   const { /*assert(0);*/ return (id < 2 ? mLocalCoord[id] : 0.0); };
  double GetCovariance(unsigned ip, unsigned iq) const { 
    return ((ip != iq || ip >= 2) ? 0.0 : mSigma[ip]*mSigma[ip]); 
  };

  protected:

 private:
  void ResetVars() { 
    //memset(mLocalCoord, 0x00, sizeof(mLocalCoord));
    memset(mSigma,      0x00, sizeof(mSigma));
  };

  //Double_t mLocalCoord[2]; // local 2D coordinates
  Double_t mSigma[2];      // gaussian sigma in all cases

 ClassDef(EicTrackingDigiHitOrth2D,6);
};

//
// FIXME: unify with 2D case later;
//

class EicTrackingDigiHit3D : public EicTrackingDigiHit
{
 public:   
 EicTrackingDigiHit3D(): EicTrackingDigiHit() { ResetVars(); };

 EicTrackingDigiHit3D(TString cname, const EicMoCaPoint *point, //unsigned kfNodeID,
		      TVector3& global, const TVector3& local, 
		      //double local[3],
		      double covariance[3][3]):
  EicTrackingDigiHit(cname, point, 0, global, local, true) {
    //for(unsigned xy=0; xy<3; xy++) 
    //mLocalCoord[xy] = local[xy];
    for(unsigned ip=0; ip<3; ip++)
      for(unsigned iq=0; iq<3; iq++)
	mCovariance[ip][iq] = covariance[ip][iq];
  };
  virtual ~EicTrackingDigiHit3D() {}; 

  unsigned GetMdim()           const { return 3; };
  double _GetCoord(unsigned id) const { return (id < 3 ? mLocalCoord[id] : 0.0); };
  double GetCovariance(unsigned ip, unsigned iq) const { 
    return ((ip<3 && iq<3) ? mCovariance[ip][iq] : 0.0); 
  };

  protected:

 private:
  void ResetVars() { 
    //memset(mLocalCoord, 0x00, sizeof(mLocalCoord));
    memset(mCovariance, 0x00, sizeof(mCovariance));
  };

  //Double_t mLocalCoord[3];    // local 3D coordinates
  Double_t mCovariance[3][3]; // full covariance matrix

  ClassDef(EicTrackingDigiHit3D,4);
};

#endif
