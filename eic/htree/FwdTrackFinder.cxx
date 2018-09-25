
#include <cassert>

#include <FairPrimaryGenerator.h>
#include <FairBaseParSet.h>

#include <ayk.h>
#include <3d.h>
#include <htclib.h>

#include <MediaBank.h>
#include <FwdTrackFinder.h>
#include <EicTrackingDigiHit.h>
#include <EicRunAna.h>

// ---------------------------------------------------------------------------------------

MediaBank *FwdTrackFinder::ConfigureMediaBank()
{
  // No length limit; KF axis off (0,0,Z-vtx) along (0,0,1); so assume head node
  // to sit right in Z-vertex as given in event generator;
  t_3d_line zaxis(TVector3(0.0, 0.0, mEicBoxGenerator->GetVz()), TVector3(0.0, 0.0, 1.0)); 
  MediaBank *mbank = new MediaBank(zaxis, 0.0);

  // Also start off (0,0,Z-vtx); FIXME: perhaps unify with HTC routine?;
  t_3d_line saxis(TVector3(0.0, 0.0, mEicBoxGenerator->GetVz()), GetMediaScanDirection());
  mbank->SetScanLine(saxis);

  return mbank;
} // FwdTrackFinder::ConfigureMediaBank()

// ---------------------------------------------------------------------------------------

// FIXME: unify these calls;

int FwdTrackFinder::DefinePhiRange(double min, double max, double gra)
{
  if (mPhiId != -1 || min > max) return -1;

  mPhiId = mPhaseSpaceVariables.size();
  mPhaseSpaceVariables.push_back(PhaseSpaceVariable(min, max, gra));

  //mPhiId = mCurrentId++;
  mFwdHoughTree->AddDimension("FI", min, max);

  return 0;
} // FwdTrackFinder::DefinePhiRange()

int FwdTrackFinder::DefineInversedMomentumRange(double min, double max, double gra)
{
  if (mInvMomentumId != -1 || mInvPtId != -1 || min > max) return -1;

  mInvMomentumId = mPhaseSpaceVariables.size();
  mPhaseSpaceVariables.push_back(PhaseSpaceVariable(min, max, gra));

  //mInvMomentumId = mCurrentId++;
  mFwdHoughTree->AddDimension("PP", min, max);

  return 0;
} // FwdTrackFinder::DefineInversedMomentumRange()

int FwdTrackFinder::DefineInversedPtRange(double min, double max, double gra)
{
  if (mInvMomentumId != -1 || mInvPtId != -1 || min > max) return -1;

  mInvPtId = mPhaseSpaceVariables.size();
  mPhaseSpaceVariables.push_back(PhaseSpaceVariable(min, max, gra));

  //mInvPtId = mCurrentId++;
  mFwdHoughTree->AddDimension("Pt", min, max);

  return 0;
} // FwdTrackFinder::DefineInversedPtRange()

int FwdTrackFinder::DefineThetaRange(double min, double max, double gra)
{
  if (mThetaId != -1 || min > max) return -1;

  mThetaId = mPhaseSpaceVariables.size();//mCurrentId++;
  mPhaseSpaceVariables.push_back(PhaseSpaceVariable(min, max, gra));

  //mThetaId = mPhaseSpaceVariables.size();//mCurrentId++;
  mFwdHoughTree->AddDimension("TH", min, max);

  return 0;
} // FwdTrackFinder::DefineThetaRange()

// ---------------------------------------------------------------------------------------

int FwdTrackFinder::ConfigureResolutionLevels(/*const PhaseSpaceVariable *psvar*/unsigned id)
{
  const PhaseSpaceVariable *psvar = &mPhaseSpaceVariables[id];

  double range = psvar->GetRange(), gra = psvar->GetGra();
  unsigned div[3] = {1,  1,  1};

  div[id] = 2;

  for( ; ; ) {
    if (range < gra) break;

    if (mFwdHoughTree->AddResolutionLevel(div)) return kERROR;

    range /= 2;
  } //for inf

  return 0;
} // FwdTrackFinder::ConfigureResolutionLevels()

// ---------------------------------------------------------------------------------------

