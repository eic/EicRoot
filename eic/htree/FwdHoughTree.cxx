//
// AYK (ayk@bnl.gov)
//
//  Hough transform track finder for STAR-specific forward tracker;
//
//  Initial port from OLYMPUS sources: Oct'2015;
//

#include <stdlib.h>

#include <Math/DistFunc.h>
#include <TMath.h>

#include <FairRunAna.h>

#include <htclib.h>

#include <FwdTrackFinder.h>
#include <FwdHoughTree.h>

// ---------------------------------------------------------------------------------------

double FwdHoughTree::GetBzAtIP()
{
  // May later introduce a boolean variable; for now just fall out if 
  // field is not available;
  if (!mBzAtIP) {
    FairRunAna *fRun = FairRunAna::Instance();
    FairField *field = fRun->GetField();
    assert(field);

    // Yes, reset b[] as well;
    double xx[3] = {0.0, 0.0, 0.0}, b[3] = { 0.0, 0.0, 0.0};
    field->Field(xx, b);

    // Yes, store in [kGs] as field->Field() returns;
    mBzAtIP = b[2];
    assert(mBzAtIP);
#if 0
    //printf("%f\n", mBzAtIP); exit(0);
    {
      xx[0] = 20.0;
      for(unsigned iq=0; iq<300; iq++) {
	xx[2] = iq*1.0;

	field->Field(xx, b);
	printf("%3d -> %7.2f %7.2f %7.2f\n", iq, b[0], b[1], b[2]);
      } //for iq
      exit(0);
    }
#endif
  } //if

  return mBzAtIP;
} // FwdHoughTree::GetBzAtIP()

// ---------------------------------------------------------------------------------------

void FwdHoughTree::MappingCall(const double par[], t_hough_range id[])
{
  double vz = mTrackFinder->GetVtxNode()->GetZ();

  // FIXME: do it better later;
  assert(mTrackFinder->GetThetaId() != -1 && mTrackFinder->GetPhiId() != -1);
  if (GetDdim() == 3) assert(mTrackFinder->GetMomentumRelatedId() != -1);// || mTrackFinder->GetInvPtId() != -1);

  double theta = deg2rad(par[mTrackFinder->GetThetaId()]), phi = deg2rad(par[mTrackFinder->GetPhiId()]);
  double invp = 0.0, invpt = 0.0;

  if (GetDdim() == 3) {
    if (mTrackFinder->GetInvMomentumId() != -1) invp  = par[mTrackFinder->GetInvMomentumId()];
    if (mTrackFinder->GetInvPtId() != -1)       invpt = par[mTrackFinder->GetInvPtId()];
  } //if

  // FIXME: may want to unify these two parts at some point;
  // Yes, check 'invp != 0' here rather than 'GetDdim() == 2' because magnet-on case 
  // may also require cell edge processing with infinitely high momentum (so de-facto 
  // the same straight tracks);
  if (!invp && !invpt) {
    // FIXME: assume (0,0,vz) vertex;
    t_3d_line track(TVector3(0,0,vz), TVector3(sin(theta)*cos(phi), sin(theta)*sin(phi), cos(theta)));

    // Ok, now need to calculate hit wire numbers for all planes;
    for(int gr=0; gr<mGroups.size(); gr++) {
      TVector3 vcrs;
      FwdHoughNodeGroup *ngroup = GetNodeGroup(gr);

      assert(!cross_p_l(ngroup->GetLocation()->GetPlane(), &track, vcrs));
      
      // Put inside the allowed limits if needed; the idea behind 
      // this trick is the following: Hough code does not need to 
      // loop up to the highest resolution level in order to decide
      // on a given edge cell range (CPU issues!); it stops few levels 
      // earlier and marks acceptance for a given (big) cell a bit 
      // larger than it actually is; still within the allowed limits;
      id[gr] = ngroup->Pack(vcrs);
    } //for gr
  } else {
    // NB: field was stored and is returned back in [kGs] here;
    double pt = invpt ? 1.0/invpt : sin(theta)/invp;
    // Well, 30MeV/c particle in 1kGs field has R=1m; just rescale and convert to [cm];
    double b = GetBzAtIP(), R = 100.0 * (pt/0.03) * (1.0/b);

    // Ok, now need to calculate hit wire numbers for all planes;
    for(int gr=0; gr<mGroups.size(); gr++) {
      FwdHoughNodeGroup *ngroup = GetNodeGroup(gr);
      TrKalmanNodeLocation *location = ngroup->GetLocation();

      // Length of the radial projection should just be (z0-zv)*tg(theta) I guess;
      // assume, that vertex XY are (0,0);
      double dr = (location->GetZ() - vz)*tan(theta), alfa = dr/R;
      double yy = R*(1.0 - cos(alfa)), xx = R*sin(alfa);
      //printf(" @PT@ pt=%f, theta=%f -> yy=%f\n", pt, theta, yy);
      // Apply 1D phi-rotation by hand; NB: this way the signs are correct;
      // FIXME: in fact need crs[2] only (see old code);
      TVector3 crs = TVector3(xx*cos(phi)+yy*sin(phi), xx*sin(phi)-yy*cos(phi), location->GetZ());
      
      id[gr] = ngroup->Pack(crs);
    } //for gr
  } //if
} // FwdHoughTree::MappingCall()

// ---------------------------------------------------------------------------------------

FwdHoughNodeGroup *FwdHoughTree::AddNodeGroup(TrKalmanNodeLocation *location, 
					      unsigned id, unsigned cdim, const double min[], 
					      const double max[], const double gra[])
{
  FwdHoughNodeGroup *ngroup = 
    dynamic_cast<FwdHoughNodeGroup*>(HoughTree::AddNodeGroup(id, cdim, min, max, gra));
  ngroup->SetLocation(location);

  return ngroup;
} // FwdHoughTree::AddNodeGroup()

// ---------------------------------------------------------------------------------------

