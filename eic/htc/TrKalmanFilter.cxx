//
// AYK (ayk@bnl.gov)
//
//    Forward detector tracking application of the Kalman filter stuff; 
//  ported from HERMES/OLYMPUS sources; cleaned up 2014/10/17;
//

#include <cassert>

#include <htclib.h>
#include <MediaSliceArray.h>
#include <TrKalmanFilter.h>
#include <SensitiveVolume.h>

// =======================================================================================

void TrKalmanFilter::SelectActiveNodes()
{
  // First reset all the active flags; yes, go through the whole node pool;
  for(std::map<double, KalmanNode*>::iterator it=mKalmanNodePool.begin(); 
      it != mKalmanNodePool.end(); it++) 
    dynamic_cast<TrKalmanNode*>(it->second)->SetActiveFlag(false); 

  // Loop through all locations and activate fired nodes; if there are no
  // fired nodes at a given location, activate just one single "dummy" node
  // (have to do this because media arrays are calculated between neighbor
  // locations; would be prohibitevely expensive to calculate them either 
  // on the fly (CPU) of for all location pairs (RAM));
  for(TrKalmanNodeLocation *location=mLocationHead; location; 
      location=location->GetNext(KalmanFilter::Forward)) {
    // Loop through all the nodes; do it in descending order, just that 
    // the only active node in case of no hits will be the 0-th one (even 
    // that this doe not really matter);
    bool fired = false;

    //printf("%f -> %d node(s)\n", location->GetZ(), location->GetNodeCount());
    for(int nd=location->GetNodeCount()-1; nd>=0; nd--) {
      TrKalmanNode *node = location->GetNode(nd);

      if (node->IsFired() || (!nd && !fired)) node->SetActiveFlag(true);

      // Ned at least one active node per location; keep track on fired ones;
      if (node->IsFired()) fired = true;
    } //for nd
  } //for location
} // TrKalmanFilter::SelectActiveNodes()

// ---------------------------------------------------------------------------------------

int TrKalmanFilter::TransportExtra(KalmanNode *from, KalmanFilter::Direction fb, unsigned mode)
{
  TrKalmanNode *qfrom = static_cast <TrKalmanNode*> (from);
  assert(qfrom);

  // Calculate process noise; want it here since it is applicable to linear case as well;
  if (mode & _CALCULATE_PROCESS_NOISE_ && CalculateProcessNoise(qfrom, fb))
    _RETURN_(_NFUN_FAILURE_, "Filter failed in root->nfun()!\n");

  // Account for energy losses if needed; 
  if (AccountIonizationLosses(qfrom, fb))
    _RETURN_(_XFUN_FAILURE_, "Filter failed in root->xfun()!\n");

  return 0;
} // TrKalmanFilter::TransportExtra()

// ---------------------------------------------------------------------------------------

int TrKalmanFilter::CalculateMagnetOffTransportMatrices()
{
  TrKalmanNode *node = static_cast <TrKalmanNode *>(GetHead());

  while (node) {
    // System equation in a HERA-B parameterization; NB: diagonal elements
    // of FF[]-matrices have been set to 1. already in add_kalman_node();
    for(int fb=0; fb<2; fb++) {
      switch (fb) {
      case KalmanFilter::Forward:
	if (node->GetNext(KalmanFilter::Forward)) 
	  node->FF[fb]->KFM(0,2) = node->FF[fb]->KFM(1,3) = 
	    node->GetNext(KalmanFilter::Forward)->GetZ() - node->GetZ();
	break;
      case KalmanFilter::Backward:
	if (node->GetNext(KalmanFilter::Backward)) 
	  node->FF[fb]->KFM(0,2) = node->FF[fb]->KFM(1,3) = 
	    node->GetNext(KalmanFilter::Backward)->GetZ() - node->GetZ();
	break;
      } //switch
    } //for fb

    node = node->GetNext(KalmanFilter::Forward);
  } //while

  return 0;
} // TrKalmanFilter::CalculateMagnetOffTransportMatrices()

// ---------------------------------------------------------------------------------------