InitStatus FwdTrackFinder::Init()
{
  FairRootManager *fManager = FairRootManager::Instance();

  // Get access to the box generator parameters; FIXME: assert() calls;
  {
    FairBaseParSet *bpset;

    fManager->GetInFile()->GetObject("FairBaseParSet", bpset);
    assert(bpset);

    FairPrimaryGenerator *pgen = bpset->GetPriGen();
    TObjArray *fGenList = pgen->GetListOfGenerators();
    TIterator *fListIter = fGenList->MakeIterator();

    {
      TObject *obj;
      unsigned gCounter = 0;
      
      while( (obj = fListIter->Next()) ) {
	gCounter++;
	mEicBoxGenerator = dynamic_cast<EicBoxGenerator*> (obj);
      } //while
      
      assert(gCounter == 1 && mEicBoxGenerator);
    }
    
    //printf("--> %f\n", mEicBoxGenerator->GetVxSmearing()); exit(0);
  }

  // Allocate fake IP Kalman filter node;
  {
    // FIXME: will not work in magnetic field;
    bool nonLinFlags[2] = {WithMagneticField(), WithMagneticField()};

    mVtxKfNode = dynamic_cast<TrKalmanNode*>
      (GetKalmanFilter()->AddNode("VTX", mEicBoxGenerator->GetVz(), 2, nonLinFlags/*, 0*/));
  }
  //printf("%f\n", mVtxKfNode->GetZ()); exit(0);

  if (EicHtcTask::Init()) return kERROR;
  //exit(0);

  //
  // NB: by this point FwdHoughTree is allocated already and AddDimension() 
  // are presumably called from reconstruction.C script;
  //

  assert(GetThetaId() != -1 && GetPhiId() != -1);
  {
    assert(mPhaseSpaceVariables.size() <= 3);

    // Start with phi division pattern;
    if (ConfigureResolutionLevels(GetPhiId()))             return kERROR;

    if (WithMagneticField() && 
	ConfigureResolutionLevels(GetMomentumRelatedId())) return kERROR;
    if (ConfigureResolutionLevels(GetThetaId()))           return kERROR;
  }

  // Guess on zero-approximation slope and inv.p(pt) cov.matrix; assume user 
  // sort of knows, that it does not make sense to push resolution beyond the 
  // "natural" limit; so just take highest resolution cell size as an estimate;
  // NB: this value will be blown up in the preliminary KF pass anyway and it 
  // will be taken from CS[][] at vtx node for th efinal pass (and again blown up);
  {
    ResolutionLevel *high = mFwdHoughTree->GetLevel(mFwdHoughTree->GetLdim()-1);

    // Assume theta cell width is directly related to {sx,sy}; 
    double thetaSize = RADIANS(high->GetCellSize(GetThetaId()));
    // Here consider larger value; tan() or sin()? well, does not matter I guess;
    double phiSize   = RADIANS(high->GetCellSize(GetPhiId()))*
      tan(RADIANS(mPhaseSpaceVariables[GetThetaId()].GetMax()));
    //printf("%f %f\n", thetaSize, phiSize);

    mAngularCovMtxEstimate = thetaSize > phiSize ? SQR(thetaSize) : SQR(phiSize);

    if (WithMagneticField()) {
      // NB: inversed units here (either invp or invpt);
      double pSize = high->GetCellSize(GetMomentumRelatedId());

      // In this case have to rescale; take larger angle;
      if (GetInvPtId() != -1) 
	pSize *= sin(RADIANS(mPhaseSpaceVariables[GetThetaId()].GetMax()));
      //printf("%f\n", pSize);

      mInversedMomentumRelatedCovMtxEstimate = SQR(pSize);
    } //if
    //exit(0);
  }

  assert(!mFwdHoughTree->AllocateLookUpTable());

    // Loop through all locations; pick up those which have sensitive volumes defined;
  for(TrKalmanNodeLocation *location=GetKalmanFilter()->GetLocationHead(); location;
      location=location->GetNext(KalmanFilter::Forward)) {

    if (!location->HasSensitiveVolumes()) continue;
    printf("%7.2f\n", location->GetZ());

    // NB: clearly Hough node groups will have different min/max limits depending 
    // on local template orientations inside respective sensitive volume;
    for(unsigned tmpl=0; tmpl<location->GetSensitiveVolumeNodeWrapperCount(); tmpl++) {
      // Templates will choose min[]/max[] definitions out of the suggested options and 
      // should distinguish between XY- and RF-cases; assume rmin=0 is fine as well as 
      // [0..2pi] for angular range;
      std::set<double> xMin, yMin, xMax, yMax, rMin, rMax;
      
      //printf("  %2d -> cyl=%d\n", tmpl, location->GetCylindricalPreference(tmpl));
      
      for(unsigned kfnd=0; kfnd<location->GetNodeCount(); kfnd++) {
	TrKalmanNode *node = location->GetNode(kfnd);
	
	SensitiveVolume *sv = node->GetSensitiveVolume();
	if (!sv) continue;
	
	//printf("   %s -> xmin=%7.2f, xmax =%7.2f, ymin=%7.2f, ymax=%7.2f\n", 
	//	 node->GetName(), sv->GetXmin(), sv->GetXmax(), sv->GetYmin(), sv->GetYmax());
	
	double xmin = sv->GetXmin(), xmax =  sv->GetXmax(), ymin = sv->GetYmin(), ymax =  sv->GetYmax();
	TVector3 vvLc[4] = {TVector3(xmin, ymin, 0.0), TVector3(xmin, ymax, 0.0), 
			    TVector3(xmax, ymax, 0.0), TVector3(xmax, ymin, 0.0)};
	
	for(unsigned iq=0; iq<4; iq++) {
	  TVector3 vvGl = LocalToMaster(sv->GetLogicalNode()->mGeoMtx, vvLc[iq]);
	  TVector3 vvNd = MasterToLocal(location->GetNodeToMaster(tmpl), vvGl);
	  double xx = vvNd[0], yy = vvNd[1];
	  
	  xMin.insert(xx);
	  xMax.insert(xx);
	  yMin.insert(yy);
	  yMax.insert(yy);
	  
	  rMax.insert(sqrt(xx*xx+yy*yy));
	} //for iq
      } //for kfnd
      
	// Don't want to do more here; give {location,templates} combination all 
      // the info concerning min/max values and let it decide how to configure itself;
      FwdHoughNodeGroup *ngroup = 
	mFwdHoughTree->AddNodeGroup(location, tmpl, mCylindricalCoordPreference, 
				    xMin, xMax, yMin, yMax, rMin, rMax);
      if (!ngroup) return kERROR;
      
      // Loop through all sensitive volumes associated with this location and assign 
      // back-door node group pointers;
      for(std::set<SensitiveVolume*>::iterator it=location->GetSensitiveVolumes().begin(); 
	  it != location->GetSensitiveVolumes().end(); it++) 
	(*it)->SetNodeGroup(tmpl, ngroup);
    } //for tmpl
  } //for location
  //exit(0);

  // The rest of configuration parameters; unless specifically booked in 
  // reconstruction.C script, default values will be used; 
  {
    assert(!mFwdHoughTree->SetBlindCellDecisionLevel(mBlindCellDecisionLevel));

    unsigned maxHitCount = GetMaxPossibleHitCount();
    assert(!mFwdHoughTree->SetOkHitCounterLimits(maxHitCount-mMissingHitCounterMax, maxHitCount));

    assert(!mFwdHoughTree->SetBorrowedHitCounterLimit(mBorrowedHitCounterMax));
    assert(!mFwdHoughTree->SetBorrowedPlusMissingHitCounterLimit(mBorrowedPlusMissingHitCounterMax));
  }

  FairRootManager::Instance()->Register("FwdTrack", "FwdTrackFinder", mTracks, kTRUE);

  return kSUCCESS;
} // FwdTrackFinder::Init()

