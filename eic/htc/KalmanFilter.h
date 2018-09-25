//
// AYK (ayk@bnl.gov)
//
//    Kalman filter class routines; ported from HERMES/OLYMPUS sources; 
//  cleaned up 2014/10/10;
//

#include <map>

#include <KfMatrix.h>
#include <KalmanNode.h>

#ifndef _KALMAN_FILTER_
#define _KALMAN_FILTER_

// Possible bits in Transport() call "mode" flag; in particular 
// CPU-consuming derivatives are not always needed;
#define _CALCULATE_DERIVATIVES_   0x0001
#define _CALCULATE_PROCESS_NOISE_ 0x0002

// Possible failure bits in filter/smoother/chain return codes; NEVER change already 
// defined bits; if ever add new bits, check (and possibly update) _FATAL_FAILURE_MASK_;
#define _TFUN_FAILURE_         0x0001
#define _NFUN_FAILURE_         0x0002
#define _XFUN_FAILURE_         0x0004
#define _HFUN_FAILURE_         0x0008
#define _DSINV_FAILURE_        0x0010
#define _POSITIVITY_FIX_       0x0020
#define _CHAIN_FAILURE_        0x0040
// These 3 bits are at present not considered as fatal ones;
#define _NDF_FAILURE_          0x0080
#define _STRUST_FAILURE_       0x0100
#define _WORST_NODE_IMMUTABLE_ 0x0200

// Certain bits mean fatal error of the whole chain; other ones 
// are more or less harmless and (like NDF failure) just mean 
// that track is probably of a low general quality;
#define _FATAL_FAILURE_MASK_   0x005F

// Possible 'mode' argument values for KalmanChain(); see code
// for further details;
#define _TRUST_SMOOTHER_FCN_ 0x0001
#define _TRUST_FILTER_FCN_   0x0002

#define _KF_RETURN_(ret, message, what) \
  _RETURN_((ret), (mVerbosityLevel >= what ? message : 0))

//#define _KF_RETURN_ERROR_  (ret, message) _KF_RETURN_(ret,message,KalmanFilter::Error)
//#define _KF_RETURN_WARNING_(ret, message) _KF_RETURN_(ret,message,KalmanFilter::Warning)
//#define _KF_RETURN_INFO_   (ret, message) _KF_RETURN_(ret,message,KalmanFilter::Info)

// Looks ugly, but suffices to append extra bits in KalmanChain() return code;
#define _CHAIN_RETURN_(ret) return((ret)|mExtraReturnBits)

// Cases with higher off-diagonal correlation coefficients are 
// indeed very rare and in general suspicious;
#define _MAX_FIXABLE_POSITIVITY_SCREWUP_DEFAULT_  (1.20)
// Fix too high off-diagonal correlation coefficients to this value;
#define _POSITIVITY_CORRELATION_FIX_DEFAULT_      (0.99)

// Keep this stuff until find a better way to detect situations with 
// too small filtered residual covariance matrix RF[][]; 
#define _RF_CUTOFF_DEFAULT_                      (1E-11)

#define _FIRED_RPLANE_PREFIX_ "fired-node-min="

// If worst hit has probability below this value (and depending on 
// kalman_chain() "mode") declare hit as outlier;
// NB: 1-dim chi^2 tail integrals:
//   >=1 sigma from 0:  0.3173
//   >=2 sigma from 0:  0.0455 
//   >=3 sigma from 0:  0.0027 
//   >=4 sigma from 0: <0.0001 
#define _MIN_SMOOTHER_CHI_SQUARE_CCDF_DEFAULT_ (0.001)

// If probability of overall filter chi^2 is below this value 
// (and depending on kalman_chain() "mode") try to remove yet 
// another node;
#define _MIN_FILTER_CHI_SQUARE_CCDF_DEFAULT_   (0.003)

class KalmanFilter {
 public:
  // The constructor & destructor; the latter one should be done properly 
  // at some point; in fact however I'm always using execution-long class, 
  // so it automatically gets cleaned up once application exits;
  KalmanFilter(int sdim);
  ~KalmanFilter() {};

  // This does not really help, since I'd like to use them as loop variables;
  // by all means never change: must be Forward=0 and Backward=1 therefore;
  enum Direction {Undefined = -1, Forward, Backward};
  enum Verbosity {Never, Error, Warning, Info};

  void SetVerbosity(Verbosity verb)              { mVerbosityLevel = verb; };

  void SetXmCalculationFlag(bool flag)           { mXmCalculationFlag = flag; };

  // FIXME: seems to not be used in magnet-off mode;
  void SetNodeGapMax              (double value) { mNodeGapMax               = value; };

  void SetMinFilterChiSquareCCDF  (double value) { mMinFilterChiSquareCCDF   = value; };
  double GetMinFilterChiSquareCCDF()       const { return mMinFilterChiSquareCCDF; };
  void SetMinSmootherChiSquareCCDF(double value) { mMinSmootherChiSquareCCDF = value; };

  void SetPositivityFixParameters(double maxFixablePositivityScrewup, 
				  double positivityCorrelationFix) {
    mMaxFixablePositivityScrewup = maxFixablePositivityScrewup;
    mPositivityCorrelationFix    = positivityCorrelationFix;
  };

  void SetRFCutoffValue(double value)  { mRFCutoffValue = value; };

  // Default way to allocate new nodes and simplified wrapper; 
  KalmanNode *AddNode(const char *name, double z, int mdim, 
		      const bool nonLinearTransportFlags[2]);
  KalmanNode *AddNodeWrapper(const char *name, const char *format, double z, int mdim);

  int Configure(const StringList *config/*, const char *format*/);

  unsigned FilterPass(KalmanNode *start, KalmanNode *end, KalmanFilter::Direction fb);
  unsigned FullChain (KalmanNode *start, KalmanNode *end, KalmanFilter::Direction fb, int mode);