// FIXME: default group; do it better later;
#define _GR77_ 77

 FwdHoughNodeGroup *FwdHoughTree::AddNodeGroup(TrKalmanNodeLocation *location, unsigned tmpl,
					       bool cylindricalPreference,
					       const std::set<double> &xMin, const std::set<double> &xMax, 
					       const std::set<double> &yMin, const std::set<double> &yMax, 
					       const std::set<double> &rMin, const std::set<double> &rMax)
{
  // Well, based on input data and on the individual template responses want to figure out 
  // whether this group will work in linear (XY) or cylindrical coordinates in terms of limits,
  // meaning of coordinates given to ngroup->Pack(crs) in the FwdHoughTree::MappingCall(), etc;
  // also guess on the optimal granularity (distinguish only isotropic spatial granularity
  // and generic angular granularity, no complications; loop through all KF nodes in this 
  // {location,tmpl} collection;
  std::set<double> spatialSigma, angularSigma;
  bool cartesianOk = true, cylindricalOk = true, useCartesian = true;
  for(unsigned kfnd=0; kfnd<location->GetNodeCount(); kfnd++) {
    TrKalmanNode *node = location->GetNode(kfnd);
    
    SensitiveVolume *sv = node->GetSensitiveVolume();
    if (!sv) continue;

    const EicKfNodeTemplate *__template = sv->GetKfNodeWrapper(tmpl)->GetKfNodeTemplate();

    // NB: in XY-case the smallest of the two will be given;
    double aSigma = __template->GetAngularSigma(), spSigma = __template->GetSpatialSigma();
    printf("%s -> %8.5f %8.5f\n", node->GetName(), aSigma, spSigma);
    if (spSigma) spatialSigma.insert(spSigma);
    if ( aSigma) angularSigma.insert( aSigma);

    // Check possible coordinate system options;
    if (__template->CylindricalThreeDeeOnly()) cartesianOk = false;
    if (__template->CartesianThreeDeeOnly()) cylindricalOk = false;
  } //for kfnd

  // Choose coordinate system; since location was checked in TrKalmanFilter::SetUpLocations(), 
  // at least one option must be available; if there is no preference to use cylindrical coordinate 
  // system and cartesian is possible, use cartesian; 
  assert(cartesianOk || cylindricalOk);
  if ((cylindricalPreference && cylindricalOk) || !cartesianOk) useCartesian = false;

  // Figure out granularity value and call respective AddNodeGroup() routines;
  {
    double spSigma = 0.0, aSigma = 0.0;

    if (useCartesian) {
      // Cartesian case is simple: there should be at least one spatial estimate and angular 
      // estimate is not needed;
      assert(spatialSigma.size());
      spSigma = *spatialSigma.begin();
    } else {
      // See whether estimates are available;
      if (spatialSigma.size()) spSigma = *spatialSigma.begin();
      if (angularSigma.size())  aSigma = *angularSigma.begin();

      // If no angular granularity avaiable, guess on it;
      if (!aSigma) {
	assert(spSigma && (*rMax.rbegin()));
	// FIXME: '5' a clear hack;
	aSigma = spSigma/(*rMax.rbegin()/5);
      } //if
    } //if

    // Blow up by some factor; I guess a single coefficient must be sufficient?;
    double spGranularity = spSigma*mTrackFinder->GetExtraGranularityFactor();
    //printf("s: %f, g: %f\n", spSigma, spGranularity);
    double  aGranularity =  aSigma*mTrackFinder->GetExtraGranularityFactor();

    const EicKfNodeTemplate *_template = location->GetTemplate(tmpl);

    double min[3], max[3], gra[3];
    _template->FillGranularityArray(useCartesian, spGranularity, aGranularity, gra);
    _template->FillMinMaxArrays(useCartesian, xMin, xMax, yMin, yMax, rMin, rMax, min, max);

    printf("%7.2f -> %2d --> %d\n", location->GetZ(), tmpl, location->GetMdim(tmpl));
    printf("   -> xmin=%7.2f, xmax =%7.2f, ymin=%7.2f, ymax=%7.2f\n", 
    	   min[0], max[0], min[1], max[1]);
    FwdHoughNodeGroup *ngroup = 
      AddNodeGroup(location, _GR77_, _template->GetMdim(), min, max, gra);

    // FIXME: perhaps get rid of this variable at all?;
    ngroup->SetMarsToTemplateMtx(location->GetNodeToMaster(tmpl));
    ngroup->SetTemplate(_template);
    ngroup->SetCartesianFlag(useCartesian);

    {
      //double sme[3];

      // FIXME: need to handle angular case as well;
      //_template->FillSmearingArray(mTrackFinder->GetAbsoluteSpatialSmearing(), 0.0, sme);
      //ngroup->SetSmearingValues(sme);
    }

    // THINK: always give some +/- 1 cell freedom in parameter space as well?;
    ngroup->SetPhaseSpaceSmearing(0);

    return ngroup;
  }
} // FwdHoughTree::AddNodeGroup()

// ---------------------------------------------------------------------------------------