void TrKalmanFilter::SetUpLocations()
{
  // Build linked list of Kalman node locations first; 
  for(std::map<double, KalmanNode*>::iterator it=mKalmanNodePool.begin(); 
      it != mKalmanNodePool.end(); it++) {
    TrKalmanNode *node = dynamic_cast<TrKalmanNode*>(it->second);

    //printf("%7.2f -> %s\n", node->GetZ(), node->GetName());

    TrKalmanNodeLocation *location;
    if (!mLocationSeparationDistance || !mLocationHead || 
	(node->GetZ() - mLocationTail->GetZ() > mLocationSeparationDistance)) {
      location = new TrKalmanNodeLocation(node->GetZ());
      //printf("location %7.3f\n", location->GetZ());

      if (!mLocationHead)
	mLocationHead = location;
      else {
	mLocationTail->SetNext(location);
	location->SetPrev(mLocationTail);
      } //if
      
      mLocationTail = location;
    }
    else 
      location = mLocationTail;

    location->AddNode(node);
    node->mLocation = location;

    // This call will also check, that pointer is non-zero;
    location->AddSensitiveVolume(node->GetSensitiveVolume());
  } //for it

  // Check, that all sensitive volumes at every location are of "similar" type;
  // for now just require, that all have the same number of KF templates;
  for(TrKalmanNodeLocation *location=mLocationHead; location;
      location=location->GetNext(KalmanFilter::Forward)) {
    // Loop through all its sensitive volumes;
    for(std::set<SensitiveVolume*>::iterator it=location->GetSensitiveVolumes().begin(); 
	it != location->GetSensitiveVolumes().end(); it++) {
      unsigned nodeWrapperCount = (*it)->GetKfNodeWrapperCount();

      // FIXME: do better later;
      if (location->GetSensitiveVolumeNodeWrapperCount()) {
	assert(location->GetSensitiveVolumeNodeWrapperCount() == nodeWrapperCount);

	// And dimension check, please; FIXME: may want to do more tricky consistency checks later;
	for(unsigned tmpl=0; tmpl<nodeWrapperCount; tmpl++) {
	  //assert((*it)->GetKfNodeWrapper(tmpl)->GetMdim() == location->GetMdim(tmpl));
	  assert((*it)->GetKfNodeWrapper(tmpl)->GetKfNodeTemplate()->IsCompatible(location->GetTemplate(tmpl)));

	  //if ((*it)->GetKfNodeWrapper(tmpl)->GetKfNodeTemplate()->FavorCylindricalThreeDee())
	  //location->SetCylindricalPreference(tmpl);
	} //for tmpl
      } else {
	for(unsigned tmpl=0; tmpl<nodeWrapperCount; tmpl++) {
	  // Well, location->mDims[] is an STL vector, filled out at once;
	  location->SetNextMdimValue((*it)->GetKfNodeWrapper(tmpl)->GetMdim());

	  location->SetNextNodeToMaster((*it)->GetKfNodeWrapper(tmpl)->GetNodeToMasterMtx());
	  location->SetNextTemplate((*it)->GetKfNodeWrapper(tmpl)->GetKfNodeTemplate());
	  //location->SetNextCylindricalPreference((*it)->GetKfNodeWrapper(tmpl)->GetKfNodeTemplate()->FavorCylindricalThreeDee());
	} //for tmpl
      } //if
    } //for it
  } //for location
} // TrKalmanFilter::SetUpLocations()

// ---------------------------------------------------------------------------------------

int TrKalmanFilter::Configure(MediaBank *media_bank, StringList *config)
{
  // Just hardcode the format for gap nodes here for now; FIXME: this will not
  // work anyway, since they should have been declared before setting up locations; 
  if (KalmanFilter::Configure(config/*, (char*)"Z@%05.1f"*/)) return -1;

  // Only after all nodes are defined; 
  if (CalculateMagnetOffTransportMatrices()) 
    _RETURN_(-1, "Failed to calculate magnet-off transport matrices!\n");

  // Map material distribution on top of the defined Kalman node grid;
  InitializeMediaSlices(media_bank);

  // And also intialize Runge-Kutta frames right here;
  if (mFieldMode == WithField) InitializeRungeKuttaFrames();

  return 0;
} // TrKalmanFilter::Configure()