// ---------------------------------------------------------------------------------------

// THINK: do I need these to be configurable?;
#define _COVARIANCE_FIRST_BLOWUP_CFF_        (1000)
#define _COVARIANCE_FINAL_BLOWUP_CFF_        (1000)

void FwdTrackFinder::ResetVtxNode(MatchCandidate *match)
{
  HtcKalmanFilter *kf = GetKalmanFilter();
  // FIXME: find a better way to figure this out?;
  unsigned sdim = kf->GetFieldMode() == NoField ? 4 : 5;
  
    // Figure out 0-th track approximation based on the track finder guess;
    // NB: I guess tricks like building linear track approximation will not
    // improve angular part and will be hopeless for momentum determination 
    // anyway -> stick to this procedure; NB: assume FwdHoughTree knows 
    // parameter meaning better that FwdTrackFinder (after all, MappingCall()
    // is in this class) -> decrypt [theta,phi,1/p(t)] triplet first;
  double par[mFwdHoughTree->GetDdim()];

  // FIXME: may want to pack this into a separate call later;
  ResolutionLevel *high = mFwdHoughTree->GetLevel(mFwdHoughTree->GetLdim()-1);
  const unsigned *id = match->GetIdPtr();
  for(unsigned iq=0; iq<mFwdHoughTree->GetDdim(); iq++) {
    const HoughDimension *dimension = mFwdHoughTree->GetDimension(iq);
    
    par[iq] = dimension->GetMin() + high->GetCellSize(iq)*(id[iq] + 0.5);
  } //for iq	
  
  double rtheta = RADIANS(par[GetThetaId()]), rphi = RADIANS(par[GetPhiId()]);
  //printf("%f %f\n", par[0], par[1]); exit(0);
  if (sdim == 5 && GetMomentumRelatedId() != -1) {
    double pvar = par[GetMomentumRelatedId()], p, pt;

    assert(pvar);
    if (GetInvMomentumId() != -1) 
      p = 1./pvar;
    else
      p = 1.0/(sin(rtheta)*pvar);

    //printf("--> %f\n", p); fflush(stdout);
    mVtxKfNode->SetMomentum(p);
  } else {
    assert(mParticleMomentumSeed);
    mVtxKfNode->SetMomentum(mParticleMomentumSeed);
  } //if
  
  // KFV() and KFM() encapsulated operations are used -> just get pointers and use them;
  {
    KfVector *x0 = mVtxKfNode->GetX0();

    // Use generated info here (see simulation.C);
    x0->KFV(0) = mEicBoxGenerator->GetVx();
    x0->KFV(1) = mEicBoxGenerator->GetVy();

    double nn[3] = {sin(rtheta)*cos(rphi), sin(rtheta)*sin(rphi), cos(rtheta)};
    
    assert(nn[2]);
    x0->KFV(2) = nn[0]/nn[2];
    x0->KFV(3) = nn[1]/nn[2];
  }

  // Yes, predicted state vector at start of chain is set to 0.0;
  {
    KfVector *x0 = mVtxKfNode->GetX0(), *xp = mVtxKfNode->GetXp();

    for(int ip=_X_; ip<sdim; ip++)
      xp->KFV(ip) = 0.;
  }

  // Cook dummy (diagonal) covariance matrix;
  {
    KfMatrix *CP = mVtxKfNode->GetCP();

    CP->Reset();
    // NB: this constraint comes in sync with the generator setting in simulation.C;
    CP->KFM(0,0) = SQR(mEicBoxGenerator->GetVxSmearing());
    CP->KFM(1,1) = SQR(mEicBoxGenerator->GetVySmearing());
    
    // Just [2..3] components (slopes) here;
    for(int ip=_SX_; ip<=_SY_; ip++)
      CP->KFM(ip,ip) = mAngularCovMtxEstimate * _COVARIANCE_FIRST_BLOWUP_CFF_;
    
    // Momentum component;
    if (sdim == 5)
      CP->KFM(_QP_,_QP_) = mInversedMomentumRelatedCovMtxEstimate * _COVARIANCE_FIRST_BLOWUP_CFF_;
  }
} // FwdTrackFinder::ResetVtxNode()

