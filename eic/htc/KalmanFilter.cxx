//
// AYK (ayk@bnl.gov)
//
//    Kalman filter class routines; ported from HERMES/OLYMPUS sources; 
//  cleaned up 2014/10/10;
//

#include <cassert>
#include <cstdlib>

#include <Math/DistFunc.h>

#include <Splitter.h>
#include <htclib.h>
#include <KalmanFilter.h>

// =======================================================================================

KalmanFilter::KalmanFilter(int sdim)
{
  // CHECK: this (void*) cast was required by Clang; need to check that the stuff still works;
  //@@@ assert(0);
  // Ok, this apparently kills "nodes" set initialization ...;
  memset((void*)this, 0x00, sizeof(KalmanFilter));

  // NB: do not need xm[] vector calculation usually -> can disable 
  // via SetXmCalculationFlag() call;
  mXmCalculationFlag = true;

  // Just copy over all the stuff;
  sDim  = sdim;

  // Allocate buffer matrices for intermediate calculations;
  SMTX = new KfMatrix(sdim, sdim); 
  SVEC = new KfVector(sdim);        
  QVEC = new KfVector(sdim);        

  mLastFilterPass = KalmanFilter::Undefined;

  // Unity matrix can be global;
  SU = new KfMatrix(sdim, sdim); 
  SU->Unity();

  // Set reasonable default values for these 2 variables ...;
  mMinFilterChiSquareCCDF      = _MIN_FILTER_CHI_SQUARE_CCDF_DEFAULT_;
  mMinSmootherChiSquareCCDF    = _MIN_SMOOTHER_CHI_SQUARE_CCDF_DEFAULT_;

  // ... and for these two as well;
  mMaxFixablePositivityScrewup = _MAX_FIXABLE_POSITIVITY_SCREWUP_DEFAULT_;
  mPositivityCorrelationFix    = _POSITIVITY_CORRELATION_FIX_DEFAULT_;

  // Force CP, CF & CS matrices positivity check at each step; think twice
  // befor making these options configurable -> high-level routines expect that 
  // if Kalman chain succeeded, all nodes have "clean" CS[][], at least; 
  mPositivityCheck = mForceSymmetrization = true;

  // NB: this value may require some task-specific tuning;  
  mRFCutoffValue   = _RF_CUTOFF_DEFAULT_;
} // KalmanFilter::KalmanFilter()

// ---------------------------------------------------------------------------------------
//   Put a new node in a proper place in the chain accoring to it's Z coordinate; 

KalmanNode *KalmanFilter::AddNode(const char *name, double z, int mdim, 
				  const bool nonLinearTransportFlags[2]/*, void *ptr*/)
{ 
  KalmanNode *node = AllocateNode();

  if (!node) return NULL;

  assert(mdim >= 0);
  node->mDim  = mdim;

  // Assign user back-door pointer;
  //node->mBackDoorPointer  = ptr;

  // Assign non-linear transport flags;
  for(unsigned fb=KalmanFilter::Forward; fb<=KalmanFilter::Backward; fb++)
    node->mNonLinearTransportFlags[fb] = nonLinearTransportFlags[fb];

  // Allocate name if given;
  if (name) {
    node->mName = strdup(name);
    if (!node->mName) return NULL;
  } //if

  node->SetZ(z);  

  node->AllocateKfMatrices(sDim);

  // As of Oct'2015 the double-linked list will be created later; just put 
  // everything in the Z-ordered multi-map and return;
  mKalmanNodePool.insert(std::pair<double, KalmanNode*>(z, node));

  return node;
} // KalmanFilter::AddNode()

// ---------------------------------------------------------------------------------------

KalmanNode *KalmanFilter::AddNodeWrapper(const char *name, const char *format, double z, int mdim) 
{
  char buffer[STRING_LEN_MAX];
  // No tricks in this call -> either both FB-directions "true" or both "false";
  bool nonLinearFlags[2] = {NeedNonLinearTransport(z), NeedNonLinearTransport(z)};

  // Construct name if NULL; use provided format and 'z' variable otherwise;
  if (!name) snprintf(buffer, STRING_LEN_MAX-1, format, z);

  return AddNode(name ? name : buffer, z, mdim, nonLinearFlags);
} // KalmanFilter::AddNodeWrapper()