// ---------------------------------------------------------------------------------------

int TrKalmanFilter::InitializeMediaSlices(MediaBank *media_bank)
{ 
  // Build media slice arrays for all nodes;
  for(TrKalmanNodeLocation *location=mLocationHead; location->GetNext(KalmanFilter::Forward);
      location=location->GetNext(KalmanFilter::Forward))
    location->mMediaSliceArray = 
      new MediaSliceArray(media_bank, location->GetZ(), 
			  location->GetNext(KalmanFilter::Forward)->GetZ());

  // And then calculate process noise basic matrices for F/B directions;
  for(TrKalmanNodeLocation *location=mLocationHead; location; 
      location=location->GetNext(KalmanFilter::Forward)) {
    if (location->GetPrev(KalmanFilter::Forward)) {
      location->mProcessNoise[KalmanFilter::Backward] = 
        location->InitializeProcessNoiseMatrices(KalmanFilter::Backward);
      if (!location->mProcessNoise[KalmanFilter::Backward]) return -1;
    } //if

    if (location->GetNext(KalmanFilter::Forward)) {
      location->mProcessNoise[KalmanFilter::Forward] = 
        location->InitializeProcessNoiseMatrices(KalmanFilter::Forward);
      if (!location->mProcessNoise[KalmanFilter::Forward]) return -1;
    } //if
  } //for node

  return 0;
} // TrKalmanFilter::InitializeMediaSlices()

// ---------------------------------------------------------------------------------------

int TrKalmanFilter::InitializeRungeKuttaFrames()
{            
  for(TrKalmanNodeLocation *location=mLocationHead; location; location=location->GetNext(KalmanFilter::Forward))
    for(unsigned fb=KalmanFilter::Forward; fb<=KalmanFilter::Backward; fb++) {
      if (!location->GetNext(fb)) continue;
	
      TrKalmanNodeLocation *to = location->GetNext(fb);
      // NB: sign matters!;
      double h = to->GetZ() - location->GetZ();
      RungeKutta *rk = location->mRungeKutta + fb;

      // Figure out which order Runge-Kutta technique should be used;
      // perhaps take into account field magnitude later as well; or even
      // create both frames and take decision based on momentum for a given
      // track; for now go simple;
#if 1
      rk->_order = fabs(h) > RK_small_step_limit ? _RK_ORDER_5_ : RK_small_step_order; 
#else
      rk->_order = _RK_ORDER_2_;
#endif
      // Consider to use 4-th order, at least for the test phase;
      //rk->_order = fabs(h) > RK_small_step_limit ? _RK_ORDER_4_ : RK_small_step_order; 

      // m1: needed for all 3 Runge-Kutta cases;
      rk->m1 = InitializeMgridSlice(location->GetZ());
      if (!rk->m1) return -1;

      // Depending on the Runge-Kutta order initialize m2..m6;
      switch (rk->_order) {
      case _RK_ORDER_2_:
	// Used by both 2-d and 4-th order algorithms;
	rk->m2 = InitializeMgridSlice(location->GetZ() + h/2.);
	if (!rk->m2) return -1;
	break;
      case _RK_ORDER_4_:
	rk->m2 = InitializeMgridSlice(location->GetZ() + h/2.);
	rk->m3 = rk->m2;
	// Yes, explicitely use "to->z" instead of "z+h" in order to avoid 
	// boundary Z mismatch due to rounding errors;
	//rk->m4 = initializeMgridSlice(to->z);
	rk->m4 = InitializeMgridSlice(to->GetZ());
	if (!rk->m2 || !rk->m4) return -1;
	break;
      case _RK_ORDER_5_:
	// Yes, because want to use "to->z";
	assert(a5 == 1.);
	rk->m2 = InitializeMgridSlice(location->GetZ() + h*a2);
	rk->m3 = InitializeMgridSlice(location->GetZ() + h*a3);
	rk->m4 = InitializeMgridSlice(location->GetZ() + h*a4);
	//rk->m5 = initializeMgridSlice(to->GetZ());
	rk->m5 = InitializeMgridSlice(to->GetZ());
	rk->m6 = InitializeMgridSlice(location->GetZ() + h*a6);
	if (!rk->m2 || !rk->m3 || !rk->m4 || !rk->m5 || !rk->m6) return -1;
	break;
      } //switch
    } //for node..fb

  return 0;
} // TrKalmanFilter::InitializeRungeKuttaFrames()

