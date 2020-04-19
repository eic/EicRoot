//
// AYK (ayk@bnl.gov), 2013/06/12
//
//  Tracking digi hit producer class;
//

#include <TRandom.h>
#include <TMath.h>

#include <EicDigiHitProducer.h>
#include <EicDigiParData.h>
#include <EicTrackingDigiHit.h>

#define RADIANS(x) ((x)*TMath::Pi()/180.0)

#ifndef _EIC_TRACKING_DIGI_HIT_PRODUCER_
#define _EIC_TRACKING_DIGI_HIT_PRODUCER_

class KfMatrix;
class SensitiveVolume;
class EicRunDigi;

class EicKfNodeTemplate: public TObject
{
  friend class EicTrackingDigiHitProducer;
  friend class EicHtcTask;
  friend class KalmanNodeWrapper;

 public:
  EicKfNodeTemplate(TGeoMatrix *node2sv = 0): mNodeToSensitiveVolume(node2sv) {};
  ~EicKfNodeTemplate() {};

  // These are either hints or exclusions based on which main code should decide
  // on basic compatibility and guess to choose either {XY} or {r,phi} generic scheme;
  //virtual bool FavorCylindricalThreeDee()                                 const { return false; };
  virtual bool CylindricalThreeDeeOnly()                                  const { return false; };
  virtual bool CartesianThreeDeeOnly()                                    const { return false; };

  // No default calls here;
  virtual void FillGranularityArray(bool useCartesian, double spGranularity, double aGranularity, 
				    double gra[]) const = 0;
  //virtual void FillSmearingArray(double spSmearing, double aSmearing, double sme[]) const = 0;
  virtual double GetSmearingValue(double spSmearing, const EicTrackingDigiHit *hit, 
				  unsigned iq)         const = 0;
  virtual void FillMinMaxArrays(bool useCartesian, const std::set<double> &xMin, 
				const std::set<double> &xMax, 
				const std::set<double> &yMin, const std::set<double> &yMax, 
				const std::set<double> &rMin, const std::set<double> &rMax, 
				double min[], double max[]) const = 0;

  virtual unsigned GetMdim()                                              const = 0;
  virtual double GetSigma(unsigned iq)                                    const = 0;
  virtual double GetPitch(unsigned iq)                                    const = 0;
  virtual double GetPixelCenterOffset(unsigned iq)                        const = 0;

  virtual KfMatrix *GetMeasurementNoise(const EicTrackingDigiHit *hit)    const = 0;

  // Well, 0.0 is equivalent to "don't know"; fine as default;
  virtual double GetSpatialSigma()                                        const { return 0.0; };
  virtual double GetAngularSigma()                                        const { return 0.0; };

  //virtual unsigned CalculateSmearing(double smearing) const {
  //assert(0);
  //};

  bool IsCompatible(const EicKfNodeTemplate *sample) {
    // Dimensions should match; otherwise nothing to talk about;
    if (GetMdim() != sample->GetMdim())                                 return false;

    if (CylindricalThreeDeeOnly() && sample->CartesianThreeDeeOnly())   return false;
    if (CartesianThreeDeeOnly()   && sample->CylindricalThreeDeeOnly()) return false;

    // FIXME: eventually check orientation of cartesian-only and origin match of 
    // cylindrical-only templates;

    return true;
  };

  int IncrementLinearTrackFitMatrices(SensitiveVolume *sv, 
				      EicTrackingDigiHit *hit, double zRef,
				      KfMatrix *A, KfMatrix *b);

  // These two calls default to linear templates and can be overriden by {r,phi} ones;
  virtual TVector3 TemplateToThreeDee(const double tmplCoord[]) const {
    TVector3 vv(0.0, 0.0, 0.0);

    for(unsigned xy=0; xy<GetMdim(); xy++)
      vv[xy] = tmplCoord[xy];

    return vv;
  }; 
  virtual void ThreeDeeToTemplate(const TVector3 &crs, double tmplCoord[]) const { 
    for(unsigned xy=0; xy<GetMdim(); xy++) 
      tmplCoord[xy] = crs[xy];
  };
  virtual void CartesianToCylindrical(const TVector3 &crs, double tmplCoord[]) const { 
    assert(0);
  };

 protected:
  double GetSmearedValue(double value, unsigned iq, EicDigiHitProducer::SmearingModel smearing_model) {
    if (iq >= GetMdim()) return value;

    switch (smearing_model) {
      case EicDigiHitProducer::Smear:
	return value + (GetSigma(iq) ? gRandom->Gaus(0.0, GetSigma(iq)) : 0.0);
      case EicDigiHitProducer::Quantize:
	if (!GetPitch(iq)) return value;

	return GetPitch(iq) * rint((value - GetPixelCenterOffset(iq))/GetPitch(iq)) + 
	  GetPixelCenterOffset(iq);
      default:
	assert(0); return 0.0;
    } //switch
  };

