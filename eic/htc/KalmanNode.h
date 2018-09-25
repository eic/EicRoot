//
// AYK (ayk@bnl.gov)
//
//    Kalman filter node and associated structures; ported from 
//  HERMES/OLYMPUS sources; cleaned up 2014/10/13;
//

#include <cstring>
//#include <stdio.h>

#include <KfMatrix.h>

#ifndef _KALMAN_NODE_
#define _KALMAN_NODE_

class Mgrid;
class KalmanNode;

class StringList {
 public:
 StringList(): mString(0), mNextString(0) {};
  // FIXME: has never been checked;
  ~StringList() { /*if (mString) free(mString);*/ };

  char *mString;

  StringList *mNextString; 
};

class NodeList {
  friend class KalmanFilter;

public:
 NodeList(): mNode(0), mNextNode(0) {};
  ~NodeList() {};

 private:
  KalmanNode *mNode;

  NodeList *mNextNode; 
};

class NodeGroup {
  friend class KalmanNode;
  friend class KalmanFilter;

public:
  // Constructor should clean up "next" pointer; just reset everything;
 NodeGroup(): mFiredNodeNumMin(0), mPrefices(0), mAllNodeNum(0), mNodeList(0), 
    mFiredNodeNum(0), mNdfControlFlag(false), mNextGroup(0) {};
  ~NodeGroup() {};

 private:
  // Min number of fired registering planes during outlier removal;
  int mFiredNodeNumMin;

  // Linked list with node name prefices;
  StringList *mPrefices;

  // Linked list of nodes belonging to this group;
  int mAllNodeNum;
  NodeList *mNodeList;

  // Presently fired number of nodes (KF working variable); FIXME: multi-threading;
  int mFiredNodeNum;

  // If 'true', minimum fired node control is enabled;
  bool mNdfControlFlag;

  NodeGroup *mNextGroup;
};

class MgridSlice {
 public:
  // Constructor should clean up "next" pointer; just reset everything;
 MgridSlice(): mZ0(0.0), mGrid(0), mLastUnboundCallStatus(0), mNext(0) {};

  // Z coordinate;
  double mZ0;
  
  // XY-mgrid at a given z0; can be NULL (Z out of magnetic field area); 
  Mgrid *mGrid;

  // It is convenient to have a flag indicating whether last 
  // call to mgrid interpolation routine with repetition_flag=0
  // succeeded or not; indeed for derivative calculations makes 
  // no sense to call the routine with repetition_flag=1 at an offset 
  // with respect to one of the [0..4] parameters if a call at 
  // nominal parameter set failed;
  int mLastUnboundCallStatus;

  // Will exist as a linked list;
  MgridSlice *mNext;
};

class KalmanNode {
  friend class KalmanFilter;

 public:
  // Constructor: just reset everything; FIXME: what a crap!;
  KalmanNode() { 
    memset((void*)this, 0x00, sizeof(KalmanNode));
  };

  void SetFiredFlag();
  void ResetFiredFlag();

  virtual bool IsActive()                 const { return true; };
  bool IsFired()                          const { return mFired; };
  void SetZ(double z)                           { mZ = z; };
  virtual double GetZ()                   const { return mZ; };
  unsigned GetMdim()                      const { return mDim; }; 

  // Would need a range check here?; hmm;
  double  GetV(unsigned ip, unsigned iq)  const { return V->KFM(ip,iq); };
  double  GetX0(unsigned ip)              const { return x0->KFV(ip); };
  double  GetXf(unsigned ip)              const { return xf->KFV(ip); };
  double  GetXs(unsigned ip)              const { return xs->KFV(ip); };
  double  GetXm(unsigned ip)              const { return xm->KFV(ip); };
  double  GetRs(unsigned ip)              const { return rs->KFV(ip); };
  double  GetRm(unsigned ip)              const { return rm->KFV(ip); };
  double  Getm (unsigned ip)              const { return m->KFV (ip); };
  double  GetSmootherChiSquare()          const { return mSmootherChiSquare; };
  double  GetSmootherChiSquareCCDF()      const { return mSmootherChiSquareCCDF; };
  double  GetCP(unsigned ip, unsigned iq) const { return CP->KFM(ip,iq); };
  double  GetCF(unsigned ip, unsigned iq) const { return CF->KFM(ip,iq); };
  double  GetCS(unsigned ip, unsigned iq) const { return CS->KFM(ip,iq); };
  double  GetRS(unsigned ip, unsigned iq) const { return RS->KFM(ip,iq); };
  double  GetRM(unsigned ip, unsigned iq) const { return RM->KFM(ip,iq); };

  KfVector *GetX0()                       const { return x0; };
  KfVector *GetXf()                       const { return xf; };
  KfVector *GetXp()                       const { return xp; };
  KfVector *GetXs()                       const { return xs; };
  KfMatrix *GetCP()                       const { return CP; };
  KfMatrix *GetCS()                       const { return CS; };

  const char *GetName()                   const { return mName; };

  KalmanNode *GetNext(unsigned fb)        const { return (fb ? mPrev : mNext); };
  KalmanNode *GetPrev(unsigned fb)        const { return (fb ? mNext : mPrev); };