// ---------------------------------------------------------------------------------------

void KalmanFilter::BuildNodeList() 
{
  mHead = mTail = 0;

  // Nodes are Z-ordered in the multi-map; so just go arrange a double-linked list;
  for(std::map<double, KalmanNode*>::iterator it=mKalmanNodePool.begin(); 
      it != mKalmanNodePool.end(); it++) {
    KalmanNode *node = it->second;

    // Part of the nodes may be de-activated on purpose (say, no hit);
    if (!node->IsActive()) continue;

    if (!mHead)
      mHead = node;
    else {
      mTail->SetNext(node);
      node->SetPrev(mTail);
    } //if

    // This is needed since BuildNodeList() can be called more than once;
    node->SetNext(0);

    mTail = node;

    //printf("KalmanFilter::BuildNodeList(): %7.2f %s -> %d %d\n", 
    //	   it->first, node->GetName(), node->IsActive(), node->IsFired());
  } //for it
} // KalmanFilter::BuildNodeList()

// ---------------------------------------------------------------------------------------

int KalmanFilter::Configure(const StringList *config/*, const char *format*/)
{
  // Use presently available nodes to build the linked list;
  BuildNodeList();

  for(const StringList *str=config; str; str=str->mNextString) {
    NodeGroup **gtail = &mNodeGroups;

    // Copy over and save original qstr pointer (help free());
    char *qstr = strdup(str->mString), *qqstr = qstr;

    assert(qqstr);
    qqstr += strlen(_FIRED_RPLANE_PREFIX_);

    // Ok, now parse the string and create node groups; 
    for( ; ; ) {
      char *comma = strchr(qqstr, ',');
	
      // Substring is over --> break;
      if (!*qqstr) break;
      
      // If comma was found, set to 0;
      if (comma) *comma = 0;
      
      {
	StringList **ltail;
	// Create new group;
	NodeGroup *group = *gtail = new NodeGroup();
	
	assert(group);
	
	// Parse group substring; first suffix: min number of fired nodes; 
	{
	  char *semicolon = strchr(qqstr, ':');
	  
	  // Semicolon must be there;
	  if (!semicolon)
	    _RETURN_(-1, "'fired-node-num': no semicolon found!\n");
	  
	  // Determine min number of fired rplanes in this group;
	  group->mFiredNodeNumMin = atoi(semicolon+1);
	  if (group->mFiredNodeNumMin < 0)
	    _RETURN_(-1, "'fired-node-num': value >=0 expected!\n");
	  
	  *semicolon = 0;
	}
	
	// Then group rplane member prefices;
	ltail = &group->mPrefices;
	for( ; ; ) {
	  char *plus = strchr(qqstr, '+');
	
	  // Substring is over --> break;
	  if (!*qqstr) break;
	  
	  // If plus was found, set to 0;
	  if (plus) *plus = 0;
	  
	  {
	    StringList *list = *ltail = new StringList();
	    
	    assert(list);
	    
	    list->mString = strdup(qqstr);
	    assert(list->mString);      
	    
	    ltail = &list->mNextString;
	  }
	  
	  // Substring is over --> break;
	  if (!plus) break;
	  
	  // Switch to the next group substring;
	  qqstr = plus + 1;
	} /*for inf*/
	
	  // And now loop through all nodes and allocate 
	  // those which match one of the prefix patterns
	  // in a linked list;
	{
	  NodeList **ntail = &group->mNodeList;
	  
	  for(KalmanNode *node=mHead; node; node=node->GetNext(KalmanFilter::Forward)) {
	    if (!node->mName) continue;

	    for(StringList *list=group->mPrefices; list; list=list->mNextString)
	      if (!check_prefix(node->mName, list->mString)) {
		NodeList *nlist = *ntail = new NodeList();

		assert(nlist);
		
		group->mAllNodeNum++;
		nlist->mNode = node;
		
		// And fill out back door pointer in node frame;
		node->mNodeGroupNum++;
		node->mNodeGroups = 
		  (NodeGroup**)realloc(node->mNodeGroups,
				       node->mNodeGroupNum*sizeof(void*));
		assert(node->mNodeGroups);
		node->mNodeGroups[node->mNodeGroupNum-1] = group;
		
		ntail = &nlist->mNextNode;
		
		break;
	      } //for list .. if
	  } //for node
	} 

	// Sanity check, please;
	//printf("%d %d\n", group->mFiredNodeNumMin, group->mAllNodeNum);
	if (group->mFiredNodeNumMin > group->mAllNodeNum)
	  _RETURN_(-1, "'fired-node-num': limit too high!\n");
	
	gtail = &group->mNextGroup;
      } 

      // Substring is over --> break;
      if (!comma) break;

	// Switch to the next group substring;
      qqstr = comma + 1;
    } //for inf

    free(qstr);
  } //for str

  if (!mNodeGroups) 
    _RETURN_(-1, "'--kalman-tuning fired-node-min' option missing!\n");

#if _LATER_
  // Well, it looks this code is of interest for the generic Kalman filter as well;
  {
    // Loop through all nodes and fill "too big" gaps in Z; 
    // NB: this call should be done AFTER parser (see above);
    if (mNodeGapMax)
      for(KalmanNode *knode=mHead; knode && knode->GetNext(KalmanFilter::Forward); 
	  knode=knode->GetNext(KalmanFilter::Forward))
      {
	double zz = knode->GetZ() + mNodeGapMax;

	// Well, if there is no magnetic field inbetween, no sense 
	// to introduce extra nodes; unless this is forced by respective 
	// mode bit;
	if (mode & _FILL_ALL_GAPS_ || knode->mNonLinearTransportFlags[KalmanFilter::Forward] ||
	    knode->GetNext(KalmanFilter::Forward)->mNonLinearTransportFlags[KalmanFilter::Backward])
	{
	  if (zz < knode->GetNext(KalmanFilter::Forward)->GetZ() && 
	      !AddNodeWrapper(NULL, format, zz, 0, NULL)) 
	    return -1; 
	} /*if*/
      } /*if..for knode*/
  }
#endif

  return 0;
} // KalmanFilter::Configure()