void FwdTrackFinder::UpdateVtxNode()
{
  HtcKalmanFilter *kf = GetKalmanFilter();
  // FIXME: find a better way to figure this out?;
  unsigned sdim = kf->GetFieldMode() == NoField ? 4 : 5;

  {
    // KFV() and KFM() encapsulated operations are used -> just get pointers and use them;
    KfVector *x0 = mVtxKfNode->GetX0();

    // NB: also use generated info here (see simulation.C); indeed this stuff should 
    // be consistent (use both coordinates and cov.matrix estimate for XY-vertex); 
    x0->KFV(0) = mEicBoxGenerator->GetVx();
    x0->KFV(1) = mEicBoxGenerator->GetVy();

    // Otherwise a normal Kalman filter iterative update for slopes;
    for(int ip=_SX_; ip<=_SY_; ip++)
      x0->KFV(ip) += mVtxKfNode->GetXs()->KFV(ip);
  }

  // Momentum expansion point; NB: head->x0[_QP_] will remain 0. in all cases!;
  if (sdim == 5) mVtxKfNode->UpdateInversedMomentum(mVtxKfNode->GetXs()->KFV(_QP_));

  // Yes, predicted state vector at start of chain is set to 0.0;
  {
    KfVector *xp = mVtxKfNode->GetXp();

    for(int ip=_X_; ip<sdim; ip++)
      xp->KFV(ip) = 0.;
  } 

  // Cook dummy (diagonal) covariance matrix;
  {
    KfMatrix *CP = mVtxKfNode->GetCP(), *CS = mVtxKfNode->GetCS();

    CP->Reset();
    // NB: this constraint will be here for the final pass as well;
    CP->KFM(0,0) = SQR(mEicBoxGenerator->GetVxSmearing());
    CP->KFM(1,1) = SQR(mEicBoxGenerator->GetVySmearing());
    
    // Here assume, that CS[][] at the head node was estimated good enough 
    // in the last smoother pass -> use it as a reference and just blow up; 
    for(int ip=_SX_; ip<=(sdim == 4 ? _SY_ : _QP_); ip++)
      CP->KFM(ip,ip) = CS->KFM(ip,ip)*_COVARIANCE_FINAL_BLOWUP_CFF_;
  }
} // FwdTrackFinder::UpdateVtxNode()