bool FwdHoughTree::SetupKalmanFilter(MatchCandidate *match)
{
  HtcKalmanFilter *kf = mTrackFinder->GetKalmanFilter();
  
  assert(kf->SetParticleGroup(mTrackFinder->mParticleHypothesis.Data()));
  kf->ResetFiredFlags();
  
  // Allocate real hits; FIXME: for now consider to take the very 1-st one
  // if several are available after the tree search procedure;
  for(unsigned gr=0; gr<mGroups.size(); gr++) {
    unsigned aliveCount = match->GetAliveMemberCount(gr);

    for(unsigned mm=0; mm<match->GetLinearMemberCount(gr); mm++) {
      GroupMember *member = match->GetSelMember(gr, mm);
      
      // NB: part of them could have been disabled already;
      if (!member) continue;

      EicDetectorGroup *dgroup = (EicDetectorGroup*)member->mPtr.first;
      EicTrackingDigiHit *hit = (EicTrackingDigiHit*)member->mPtr.second;
      SensitiveVolume *sv = dgroup->GetSensitiveVolume(hit);
      
      // FIXME: this will not work for "mosaic" configuration; need to keep track on 
      // hit count per "patch"; 
      KalmanNodeWrapper *wrapper = sv->GetKfNodeWrapper(hit->GetKfNodeID());
      // FIXME: first, this is not efficient; second, may not be correct; just 
      // use 'mm' index throughout all the sensitive volumes; allocate that many, 
      // so that 'mm' index is occupied;
      //if (!wrapper->GetKfNode(mm)) wrapper->AllocateNewKfNode(kf, sv);
      while (!wrapper->GetKfNode(mm)) wrapper->AllocateNewKfNode(kf, sv);

      // FIXME: this will not work for "mosaic" configuration; need to keep track on 
      // hit count per "patch"; 
      TrKalmanNode *trknode = static_cast <TrKalmanNode*> (wrapper->GetKfNode(mm));
      // FIXME: remove when debugging is finished;
      assert(trknode && !trknode->IsFired());
      //assert(!trknode->IsFired());
      trknode->SetHit(hit);
      
      EicKfNodeTemplate *kftmpl = dgroup->mDigi->mKfNodeTemplates[hit->GetKfNodeID()];
      trknode->SetMeasurementNoise(kftmpl->GetMeasurementNoise(hit));
      // If there are >1 hit per node group, blow up their resolutions respectively;
      // the actual "heating" and "annealing" scheme may be different; THINK later;
      if (aliveCount > 1) 
	trknode->InflateMeasurementNoise(mTrackFinder->GetMeasurementNoiseInflationFactor());

#if 0      								    
      {
	Int_t mchitid = hit->GetRefIndex();
	assert(mchitid >= 0);
	FairMCPoint *myPoint = (FairMCPoint*)(dgroup->_fMCPoints->At(mchitid));
	assert(myPoint);
	//printf("gr#%02d, mm#%02d %p -> chi^2 %7.3f, CCDF %10.7f ... %3d\n", gr, mm, member, 
	//     trknode->GetSmootherChiSquare(),
	//     ROOT::Math::chisquared_cdf_c(trknode->GetSmootherChiSquare(), trknode->GetMdim()),
	//     myPoint->GetTrackID());
	if (myPoint->GetTrackID() != 2) {
	  match->ResetMemberPtr(gr, mm);
	  member->mMatchCandidates.erase(match);
	  continue;
	} //if
      }
#endif
      trknode->SetFiredFlag();
    } //for mm
  } //for gr

  // Leave at least 1 node per "mosaic" location; but no multiple "no-hit" nodes
  // per location (just in order to speed up KF calculations);
  kf->SelectActiveNodes();
  // And eventually build linked list of active nodes which can be fed to the KF engine;
  // NB: this routinwe will call kf->CalculateMagnetOffTransportMatrices() internally;
  kf->BuildNodeList();

  //printf(" mCurrMinOkHitCounter -> %d\n", mCurrMinOkHitCounter);
  //assert(mCurrMinOkHitCounter == 6);
  kf->HackGroupHitCountLimit(mCurrMinOkHitCounter);
  kf->LatchGroupNdfControlFlags();

#if _OLD_
  TrKalmanNode *head = static_cast <TrKalmanNode *>(kf->GetHead());
  //TrKalmanNode *tail = static_cast <TrKalmanNode *>(kf->GetTail());
  {
    // THINK: EicBox Generator can not help here, or?;
    if (mTrackFinder->mParticleMomentumSeed)
      head->SetMomentum(mTrackFinder->mParticleMomentumSeed);
    else {
      // FIXME: pull this out either from MCTrack or from IdealGenTrack (?);
      assert(0);
    } //if
  
    // Figure out 0-th track approximation based on the track finder guess;
    // NB: I guess tricks like building linear track approximation will not
    // improve angular part and will be helpless for momentum determination 
    // anyway -> stick to this procedure; NB: assume FwdHoughTree knows 
    // parameter meaning better that FwdTrackFinder (after all, MappingCall()
    // is in this class) -> decrypt [theta,phi] pair and call ResetIpNode();
    double par[GetDdim()];

    // FIXME: may want to pack this into a separate call later;
    ResolutionLevel *high = GetLevel(GetLdim()-1);
    const unsigned *id = match->GetIdPtr();
    for(unsigned iq=0; iq<GetDdim(); iq++) {
      const HoughDimension *dimension = GetDimension(iq);

      par[iq] = dimension->GetMin() + high->GetCellSize(iq)*(id[iq] + 0.5);
    } //for iq	
       
    //printf("%f %f\n", par[0], par[1]); exit(0);
    mTrackFinder->ResetVtxNode(par[0], par[1], _USE_00_);
  }
#endif

  return true;
} // FwdHoughTree::SetupKalmanFilter()