// ---------------------------------------------------------------------------------------

void KalmanFilter::ResetFiredFlags()
{
  // Consider to reset ALL node fired flags, even those who are not presently included 
  // in the actually used double-linked list;
  for(std::map<double, KalmanNode*>::iterator it=mKalmanNodePool.begin(); 
      it != mKalmanNodePool.end(); it++) 
    // FIXME: yes, for now do it by hand; group counters will be reset below; 
    // see node->ResetFiredFlag() source code for more details;
    it->second->mFired = false; 

  // Reset group fired counters;
  for(NodeGroup *group=mNodeGroups; group; group=group->mNextGroup)
    group->mFiredNodeNum = group->mNdfControlFlag = 0;  
} // KalmanFilter::ResetFiredFlags() 

// ---------------------------------------------------------------------------------------

void KalmanFilter::LatchGroupNdfControlFlags()
{
  for(NodeGroup *group=mNodeGroups; group; group=group->mNextGroup) 
    if (group->mFiredNodeNum) group->mNdfControlFlag = 1;
} // KalmanFilter::LatchGroupNdfControlFlags()

// ---------------------------------------------------------------------------------------

//
//  FIXME: this hack indeed assumes a single global node group;
//

void KalmanFilter::HackGroupHitCountLimit(unsigned min)
{
  for(NodeGroup *group=mNodeGroups; group; group=group->mNextGroup) 
    group->mFiredNodeNumMin = min;
} // KalmanFilter::HackGroupHitCountLimit()

// =======================================================================================

unsigned KalmanFilter::FilterPass(KalmanNode *start, KalmanNode *end, KalmanFilter::Direction fb)
{
  int ret;
  // If 'start' pointer is specified, use it; otherwise take head/tail; 
  KalmanNode *snode = start ? start : (fb == KalmanFilter::Forward ? mHead : mTail);

  mLastFilterPass = KalmanFilter::Undefined;

  // Save start/end node pointers for the (possible) smoother pass; 
  mLastStart = snode;
  mLastEnd   = end ? end : (fb == KalmanFilter::Forward ? mTail : mHead);

  // Calculate x0[], FR[][], Q[][] for all requested nodes; NB: it is 
  // however assumed that snode->x0[] is given correctly;
  if ((ret = Calculate_x0_FR_Q(snode, end, fb, 
			       _CALCULATE_DERIVATIVES_|_CALCULATE_PROCESS_NOISE_))) 
    return ret; 

  // Do Kalman filter matrix calculations;
  if ((ret = DoFilterAlgebra(snode, end, fb))) return ret; 

  // Make smoother aware of the filter arrays status; 
  mLastFilterPass = fb;

  CalculateFilterStat();

  return 0;
} // KalmanFilter::FilterPass()