// =======================================================================================

int TrKalmanFilter::Transport(KalmanNode *from, KalmanFilter::Direction fb, unsigned mode)
{
  double pout[6], rkd[5][5];
  TrKalmanNode *trfrom = (static_cast <TrKalmanNode*>(from));
  TrKalmanNode *trto = fb == KalmanFilter::Forward ? 
    trfrom->GetNext(KalmanFilter::Forward) : trfrom->GetPrev(KalmanFilter::Forward);

  // Just use the same code as in HTC (which was in turn interfaced
  // in such a way that it mimics the old HERA-B Runge-Kutta code);
  if ((static_cast <TrKalmanNode*>(from))->
      PerformRungeKuttaStep(fb, pout+1, mode & _CALCULATE_DERIVATIVES_ ? rkd : NULL)) 
    return -1;

  // Assign [x,y,sx,sy] part of predicted state vector for the 'to' node; 
  for(int ip=_X_; ip<=_SY_; ip++)
    trto->x0->KFV(ip) = pout[ip+1];
    
  if (mode & _CALCULATE_DERIVATIVES_) {
    // Assign transport matrix; NB: transposition required!;
    for(int ip=_X_; ip<=_SY_; ip++)
      for(int iq=_X_; iq<=_SY_; iq++)
	trfrom->FM->KFM(iq,ip) = rkd[ip][iq];

    // Also momentum part of transport matrix, if needed; 
    // NB: calculate_magnet_off_transport_matrices() resets diagonal 
    // FM[<1/p>][<1/p>] term to 1. and d(1/p)/d{x,y,sx,sy} terms to 0.
    // once forever; so need to assign only 4 d{x,y,sx,sy}/d(1/p) ones;
    for(int iq=_X_; iq<=_SY_; iq++)
      trfrom->FM->KFM(iq,_QP_) = rkd[4][iq];
  } //if

  return 0;
} // TrKalmanFilter::Transport()

// ---------------------------------------------------------------------------------------

int TrKalmanFilter::AccountIonizationLosses(TrKalmanNode *from, KalmanFilter::Direction fb)
{
  TrKalmanNode *to = from->GetNext(fb);

  // Again, put this poor-man check; do it better later;
  //if (dE_dx_flag && from->GetZ() != to->GetZ()) {
  if (mAccountEnergyLosses && from->GetZ() != to->GetZ()) {
    //assert(0);
    MediaSliceArray *array = fb == KalmanFilter::Forward ? 
      from->mLocation->mMediaSliceArray : to->mLocation->mMediaSliceArray;
    //assert(array);
    double invp0 = from->mInversedMomentum, losses, mass = mParticleGroup->mass;
    // Yes, GDREL*() routines expect kinetic energy as an argument;
    // this bug was fixed on 2009/09/22 only; funny enough, look-up
    // tables in SC150 software have always been filled correctly;
    double e0_kin = sqrt(1./SQR(invp0) + SQR(mass)) - mass;

    // Consider to use average as of Feb'2008;
    double tx = (from->x0->KFV(2) + to->x0->KFV(2))/2.;
    double ty = (from->x0->KFV(3) + to->x0->KFV(3))/2.;
    
    double len_cff = 1. + SQR(tx) + SQR(ty);

    // Call respective GEANT routine wrapper and get total energy loss; 
    // correct it for slope (thinckness) and direction (forward/backward: sign);
    losses = array->GetDE(mParticleGroup, invp0 > 0.? 1 : -1, e0_kin)*sqrt(len_cff);
    if (fb == KalmanFilter::Backward) losses = -losses;

    {
      double e1_kin = e0_kin - losses, invp1;

      // Some crude regularization, please;
      if (e1_kin > 0.) {
	invp1 = 1./sqrt(e1_kin*(e1_kin+2.*mass));
	if (invp0 < 0.) invp1 = -invp1;
      } 
      else
	invp1 = invp0;

      to->mInversedMomentum = invp1;
    }
  }
  else
    // Just copy over inversed momentum to the next node;
    to->mInversedMomentum = from->mInversedMomentum;

  return 0;
} // TrKalmanFilter::AccountIonizationLosses()