// ---------------------------------------------------------------------------------------
#if _OFF_
void FwdHoughTree::SeparateSiamTracks(MatchCandidate *match, unsigned minHitCount, 
				      std::vector<MatchCandidate*> *newMatches)
{
  //return;

  // FIXME: merge this logic with MatchCandidate::SiamGroupCandidate() later;

  // Figure out how many valid tracks can be here at all;
  unsigned overallHitCount = 0;
  for(int gr=0; gr<GetGdim(); gr++) 
    overallHitCount += match->GetAliveMemberCount(gr);

  unsigned maxTrackCount = overallHitCount / minHitCount;
  printf("%d %d\n", overallHitCount, maxTrackCount); //return; //exit(0);

  assert(maxTrackCount);
  // Check how many would actually fit the 'minHitCount' requirement;
  unsigned hCounters[maxTrackCount], toggle = 0;
  memset(hCounters, 0x00, sizeof(hCounters));

  for(int gr=0; gr<GetGdim(); gr++) {
    unsigned multi = match->GetAliveMemberCount(gr);
    
    if (multi > maxTrackCount) multi = maxTrackCount;
    for(unsigned iq=0; iq<multi; iq++) {
      hCounters[toggle]++;
      toggle = (toggle+1)%maxTrackCount;
    } //for iq
  } //for gr

  // Figure out really max. possible track count;
  unsigned splitValue = 0;
  for(unsigned tr=0; tr<maxTrackCount; tr++)
    if (hCounters[tr] >= minHitCount)
      splitValue++;

  printf("Double counts: %2d %2d\n", hCounters[0], hCounters[1]);
  printf("Will try to split into %2d tracks\n", splitValue);

  // Good; now first perform KF fit of the original track; FIXME: unify with 
  // ResolveAmbiguities() call later;
  HtcKalmanFilter *kf = mTrackFinder->GetKalmanFilter();

  printf("    Building KF fit of original siam group track ...\n");

  // Select useful nodes and construct double linked list;
  SetupKalmanFilter(match);
  mTrackFinder->ResetVtxNode(match);

  {
    //FwdMatchCandidate *fwdmatch = dynamic_cast<FwdMatchCandidate*>(match);

    TrKalmanNode *head = static_cast <TrKalmanNode *>(kf->GetHead());
    TrKalmanNode *tail = static_cast <TrKalmanNode *>(kf->GetTail());

    // Run manually Kalman filter and smoother passes; no need to remove outliers automatically;
    kf->FilterPass(head, tail, KalmanFilter::Forward);
    kf->SmootherPass();
    //fwdmatch->AssertKalmanFilterPassedFlag();

    // Loop through all plane groups and find the worst one with hit count
    // exactly matching the split value;
    std::multimap<double, unsigned> suggestions;//[mGroups.size()];
    for(unsigned gr=0; gr<mGroups.size(); gr++) {
      if (match->GetAliveMemberCount(gr) != splitValue) continue; 

      double smootherDimSum = 0;
      double smootherChiSquareSum = 0.0;
      for(unsigned mm=0; mm<match->GetLinearMemberCount(gr); mm++) {
	GroupMember *member = match->GetSelMember(gr, mm); 
	if (!member) continue;
      
	EicDetectorGroup *dgroup = (EicDetectorGroup*)member->mPtr.first;
	EicTrackingDigiHit *hit = (EicTrackingDigiHit*)member->mPtr.second;
	SensitiveVolume *sv = dgroup->GetSensitiveVolume(hit);
	
	// FIXME: this will not work for "mosaic" configuration; need to keep track on 
	// hit count per "patch"; 
	KalmanNodeWrapper *wrapper = sv->GetKfNodeWrapper(hit->GetKfNodeID());
	assert(wrapper);
	
	// FIXME: this will not work for "mosaic" configuration; need to keep track on 
	// hit count per "patch"; 
	TrKalmanNode *trknode = static_cast <TrKalmanNode*> (wrapper->GetKfNode(mm));
	assert(trknode);

	printf("gr#%02d mm#%02d: %10.7f (%d)\n", gr, mm, trknode->GetSmootherChiSquare(), trknode->GetMdim());
	smootherDimSum       += trknode->GetMdim();
	smootherChiSquareSum += trknode->GetSmootherChiSquare();
      } //for mm

      // Calculate overall chi^2 CCDF;
      suggestions.insert(std::pair<double, 
			 unsigned>(ROOT::Math::chisquared_cdf_c(smootherChiSquareSum, smootherDimSum), gr));     
    } //for gr

    if (!suggestions.size()) {
      printf("Give up on this siam group, sorry ...\n");
      match->SetInactive();
      return;
    } //if

    printf("  -> %d suggestions\n", suggestions.size());
    
    // Well, a few matching plane group(s) found; figure out the most attractive one
    // to be used as the original split; NB: numerical difference can be small (depends 
    // on the tf->SetMeasurementNoiseInflationFactor() setting in reconstruction.C), but
    // since all errors are blown proportionally the same way, at least among similar
    // planes it gives a correct worst candidate;
    for(std::multimap<double, unsigned>::iterator it=suggestions.begin(); it!=suggestions.end(); it++) {
      printf("%10.7f -> gr#%02d\n", it->first, it->second);
    } //for it

    unsigned worstGroup = suggestions.begin()->second;
    printf("   -> so worst group is gr#%02d\n", worstGroup);

    // Create split track array and populate it;
    FwdMatchCandidate *tracks[splitValue];
    for(unsigned tr=0; tr<splitValue; tr++) {
      //FwdMatchCandidate *track = tracks[tr] = dynamic_cast<FwdMatchCandidate*>(AllocateMatchCandidate());
      if (mMatchCandidateCount == mMatchCandidates.size()) 
	mMatchCandidates.push_back(AllocateMatchCandidate());
      FwdMatchCandidate *track = tracks[tr] = dynamic_cast<FwdMatchCandidate*>(mMatchCandidates[mMatchCandidateCount]);

      // THINK: is this all really needed?;
      //track->ResetOkGroupCounter();

      for(unsigned gr=0; gr<mGroups.size(); gr++) {
	unsigned mmCounter = 0;

	track->ResetMemberCount(gr);

	// Loop through all member hits of the original (siam) track;
	for(unsigned mm=0; mm<match->GetLinearMemberCount(gr); mm++) {
	  GroupMember *member = match->GetSelMember(gr, mm); 
	  if (!member) continue;

	  // FIXME: here I explicitely use the fact, that alive member count in this 
	  // group is exactly equal to split value;
	  if (gr == worstGroup && mmCounter++ != tr) continue;

	  // Copy member pointer over;
	  track->AddMember(gr, member);	  
	} //for mm
      } //for gr

      // Reset flags and let member hits know they have new candidate owner;
      track->ShapeItUpForInspection(this, match->GetIdPtr());
      mMatchCandidateCount++;

      newMatches->push_back(track);
      //mgroup->AddCandidate(track);
    } //for tr

    // Kill the original siam track; get max alive member count;
    unsigned maxAliveMemberCount = 0;
    for(unsigned gr=0; gr<mGroups.size(); gr++) {
      if (!maxAliveMemberCount || match->GetAliveMemberCount(gr) > maxAliveMemberCount)
	maxAliveMemberCount = match->GetAliveMemberCount(gr);
#if 0
      for(unsigned mm=0; mm<match->GetLinearMemberCount(gr); mm++) {
	GroupMember *member = match->GetMember(gr, mm); 
	if (!member) continue;
    
	member->mMatchCandidates.erase(match);
      } //for mm
#endif
    } //for gr
    match->SetInactive();

    unsigned unresolvedGroupCount = mGroups.size() - 1;
    bool resolved[mGroups.size()]; memset(resolved, 0x00, sizeof(resolved));
    resolved[worstGroup] = true;

    // Enter infinite loop attempting to distribute other ambiguous hits over 
    // split track candidates; since there can be less hits than tracks in a given 
    // plane group, need to apply tricky ordering scheme;
    for( ; ; ) {
      // Arrange temporary stack storage;
      unsigned mnum[mGroups.size()];
      double ccdf[mGroups.size()][splitValue][maxAliveMemberCount];

      // Build new KF fits for all split tracks separately;
      for(unsigned tr=0; tr<splitValue; tr++) {
	FwdMatchCandidate *track = tracks[tr];
	
	// Select useful nodes and construct double linked list;
	SetupKalmanFilter(track);
	mTrackFinder->ResetVtxNode(track);

	TrKalmanNode *head = static_cast <TrKalmanNode *>(kf->GetHead());
	TrKalmanNode *tail = static_cast <TrKalmanNode *>(kf->GetTail());
	
	// Run manually Kalman filter and smoother passes; no need to remove outliers automatically;
	kf->FilterPass(head, tail, KalmanFilter::Forward);
	kf->SmootherPass();

	printf("tr#%02d -> %f\n", tr, kf->GetFilterChiSquare());
	for(unsigned gr=0; gr<mGroups.size(); gr++) {
	  if (resolved[gr]) continue;

	  // FIXME: will be assigned more than once; do it better later; can use linear
	  // count here, right (only non-zero members were inserted in split tracks);
	  mnum[gr] = track->GetLinearMemberCount(gr);

	  for(unsigned mm=0; mm<track->GetLinearMemberCount(gr); mm++) {
	    GroupMember *member = track->GetSelMember(gr, mm); assert(member);
	    if (!member) continue;
	    
	    // FIXME: unify this stuff to get down to trknode pointer in a standard call;
	    EicDetectorGroup *dgroup = (EicDetectorGroup*)member->mPtr.first;
	    EicTrackingDigiHit *hit = (EicTrackingDigiHit*)member->mPtr.second;
	    SensitiveVolume *sv = dgroup->GetSensitiveVolume(hit);
	
	    // FIXME: this will not work for "mosaic" configuration; need to keep track on 
	    // hit count per "patch"; 
	    KalmanNodeWrapper *wrapper = sv->GetKfNodeWrapper(hit->GetKfNodeID());
	    assert(wrapper);
	    
	    // FIXME: this will not work for "mosaic" configuration; need to keep track on 
	    // hit count per "patch"; 
	    TrKalmanNode *trknode = static_cast <TrKalmanNode*> (wrapper->GetKfNode(mm));
	    assert(trknode);

	    // Store CCDF value;
	    ccdf[gr][tr][mm] = ROOT::Math::chisquared_cdf_c(trknode->GetSmootherChiSquare(), 
							    trknode->GetMdim());
	    printf("gr#%02d mm#%02d: %10.7f (%d)\n", gr, mm, trknode->GetSmootherChiSquare(), trknode->GetMdim());
	  } //for mm
	} //for gr
      } //for tr

      // Ok, now that all hits' chi^2 CCDF values are stored, loop through 
      // all groups and figure out the "most clean" candidate to be resolved;
      {
	bool conflicts[mGroups.size()], cleanGroupsExist = false;
	memset(conflicts, 0x00, sizeof(conflicts));

	int hitIds[mGroups.size()][splitValue];
	for(unsigned gr=0; gr<mGroups.size(); gr++)
	  for(unsigned tr=0; tr<splitValue; tr++) 
	    hitIds[gr][tr] = -1;

	std::multimap<double, unsigned> significances;

	for(unsigned gr=0; gr<mGroups.size(); gr++) {
	  if (resolved[gr]) continue;
	
	  bool usedHit[mnum[gr]];      memset(usedHit, 0x00, sizeof(usedHit));
	  bool happyTrack[splitValue]; memset(happyTrack, 0x00, sizeof(happyTrack));

	  // Order hits in all tracks according to their chi^2 CCDF;
	  std::multimap<double, unsigned> tracksToHits[splitValue];
	  // Order tracks at all hits according to their chi^2 CCDF value;
	  //std::multimap<double, unsigned> hitsToTracks[mnum[gr]];
	  for(unsigned tr=0; tr<splitValue; tr++) {
	    for(unsigned mm=0; mm<mnum[gr]; mm++) {
	      tracksToHits[tr].insert(std::pair<double, unsigned>(ccdf[gr][tr][mm], mm));

	      //hitsToTracks[mm].insert(std::pair<double, unsigned>(ccdf[gr][tr][mm], tr));
	    } //for mm
	  } //for tr
	  
	  // Calculate "significance" of the made choice; 
	  double significance = 1.0;

	  // Now want to give all tracks their "best matching" hits;
	  for(unsigned tr=0; tr<splitValue; tr++) {
	    for(std::multimap<double, unsigned>::reverse_iterator it = tracksToHits[tr].rbegin(); 
		it != tracksToHits[tr].rend(); it++) {
	      // This hit (and possible worse ones) can hardly belong to track anyway -> break;
	      if (!it->first) break;

	      if (!usedHit[it->second]) {
		// FIXME: for now let the very first track to grab the hit;
		printf("#gr%02d: setting tr%02d as owner of mm#%02d\n", gr, tr, it->second);
		hitIds[gr][tr]      = it->second;

		happyTrack[tr]      = true;
		usedHit[it->second] = true;

		{
		  std::multimap<double, unsigned>::reverse_iterator is = it; is++;
		  printf("#gr%02d, tr%02d: %10.6f, %10.6f\n", gr, tr, it->first, is->first);
		  if (is != tracksToHits[tr].rend()) significance *= is->first/it->first;
		}

		break;
	      } //if
	    } //for it

	    // Check for conflicts;
	    if (!happyTrack[tr] || hitIds[gr][tr] != tracksToHits[tr].rbegin()->second) {
	      printf("Track tr#%02d is not happy about gr#%02d\n", tr, gr);
	      conflicts[gr] = true;
	    } //if
	    //else
	    //cleanGroupsExist = true;
	  } //for tr

	  if (!conflicts[gr]) cleanGroupsExist = true;

	  printf("gr#%02d (%d) %10.6f\n", gr, conflicts[gr], significance);
	  significances.insert(std::pair<double, unsigned>(significance, gr));
	} //for gr

	printf("Clean group flag: %d\n", cleanGroupsExist);
	// Choose "weak" group; the smaller significance, the better;
	int weakGroup = -1; assert(significances.size());
	for(std::multimap<double, unsigned>::iterator it = significances.begin(); 
	    it != significances.end(); it++) {
	  //printf("%10.6f -> gr#%02d\n", it->first, it->second);
	  unsigned gr = it->second;

	  if (cleanGroupsExist && conflicts[gr]) continue;

	  weakGroup = gr;
	  break;
	} //for significances
	assert(weakGroup != -1);
	printf("Weak group: %02d\n", weakGroup);

	// Loop through all tracks and assign hits in this group accordingly;
	for(unsigned tr=0; tr<splitValue; tr++) {
	  FwdMatchCandidate *track = tracks[tr];

	  for(unsigned mm=0; mm<track->GetLinearMemberCount(weakGroup); mm++) {
	    GroupMember *member = track->GetSelMember(weakGroup, mm); assert(member);
	    if (!member) continue;

	    // NB: track may have not grabbed any hits (if there were less hits 
	    // than tracks in this plane group);
	    if (hitIds[weakGroup][tr] == -1 || mm != hitIds[weakGroup][tr])
	      track->ResetMemberPtr(weakGroup, mm);
	  } //for mm
	} //for tr

	resolved[weakGroup] = true;
	unresolvedGroupCount--;
      }       

      if (!unresolvedGroupCount) break;
    } //for inf
    
    //if (!fwdmatch->HasAmbiguousHits()) return;
  }
} // FwdHoughTree::SeparateSiamTracks()
#endif
// ---------------------------------------------------------------------------------------