// ---------------------------------------------------------------------------------------

unsigned KalmanFilter::Calculate_x0_FR_Q(KalmanNode *start, KalmanNode *end, 
					 KalmanFilter::Direction fb, unsigned mode)
{                   
  for(KalmanNode *node=start; node; node=node->GetNext(fb)) {           
    if (node != start) {           
      KalmanNode *from = node->GetPrev(fb);

      // Extrapolate x0[]; depends on the transport non-linearity 
      // (magnetic field presence in tracking case);
      if (node->GetZ() == from->GetZ()) {
	node->x0->CopyFrom(from->x0);

	from->FR = SU; 
      }
      else {
	if (node->mNonLinearTransportFlags[(fb+1)%2] || 
	    from->mNonLinearTransportFlags[ fb     ]) {
	  if (Transport(from, fb, mode)) 
	    _RETURN_(_TFUN_FAILURE_, "Filter failed in root->transport()!\n");
	  
	  from->FR = from->FM;
	}
	else {
	  // Just use pre-calculated FF[][]-matrix for the pure linear case;
	  node->x0->SetToProduct(from->FF[fb], from->x0);
	  
	  //assert(0);
	  from->FR = from->FF[fb]; 
	} //if
      } //if

      // There may be extra stuff, application-specific (say, multiple scattering 
      // calculation in case of tracking Kalman Filter;
      if (TransportExtra(from, fb, mode)) 
	  _RETURN_(_NFUN_FAILURE_, "Filter failed in root->extra()!\n");
    } //if

    if (end && node == end) break;
  } //for node

  return 0;
} // KalmanFilter::Calculate_x0_FR_Q() 

// ---------------------------------------------------------------------------------------