// ---------------------------------------------------------------------------------------

int TrKalmanFilter::CalculateProcessNoise(TrKalmanNode *from, KalmanFilter::Direction fb) 
{
  // FIXME: isn't it the only thing needed if both nodes are at the same Z-location?;
  for(int ip=_X_; ip<=_SY_; ip++)
    for(int iq=_X_; iq<=_SY_; iq++)
      from->Q->KFM(ip,iq) = 0.0;

  KalmanNode *to = from->GetNext(fb);
  if (from->GetZ() == to->GetZ()) return 0;

  ProcessNoise *noise = from->mLocation->mProcessNoise[fb];
  // Helps bridge nodes (?);
  if (!noise) return 0;

  //double len_cff, cxx, cyy, cxy, fcff;
  // 'p': which units are expected? -> CHECK!; 
  double e = sqrt(1. + SQR(mParticleGroup->mass*from->mInversedMomentum))/fabs(from->mInversedMomentum);
  double p = 1/from->mInversedMomentum;
  double beta = 1./fabs(from->mInversedMomentum*e);

  // Consider to use average as of Feb'2008; 
  double tx = (from->x0->KFV(_SX_) + to->GetX0(_SX_))/2.;
  double ty = (from->x0->KFV(_SY_) + to->GetX0(_SY_))/2.;

  double len_cff = 1. + SQR(tx) + SQR(ty);
  double cxx = len_cff*(1. + SQR(tx));
  double cyy = len_cff*(1. + SQR(ty));
  double cxy = len_cff*tx*ty;

  // Use precalculated (approximate) C** matrices and put in missing 
  // slope- and momentum-dependent coefficients;  
  //#ifdef _USE_GEANT3_MOLIERE_CHC_
  // MULS=3 model in GEANT; it turns out that MULS=1 model description  
  // using this ansatz is also significantly better than with a 'usual' 
  // formula (see above line);                                          
  //double fcff = sqrt(len_cff)/SQR(e*SQR(beta));
  double fcff = sqrt(len_cff)/SQR(p*beta);
  //#else
  //double fcff = sqrt(len_cff)/SQR(p*beta);
  //#endif     

  // Fill upper triangle and diagonal terms;
  for(int ip=0; ip<4; ip++)
    for(int iq=ip; iq<4; iq++)
      from->Q->KFM(ip,iq) = fcff*(cxx*noise->mCxx[ip][iq] + cyy*noise->mCyy[ip][iq] + 
				  cxy*noise->mCxy[ip][iq]);
  // Fill lower triangle;
  for(int ip=1; ip<4; ip++)
    for(int iq=0; iq<ip; iq++)
      from->Q->KFM(ip,iq) = from->Q->KFM(iq,ip);

  return 0;
} // TrKalmanFilter::CalculateProcessNoise()

// ---------------------------------------------------------------------------------------
//  Parsing all intermediate variables for the 2-d time (if called from a     
// "big N" wrapper function in case of HTC alignment mode) is suboptimal, but 
// it is assumed that typically this calculate_tracking_H_matrix() is called standalone;  