  // These two mathods are template-specific;
  virtual void SmearLocalCoord(TVector3 &local, EicDigiHitProducer::SmearingModel smearing_model) = 0;
  virtual void PackSmearedHit(TClonesArray *arr, 
			   const TString &detName, 
			   const EicMoCaPoint *point, unsigned kfNodeID, 
			   TVector3 &global, 
			   TVector3 &local) = 0;

  // This one is sort of universal;
  void StoreDigiHit(TClonesArray *arr, 
		   const TString &detName, 
		   const EicMoCaPoint *point, unsigned kfNodeID, 
		   TVector3 &global, 
		   TVector3 &local, EicDigiHitProducer::SmearingModel smearing_model) {    
    SmearLocalCoord(local, smearing_model);

    PackSmearedHit(arr, detName, point, kfNodeID, global, local);
  };

 private:
  TGeoMatrix *mNodeToSensitiveVolume; // transformation from KF node to the sensitive volume 

  ClassDef(EicKfNodeTemplate,2)
};

// ---------------------------------------------------------------------------------------

class EicKfNodeTemplate1D: public EicKfNodeTemplate
{
 public:
 EicKfNodeTemplate1D(TGeoMatrix *transformation = 0): EicKfNodeTemplate(transformation), 
    mSigma(0.0), mPitch(0.0), mPixelCenterOffset(0.0) {};
  ~EicKfNodeTemplate1D() {};

  void SetSigma(double sigma)        { mSigma = sigma; };
  void SetPitch(double pitch)        { mPitch = pitch; mSigma = pitch/sqrt(12.); };

  unsigned GetMdim()           const { return 1; };
  double GetSigma(unsigned iq) const { return (iq == 0 ? mSigma : 0.0); };
  double GetPitch(unsigned iq) const { return (iq == 0 ? mPitch : 0.0); };
  double GetPixelCenterOffset(unsigned iq) const { 
    return (iq == 0 ? mPixelCenterOffset : 0.0); 
  };

  // This call is the same for all 1D templates;
  void PackSmearedHit(TClonesArray *arr, 
		   const TString &detName, 
		   const EicMoCaPoint *point, unsigned kfNodeID, 
		   TVector3 &global, 
		   TVector3 &local) {
    assert(0);
    new((*arr)[arr->GetEntriesFast()])  
      EicTrackingDigiHit1D(detName, point, kfNodeID, global, local, mSigma);
  };

  KfMatrix *GetMeasurementNoise(const EicTrackingDigiHit *hit) const;

 protected:
  Double_t mSigma;             // gaussian sigma in all cases

 private:
  Double_t mPitch;             // 1D pitch in case of 'Quantize' digitization
  // Unbiased quantization will be done with this overall offset of pixel grid; 
  Double_t mPixelCenterOffset; // any pixel center offset

  ClassDef(EicKfNodeTemplate1D,6)
};

// THINK: does nothing; yet want to have a separate class, "symmetric" to the 
// Radial and Asimuthal ones;
class EicKfNodeTemplateLinear1D: public EicKfNodeTemplate1D
{
 public:
 EicKfNodeTemplateLinear1D(TGeoMatrix *transformation = 0): 
  EicKfNodeTemplate1D(transformation) {};
  ~EicKfNodeTemplateLinear1D() {};

  void FillGranularityArray(bool useCartesian, double spGranularity, 
			    double aGranularity, double gra[]) const {
    gra[0] = spGranularity;
  };
  //void FillSmearingArray(double spSmearing, double aSmearing, double sme[]) const {
  // sme[0] = spSmearing;
  //};
  double GetSmearingValue(double spSmearing, const EicTrackingDigiHit *hit, unsigned iq) const { 
    return (iq ? 0.0 : spSmearing); 
  };
  void FillMinMaxArrays(bool useCartesian, const std::set<double> &xMin, const std::set<double> &xMax, 
			const std::set<double> &yMin, const std::set<double> &yMax,
			const std::set<double> &rMin, const std::set<double> &rMax, 
			double min[], double max[]) const {
    min[0] = *xMin.begin();
    max[0] = *xMax.rbegin();
  };

  void SmearLocalCoord(TVector3 &local, EicDigiHitProducer::SmearingModel smearing_model) {
    assert(0);

    local.SetX(GetSmearedValue(local[0], 0, smearing_model));
  };

  double GetSpatialSigma()     const { return mSigma; };
  bool CartesianThreeDeeOnly() const { return true; };

  ClassDef(EicKfNodeTemplateLinear1D,1)
};

class EicKfNodeTemplateRadial1D: public EicKfNodeTemplate1D
{
 public:
 EicKfNodeTemplateRadial1D(TGeoMatrix *transformation = 0): 
  EicKfNodeTemplate1D(transformation) {};
  ~EicKfNodeTemplateRadial1D() {};

  double GetSpatialSigma()     const { return mSigma; };
  bool CylindricalThreeDeeOnly() const { return true; };

