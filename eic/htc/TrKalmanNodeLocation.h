//
// AYK (ayk@bnl.gov)
//
//    Extracted from KalmanNode in Oct'2015;
//

#include <Mgrid.h>

#include <MediaSliceArray.h>
#include <KalmanFilter.h>

#include <RungeKuttaRequest.h>

#ifndef _TR_KALMAN_NODE_LOCATION_
#define _TR_KALMAN_NODE_LOCATION_

class MediaSliceArray;

struct ProcessNoise {
public:
ProcessNoise(): mCxx(0), mCyy(0), mCxy(0) {};

  // If transport matrix does not mix XY-coordinates (like in a field-free     
  // region or - approximately? - when field is mostly oriented along Y axis), 
  // it makes sense to split process noise covariance matrix into 3            
  // non-overlapping blocks and add all slices up; in general one should        
  // always recalculate full covariance matrix using real transport matrix     
  // in this given XYZ-point, momentum and slopes; want them to be pointers    
  // so that FillLowerTriangle() works on them :-);                          
  double **mCxx, **mCyy, **mCxy;
};

class TrKalmanNode;
class SensitiveVolume;
class EicKfNodeTemplate;

class TrKalmanNodeLocation {
 public:
 TrKalmanNodeLocation(double z): mPrev(0), mNext(0), mMediaSliceArray(0)/*, 
									  mSensitiveVolumeNodeWrapperCount(0)*/  {
    mProcessNoise[0] = mProcessNoise[1] = 0;
    memset(mRungeKutta, 0x00, sizeof(mRungeKutta));

    mPlane = t_3d_plane(TVector3(0,0,z), TVector3(0,0,1));
  };
  ~TrKalmanNodeLocation() {};

  void SetPrev(TrKalmanNodeLocation *location) { mPrev = location; };
  void SetNext(TrKalmanNodeLocation *location) { mNext = location; };

  ProcessNoise *InitializeProcessNoiseMatrices(KalmanFilter::Direction fb);

  // Media layers from this node to the next one (for the 
  // case of tracking Kalman filter); 
  MediaSliceArray *mMediaSliceArray;

  // [2]: F/B; 
  ProcessNoise *mProcessNoise[2];

  // '[2]': FORWARD/BACKWARD separately;
  RungeKutta mRungeKutta[2];

  void AddNode(TrKalmanNode *node)                             { mNodes.push_back(node); };
  void AddSensitiveVolume(SensitiveVolume *sv)                 { if (sv) mSensitiveVolumes.insert(sv); };

  TrKalmanNodeLocation *GetNext(unsigned fb)                   { return (fb ? mPrev : mNext); };
  TrKalmanNodeLocation *GetPrev(unsigned fb)                   { return (fb ? mNext : mPrev); };

  unsigned GetNodeCount()                                const { return mNodes.size(); };
  TrKalmanNode *GetNode(unsigned nd)                     const { return (nd < mNodes.size() ? mNodes[nd] : 0); };
  double GetZ()                                          const { return mPlane.GetCoord()[2]; };

  unsigned GetFiredNodeCount();

  bool HasSensitiveVolumes()                              const { return (mSensitiveVolumes.size() > 0); };
  const std::set<SensitiveVolume*> &GetSensitiveVolumes() const {
    return mSensitiveVolumes;
  };

  const t_3d_plane *GetPlane()                            const { return &mPlane; };

  void SetNextMdimValue(unsigned mdim) {
    // This indeed assumes, that TrKalmanFilter::SetUpLocations() fills out
    // all entries at once (so they are guaranteed to be in the right order);
    mDims.push_back(mdim);
  };
  unsigned GetSensitiveVolumeNodeWrapperCount()            const { return mDims.size(); };
  unsigned GetMdim(unsigned nd)                            const { return (nd < mDims.size() ? mDims[nd] : 0); };
  void SetNextNodeToMaster(const TGeoHMatrix *mtx) {
    // See SetNextMdimValue() above, same implicit assumption;
    mNodeToMasters.push_back(mtx);
  };
  const TGeoHMatrix *GetNodeToMaster(unsigned nd)          const { 
    return (nd < mNodeToMasters.size() ? mNodeToMasters[nd] : 0); 
  };
  void SetNextTemplate(const EicKfNodeTemplate *tmpl) {
    // See SetNextMdimValue() above, same implicit assumption;
    mDigiTemplates.push_back(tmpl);
  };
  const EicKfNodeTemplate *GetTemplate(unsigned nd)        const {
    return (nd < mDigiTemplates.size() ? mDigiTemplates[nd] : 0); 
  };
  //void SetNextCylindricalPreference(bool what)                   { 
  //mCylindricalPreferences.push_back(what); 
  //};
  //void SetCylindricalPreference(unsigned nd)                     { mCylindricalPreferences[nd] = true; };
  //bool GetCylindricalPreference(unsigned nd)               const { 
  //return (nd < mCylindricalPreferences.size() ? mCylindricalPreferences[nd] : false); 
  //};

 private:
  // Well, admittedly, this is forward-application-specific;
  t_3d_plane mPlane;

  // Yes, it looks like I want to know the nodes participating in this location; 
  // sometimes it is useful to loop through locations rather than through the KF
  // node pool;
  std::vector<TrKalmanNode*> mNodes; 

  // Previous/next location in "natural" direction of ascending Z;
  TrKalmanNodeLocation *mPrev, *mNext;

  // Can probably live without this set, as well as the other value below, 
  // but it allows to simplify logic in TrKalmanFilter::SetUpLocations();
  //unsigned mSensitiveVolumeNodeWrapperCount;
  std::set<SensitiveVolume*> mSensitiveVolumes;

  // FIXME: may want to put all this stuff into a separate structure;

  // Will store a mSensitiveVolumeNodeWrapperCount-long vector where nd-th value 
  // is a copy of respective KF template GetMdim(); 
  std::vector<unsigned> mDims;
  // And the same for 3D transformation from the respective KF nd-th node
  // to MARS; NB: it is taken from the very first node at this location (and it
  // is assumed, that transformations are "more or less the same"); THINK: some
  // misalignment-caused smearing?; THINK: I guess this should work for phi-sectors
  // as well (everything will be presentetd in the coordinate system of the very 
  // first sector, fine);
  std::vector<const TGeoHMatrix*> mNodeToMasters;

  std::vector<const EicKfNodeTemplate*> mDigiTemplates;
  //std::vector<bool> mCylindricalPreferences;
};

#endif