void FwdHoughTree::ResolveAmbiguitiesNg(MatchCandidate *match)
{
  HtcKalmanFilter *kf = mTrackFinder->GetKalmanFilter();

  printf("    Resolving ambiguities ...\n");

  // Select useful nodes and construct double linked list;
  SetupKalmanFilter(match);
  mTrackFinder->ResetVtxNode(match);

  {
    FwdMatchCandidate *fwdmatch = dynamic_cast<FwdMatchCandidate*>(match);

    TrKalmanNode *head = static_cast <TrKalmanNode *>(kf->GetHead());
    TrKalmanNode *tail = static_cast <TrKalmanNode *>(kf->GetTail());

    // Run manually Kalman filter and smoother passes; no need to remove outliers automatically;
    kf->FilterPass(head, tail, KalmanFilter::Forward);
    kf->SmootherPass();
    fwdmatch->AssertKalmanFilterPassedFlag();

    if (!fwdmatch->HasAmbiguousHits()) return;
  }

  if (mAmbiguityResolutionViaWorstHit) {
    // Try the easiest universal way: remove ambiguous hits with lowest chi^2 CCDF 
    // one by one; 
    std::multimap<double, std::pair<unsigned,unsigned> > ccdfs;

    for(unsigned gr=0; gr<mGroups.size(); gr++) {
      // No need to resolve single hit situations -> skip; 
      if (match->GetAliveMemberCount(gr) <= 1) continue;
      
      for(unsigned mm=0; mm<match->GetLinearMemberCount(gr); mm++) {
	TrKalmanNode *trknode = GetKfNode(match, gr, mm);
	
	// NB: 'member' could have been equal 0 (see FwdHoughTree::GetKfNode()) -> check on that;
	if (trknode)
	  ccdfs.insert(std::pair<double, std::pair<unsigned,unsigned> > 
		       (trknode->GetSmootherChiSquareCCDF(), std::pair<unsigned,unsigned>(gr,mm)));
      } //for mm
    } //for gr
    
    // There should be at least one entry;
    assert(ccdfs.size());
    std::pair<unsigned,unsigned> worstMember = ccdfs.begin()->second;
    match->ResetMemberPtr(worstMember.first, worstMember.second);
  } else {
    // The other way around: choose groop with the largest gap between best and worst hits;
    std::multimap<double, unsigned> separations;

#ifdef __APPLE__
    assert(0);
#else
    // Same story as in HoughTree::CheckCell(): need a Mac OS workaround at some point;
    std::multimap<double, unsigned> inspections[mGroups.size()];

    for(unsigned gr=0; gr<mGroups.size(); gr++) {
      // No need to resolve single hit situations -> skip; 
      if (match->GetAliveMemberCount(gr) <= 1) continue;
      
      for(unsigned mm=0; mm<match->GetLinearMemberCount(gr); mm++) {
	TrKalmanNode *trknode = GetKfNode(match, gr, mm);
	
	// NB: 'member' could have been equal 0 (see FwdHoughTree::GetKfNode()) -> check on that;
	if (trknode)
	  inspections[gr].insert(std::pair<double,unsigned>(trknode->GetSmootherChiSquareCCDF(), mm));
      } //for mm

      double best = inspections[gr].rbegin()->first, worst = inspections[gr].rend()->first;
      // THINK: should be done better, otherwise have all chances to pick up a wrong hit; well, 
      // if with a blown up cov.matrix chi^2 is that bad, neither of the hits will fit I guess;
      //assert(best >= 0.0);
      //assert(worst >= 0.0);
      printf("%f %f\n", best, worst); fflush(stdout);
      double separation = (best > 0.0 && worst > 1E-10) ? best / worst : 1E10;
      separations.insert(std::pair<double, unsigned>(separation, gr));
    } //for gr

    unsigned highestSeparationGroup = separations.rbegin()->second;
    match->ResetMemberPtr(highestSeparationGroup, inspections[highestSeparationGroup].begin()->second);
#endif
  } //if
} // FwdHoughTree::ResolveAmbiguitiesNg()