  void FillGranularityArray(bool useCartesian, double spGranularity, 
			    double aGranularity, double gra[]) const {
    gra[0] = spGranularity;
  };
  //void FillSmearingArray(double spSmearing, double aSmearing, double sme[]) const {
  //sme[0] = spSmearing;
  //};
  double GetSmearingValue(double spSmearing, const EicTrackingDigiHit *hit, unsigned iq) const { 
    return (iq ? 0.0 : spSmearing); 
  };
  void FillMinMaxArrays(bool useCartesian, const std::set<double> &xMin, const std::set<double> &xMax, 
			const std::set<double> &yMin, const std::set<double> &yMax,
			const std::set<double> &rMin, const std::set<double> &rMax, 
			double min[], double max[]) const {
    min[0] = 0.0; 
    max[0] = *rMax.rbegin();
  };

  void SmearLocalCoord(TVector3 &local, EicDigiHitProducer::SmearingModel smearing_model) {
    assert(0);
  };

  void CartesianToCylindrical(const TVector3 &crs, double tmplCoord[]) const { 
    ThreeDeeToTemplate(crs, tmplCoord);
  };
  TVector3 TemplateToThreeDee(const double tmplCoord[]) const { 
    TVector3 vv(0.0, 0.0, 0.0);

    // NB: 'phi' can be any (?) -> take 0.0;
    double r = tmplCoord[0], phi = 0.0;
    vv[0] = r*cos(phi); vv[1] = r*sin(phi);

    return vv;
  }; 
  void ThreeDeeToTemplate(const TVector3 &crs, double tmplCoord[]) const {
    double x = crs[0], y = crs[1]; 
    tmplCoord[0] = sqrt(x*x+y*y);
  }; 

  ClassDef(EicKfNodeTemplateRadial1D,1)
};

class EicKfNodeTemplateAsimuthal1D: public EicKfNodeTemplate1D
{
 public:
 EicKfNodeTemplateAsimuthal1D(TGeoMatrix *transformation = 0): 
  EicKfNodeTemplate1D(transformation) {};
  ~EicKfNodeTemplateAsimuthal1D() {};

  double GetAngularSigma()     const { return mSigma; };
  bool CylindricalThreeDeeOnly() const { return true; };

  void FillGranularityArray(bool useCartesian, double spGranularity, 
			    double aGranularity, double gra[]) const {
    gra[0] = aGranularity;
  };
  //void FillSmearingArray(double spSmearing, double aSmearing, double sme[]) const {
  //sme[0] = aSmearing;
  //};
  // Don't know what to do with this value, sorry;
  double GetSmearingValue(double spSmearing, const EicTrackingDigiHit *hit, unsigned iq) const { 
    return 0.0; 
  };
  void FillMinMaxArrays(bool useCartesian, const std::set<double> &xMin, const std::set<double> &xMax, 
			const std::set<double> &yMin, const std::set<double> &yMax, 
			const std::set<double> &rMin, const std::set<double> &rMax, 
			double min[], double max[]) const {
    min[0] = -TMath::Pi(); 
    max[0] =  TMath::Pi();
  };
  TVector3 TemplateToThreeDee(const double tmplCoord[]) const { 
    TVector3 vv(0.0, 0.0, 0.0);

    // NB: 'r' can be any (?) -> take 1.0;
    double r = 1.0, phi = tmplCoord[0];
    vv[0] = r*cos(phi); vv[1] = r*sin(phi);

    return vv;
  }; 
  void CartesianToCylindrical(const TVector3 &crs, double tmplCoord[]) const { 
    ThreeDeeToTemplate(crs, tmplCoord);
  };
  void ThreeDeeToTemplate(const TVector3 &crs, double tmplCoord[]) const { 
    double x = crs[0], y = crs[1]; 
    tmplCoord[0] = atan2(y, x);
  }; 

  void SmearLocalCoord(TVector3 &local, EicDigiHitProducer::SmearingModel smearing_model) {
    assert(0);
  };

  ClassDef(EicKfNodeTemplateAsimuthal1D,1)
};

// ---------------------------------------------------------------------------------------

class EicKfNodeTemplateOrth2D: public EicKfNodeTemplate
{
  friend class EicTrackingDigiHitProducer;

 public:
 EicKfNodeTemplateOrth2D(TGeoMatrix *transformation = 0, bool xy_mode = true): 
  EicKfNodeTemplate(transformation), mXYmode(xy_mode) {
    mSigma[0] = mSigma[1] = mPitch[0] = mPitch[1] = 0.0;
    mPixelCenterOffset[0] = mPixelCenterOffset[1] = 0.0;
  };
  ~EicKfNodeTemplateOrth2D() {};

  unsigned GetMdim()           const { return 2; };
  double GetSigma(unsigned iq) const { return (iq <= 1 ? mSigma[iq] : 0.0); };
  double GetPitch(unsigned iq) const { return (iq <= 1 ? mPitch[iq] : 0.0); };
  double GetPixelCenterOffset(unsigned iq) const { 
    return (iq <= 1 ? mPixelCenterOffset[iq] : 0.0); 
  };

