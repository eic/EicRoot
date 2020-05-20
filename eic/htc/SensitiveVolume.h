//
// AYK (ayk@bnl.gov), 2014/09/19
//
//  Re-mastered RegisteringPlane class;
//

#include <TGeoShape.h>
#include <TGeoNode.h>
#include <TGeoBBox.h>

#include <3d.h>
#include <EicHtcTask.h>

#include <EicTrackingDigiHit.h>
#include <EicTrackingDigiHitProducer.h>

#ifndef _SENSITIVE_VOLUME_
#define _SENSITIVE_VOLUME_

class FwdHoughNodeGroup;

class KalmanNodeWrapper {
 public:
 KalmanNodeWrapper(KalmanNode *node, EicKfNodeTemplate *kftmpl, TGeoMatrix *sv2master): mKFtmpl(kftmpl) { 
    mKfNodes.push_back(node);

    // Calculate complete 3D transformation from KF node to MARS; THINK: why the order
    // of 3D transformations should be reversed to work properly?!;
#if 1
    //{
      //TGeoMatrix *m1 = new TGeoMatrix(), m2;
    //}
    //assert(0);
    //mNodeToMaster = new TGeoHMatrix(kftmpl->mNodeToSensitiveVolume ? 
    //				    (*sv2master) * (*kftmpl->mNodeToSensitiveVolume) : 
    //				    *sv2master);
    //mNodeToMaster = new TGeoHMatrix(kftmpl->mNodeToSensitiveVolume ? 
    //				    (*(new TGeoHMatrix(*sv2master))) * (*kftmpl->mNodeToSensitiveVolume) : 
    //				    *sv2master);
    //if (kftmpl->mNodeToSensitiveVolume)
    //mNodeToMaster = new TGeoHMatrix(TGeoHMatrix(*sv2master) * (*kftmpl->mNodeToSensitiveVolume));
    //else
    //mNodeToMaster = new TGeoHMatrix(*sv2master);

    mNodeToMaster = kftmpl->mNodeToSensitiveVolume ? 
      new TGeoHMatrix(TGeoHMatrix(*sv2master) * (*kftmpl->mNodeToSensitiveVolume)) :
      new TGeoHMatrix(*sv2master);
    //else
      

    //(*sv2master) * (*kftmpl->mNodeToSensitiveVolume) : 
    //				    *sv2master);
#else
    mNodeToMaster = new TGeoHMatrix(kftmpl->mNodeToSensitiveVolume ? 
    				    (*kftmpl->mNodeToSensitiveVolume) * (*sv2master) : 
    				    *sv2master);
#endif

    // Calculate KF template origin in MARS;
    TVector3 xnode(0,0,0);
    mOrigin = LocalToMaster(mNodeToMaster, xnode);
    
    // Calculate basis vectors in MARS;
    TVector3 nxnode(1,0,0), nznode(0,0,1);
    mBasis[0] = LocalToMasterVect(mNodeToMaster, nxnode);
    mBasis[2] = LocalToMasterVect(mNodeToMaster, nznode);
    mBasis[1] = mBasis[2].Cross(mBasis[0]);

    mNodeGroup = 0;
  };
  ~KalmanNodeWrapper() {};

  EicKfNodeTemplate *GetKfNodeTemplate() const { return mKFtmpl; };

  KalmanNode *GetKfNode(unsigned id) const { return (id < mKfNodes.size() ? mKfNodes[id] : 0); }

  void AllocateNewKfNode(HtcKalmanFilter *kf, SensitiveVolume *sv) {
    assert(mKfNodes.size());

    char name[1024];
    TrKalmanNode *node =  dynamic_cast<TrKalmanNode*>(mKfNodes[0]);

    snprintf(name, 1024-1, "%s-%03d", node->GetName(), (unsigned)mKfNodes.size());
    TrKalmanNode *clone = 
      dynamic_cast<TrKalmanNode*>(kf->AddNodeWrapper(name, NULL, node->GetZ(), node->GetMdim()));
    clone->SetSensitiveVolume(sv);

    // FIXME: all this stuff should be encapsulated in KF node class;
    clone->SetActiveFlag(false);
    clone->SetLocation(node->GetLocation());
    clone->CopyOverGroupInfo(node);
    node->GetLocation()->AddNode(clone);

    mKfNodes.push_back(clone);
  };