unsigned KalmanFilter::DoFilterAlgebra(KalmanNode *start, KalmanNode *end, 
				       KalmanFilter::Direction fb) 
{                      
  // Calculate everything (but tnode->x0[]) in one pass; 
  for(KalmanNode *node=start; node; node=node->GetNext(fb)) {         
    // All the stuff which makes sense only if there were 
    // calculated nodes before in the chain;     
    if (node != start) {
      KalmanNode *from = node->GetPrev(fb);

      node->xp->SetToProduct(from->FR, from->xf);

      node->CP->SetToProduct(from->FR, from->CF, from->FR, _TRANSPOSE_IN3_);

      node->CP->Add(from->Q);

      if (mPositivityCheck && node->CP->CheckPositivity())
	_KF_RETURN_(_DSINV_FAILURE_, 
		    "node->CP is not positive-definite (filter)!\n", KalmanFilter::Error);
      if (mForceSymmetrization) node->CP->ForceSymmetric();
    } //if

    // If node was fired (has it's own valuable measurement), 
    // usual Kalman technique is applied;
    if (node->mFired) {
      // Calculate H-matrix (and perhaps m[]) if root->hfun() != NULL;
      // yes, it looks like this is needed for fired nodes only;
      if (CalculateHMatrix(node)) 
	_RETURN_(_HFUN_FAILURE_, "Filter failed in node->hfun()!\n");

      // Prediction cov.matrix and Kalman gain matrix calculation;
      node->MMTX->SetToProduct(node->H, node->CP, node->H, _TRANSPOSE_IN3_);
      node->RPI->SetToSum(node->MMTX, node->V); 
      if (mForceSymmetrization) node->RPI->ForceSymmetric();
      if (node->RPI->Invert(KfMatrix::Symmetric)) 
	_RETURN_(_DSINV_FAILURE_, "DSINV() failed (filter) #1!\n");
      node->K->SetToProduct(node->CP, node->H, node->RPI, _TRANSPOSE_IN2_);

      // Prediction error; state vector update; 
      node->ep->SetToProduct(node->H, node->xp);
      node->ep->SetToDifference(node->m, node->ep);
      SVEC->SetToProduct(node->K, node->ep);
      node->xf->SetToSum(node->xp, SVEC);

      // Filtered covariance matrix update; since now use De Jong smoother 
      // formalism, which requires "I-KH" matrix anyway, makes sense to 
      // introduce it as intermediate LB[][] matrix right here;
      SMTX->SetToProduct(node->K, node->H);
      node->LB->SetToDifference(SU, SMTX);
      node->CF->SetToProduct(node->LB, node->CP, node->LB, _TRANSPOSE_IN3_);
      SMTX->SetToProduct(node->K, node->V, node->K, _TRANSPOSE_IN3_);
      node->CF->Add(SMTX);
      if (mPositivityCheck && node->CF->CheckPositivity())
	_KF_RETURN_(_DSINV_FAILURE_, 
		    "node->CF is not positive-definite (filter)!\n", KalmanFilter::Error);
      if (mForceSymmetrization) node->CF->ForceSymmetric();

      // The rest in principle makes sense if only there were already sufficient 
      // fired nodes passed before; there is however a complication that say for 
      // non-fixed-momentum tracking and weak fringe field 5-th fired node can be 
      // still out of strong field, so mNdf>0 check is perhaps not the best criterion; 
      // since these days node->chi2_filter_increment is actually used to calculate 
      // overall filter pass chi^2 sum (and not to determine outlier
      // nodes), one can simply set a cut on RF[0][0] (assuming 
      // mdim=1) and forget about the problem;   
      {
	// Covariance matrix of filtered residuals; 
	node->RF->SetToProduct(node->H, node->CF, node->H, _TRANSPOSE_IN3_);
	node->RF->SetToDifference(node->V, node->RF);
	if (mForceSymmetrization) node->RF->ForceSymmetric();

	bool ok = true;

	// Check that there are no "too small" cov.matrix diagonal elements;
	for(unsigned iq=0; iq<node->GetMdim(); iq++) 
	  if (node->RF->KFM(0,0) <= mRFCutoffValue) {
	    ok = false;
	    break;
	  } //for iq..if 

	if (ok) {
	  // Filtered residuals; 
	  node->rf->SetToProduct(node->H, node->xf);
	  node->rf->SetToDifference(node->m, node->rf);

	  // Calculate chi^2 stuff; 
	  node->MMTX->CopyFrom(node->RF);
	  if (node->MMTX->Invert(KfMatrix::Symmetric)) 
	    _KF_RETURN_(_DSINV_FAILURE_, "DSINV() failed (filter) #2!\n", KalmanFilter::Error);
	  node->rf->VectorLengthSquare(node->MMTX, &node->mFilterChiSquareIncrement);
	}
	else {
	  node->rf->Reset();
	  node->mFilterChiSquareIncrement = 0.;
	} //if
      }
    } 
    else
    {
      // Yes, just predicted values in this case;
      node->xf->CopyFrom(node->xp);
      node->CF->CopyFrom(node->CP);

      // Effectively this means that Kalman gain is 0;
      node->LB->Unity();
    } //if

    if (end && node == end) break;
  } //for node

  return 0;
} // KalmanFilter::DoFilterAlgebra()

// ---------------------------------------------------------------------------------------

int KalmanFilter::CalculateFilterStat()
{                      
  if (mLastFilterPass == KalmanFilter::Undefined) return -1;

  mFilterChiSquare = 0.; mNdf = -sDim + mExtraNdfCount;

  for(KalmanNode *node=mLastStart; node; node=node->GetNext(mLastFilterPass)) {
    if (node->mFired) {
      // Ok, this is clear; fix 2009/09/25; fired _PIXEL_ node
      // for instance will increment ndf by 2;
      mNdf += node->mDim;

      //printf("%s %f -> %f\n", node->GetName(), node->GetZ(), node->mFilterChiSquareIncrement);
      mFilterChiSquare += node->mFilterChiSquareIncrement;
    } //if

    if (node == mLastEnd) break;
  } //for node
  //exit(0);

  // Prefer to always calculate this value;
  mFilterChiSquareCCDF = mNdf > 0 ? ROOT::Math::chisquared_cdf_c(mFilterChiSquare, mNdf) : 0.;

  return 0;
} // KalmanFilter::CalculateFilterStat()

// =======================================================================================