  // THINK: so it is the same for EicKfNodeTemplateCartesian2D & EicKfNodeTemplateCylindrical2D?;
  void FillGranularityArray(bool useCartesian, double spGranularity, 
			    double aGranularity, double gra[]) const {
    gra[0] = spGranularity;
    gra[1] = useCartesian ? spGranularity : aGranularity;
  };
  void FillMinMaxArrays(bool useCartesian, const std::set<double> &xMin, const std::set<double> &xMax, 
			const std::set<double> &yMin, const std::set<double> &yMax, 
			const std::set<double> &rMin, const std::set<double> &rMax, 
			double min[], double max[]) const {
    if (useCartesian) {
      // Well, I guess can do by hand?;
      min[0] = *xMin.begin();
      max[0] = *xMax.rbegin();
      min[1] = *yMin.begin();
      max[1] = *yMax.rbegin();
    } else {
      min[0] = 2.0; 
      max[0] = *rMax.rbegin();
      // Should work for stere skewed option as well, right?;
      min[1] = -TMath::Pi(); 
      max[1] =  TMath::Pi();
    } //if
  };

  // This call is the same for all 2D templates;
  void PackSmearedHit(TClonesArray *arr, 
		   const TString &detName, 
		   const EicMoCaPoint *point, unsigned kfNodeID, 
		   TVector3 &global, 
		   TVector3 &local) {
    new((*arr)[arr->GetEntriesFast()])  
      EicTrackingDigiHitOrth2D(detName, point, kfNodeID, global, local, mXYmode, mSigma);
  };

  KfMatrix *GetMeasurementNoise(const EicTrackingDigiHit *hit) const;

  void CartesianToCylindrical(const TVector3 &crs, double tmplCoord[]) const { 
    double x = crs[0], y = crs[1], r = sqrt(x*x+y*y); 
    double phi = atan2(y, x);

    tmplCoord[0] = r;
    tmplCoord[1] = phi;
  };

  bool mXYmode;

 protected:
  // Well, do not see any good reason to introduce true correlations; if such 
  // a detector ever becomes needed, just create a separate class;
  Double_t mSigma[2]; // gaussian sigma in all cases
  Double_t mPixelCenterOffset[2];

 private:
  void SetSigma(double sigmaX, double sigmaY) { 
    mSigma[0] = sigmaX; mSigma[1] = sigmaY;
  };
  void SetPitch(double pitchX, double pitchY) { 
    mPitch[0] = pitchX; mPitch[1] = pitchY;

    for(unsigned xy=0; xy<2; xy++)
      mSigma[xy] = mPitch[xy]/sqrt(12.);
  };

  Double_t mPitch[2]; // 2D pitch in case of 'Quantize' digitization

  ClassDef(EicKfNodeTemplateOrth2D,5);
};

// THINK: does nothing; yet want to have a separate class, "symmetric" to the 
// Cylindrical one;
class EicKfNodeTemplateCartesian2D: public EicKfNodeTemplateOrth2D
{
 public:
  // NB: typically use XY-based mode (true); the other option iz TZ-mode (false);
 EicKfNodeTemplateCartesian2D(TGeoMatrix *transformation = 0, bool xy_mode = true): 
  EicKfNodeTemplateOrth2D(transformation, xy_mode) {};
  ~EicKfNodeTemplateCartesian2D() {};

  void SmearLocalCoord(TVector3 &local, EicDigiHitProducer::SmearingModel smearing_model) {
    if (mXYmode) {
      // In this case smear XY-coordinates;
      local.SetX(GetSmearedValue(local[0], 0, smearing_model));
      local.SetY(GetSmearedValue(local[1], 1, smearing_model));
    } else {
      // FIXME: for now do not want these extra complications;
      assert(smearing_model == EicDigiHitProducer::Smear);

      // In this case smear RZ-coordinates;
      {
	// NB: should be in sync with EicPlanarRecoHit::EicPlanarRecoHit();
	TVector3 uu = TVector3(local.Y(),-local.X(),0).Unit();
	double smeared_value = GetSmearedValue(0.0, 0, smearing_model);

	local += smeared_value * uu;
      }
      local.SetZ(GetSmearedValue(local[2], 1, smearing_model));
    } //if
  };

  double GetSmearingValue(double spSmearing, const EicTrackingDigiHit *hit, unsigned iq) const {
    return (iq <= 1 ? spSmearing : 0.0); 
  };
  double GetSpatialSigma() const { 
    return mSigma[0] < mSigma[1] ? mSigma[0] : mSigma[1]; 
  };

  ClassDef(EicKfNodeTemplateCartesian2D,1);
};