// ---------------------------------------------------------------------------------------
 
TrKalmanNode *FwdHoughTree::GetKfNode(MatchCandidate *match, unsigned gr, unsigned mm)
{
  GroupMember *member = match->GetSelMember(gr, mm); 
  if (!member) return 0;

  EicDetectorGroup *dgroup = (EicDetectorGroup*)member->mPtr.first;
  EicTrackingDigiHit *hit = (EicTrackingDigiHit*)member->mPtr.second;
  SensitiveVolume *sv = dgroup->GetSensitiveVolume(hit);
      
  // FIXME: this will not work for "mosaic" configuration; need to keep track on 
  // hit count per "patch"; 
  KalmanNodeWrapper *wrapper = sv->GetKfNodeWrapper(hit->GetKfNodeID());
  assert(wrapper);

  // FIXME: this will not work for "mosaic" configuration; need to keep track on 
  // hit count per "patch"; 
  TrKalmanNode *trknode = static_cast <TrKalmanNode*> (wrapper->GetKfNode(mm));
  assert(trknode);

  return trknode;
} // FwdHoughTree::GetKfNode()

// ---------------------------------------------------------------------------------------

#if _OLD_
// FIXME: make configurable; NB: these values are bogus anyway until multiple hits 
// per plane are accounted in a way their errors are blown up properly;
#define _CCDF_MIN_       ( 0.010)
#define _SEPARATION_MIN_ (10.000)