int TrKalmanFilter::CalculateHMatrix(KalmanNode *node)
{
  TrKalmanNode *trnode = (static_cast <TrKalmanNode*>(node));

  t_3d_line line/* = parametrize_straight_line*/(trnode->x0->ARR(), trnode->GetZ());

  {
    // '4': yes, no momentum component needed anyway;
    double S[4], arr[2][4][trnode->mDim];

    //SensitiveVolume *sv  = (SensitiveVolume*)trnode->mBackDoorPointer;

    // Calling routine should take care to assign used_hit pointer;
    if (!trnode->mHit) return 0;
 
    //
    // Requirement "knode->fired=1" would disturb the code which attaches
    // new hits; perhaps introduce an extra parameter later and save some 
    // CPU time on this;
    //

    // Calculate actual knode->m[] vector;
    if (trnode->mSensitiveVolume->TrackToHitDistance(&line, trnode->mHit, trnode->m->ARR())) 
      return -1;

    // Then knode->H[][] projector;
    for(int pm=0; pm<2; pm++)
      for(int ip=_X_; ip<=_SY_; ip++) {
	memcpy(S, trnode->x0->ARR(), sizeof(S));

	S[ip] += _drv_steps[ip]*(pm ? 0.5 : -0.5);
    
	//@@@ line = parametrize_straight_line(S, trnode->GetZ());
	t_3d_line qline(S, trnode->GetZ());
	if (trnode->mSensitiveVolume->TrackToHitDistance(&qline, trnode->mHit, arr[pm][ip])) 
	  return -1;
      } //for pm..ip

    for(int im=0; im<trnode->mDim; im++)
      for(int ip=_X_; ip<=_SY_; ip++)
	trnode->H->KFM(im,ip) = -(arr[1][ip][im] - arr[0][ip][im])/_drv_steps[ip];
  }
  // NB: H[0][_QP_] will remain 0 in all cases;

  return 0;
} // TrKalmanFilter::CalculateHMatrix()

// =======================================================================================

#define _FLYSUB_TYPICAL_XY_COORD_ERROR_  (  0.010)
#define _FLYSUB_TYPICAL_SLOPE_ERROR_     (  0.0001)
//#define _FLYSUB_TYPICAL_XY_COORD_ERROR_  (  10.000)
//#define _FLYSUB_TYPICAL_SLOPE_ERROR_     (  1.)
#define _FLYSUB_TYPICAL_MOMENTUM_ERROR_  (  1.000)
#define _FLYSUB_COVARIANCE_BLOWUP_CFF_        (30)

void TrKalmanFilter::ResetNode(TrKalmanNode *node, double S[], int assignmentMode)
{
  // FIXME: next time when want to use this call make cov.matrix parameters 
  // configurable as well;
  //@@@ assert(0);

  unsigned sdim = mFieldMode == NoField ? 4 : 5;

  if (S)
    for(int ip=_X_; ip<sdim; ip++)
      node->x0->KFV(ip) = S[ip];

  // Initialization depends on whether it's a 0-th iteration or not;
  if (assignmentMode != _USE_00_) {
    // Otherwise a normal Kalman filter iterative update;
    KfVector *add = assignmentMode == _USE_XF_ ? node->xf : node->xs;

    for(int ip=_X_; ip<=_SY_; ip++)
      node->x0->KFV(ip) += add->KFV(ip);

    // Momentum expansion point;
    // NB: head->x0[_QP_] will remain 0. in all cases!;
    if (sdim == 5) node->mInversedMomentum += add->KFV(_QP_);
  } //if

  // Yes, predicted state vector at start of chain is set to 0.0;
  for(int ip=_X_; ip<sdim; ip++)
    node->xp->KFV(ip) = 0.;

  // Cook dummy (diagonal) covariance matrix;
  node->CP->Reset();
  // Just [0..3] components;
  for(int ip=_X_; ip<=_SY_; ip++) {
    double diag;

    switch (ip) {
    case _X_:;
    case _Y_:
      diag = _FLYSUB_TYPICAL_XY_COORD_ERROR_;
      break;
    case _SX_:;
    case _SY_:
      diag = _FLYSUB_TYPICAL_SLOPE_ERROR_;
      break;
    default:
      assert(0);
    } //switch
	
    node->CP->KFM(ip,ip) = SQR(diag*_FLYSUB_COVARIANCE_BLOWUP_CFF_);
  } //for ip

  if (sdim == 5)
    node->CP->KFM(_QP_,_QP_) = 
      SQR(_FLYSUB_TYPICAL_MOMENTUM_ERROR_*(node->mInversedMomentum)*
	  _FLYSUB_COVARIANCE_BLOWUP_CFF_);
} // TrKalmanFilter::ResetNode()

// =======================================================================================