class EicKfNodeTemplateCylindrical2D: public EicKfNodeTemplateOrth2D
{
 public:
 EicKfNodeTemplateCylindrical2D(TGeoMatrix *transformation = 0): 
  EicKfNodeTemplateOrth2D(transformation), mStereoSkewRadius(0.0) {};
  ~EicKfNodeTemplateCylindrical2D() {};

  double GetSpatialSigma()                                        const { return mSigma[0]; };
  double GetAngularSigma()                                        const { return mSigma[1]; };

  double GetSmearingValue(double spSmearing, const EicTrackingDigiHit *hit, unsigned iq) const { 
    switch (iq) {
    case 0:
      return spSmearing;
    case 1:
      {
	double r = hit->_GetCoord(0); assert(r);
	return spSmearing/r;
      }
    default:
      return 0.0;
    } //switch
  };
  TVector3 TemplateToThreeDee(const double tmplCoord[])           const { 
    TVector3 vv(0.0, 0.0, 0.0);

    double r = tmplCoord[0], phi = tmplCoord[1];
    if (mStereoSkewRadius) {
      // FIXME: do a proper check here; this regularization is really crap;
      //assert(mStereoSkewRadius < r);
      if (r < mStereoSkewRadius) r = mStereoSkewRadius;

      // NB: mStereoSkewRadius sign will be automatically taken into account here;
      if (mStereoSkewRadius <= r) phi -= asin(mStereoSkewRadius/r);
    } //if
    vv[0] = r*cos(phi); vv[1] = r*sin(phi);

    return vv;
  };
  void ThreeDeeToTemplate(const TVector3 &crs, double tmplCoord[]) const { 
    CartesianToCylindrical(crs, tmplCoord);

    // If stereo skew is defined, modify asimuthal angle (which is tmplCoord[1]);
    if (mStereoSkewRadius) {
      // FIXME: do a proper check here;
      double r = tmplCoord[0]; assert(mStereoSkewRadius < r);

      // NB: mStereoSkewRadius sign will be automatically taken into account here;
      tmplCoord[1] += asin(mStereoSkewRadius/r);
    } //if    
  };

  void SmearLocalCoord(TVector3 &local, EicDigiHitProducer::SmearingModel smearing_model) {
    assert(0);
  };
#if _OFF_
  void StoreDigiHit(TClonesArray *arr, 
		   const TString &detName, 
		   const EicMoCaPoint *point, unsigned kfNodeID, 
		   TVector3 &global, 
		   TVector3 &local, EicDigiHitProducer::SmearingModel smearing_model) {
    assert(0);
  };
#endif
  void SetStereoSkewRadius(double radius) { mStereoSkewRadius = radius; };
  // FIXME: may later want to use for other templates as well;
  void SetPixelCenterOffsets(double offsetR, double offsetA = 0.0) {
    mPixelCenterOffset[0] = offsetR;
    mPixelCenterOffset[1] = offsetA;
  };

 private:
  double mStereoSkewRadius;

  ClassDef(EicKfNodeTemplateCylindrical2D,2);
};

// ---------------------------------------------------------------------------------------

class EicKfNodeTemplateOrth3D: public EicKfNodeTemplate
{
  friend class EicTrackingDigiHitProducer;

 public:
 EicKfNodeTemplateOrth3D(TGeoMatrix *transformation = 0): EicKfNodeTemplate(transformation) {
    mSigma[0] = mSigma[1] = mSigma[2] = 0.0;
  };
  ~EicKfNodeTemplateOrth3D() {};

  unsigned GetMdim() const { return 3; };
  double GetSigma(unsigned iq) const { return (iq <= 2 ? mSigma[iq] : 0.0); };
  double GetPitch(unsigned iq) const { assert(0); return 0.0; };
  double GetPixelCenterOffset(unsigned iq) const { assert(0); return 0.0; };

  void FillGranularityArray(bool useCartesian, double spGranularity, 
			    double aGranularity, double gra[]) const {
    assert(0);
  };
  double GetSmearingValue(double spSmearing, const EicTrackingDigiHit *hit, unsigned iq) const { 
    assert(0); return 0.0;
  };
  void FillMinMaxArrays(bool useCartesian, const std::set<double> &xMin, const std::set<double> &xMax, 
			const std::set<double> &yMin, const std::set<double> &yMax, 
			const std::set<double> &rMin, const std::set<double> &rMax,
			double min[], double max[]) const {
    assert(0); 
  };
  //void FillSmearingArray(double spSmearing, double aSmearing, double sme[]) const {
  //assert(0);
  //};