// ---------------------------------------------------------------------------------------

void FwdTrackFinder::Exec(Option_t* opt)
{
  {
    static unsigned evCounter;
    printf("\n\n FwdTrackFinder::Exec() ... Ev#%03d\n", evCounter++);
  }
  //EicHtcTask::Exec(opt);
  //return;

  mTracks->Clear();

  // Reset internal node-group-related hit counters; 
  for(unsigned gr=0; gr<mFwdHoughTree->GetGroupCount(); gr++)
    mFwdHoughTree->GetGroup(gr)->ResetMemberCounter();

  // Populate node group hit arrays ("members"); 
  for(unsigned gr=0; gr<mIdealTrCode->fGroups.size(); gr++) {
    EicDetectorGroup *dgroup = &mIdealTrCode->fGroups[gr];

    if (!dgroup->_fHits) continue;
    unsigned hnum = dgroup->_fHits->GetEntriesFast();

    for(unsigned ih=0; ih<hnum; ih++) {
      EicTrackingDigiHit *hit = (EicTrackingDigiHit*)dgroup->_fHits->At(ih);
      //assert(hit);

      // Part of the sensitive volumes may be of no interest -> just skip;
      SensitiveVolume *sv = dgroup->GetSensitiveVolume(hit);
      if (!sv) continue;

      FwdHoughNodeGroup *ngroup = sv->GetNodeGroup(hit->GetKfNodeID());

      KalmanNodeWrapper *kfwrapper = sv->GetKfNodeWrapper(hit->GetKfNodeID());
      t_hough_range from = ngroup->PackFrom(hit, mRelativeHitSmearing, mAbsoluteSpatialSmearing, kfwrapper);
      t_hough_range   to = ngroup->PackTo  (hit, mRelativeHitSmearing, mAbsoluteSpatialSmearing, kfwrapper);
      
      // THINK: return code here (indicate some error)?; 
      assert(!ngroup->AddMember(std::pair<void *, void *>(dgroup, hit), from, to));
    } //for ih
  } //for gr

  //return;
  // Launch track finder;
  mFwdHoughTree->LaunchPatternFinder();

  //return;
  // Fill out output track arrays;
  for(unsigned tc=0; tc<mFwdHoughTree->GetLinearMatchCandidateCount(); tc++) {
    FwdMatchCandidate *match = 
      dynamic_cast<FwdMatchCandidate*>(mFwdHoughTree->GetMatchCandidate(tc));

    // Yes, I need only the good ones;
    if (!match->IsActive()) continue;

    // FIXME: this copying is not exactly efficient; at some point may want 
    // to unify HoughTree->mMatchCandidates and FwdTrackFinder->mTracks; the
    // issue however is the 'new []' calls in MatchCandidate constructor which 
    // I do not want to do every time new; 
    new ((*mTracks)[mTracks->GetEntriesFast()]) FwdMatchCandidate(*match);
  } //for tc
} // FwdTrackFinder::Exec()

// ---------------------------------------------------------------------------------------

void FwdTrackFinder::FinishTask()
{
  FairRun *fRun = FairRun::Instance();

  // I hope there is no need to save/restore current directory here?;
  fRun->GetOutputFile()->cd();
  ccdf->Write();

  FairTask::FinishTask();
} // FwdTrackFinder::FinishTask()

// ---------------------------------------------------------------------------------------

ClassImp(PhaseSpaceVariable)
ClassImp(ResolutionLevelPattern)
ClassImp(FwdTrackFinder)