int KalmanFilter::SmootherPass()
{
  // This variable determines *smoother* direction; NB: it is per definition 
  // opposite to the filter direction which is supposed to be ran before;
  // and indeed smoother starts at the node where filter finished its work;
  int fb;

  switch (mLastFilterPass) {
    // Yes, just return _CHAIN_FAILURE_ here, who cares;
  case KalmanFilter::Undefined: 
    return _CHAIN_FAILURE_;
  case KalmanFilter::Forward:
    fb = KalmanFilter::Backward;
    break;
  case KalmanFilter::Backward:
    fb = KalmanFilter::Forward;
    break;
  default:
    assert(0);
  } //switch

  mWorstSmootherNode = mWorstResettableSmootherNode = NULL;

  // Loop through all the start-end chain in [fb] direction;
  for(KalmanNode *node=mLastEnd; node; node=node->GetNext(fb)) {
    // This initialization is correct for root->mLastEnd node as well;
    if (node->mFired) {
      node->QQ->SetToProduct(node->H, node->RPI, node->H, _TRANSPOSE_IN1_);
      if (mForceSymmetrization) node->QQ->ForceSymmetric();
      node->qq->SetToProduct(node->H, node->RPI, node->ep, _TRANSPOSE_IN1_);      
    }
    else {
      node->QQ->Reset();
      node->qq->Reset();
    } //if

    if (node == mLastEnd) {
      node->CS->CopyFrom(node->CF);
      node->xs->CopyFrom(node->xf);
    }
    else {
      KalmanNode *prev = node->GetPrev(fb);

      // Accomplish node->L[][] calculation; NB: do NOT follow original
      // Merkus-Pollock-Vos node counting scheme; namely calculate 
      // L-matrix for CURRENT node, not for the PREVIOUS one; thus node->L
      // is used in the below formulae instead of prev->L;
      node->L->SetToProduct(node->FR, node->LB);

      // Calculate smoothed covariance matrix; use De Jong formalism 
      // (no smoother gain matrix usage), since it is indeed more 
      // numerically stable (no CP-matrix inversion needed);
      SMTX->SetToProduct(node->L, prev->QQ, node->L, _TRANSPOSE_IN1_);
      node->QQ->Add(SMTX);
      if (mForceSymmetrization) node->QQ->ForceSymmetric();
      SMTX->SetToProduct(node->CP, node->QQ, node->CP);
      node->CS->SetToDifference(node->CP, SMTX);
      if (mPositivityCheck && node->CS->CheckPositivity()) {
	// Append positivity fix attempt to the overall mask;
	mExtraReturnBits |= _POSITIVITY_FIX_;

	if (mVerbosityLevel >= KalmanFilter::Warning)
	  printf("CS[][] positivity check failed at Z=%7.2f\n", node->GetZ());

	// Actually try to fix the problem; assume that if off-diagonal 
	// correlation coefficients are a bit higher than 1.0 (happens 
	// rarely for high-momenta tracks), this is caused by certain 
	// numerical instability and is probably not that harmful; 
	// if fail, return right here; NB: CheckPositivity() will 
	// be called by positivity_fix() internally;
	if (node->CS->FixPositivity(mMaxFixablePositivityScrewup, 
			   mPositivityCorrelationFix)) {
	  if (mVerbosityLevel >= KalmanFilter::Error) {
	    node->CS->Print();
	    node->CS->CorrelationPrint();
	  } //if

	  _KF_RETURN_(_DSINV_FAILURE_, 
		      "node->CS is not positive-definite (smoother)!\n", KalmanFilter::Error);
	} //if
      } //if
      if (mForceSymmetrization) node->CS->ForceSymmetric();

      // Calculate smoothed state vector;
      SVEC->SetToProduct(node->L, prev->qq, _TRANSPOSE_IN1_);
      node->qq->Add(SVEC);
      SVEC->SetToProduct(node->CP, node->qq);
      node->xs->SetToSum(node->xp, SVEC);
    } //if

    // Perform xm[] calculation if needed; (12d) & (12e) in original 1987 paper;
    if (mXmCalculationFlag) {
      node->CM->CopyFrom(node->CS);
      node->CM->Invert(KfMatrix::Symmetric);

      node->MMTX->CopyFrom(node->V);
      node->MMTX->Invert(KfMatrix::Symmetric);

      // Use inverted smoother cov.matrix to calculate (12d);
      SVEC->SetToProduct(node->CM, node->xs);
      QVEC->SetToProduct(node->H, node->MMTX, node->m, _TRANSPOSE_IN1_);
      SVEC->Subtract(QVEC);

      SMTX->SetToProduct(node->H, node->MMTX, node->H, _TRANSPOSE_IN1_);
      
      node->CM->Subtract(SMTX);
      node->CM->Invert(KfMatrix::Symmetric);

      node->xm->SetToProduct(node->CM, SVEC);

      // And respective residuals;
      node->rm->SetToProduct(node->H, node->xm);
      node->rm->SetToDifference(node->m, node->rm);

      // And eventually project node->CM onto the node space;
      node->RM->SetToProduct(node->H, node->CM, node->H, _TRANSPOSE_IN3_);
    } //if

    // If node was dead for this event, skip all the rest; 
    if (!node->mFired) continue;
    
    // Smoothed residuals; 
    node->rs->SetToProduct(node->H, node->xs);
    node->rs->SetToDifference(node->m, node->rs);

    // Covariance matrix of smoothed residuals;
    node->RS->SetToProduct(node->H, node->CS, node->H, _TRANSPOSE_IN3_);
    //node->RS->Print();
    node->RS->SetToDifference(node->V, node->RS);
    if (mForceSymmetrization) node->RS->ForceSymmetric();
    
    // Smoothed chi^2; 
    node->MMTX->CopyFrom(node->RS);
    if (node->MMTX->Invert(KfMatrix::Symmetric)) {
      if (mVerbosityLevel >= KalmanFilter::Error)
	printf("  --> Z=%7.1f, RS[0][0] = %20.15f\n", node->GetZ(), node->RS->KFM(0,0));
      _KF_RETURN_(_DSINV_FAILURE_, "DSINV() failed (smoother)!\n", KalmanFilter::Error);
    } //if
    node->rs->VectorLengthSquare(node->MMTX, &node->mSmootherChiSquare);
    if (node->mSmootherChiSquare <= 0.0) {
      //printf("%f %f %f %f\n", node->MMTX->KFM(0,0), node->MMTX->KFM(1,0), 
      //     node->MMTX->KFM(0,1), node->MMTX->KFM(1,1));
      //node->V->Print();
      printf("%f: smother chi^2 = %f\n", node->GetZ(), node->mSmootherChiSquare);
      //exit(0);
    }
    // FIXME: use this value in all places rather than calculating it every time new;
    node->mSmootherChiSquareCCDF = 
      ROOT::Math::chisquared_cdf_c(node->GetSmootherChiSquare(), node->GetMdim());

    // Assign worst knode pointers;
    if (!mWorstSmootherNode || 
	node->mSmootherChiSquare > 
	mWorstSmootherNode->mSmootherChiSquare)
      mWorstSmootherNode = node;
    if (!mWorstResettableSmootherNode || 
	node->mSmootherChiSquare > 
	mWorstResettableSmootherNode->mSmootherChiSquare) {
      // Check that knode can be reset;
      int gr, resettable = 1;

      for(gr=0; gr<node->mNodeGroupNum; gr++) {
	NodeGroup *group = (NodeGroup*)node->mNodeGroups[gr];

	if (group->mNdfControlFlag && 
	    group->mFiredNodeNum <= group->mFiredNodeNumMin) {
	  resettable = 0;
	  break;
	} //if
      } //for gr

      if (resettable) mWorstResettableSmootherNode = node;
    } //if

    // Really so?;
    if (node == mLastStart) break;
  } //for node

  return 0;
} // KalmanFilter::SmootherPass()