  void SmearLocalCoord(TVector3 &local, EicDigiHitProducer::SmearingModel smearing_model) {
    assert(0);
  };
  void PackSmearedHit(TClonesArray *arr, 
		   const TString &detName, 
		   const EicMoCaPoint *point, unsigned kfNodeID, 
		   TVector3 &global, 
		   TVector3 &local) {
    assert(0);
  };
#if _OLD_
  int PackSmearedHit(TClonesArray *arr, const TString &detName, 
		     const EicMoCaPoint *point, unsigned kfNodeID, 
		     double localCoord[], double localDirection[], TVector3 &global) {
    assert(0);
    // It has never been checked this stuff works after Nov'2015 changes;

    // And diagonal cov.matrix, please;
    double localCov[3][3];
    memset(localCov, 0x00, sizeof(localCov));
    for(int iq=0; iq<3; iq++)
      localCov[iq][iq] = mSigma[iq] * mSigma[iq];
    
    TVector3 vCoord = TVector3(localCoord);
    new((*arr)[arr->GetEntriesFast()])  
      //EicTrackingDigiHit3D(detName, point, global, localCoord, localCov);
      EicTrackingDigiHit3D(detName, point, global, vCoord, localCov);
  };
#endif

  KfMatrix *GetMeasurementNoise(const EicTrackingDigiHit *hit) const;

 private:
  void SetSigma(double sigmaX, double sigmaY, double sigmaZ) { 
    mSigma[0] = sigmaX; mSigma[1] = sigmaY; mSigma[2] = sigmaZ;
  };

  // Well, do not see any good reason to introduce true correlations; if such 
  // a detector ever becomes needed, just create a separate class;
  Double_t mSigma[3]; // gaussian sigma in volume local coordinate system

  ClassDef(EicKfNodeTemplateOrth3D,1)
};

// ---------------------------------------------------------------------------------------

#if _LATER_
class EicKfNodeTemplateAxial3D: public EicKfNodeTemplate
{
  friend class EicTrackingDigiHitProducer;

 public:
 EicKfNodeTemplateAxial3D(TGeoMatrix *transformation = 0): EicKfNodeTemplate(transformation) {
    mSigmaL = mSigmaT = 0.0;
  };
  ~EicKfNodeTemplateAxial3D() {};

  unsigned GetMdim() const { return 3; };
  // This call is sort of fake here;
  double GetSigma(unsigned iq) const { 
    switch (iq) {
    case 0:;
    case 1:
      return mSigmaT;
    case 2:
      return mSigmaL;
    default: 
      return 0.0;
    } //switch
  };     
  double GetPitch(unsigned iq) const { assert(0); };
  double GetPixelCenterOffset(unsigned iq) const { assert(0); };
  int PackSmearedHit(TClonesArray *arr, const TString &detName, 
		     const EicMoCaPoint *point, unsigned kfNodeID, 
		     TVector3 &local, double localDirection[], TVector3 &global) {
    assert(0);
  };
  //void FillSmearingArray(double spSmearing, double aSmearing, double sme[]) const {
  //assert(0);
  //};

  void FillGranularityArray(bool useCartesian, double spGranularity, 
			    double aGranularity, double gra[]) const {
    assert(0);
  };
  double GetSmearingValue(double spSmearing, const EicTrackingDigiHit *hit, unsigned iq) const { 
    assert(0); return 0.0;
  };
  void FillMinMaxArrays(bool useCartesian, const std::set<double> &xMin, const std::set<double> &xMax, 
			const std::set<double> &yMin, const std::set<double> &yMax,
			const std::set<double> &rMin, const std::set<double> &rMax,
			double min[], double max[]) const {
    assert(0);
  };