  const TVector3& GetOrigin()                const { return mOrigin; };
  const TVector3 *GetAxis(unsigned iq)       const { return (iq < 3 ? mBasis + iq : 0); };
  double GetAxisComponent(unsigned iq, unsigned xyz) const { 
    return (iq < 3 && xyz < 3 ? mBasis[iq][xyz] : 0.0); };
  const TGeoHMatrix *GetNodeToMasterMtx()    const { return mNodeToMaster; };

  void SetNodeGroup(FwdHoughNodeGroup *ngroup)     { mNodeGroup = ngroup; };
  FwdHoughNodeGroup *GetNodeGroup()          const { return mNodeGroup; };

  unsigned GetMdim()                         const { return mKFtmpl->GetMdim(); };

 private:
  EicKfNodeTemplate *mKFtmpl;

  FwdHoughNodeGroup *mNodeGroup;

  // A vector here to be able to accomodate several hits independently;
  std::vector<KalmanNode*> mKfNodes;

  TGeoHMatrix *mNodeToMaster;  // transformation from KF node directly to MARS

  TVector3 mOrigin, mBasis[3]; // KF node origin and 3 basis vectore in MARS system         
};

class SensitiveVolume {
  // FIXME: pleeease!;
  friend class EicKfNodeTemplate;
  friend class EicHtcTask;

 public:
 SensitiveVolume(LogicalVolumeLookupTableEntry *lNode, const TGeoNode *node, double z0): 
  mLogicalNode(lNode) /*mXmin(0.0), mXmax(0.0), mYmin(0.0), mYmax(0.0)*/ { 
    // Calculate bounding box in XY projection in MARS; FIXME: this will work only 
    // if KF axis is a MARS Z-axis; 
    TGeoVolume *volume = node->GetVolume();
    TGeoShape *shape = volume->GetShape();
    //shape->ComputeBBox();

    TGeoBBox *box = dynamic_cast<TGeoBBox*>(shape);
    
    // NB: do NOT move bounding box corners to MARS; this will be done in FwdTrackFinder::Init()
    // taking back transformation to KF node template coord.system;
    mXmin = -box->GetDX();
    mXmax =  box->GetDX();
    mYmin = -box->GetDY();
    mYmax =  box->GetDY();
  };
  ~SensitiveVolume() {};

  int TrackToHitDistance(t_3d_line *line, EicTrackingDigiHit *hit, double qdist[]);

  unsigned GetKfNodeWrapperCount() const { return mKfNodeWrappers.size(); };
  KalmanNodeWrapper *GetKfNodeWrapper(unsigned id) { 
    return (id < mKfNodeWrappers.size() ? &mKfNodeWrappers[id] : 0);
  };

  void SetNodeGroup(unsigned wr, FwdHoughNodeGroup *ngroup) {
    if (wr < mKfNodeWrappers.size()) mKfNodeWrappers[wr].SetNodeGroup(ngroup);
  };
  FwdHoughNodeGroup *GetNodeGroup(unsigned wr) {
    return (wr < mKfNodeWrappers.size() ? mKfNodeWrappers[wr].GetNodeGroup() : 0);
  };

  // No reason to make arrays, etc?;
  double GetXmin() const { return mXmin; };
  double GetXmax() const { return mXmax; };
  double GetYmin() const { return mYmin; };
  double GetYmax() const { return mYmax; };

  const LogicalVolumeLookupTableEntry *GetLogicalNode() const { return mLogicalNode; };

 private:
  // Kalman filter wrapper nodes associated with this sensitive volume;
  std::vector<KalmanNodeWrapper> mKfNodeWrappers;

  //  std::map<unsigned, EicKfNodeTemplateOrth2D::CoordinateModel> mCoordinateModels2D;

  // Bounding box in MARS coordinate system;
  double mXmin, mXmax, mYmin, mYmax;

  LogicalVolumeLookupTableEntry *mLogicalNode;
};

#endif