  void SetPrev(KalmanNode *node)                { mPrev = node; };
  void SetNext(KalmanNode *node)                { mNext = node; };

  void CopyOverGroupInfo(KalmanNode *sample) {
    // FIXME: well, this breaks mNodeGroups ownership model I guess;
    mNodeGroupNum = sample->mNodeGroupNum;
    mNodeGroups   = sample->mNodeGroups;
  };

 protected:
  // Node ordering parameter; Z-coordinate for application to charged particle 
  // tracking in forward spectometers; then just call it 'mZ';
  double mZ;

  // Measurement vector dimension;
  int mDim;

  // User back-door pointer;
  //void *mBackDoorPointer;

  //
  // Prefer not to prepend 'm' prefix (would really spoil readability);
  //

  // Non-stochastic part of the system equation matrix; 
  // assume that linear transport (magnet-free in case of tracking)
  // matrices never change => precalculate them and store for 
  // forward and backward filters separately; if this situation ever
  // changes, will have to create these FF's thread-specific;
  KfMatrix *FF[2];    //[SDIM][SDIM]

  // Keep 'usual' Kalman notation; 'measured' coordinate; 
  KfVector *m;        //[MDIM]

  // Measurement noise covariance (matrix); 
  KfMatrix *V;        //[MDIM][MDIM]

  // Non-stochastic part of the measurement matrix; 
  KfMatrix *H;        //[MDIM][SDIM]

  // Expansion point; used for the pure linear case as well, but makes              
  // real sense for (nonlinear) tracking in magnetic field; 
  KfVector *x0;       //[SDIM]

  // Process noise covariance matrix; 
  KfMatrix *Q;        //[SDIM][SDIM]

  // 'FR' will point to either FF or FM depending on what is actually wanted;              
  KfMatrix *FM;       //[SDIM][SDIM]
  KfMatrix *FR;

  // Buffer matrices;
  KfMatrix *MMTX;     //[MDIM][MDIM]
  KfVector *MVEC;     //[MDIM]

  // Predicted state vector based on the previous (k-1) nodes; 
  // in case of tracing in the magnetic field it will stay 0.; 
  KfVector *xp;       //[SDIM]
  // Filtered state vector based on all (k) nodes up to this one;          
  // in case of tracing in the magnetic field it is a deviation 
  // from x0[];
  KfVector *xf;       //[SDIM]

  // Smoothed state vector with this node measurement excluded;
  KfVector *xm;       //[SDIM]
  // Respective residuals
  KfVector *rm;       //[MDIM]

  // Predicted covariance matrix based on the previous (k-1) nodes; 
  KfMatrix *CP;       //[SDIM][SDIM]
  // Updated (filtered) covariance matrix based on all (k) nodes; 
  KfMatrix *CF;       //[SDIM][SDIM]

  // Kalman gain matrix;
  KfMatrix *K;        //[SDIM][MDIM]

  // Smoothed state vector based on all (n) nodes; 
  KfVector *xs;       //[SDIM]
  // Intermediate vector for xs[] calculation (see De Jong);
  KfVector *qq;       //[SDIM]

  // Prediction error, filtered residuals and their covariance matrix; 
  KfVector *ep, *rf;  //[MDIM]
  KfMatrix *RPI, *RF; //[MDIM][MDIM]

  // Updated (smoothed) covariance matrix; 
  KfMatrix *CS;       //[SDIM][SDIM]
  // Smoothed residuals and their covariance matrix; 
  KfVector *rs;       //[MDIM]
  KfMatrix *RS;       //[MDIM][MDIM]

  // Smoother dovariance matrix with current node measurement subtracted; 
  KfMatrix *CM;       //[SDIM][SDIM]
  // Respective matrix projected onto the node space (H*CM*H^T);
  KfMatrix *RM;       //[MDIM][MDIM]

  // LB=(I-KH) for this node; L=F*LF - true node->L needed
  // for CS[][] calculation (De Jong); QQ[] is an intermediate
  // matrix for the same purposes;
  KfMatrix *LB, *L;   //[SDIM][SDIM]
  KfMatrix *QQ;       //[SDIM][SDIM]

 private:
  // Well, it turns out that name helps; can be NULL though;
  char *mName;

  // Previous/next in "natural" direction of ascending Z;
  KalmanNode *mPrev, *mNext;

  // Well, not really tracking-specific parameter; NB: even if 
  // root->non_linear_transport_flag is 1 few nodes may require 
  // a pure linear transport (like BC3/4 in HTC case --> no 
  // magnetic field); as of 2010/08/17 this flag is direction-specific
  // for any given node (which actually makes sense only for complicated 
  // cases like bridging);
  bool mNonLinearTransportFlags[2];

  // Node groups this node belongs to;
  int mNodeGroupNum;
  NodeGroup **mNodeGroups;

  // If '1', this node actually has a measurement;  
  bool mFired;

  // chi^2 increment (filtering); 
  double mFilterChiSquareIncrement;

  // chi^2 for the smoothed residuals (one value per node); 
  double mSmootherChiSquare, mSmootherChiSquareCCDF;

  void AllocateKfMatrices(unsigned sdim);
} ;

#endif