  int StoreDigiHit(TClonesArray *arr, EicDigiHitProducer::SmearingModel originalSmearingModel,
		  EicDigiHitProducer::SmearingModel effectiveSmearingModel,
		  const TString &detName, 
		  const EicMoCaPoint *point, unsigned kfNodeID, 
		  TVector3 &local, double localDirection[], TVector3 &global) {
    ConvertLocalCoordInPlace(localCoord);

    //
    // FIXME: "sigma != 0" case?;
    //

    // This part is a bit nasty; may want to write a more generic call; basically all I need
    // is to calculate 3D rotation which moves local Z axis to localDirection[] (or vice versa?);
    TVector3 zLocal(0.0, 0.0, 1.0), zAxis(localDirection[0], localDirection[1], localDirection[2]);
    TVector3 xAxis = (zLocal.Cross(zAxis)).Unit(), yAxis = zAxis.Cross(xAxis);

    double data[3][3] = {{xAxis.X(), yAxis.X(), zAxis.X()},
			 {xAxis.Y(), yAxis.Y(), zAxis.Y()},
			 {xAxis.Z(), yAxis.Z(), zAxis.Z()}};
    TGeoRotation grr; 
    grr.SetMatrix((double*)data); 

    // Ok, now ehen rotation matrix is calculated, need to move localCoord[] into the rotated 
    // system (where it is easy to smear it);
    double rotatedCoord[3];
    grr.MasterToLocal(localCoord, rotatedCoord);
#if 0
    printf("loc(xx): %f %f %f\n", localCoord  [0], localCoord  [1], localCoord  [2]);
    printf("rot(xx): %f %f %f\n", rotatedCoord[0], rotatedCoord[1], rotatedCoord[2]);
    {
      double bff[3];
      grr.MasterToLocalVect(localDirection, bff);
      printf("loc(nn): %f %f %f\n", localDirection  [0], localDirection  [1], localDirection  [2]);
      printf("rot(nn): %f %f %f\n\n", bff[0], bff[1], bff[2]);
    }
#endif

    // In rotated coordinate system cov.matrix is trivially diagonal (do not care about 
    // arbitrary phi rotation around Z axis);
    double qsigma[3] = {mSigmaT, mSigmaT, mSigmaL};

    // If no action was requested (like in case of real hit import), 
    // do not touch "original" local[] at all (so no smearing); 
    if (effectiveSmearingModel != EicDigiHitProducer::NoAction) {
      // This is just a dummy call for now;
      for(int iq=0; iq<3; iq++)
	rotatedCoord[iq] += gRandom->Gaus(0.0, qsigma[iq]);
    } //if

    // Move back; localCoord[] is now smeared properly;
    grr.LocalToMaster(rotatedCoord, localCoord);

    // Now need to calculate cov.matrix; just fill out diagonal elements and then move 
    // back from rotated to local system;
    double rotatedCov[3][3], localCov[3][3];
    memset(rotatedCov, 0x00, sizeof(rotatedCov));
    for(int iq=0; iq<3; iq++)
      rotatedCov[iq][iq] = qsigma[iq] * qsigma[iq];

    memset(localCov, 0x00, sizeof(localCov));
    // FIXME: not too much efficient;
    for(unsigned ip=0; ip<3; ip++)
      for(unsigned ir=0; ir<3; ir++)
	for(unsigned is=0; is<3; is++)
	  for(unsigned iq=0; iq<3; iq++)
	    //localCov[ip][iq] += data[ir][ip] * rotatedCov[ir][is] * data[is][iq]; 
	    localCov[ip][iq] += data[ip][ir] * rotatedCov[ir][is] * data[iq][is]; 
    
    new((*arr)[arr->GetEntriesFast()])  
      EicTrackingDigiHit3D(detName, point, global, localCoord, localCov);

    return 0;
  };

  KfMatrix *GetMeasurementNoise(const EicTrackingDigiHit *hit) const;

  TVector3 TemplateToThreeDee(               const double tmplCoord[]) const { assert(0); };
  void     ThreeDeeToTemplate(const TVector3 &crs, double tmplCoord[]) const { assert(0); };

 private:
  void SetSigma(double sigmaL, double sigmaT) { 
    mSigmaL = sigmaL; mSigmaT = sigmaT;
  };

  // Well, do not see any good reason to introduce true correlations; if such 
  // a detector ever becomes needed, just create a separate class;
  Double_t mSigmaL; // gaussian sigma along track direction
  Double_t mSigmaT; // gaussian sigma in "both" transverse directions

  ClassDef(EicKfNodeTemplateAxial3D,1)
};
#endif
// ---------------------------------------------------------------------------------------

class EicTrackingDigiHitProducer: public EicDigiHitProducer
{
  friend class EicHtcTask;

 public:
  EicTrackingDigiHitProducer()/*: mHitImportMode(false), mForceRealHitSmearing(false)*/ {};
  EicTrackingDigiHitProducer(const char *name, 
			     SmearingModel smearingModel = EicDigiHitProducer::Smear);
  ~EicTrackingDigiHitProducer() {};

  // Want just to allocate mDigiHitArray of proper type objects;
  InitStatus ExtraInit();

  void DefineKfNodeTemplate1D(double angle, double sigmaOrPitch) {
    TGeoMatrix *mtx = 0;

    if (angle) {
      mtx = new TGeoRotation();
      mtx->RotateZ(angle);
    } //if 

    EicKfNodeTemplate1D *node = new EicKfNodeTemplateLinear1D(mtx);

    DefineKfNodeTemplateCore1D(node, sigmaOrPitch);
  };

  void DefineKfNodeTemplateX(double sigmaOrPitch) {
    DefineKfNodeTemplate1D( 0.0, sigmaOrPitch);
  };
  void DefineKfNodeTemplateY(double sigmaOrPitch) {
    DefineKfNodeTemplate1D(90.0, sigmaOrPitch);
  };
  void DefineKfNodeTemplateR(double sigmaOrPitch) {
    EicKfNodeTemplate1D *node = new EicKfNodeTemplateRadial1D();

    DefineKfNodeTemplateCore1D(node, sigmaOrPitch);
  };
  void DefineKfNodeTemplateA(double sigmaOrPitch) {
    EicKfNodeTemplate1D *node = new EicKfNodeTemplateAsimuthal1D();

    DefineKfNodeTemplateCore1D(node, RADIANS(sigmaOrPitch));
  };