void FwdHoughTree::ResolveAmbiguities(MatchCandidate *match)
{
  HtcKalmanFilter *kf = mTrackFinder->GetKalmanFilter();

  printf("    Resolving ambiguities ...\n");

  // Select useful nodes and construct double linked list;
  SetupKalmanFilter(match);
  mTrackFinder->ResetVtxNode(match);

  {
    FwdMatchCandidate *fwdmatch = dynamic_cast<FwdMatchCandidate*>(match);

    TrKalmanNode *head = static_cast <TrKalmanNode *>(kf->GetHead());
    TrKalmanNode *tail = static_cast <TrKalmanNode *>(kf->GetTail());

    // Run manually Kalman filter and smoother passes; no need to remove outliers automatically;
    kf->FilterPass(head, tail, KalmanFilter::Forward);
    kf->SmootherPass();
    fwdmatch->AssertKalmanFilterPassedFlag();

    if (!fwdmatch->HasAmbiguousHits()) return;
  }

  //printf("%2d -> chi^2: %7.2f; CCDF: %10.7f\n", kf->GetFilterNdf(), kf->GetFilterChiSquare(), 
  //	 kf->GetFilterChiSquareCCDF());

  // Let STL do the job; resolve everything which is well separated 
  // (namely chi^2 CCDF value of best hit is higher threshold and separation 
  // to the "second best hit" is large enough); if no such groups found, 
  // choose the group with largest gap between best and worst hit and remove this
  // single worst hit;
  std::multimap<double, unsigned> inspections[mGroups.size()];
  // Need at least one clean resolution; otherwise fall back to 'separations';
  bool resolved = false;
  std::multimap<double, unsigned> separations;

  // Consider simple algorithm first; just remove all duplicate hits with highest 
  // smoother chi^2 at once; so loop once again through all hits;
  for(unsigned gr=0; gr<mGroups.size(); gr++) {
    // No need to resolve single hit situations -> skip; 
    //#if _BACK_
    if (match->GetAliveMemberCount(gr) <= 1) continue;   
    //#endif

    for(unsigned mm=0; mm<match->GetLinearMemberCount(gr); mm++) {
      GroupMember *member = match->GetSelMember(gr, mm); 
      if (!member) continue;
      
      EicDetectorGroup *dgroup = (EicDetectorGroup*)member->mPtr.first;
      EicTrackingDigiHit *hit = (EicTrackingDigiHit*)member->mPtr.second;
      SensitiveVolume *sv = dgroup->GetSensitiveVolume(hit);
      
      // FIXME: this will not work for "mosaic" configuration; need to keep track on 
      // hit count per "patch"; 
      KalmanNodeWrapper *wrapper = sv->GetKfNodeWrapper(hit->GetKfNodeID());
      assert(wrapper);

      // FIXME: this will not work for "mosaic" configuration; need to keep track on 
      // hit count per "patch"; 
      TrKalmanNode *trknode = static_cast <TrKalmanNode*> (wrapper->GetKfNode(mm));
      assert(trknode);
      inspections[gr].insert(std::pair<double, 
			     unsigned>(ROOT::Math::chisquared_cdf_c(trknode->GetSmootherChiSquare(), 
								    trknode->GetMdim()), mm));
      {
	Int_t mchitid = hit->GetRefIndex();
	assert(mchitid >= 0);
	FairMCPoint *myPoint = (FairMCPoint*)(dgroup->_fMCPoints->At(mchitid));
	assert(myPoint);
	printf("gr#%02d, mm#%02d %p (%d,%d) %p -> chi^2 %7.3f, CCDF %10.7f ... %3d\n", 
	       gr, mm, member, member->IsBusy(), member->IsBooked(), hit, 
	       trknode->GetSmootherChiSquare(),
	       ROOT::Math::chisquared_cdf_c(trknode->GetSmootherChiSquare(), trknode->GetMdim()),
	       myPoint->GetTrackID());
      }
    } //for mm
    //continue;

    // This means at least one good candidate;
    //assert(inspections[gr].rbegin()->first);
    //printf("%f\n", 
    assert(inspections[gr].size());
    if (inspections[gr].rbegin()->first >= _CCDF_MIN_) {
      // NB: well, since match->GetAliveMemberCount(gr) check was made I'm guaranteed to have 
      // at least two different entries in this multimap; need separation between the best and 
      // the next-to-best here;
      std::multimap<double, unsigned>::reverse_iterator it = inspections[gr].rbegin(); it++;
      //assert(it->first > 0.0);// && inspections[gr].rbegin()->first > 0.0);
      //assert(inspections[gr].rbegin()->first > 0.0);
      //+double separation = fabs(log(it->first / inspections[gr].rbegin()->first));
      double separation = it->first > 0.0 ? fabs(log(it->first / inspections[gr].rbegin()->first)) : 0.0;
      separations.insert(std::pair<double, unsigned>(separation, gr));
      printf(" best-to-next separation %10.8f; best is mm#%02d ...\n", separation, 
	     inspections[gr].rbegin()->second);
      
      // Check whether 1) best node is "good enough", 2) next-to-best is "much worse";
      // THINK: may want to check, that next-to-best is just "bad" instead of #2?; 
      if (inspections[gr].rbegin()->first >= _CCDF_MIN_ && 
	  separation > _SEPARATION_MIN_) {
	resolved = true;
	
	for(unsigned mm=0; mm<match->GetLinearMemberCount(gr); mm++) {
	  GroupMember *member = match->GetSelMember(gr, mm); 
	  if (!member) continue; 
	  
#if _OLD_
	  if (mm == inspections[gr].rbegin()->second) {
	    // THINK: only after the final fit?;
	    //member->IncrementBusyCounter();
	  }
	  else {
	    match->ResetMemberPtr(gr, mm);
	    //member->mMatchCandidates.erase(match);
	  } //if
#else
	  if (mm != inspections[gr].rbegin()->second) match->ResetMemberPtr(gr, mm);	  
#endif
	} //for mm
      } //if
    } 
    else
      // FIXME: this is just a stupid hack to bypass bad tracks;
      separations.insert(std::pair<double, unsigned>(0.0, gr));
  } //for gr
  //exit(0);

  // No reliable outlier detection happened in either of the groups; then a fall-back 
  // option: choose the group with highest best-to-worst separation and remove the worst one;
  if (!resolved) {
    assert(separations.size());
    unsigned highestSeparationGroup = separations.rbegin()->second;

    unsigned mm = inspections[highestSeparationGroup].begin()->second;
    printf("Highest best-to-worst separation group: gr#%02d -> removing mm#%02d ...\n", 
	   highestSeparationGroup, mm);
    GroupMember *member = match->GetSelMember(highestSeparationGroup, mm); assert(member);

    match->ResetMemberPtr(highestSeparationGroup, mm);
  } //if

  printf("%2d -> chi^2: %7.2f; CCDF: %10.7f\n", kf->GetFilterNdf(), kf->GetFilterChiSquare(), 
	 kf->GetFilterChiSquareCCDF());
} // FwdHoughTree::ResolveAmbiguities() 
#endif
// ---------------------------------------------------------------------------------------

void FwdHoughTree::SetupTrackQualityIteration(unsigned qua) {
  // Otherwise just a dummy call, right?;
  if (!mFastTreeSearchMode) return;

  // Not too much efficient to calculate this every time new, but overhead is negligible;
  double minCCDF = mTrackFinder->GetStoredMinFilterChiSquareCCDF();

  double bWidth = (log(1.0) - log(minCCDF))/mTrackQualityIterationNum;

  double ccdf = exp(log(minCCDF) + (mTrackQualityIterationNum - qua - 1)*bWidth);
  //printf("%f\n", ccdf); exit(0);

  mTrackFinder->GetKalmanFilter()->SetMinFilterChiSquareCCDF(ccdf);
} // FwdHoughTree::SetupTrackQualityIteration()

// ---------------------------------------------------------------------------------------

//FIXME: may want to use xs[] update to initialize head node if ambiguity pass was made;