// =======================================================================================

unsigned KalmanFilter::FullChain(KalmanNode *start, 
				 KalmanNode *end, KalmanFilter::Direction fb, int mode)
{
  unsigned ret, outlier_order_violation = 0;

  // Sanity check; do not mind to do it every time and set special bit;
  if (((mode & _TRUST_SMOOTHER_FCN_) && !mMinSmootherChiSquareCCDF) ||
      ((mode & _TRUST_FILTER_FCN_)   && !mMinFilterChiSquareCCDF))
    return _CHAIN_FAILURE_;

  // Initialize few global root frame variables;
  mChainRejectedNodeNum = mExtraReturnBits = 0;

  // Run forward filter; if failed, return immediately;
  if ((ret = FilterPass(start, end, fb))) _CHAIN_RETURN_(ret);

  // Then iteratively run smoother, remove worst outlier and rerun 
  // forward filter starting from the removed node;
  for( ; ; )
  {
    // If smoother failed, return failure code immediately; 
    if ((ret = SmootherPass())) _CHAIN_RETURN_(ret|outlier_order_violation);
    
    // Check that have enough fired nodes at start up; but do it after 
    // smoother so that some track fit is available anyway;
    {
      // NB: assume that if there were no hits in this group at all,
      // this is NOT a failure (for instance short HERMES tracks may
      // have no BC3/4 hits at all; or long tracks may have no MC hits); 
      // this is a ~hack, I admit; and actually this loop should be 
      // run only once, since 'group->mFiredNodeNum' can not go below
      // 'group->mFiredNodeNumMin' in the above infinite loop;
      for( NodeGroup *group=mNodeGroups; group; group=group->mNextGroup) {
	//printf("%d fired nodes ...\n", group->mFiredNodeNum);

	if (group->mNdfControlFlag && 
	    group->mFiredNodeNum < group->mFiredNodeNumMin)
	  // So if _NDF_FAILURE_ bit is set and _WORST_NODE_IMMUTABLE_
	  // is not, this means start-up failure here, right?;
	  _CHAIN_RETURN_(_NDF_FAILURE_);
      } //for group
    } 
    
    // Ok, so now take a decision; looks tricky; 
    {
      double worst_smoother_prob = 
	ROOT::Math::chisquared_cdf_c(mWorstSmootherNode->mSmootherChiSquare, 
				     mWorstSmootherNode->mDim);

      // The only way to return 0 code; both overall filter chi^2
      // and worst node smoother chi^2 are either within limits or
      // should not be trusted at all;
      if ((worst_smoother_prob > mMinSmootherChiSquareCCDF || 
	   !(mode & _TRUST_SMOOTHER_FCN_)) &&
	  (mFilterChiSquareCCDF > mMinFilterChiSquareCCDF || 
	   !(mode & _TRUST_FILTER_FCN_)))
	_CHAIN_RETURN_(outlier_order_violation);

      // Ok, then something is not good; if smoother chi^2 is trusted
      // and there is a bad node, outlier removal procedure will 
      // continue, no care needed; if filter chi^2 is trusted, it is
      // a bit more complicated since smoother chi^2 may also be 
      // trusted and there may be no bad nodes left;
      if (mFilterChiSquareCCDF < mMinFilterChiSquareCCDF &&
	  (mode & _TRUST_FILTER_FCN_) &&
	  worst_smoother_prob > mMinSmootherChiSquareCCDF &&
	  (mode & _TRUST_SMOOTHER_FCN_))
	_CHAIN_RETURN_(_STRUST_FAILURE_|outlier_order_violation);
    } 

    // NB: there can be no node which can be removed --> 
    // then return immediately; and set both bits;
    if (!mWorstResettableSmootherNode)
      _CHAIN_RETURN_(_NDF_FAILURE_|_WORST_NODE_IMMUTABLE_);

    // Check that node to be removed is really the worst one;
    if (mWorstResettableSmootherNode != 
	mWorstSmootherNode)
      outlier_order_violation = _WORST_NODE_IMMUTABLE_;

    // Turn worst node off and redo forward filtering algebra 
    // starting from this node; NB: no need to recalculate 
    // x0[], FR[][] & Q[][] --> save CPU time;
    if (mVerbosityLevel >= KalmanFilter::Info)
      printf("chi^2 = %8.2f -> prob %10.7f -> removing %s (chi^2 incr %8.2f)\n", 
	     mFilterChiSquare, mFilterChiSquareCCDF, mWorstResettableSmootherNode->mName,
	     mWorstSmootherNode->mSmootherChiSquare);
    mWorstResettableSmootherNode->ResetFiredFlag();
    mChainRejectedNodeNum++;

    // Repeat Kalman filter matrix calculations starting from 
    // the removed outlier node;
    if ((ret = DoFilterAlgebra(mWorstResettableSmootherNode, end, fb)))
      _CHAIN_RETURN_(ret|outlier_order_violation);

    // Recalculate filter ndf & chi^2 from scratch;
    CalculateFilterStat();
  } //for inf
} // KalmanFilter::FullChain()

// =======================================================================================