  void DefineKfNodeTemplateXY(double sigmaOrPitchX, double sigmaOrPitchY) {
    EicKfNodeTemplateCartesian2D *node = new EicKfNodeTemplateCartesian2D();

    DefineKfNodeTemplateCore2D(node, sigmaOrPitchX, sigmaOrPitchY);
  };
  void DefineKfNodeTemplateTZ(double sigmaOrPitchX, double sigmaOrPitchY) {
    EicKfNodeTemplateCartesian2D *node = new EicKfNodeTemplateCartesian2D(0, false);

    DefineKfNodeTemplateCore2D(node, sigmaOrPitchX, sigmaOrPitchY);
  };
  EicKfNodeTemplateCylindrical2D *DefineKfNodeTemplateRA(double sigmaOrPitchR, 
							 double sigmaOrPitchA) {
    EicKfNodeTemplateCylindrical2D *node = new EicKfNodeTemplateCylindrical2D();

    DefineKfNodeTemplateCore2D(node, sigmaOrPitchR, RADIANS(sigmaOrPitchA));

    return node;
  };

  void DefineKfNodeTemplateOrth3D(double sigmaX, double sigmaY, double sigmaZ) {
    EicKfNodeTemplateOrth3D *node = new EicKfNodeTemplateOrth3D();

    //assert(mOriginalSmearingModel == EicDigiHitProducer::Smear);
    assert(mSmearingModel == EicDigiHitProducer::Smear);
    node->SetSigma(sigmaX, sigmaY, sigmaZ);

    mKfNodeTemplates.push_back(node);
    AssignDigiHitClassName("EicTrackingDigiHit3D");
  };
#if _LATER_
  void DefineKfNodeTemplateAxial3D(double sigmaL, double sigmaT) {
    EicKfNodeTemplateAxial3D *node = new EicKfNodeTemplateAxial3D();

    assert(mOriginalSmearingModel == EicDigiHitProducer::Smear);
    //mOriginalSmearingModel == EicDigiHitProducer::Smear ? 
    node->SetSigma(sigmaL, sigmaT);// : node->SetPitch(sigmaOrPitchX, sigmaOrPitchY);

    mKfNodeTemplates.push_back(node);
    AssignDigiHitClassName("EicTrackingDigiHit3D");
  };
#endif

  // Tracking-specific generic (ideal) call; NB: TPC will handle hits totally 
  // differently; also at some point will have to implement hit overlap and 
  // things like this;
  int HandleHit(const EicMoCaPoint *point);

  //void ForceRealHitSmearing() { 
  //mForceRealHitSmearing = true;
  // mEffectiveSmearingModel = mOriginalSmearingModel; 
  //};

 protected:

  void AssignDigiHitClassName(const char *name) {
    // Protected method -> assume 0 pointer can not happen;
    if (!mDigiHitClassName.IsNull() && !mDigiHitClassName.EqualTo(name))
      fLogger->Fatal(MESSAGE_ORIGIN, "\033[5m\033[31m attempt to define two different output"
		     " hit classes (%s & %s)! Not suported yet ... \033[0m", 
		     mDigiHitClassName.Data(), name); 

    mDigiHitClassName = name;
  };

 private:
  TString mDigiHitClassName; // either EicTrackingDigiHit1D or EicTrackingDigiHitOrth2D

  // Move to EicDigiHitProducer and merge with Calorimeter codes later;
  virtual EicDigiParData *getEicDigiParDataPtr() { return 0; };
  virtual void Finish();

  // Covariance matrix will be calculated differently (sqrt(12) involved in case of 
  // 'Quantize' mode);
  //SmearingModel mOriginalSmearingModel; // the mode given in constructor arguments
  SmearingModel mSmearingModel;// the effective smearing mode (NoAction if import hits)

  //EicRunDigi *mDigiRun;  //! transient pointer to EicRunDigi instance
  //Bool_t mHitImportMode; // 'true' if real hit import is requested
 public:
  std::vector<EicKfNodeTemplate*> mKfNodeTemplates;
 private:
  //Bool_t mForceRealHitSmearing; // may want to force real hit smearing a-la MC mode

  void DefineKfNodeTemplateCore1D(EicKfNodeTemplate1D *node, double sigmaOrPitch) {
    mSmearingModel == EicDigiHitProducer::Smear ? 
      node->SetSigma(sigmaOrPitch) : node->SetPitch(sigmaOrPitch);

    mKfNodeTemplates.push_back(node);
    AssignDigiHitClassName("EicTrackingDigiHit1D");
  };
  void DefineKfNodeTemplateCore2D(EicKfNodeTemplateOrth2D *node, 
				  double sigmaOrPitch1, double sigmaOrPitch2) {
    mSmearingModel == EicDigiHitProducer::Smear ? 
      node->SetSigma(sigmaOrPitch1, sigmaOrPitch2) : node->SetPitch(sigmaOrPitch1, sigmaOrPitch2);

    mKfNodeTemplates.push_back(node);
    AssignDigiHitClassName("EicTrackingDigiHitOrth2D");
  };

  ClassDef(EicTrackingDigiHitProducer,20);
};

#endif