void FwdHoughTree::FinalFit(MatchCandidate *match)
{
  unsigned kfret = 0;
  FwdMatchCandidate *fwdmatch = dynamic_cast<FwdMatchCandidate*>(match);
  HtcKalmanFilter *kf = mTrackFinder->GetKalmanFilter();
  TrKalmanNode *head = static_cast <TrKalmanNode *>(kf->GetHead());

  printf("    Final fit started ...\n");

  // NB: by this time filter-smoother chain was run at least once;
  for(unsigned itr=0; itr<(mTrackFinder->WithMagneticField() ? 1 : 2); itr++) {
    SetupKalmanFilter(match);
    mTrackFinder->UpdateVtxNode();
    
    TrKalmanNode *tail = static_cast <TrKalmanNode *>(kf->GetTail());
    
    // Run Kalman filter and smoother passes with automatic outlier removal; FIXME: min. hit 
    // count will always be set to mMinOkHitCounter (no matter what the current limit in 
    // HoughTree loop is); this is sort of a waste of CPU time (so may want to change this 
    // limit dynamically); but if final fit has less good hits than the current limit, track 
    // will NOT be considered as good and its hits will not be marked as "occupied" as well;
    kfret = 
      kf->FullChain(head, tail, KalmanFilter::Forward, 
		    kf->GetMinFilterChiSquareCCDF() ? _TRUST_FILTER_FCN_ : 0/*|_TRUST_SMOOTHER_FCN_*/);
  } //for itr

  {
    // Also figure out most probable MC track prototype; could use 'unordered_map' here;
    std::map<unsigned, unsigned> ptracks;
    // Will be used in the 2-d step (order tracks based on hit count);
    std::multimap<unsigned, unsigned> ordered;

    for(unsigned gr=0; gr<mGroups.size(); gr++) {
      // No need to resolve single hit situations -> skip; 
      assert(match->GetAliveMemberCount(gr) <= 1); 
      
      for(unsigned mm=0; mm<match->GetLinearMemberCount(gr); mm++) {
	GroupMember *member = match->GetSelMember(gr, mm); 
	if (!member) continue;
	
	{
	  EicDetectorGroup *dgroup = (EicDetectorGroup*)member->mPtr.first;
	  EicTrackingDigiHit *hit = (EicTrackingDigiHit*)member->mPtr.second;
	  SensitiveVolume *sv = dgroup->GetSensitiveVolume(hit);
	  
	  KalmanNodeWrapper *wrapper = sv->GetKfNodeWrapper(hit->GetKfNodeID());
	  assert(wrapper);
	  
	  // FIXME: this will not work for "mosaic" configuration; need to keep track on 
	  // hit count per "patch"; 
	  TrKalmanNode *trknode = static_cast <TrKalmanNode*> (wrapper->GetKfNode(mm));
	  assert(trknode);
	  
	  Int_t mchitid = hit->GetRefIndex();
	  assert(mchitid >= 0);
	  FairMCPoint *myPoint = (FairMCPoint*)(dgroup->_fMCPoints->At(mchitid));
	  assert(myPoint);
	  ptracks[myPoint->GetTrackID()]++;
	  printf(" @@@ gr#%02d, mm#%02d %p (%d,%d) %p -> chi^2 %c%7.3f (%d), CCDF %8.5f ... %3d\n", 
		 gr, mm, member, member->IsBusy(), member->IsBooked(), hit, 
		 trknode->GetSmootherChiSquare() < 100. ? ' ' : '>', 
		 trknode->GetSmootherChiSquare() < 100. ? trknode->GetSmootherChiSquare() : 100.,
		 trknode->GetMdim(),
		 ROOT::Math::chisquared_cdf_c(trknode->GetSmootherChiSquare(), trknode->GetMdim()),
		 myPoint->GetTrackID());
	}
      } //for mm
    } //for gr

    // Loop through all 'ptrack' entries and order them based on hit count; 
    // FIXME: allocate iterator once;
    for(std::map<unsigned, unsigned>::iterator it=ptracks.begin(); it != ptracks.end(); it++) {
      ordered.insert(std::pair<unsigned, unsigned>(it->second, it->first));
    } //for
    
    // Decrypt 'ordered' contents;
    fwdmatch->SetMcTrackId(ordered.rbegin()->second);
    fwdmatch->SetAmbiguityFlag(ordered.size() != 1);
    // FIXME: way want to actually calculate all minor contributions;
    fwdmatch->SetWrongHitCount(match->GetAliveGroupCount() - ordered.rbegin()->first);
    // Well, assume VTX node is the head one?;
    fwdmatch->SetVtxCoord(head->GetX0(0) + head->GetXs(0), head->GetX0(1) + head->GetXs(1), head->GetZ());
    fwdmatch->SetVtxCoordSigma(sqrt(head->GetCS(0,0)), sqrt(head->GetCS(1,1)));
    fwdmatch->SetVtxSlopeSigma(sqrt(head->GetCS(2,2)), sqrt(head->GetCS(3,3)));
    if (kf->GetFieldMode() == WithField)
      fwdmatch->SetVtxMomentum(head->GetX0(2) + head->GetXs(2), head->GetX0(3) + head->GetXs(3), 
			       head->GetInversedMomentum() + head->GetXs(4));
    else
      fwdmatch->SetVtxMomentum(head->GetX0(2) + head->GetXs(2), head->GetX0(3) + head->GetXs(3), 
			       head->GetInversedMomentum());
  }

  printf(" @@@ %8X -> %2d -> chi^2: %7.2f; CCDF: %10.7f\n", kfret, kf->GetFilterNdf(), 
	 kf->GetFilterChiSquare(), kf->GetFilterChiSquareCCDF());
  mTrackFinder->ccdf->Fill(kf->GetFilterChiSquareCCDF());

  // If Kalman filter-smoother chain had issues, check all nodes whether
  // they still carry "fired" flag; otherwise remove respective member 
  // from the array and remove entry in member->mMatchCandidates as well;
  // FIXME: once debugging is over may want to unify loops, etc; 
  if (kfret) {
    for(unsigned gr=0; gr<mGroups.size(); gr++) {
      assert(match->GetAliveMemberCount(gr) <= 1);
      
      for(unsigned mm=0; mm<match->GetLinearMemberCount(gr); mm++) {
	GroupMember *member = match->GetSelMember(gr, mm); 
	if (!member) continue;
	
	{
	  EicDetectorGroup *dgroup = (EicDetectorGroup*)member->mPtr.first;
	  EicTrackingDigiHit *hit = (EicTrackingDigiHit*)member->mPtr.second;
	  SensitiveVolume *sv = dgroup->GetSensitiveVolume(hit);
	  
	  KalmanNodeWrapper *wrapper = sv->GetKfNodeWrapper(hit->GetKfNodeID());
	  assert(wrapper);
	  
	  // FIXME: this will not work for "mosaic" configuration; need to keep track on 
	  // hit count per "patch"; 
	  TrKalmanNode *trknode = static_cast <TrKalmanNode*> (wrapper->GetKfNode(mm));
	  assert(trknode);
	  if (trknode->HasHit() && !trknode->IsFired()) match->ResetMemberPtr(gr, mm);
	}
      } //for mm
    } //for gr
  } //if

  // Decide whether track is good;
  bool trackIsOk = !kfret && match->GetAliveGroupCount() >= mCurrMinOkHitCounter;

  if (trackIsOk) {
    fwdmatch->SetFilterChiSquare(kf->GetFilterChiSquare(), kf->GetFilterChiSquareCCDF(), 
				 kf->GetFilterNdf());

  }
  else
    match->SetInactive();
} // FwdHoughTree::FinalFit()

// ---------------------------------------------------------------------------------------

ClassImp(FwdHoughTree)