  virtual void BuildNodeList();
  void ResetFiredFlags();
  void HackGroupHitCountLimit(unsigned min);
  void LatchGroupNdfControlFlags();

  //unsigned GetNodeCount()         const { return mNodeCount; };

  KalmanNode *GetHead()           const { return mHead; };
  KalmanNode *GetTail()           const { return mTail; };

  int    GetFilterNdf()           const { return mNdf; };
  double GetFilterChiSquare()     const { return mFilterChiSquare; };
  double GetFilterChiSquareCCDF() const { return mFilterChiSquareCCDF; };

#if _OLD_
  // It looks like users should have no access to this routine (call FullChain() instead);
#endif
  int SmootherPass();

  void SetExtraNdfCount(int count)  { mExtraNdfCount = count; };

 protected:
  std::multimap<double, KalmanNode*> mKalmanNodePool;

  // NB: there is a certain trick involved here -> see respective TrKalmanFilter call;
  virtual KalmanNode *AllocateNode() { return new KalmanNode();};

  virtual bool NeedNonLinearTransport(double z) const { return true; };

 private:
  //
  //  Generic variables;
  //

  // State vector dimension: 4 (no field) or 5 (with magnetic field); 
  int sDim;

  // 0 - so Verbosity::Never - per default (no printouts);
  Verbosity mVerbosityLevel;

  // Usually don't need xm[] vector calculation; FIXME: there are other parts
  // which not always needed -> optimize and use similar flags;
  bool mXmCalculationFlag;

  // Several node groups can be declared; smoother should take 
  // care to guarantee that when removing outliers min number of
  // nodes in each group is granted;
  NodeGroup *mNodeGroups;
  // Total number of nodes; useless variable in fact;
  //unsigned mNodeCount;

  // Max allowed gap between two neighboring Kalman nodes; extra nodes
  // are added automatically (CHECK!) during initialization stage (if needed);
  double mNodeGapMax;

  // Configuration variables which define filter-smoother chain behavior;
  double mMinSmootherChiSquareCCDF, mMinFilterChiSquareCCDF;

  //
  // Buffer matrices; SU[][] will be a unity matrix of respective dimension; 
  //

  KfMatrix *SMTX;        //[SDIM][SDIM]
  KfVector *SVEC, *QVEC; //[SDIM]
  KfMatrix *SU;          //[SDIM][SDIM]

  // Well, it would probably make sense to use C++ STL container class here;
  // however the overhead to port 2-way navigation may be too big; in particular
  // will have to replace KalmanNode pointers by set (or list) iterators;
  // so leave things as they are?;
  KalmanNode *mHead, *mTail;

  // Well, say a strict cov.matrix setting at the head node can be seen as 
  // an extra measurement; clearly the core KF code can not judge on that; 
  // as of 2015/11/05 this value will actually be added to mNdf (and therefore
  // accounted not only in CCDF calculation, but in GetFilterNdf() call);
  int mExtraNdfCount;

  // Parameters which guide KF core code how to handle symmetric matrix 
  // numeric instabilities; FIXME: need to look into this more carefully;
  bool mPositivityCheck, mForceSymmetrization;
  double mMaxFixablePositivityScrewup;
  double mPositivityCorrelationFix;
  double mRFCutoffValue;

  //
  // Filter working variables;
  //

  // Make smoother aware about what was the last successful filter pass direction;
  KalmanFilter::Direction mLastFilterPass;
  // Also save last working filter conditions (for the smoother); 
  KalmanNode *mLastStart, *mLastEnd;

  // Number of degrees of freedom for the current filter pass; NB: may indeed 
  // vary from event to event depending on the actual number of hits used;
  int mNdf;
  // chi^2 and complement of its cumulative distribution function for
  // the given number of degrees of freedom; one value for the whole chain;
  double mFilterChiSquare, mFilterChiSquareCCDF;

  //
  // Smoother working variables;
  //

  // Node with the worst smoother chi^2 after the last smoother pass;
  KalmanNode *mWorstSmootherNode;
  // Same, but either not coupled to any node group or belonging 
  // to a node group where mFiredNodeNumMin is not reached yet;
  KalmanNode *mWorstResettableSmootherNode;

  //
  // Complete chain working variables;
  //

  // Number of nodes rejected during filter-smoother pass;
  int mChainRejectedNodeNum;

  // If for instance smoother had a recoverable trouble, it is
  // convenient to record it in some extra bitmask instead of returning
  // non-zero code and keep track on it in KalmanChain(); these bits will 
  // be OR'ed with the rest of Kalman chain return code;
  unsigned mExtraReturnBits;

  //
  // Private methods;
  //

  // Service matrix calculation for the current filter pass;
  unsigned Calculate_x0_FR_Q(KalmanNode *start, KalmanNode *end, 
			     KalmanFilter::Direction fb, unsigned mode);

  // Core KF calculations;
  unsigned DoFilterAlgebra(KalmanNode *start, KalmanNode *end, KalmanFilter::Direction fb);

  // chi^2, CCDF's, etc after the last pass;
  int CalculateFilterStat();

  // H-projector matrix calculation; it would be more natural to put these 
  // functions as virtual into KalmanNode, but it does not work; think later;
  virtual int CalculateHMatrix(KalmanNode *node) = 0;

  // Main transport routine; should be provided by user;
  virtual int Transport(KalmanNode *from, KalmanFilter::Direction fb, unsigned mode) = 0;

  // Extra transport code (process noise and dE/dx losses calculation 
  // in case of tracking); may be empty I guess?;
  virtual int TransportExtra(KalmanNode *from, KalmanFilter::Direction fb, unsigned mode) { 
    return 0; 
  };
};

#endif
