//
// AYK (ayk@bnl.gov)
//
//  Hough transform node definitions for a forward tracker; 
//

#include <HoughNodeGroup.h>

#ifndef _FWD_HOUGH_NODE_GROUP_
#define _FWD_HOUGH_NODE_GROUP_

class TrKalmanNodeLocation;

class FwdHoughNodeGroup: public HoughNodeGroup
{
 public:
 FwdHoughNodeGroup(unsigned id): HoughNodeGroup(id), mLocation(0), mMarsToTemplate(0), 
    mTemplate(0), mCylindricalCoordSystem(false), mUseCartesian(true) {};
  ~FwdHoughNodeGroup() {};

  void SetLocation(TrKalmanNodeLocation *location)   { mLocation = location; };
  TrKalmanNodeLocation *GetLocation() const          { return mLocation; };

  t_hough_range Pack(const TVector3 &vtx) {
    return PackCore(vtx, mTemplate);
  };
  t_hough_range PackFromTo(const EicTrackingDigiHit *hit, double nSigma, double spSmearing, 
			   const KalmanNodeWrapper *kfwrapper, double sign) {

    EicKfNodeTemplate *kftmpl = kfwrapper->GetKfNodeTemplate();

    double tmplCoord[hit->GetMdim()];

    for(unsigned iq=0; iq<hit->GetMdim(); iq++) 
      tmplCoord[iq] = hit->_GetCoord(iq) + 
	nSigma*kftmpl->GetSigma(iq) + sign*kftmpl->GetSmearingValue(spSmearing, hit, iq);

    TVector3 vvNd = kftmpl->TemplateToThreeDee(tmplCoord);

    // FIXME: this clearly should be optimized;
    TVector3 vvGl = LocalToMaster(kfwrapper->GetNodeToMasterMtx(), vvNd);

    return PackCore(vvGl, kftmpl);
  };
  t_hough_range PackFrom(const EicTrackingDigiHit *hit, double nSigma, double spSmearing, 
			 const KalmanNodeWrapper *kfwrapper) {
    return PackFromTo(hit, -nSigma, spSmearing, kfwrapper, -1.);
  };
  t_hough_range PackTo  (const EicTrackingDigiHit *hit, double nSigma, double spSmearing,  
			 const KalmanNodeWrapper *kfwrapper) {
    return PackFromTo(hit,  nSigma, spSmearing, kfwrapper,  1.);
  };

  //void SetSmearingValues(const double sme[]) {
  //for(unsigned iq=0; iq<mTemplate->GetMdim(); iq++)
  //  mSmearingValues.push_back(sme[iq]);
  //};
  void SetTemplate(const EicKfNodeTemplate *tmpl) { mTemplate = tmpl; };

  void SetMarsToTemplateMtx(const TGeoHMatrix *mtx) { mMarsToTemplate = mtx; };
  const TGeoHMatrix *GetMarsToTemplateMtx() const   { return mMarsToTemplate; };

  void SetCartesianFlag(bool useCartesian) { mUseCartesian = useCartesian; }
  bool GetCartesianFlag() const { return mUseCartesian; }

 private:
  bool mCylindricalCoordSystem;

  bool mUseCartesian;

  TrKalmanNodeLocation *mLocation;

  // FIXME: is not mTemplate enough?;
  const TGeoHMatrix *mMarsToTemplate;
  
  const EicKfNodeTemplate *mTemplate;

  //std::vector<double> mSmearingValues;

  t_hough_range PackCore(const TVector3 &vvGl, const EicKfNodeTemplate *kftmpl) {
    double crs[2];

    TVector3 vvTmpl = MasterToLocal(mMarsToTemplate, vvGl);
    if (mUseCartesian)
      for(unsigned xy=0; xy<kftmpl->GetMdim(); xy++)
	crs[xy] = vvTmpl[xy];
    else
      kftmpl->CartesianToCylindrical(vvTmpl, crs);

    {
      int strips[2];

      for(unsigned xy=0; xy<kftmpl->GetMdim(); xy++)
	strips[xy] = (int)floor((crs[xy] - GetMin(xy))/GetGra(xy));

      return HoughPack(strips);
    }
  };
};

#endif
