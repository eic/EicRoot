//
// AYK (ayk@bnl.gov), 2014/02/03
//
//  An attempt to port HERMES & OLYMPUS forward tracking KF code to EicRoot;
//

#include <assert.h>
#include <iostream>

#include <TGeoManager.h>
#include <TGeoVolume.h>
#include <TGeoNode.h>
#include <TGeoMaterial.h>
#include <TGeoNavigator.h>
#include <TClonesArray.h>
#include <Math/DistFunc.h>

#include <FairRunAna.h>
#include <FairField.h>

#include <PndTrack.h>

#include <3d.h>
#include <htclib.h>
#include <TrKalmanFilter.h>
#include <SensitiveVolume.h>
#include <MediaBank.h>
#include <MediaSliceArray.h>

#include <EicHtcTask.h>
#include <EicTrackingDigiHit.h>
#include <EicGeoParData.h>
#include <EicRunAna.h>

using namespace std;

#define GCBANK_SIZE 30000000

typedef struct {
    int geant[GCBANK_SIZE];
} GCBANK_DEF;
#define GCBANK COMMON_BLOCK(GCBANK, gcbank)
COMMON_BLOCK_DEF(GCBANK_DEF, GCBANK);

GCBANK_DEF GCBANK;

// ---------------------------------------------------------------------------------------

EicHtcTask::EicHtcTask(EicIdealTrackingCode *ideal, MfieldMode mode):
  FairTask("EIC HTC Task")
{
  ResetVars();

  mKalmanFilter = new HtcKalmanFilter(mode);

  mIdealTrCode = ideal;

  mFitTrackArray = new TClonesArray("PndTrack");
} // EicHtcTask::EicHtcTask()

// ---------------------------------------------------------------------------------------

// --> FIXME!;
#define _MG_CELL_SIZE_  ( 2.0)
#define _MG_WIDTH_      (80.0)
//#define _MG_CELL_SIZE_  (  2.0)
//#define _MG_WIDTH_      (240.0)

MgridSlice *HtcKalmanFilter::InitializeMgridSlice(double z0)
{
  MgridSlice **pslice;

  // Loop through already defined slices; 
  for(pslice=&mMgslices; *pslice; pslice=&(*pslice)->mNext) ;

  MgridSlice *slice = *pslice = new MgridSlice();
  slice->mZ0 = z0;

  // Create empty mgrid; NB: HRC-mimic would be {4, 3, 1} - if full grid!;
  int dim[3];
  double min[3], max[3];
  // Yes, prefer to use a "normal" Z direction (no fake tricks); 
  MgridDirection *fdir[3];
  CoordSystem *csystem = new CoordSystem(_CARTESIAN_, 3, XYZ);
  CoordSystem *fsystem = new CoordSystem(_CARTESIAN_, 3, XYZ);
    
  assert(csystem && fsystem);

  for(int xy=_X_; xy<=_Y_; xy++)
  {
    min[xy] = -_MG_WIDTH_/2.;
    max[xy] =  _MG_WIDTH_/2.;
    
    // Use hardcoded cell size and "window" size; well, two numbers
    // should divide well; fix later;
    dim[xy] = (int)rint(_MG_WIDTH_/_MG_CELL_SIZE_);
  } /*for xy*/

  // Reset Z direction; Z-thickness does not matter, clear;
  dim[_Z_] = 1; 
  min[_Z_] = z0 - 1./2.; max[_Z_] = z0 + 1./2.;

  // Create direction frames; NB: NO fake Z!;
  for(int ik=0; ik<3; ik++)
    fdir[ik] = new MgridDirection(dim[ik], min[ik], max[ik]);

  // Eventually create and initialize mgrid;
  slice->mGrid = create_single_mgrid_header((char*)"DUMMY", csystem, fsystem, 
					    fdir, _FIELD_COMPONENT_VALUES_);
  if (!slice->mGrid || slice->mGrid->initializeAsSingleMgrid(0)) 
  {
    printf("\n !!! Most likely you ran out of memory !!!\n\n");
    fflush(stdout);
    return NULL;
  } /*if*/

  // Set desired interpolation mode;
  if (slice->mGrid->setHtcInterpolationMode(&RK_htci)) return NULL;

  // Fill it right here; may take few seconds if step is small;
  for(unsigned ip=0; ip<slice->mGrid->getCellNum(); ip++)
  {
    TVector3 xx;
    MgridCell *cell = slice->mGrid->linearAddrToCellPtr(ip);

    assert(cell);
    assert(!slice->mGrid->linearAddrToCoord(ip, xx));

    // --> FIXME: use KF-axis-aligned magnetic field routine;
    {
      TVector3 bff;

      if (!KalmanFilterMagneticField(xx, bff)) slice->mGrid->markCellAsSafe(ip);

      for(unsigned ip=0; ip<3; ip++)
	cell->B[ip] = bff[ip];
    }
    //if (!kalmanFilterMagneticField(xx, cell->B)) slice->mgrid->markCellAsSafe(ip);
    //printf("@@@ %4d: %f %f %f\n", ip, cell->B[0], cell->B[1], cell->B[2]);
  } /*for ip*/

  return slice;
} // HtcKalmanFilter::InitializeMgridSlice()

// ---------------------------------------------------------------------------------------

void EicHtcTask::SetMediaScanThetaPhi(double theta, double phi) 
{
  double rtheta = theta*TMath::Pi()/180., rphi = phi*TMath::Pi()/180.;

  mMediaScanDirection = 
    TVector3(sin(rtheta)*cos(rphi), sin(rtheta)*sin(rphi), cos(rtheta));
} // EicHtcTask::SetMediaScanThetaPhi()

// ---------------------------------------------------------------------------------------

MediaBank *EicHtcTask::ConfigureMediaBank()
{
  // No length limit; KF axis off (0,0,0) along (0,0,1);
  t_3d_line zaxis(TVector3(0.0, 0.0, 0.0), TVector3(0.0, 0.0, 1.0)); 
  MediaBank *mbank = new MediaBank(zaxis, 0.0);

  // Some configurability in scan axis;
  t_3d_line saxis(TVector3(0.0, 0.0, 0.0), mMediaScanDirection);
  mbank->SetScanLine(saxis);

  return mbank;
} // EicHtcTask::ConfigureMediaBank()

// ---------------------------------------------------------------------------------------

//
// FIXME: take axis slope and boundary epsilon into account!;
//

int EicHtcTask::PerformMediaScan()
{
  printf("Starting media scan ...\n");

  // Set up KF axis and media scan axis; 
  mMediaBank = ConfigureMediaBank();

  TGeoNavigator *navi = (TGeoNavigator *)gGeoManager->GetListOfNavigators()->At(0);
  
  // Initialize gGeoManager location & direction to perform the scan;
  {
    double x0[3], nx0[3];

    for(unsigned iq=0; iq<3; iq++) {
      x0 [iq] = mMediaBank->GetScanLine(). x[iq];
      nx0[iq] = mMediaBank->GetScanLine().nx[iq];
    } //for iq

    gGeoManager->SetCurrentPoint    ( x0);
    gGeoManager->SetCurrentDirection(nx0);
  }

  for(TGeoNode *node = gGeoManager->GetCurrentNode(); ; ) {
    mMediaBank->StartNextLayer(node->GetVolume()->GetMaterial(), gGeoManager->GetCurrentPoint());

    gGeoManager->FindNextBoundary();
    mMediaBank->SetCurrentLayerThickness(gGeoManager->GetStep());

    node = gGeoManager->Step();
    assert(gGeoManager->IsEntering());
    // Out of volume or out of tracker -> break;
    if (mMediaBank->IsOutOfRange() || gGeoManager->IsOutside()) break;
  } //for inf

  mMediaBank->Print(); //exit(0);

  return 0;
} // EicHtcTask::PerformMediaScan()

// ---------------------------------------------------------------------------------------

int EicHtcTask::DeclareSensitiveVolumes()
{  
  TIter next( gGeoManager->GetListOfVolumes() );
  TGeoVolume *volume;

  while ((volume=(TGeoVolume*)next())) {
    TObjArray *arr = volume->GetNodes(); if (!arr) continue;

    for(unsigned iq=0; iq<arr->GetEntriesFast(); iq++) {
      TGeoNode *node = (TGeoNode*)arr->At(iq);

      // FIXME: this should be optimized;
      for(unsigned gr=0; gr<mIdealTrCode->fGroups.size(); gr++) {
	EicDetectorGroup *dgroup = &mIdealTrCode->fGroups[gr];
  
	LogicalVolumeLookupTableEntry *lNode = dgroup->mGptr->GetLookupTableNode(node);
	if (!lNode) continue;

	//printf("  %s\n", node->GetName());
	
	// Figure out Z-coordinate;
	double local[3] = {0,0,0};
	TVector3 global = LocalToMaster(lNode->mGeoMtx, local);
	// FIXME: this is a hack!;
	double z0 = global[2];
	
	SensitiveVolume *sv = new SensitiveVolume(lNode, node, z0);

	for(unsigned nd=0; nd<dgroup->mDigi->mKfNodeTemplates.size(); nd++) {
	  EicKfNodeTemplate *kftmpl = dgroup->mDigi->mKfNodeTemplates[nd];

	  {
	    char buffer[128];

	    // FIXME: naming scheme will be screwed up for say phi-segmented disks (?); 
	    snprintf(buffer, 128-1, "%s-%02d-%02d", dgroup->dname->Name().Data(), 
		     dgroup->svCounter, nd);
  
	    // FIXME: later may want to take KF template transformation into account and correct
	    // z0 value accordingly (say project something on scan axis direction);
	    KalmanNode *kfnode = mKalmanFilter->AddNodeWrapper(buffer, NULL, z0, kftmpl->GetMdim());
	    // FIXME: should better overload AddNodeWrapper I guess?;
	    dynamic_cast<TrKalmanNode*>(kfnode)->SetSensitiveVolume(sv);
	    assert(kfnode);
	    
	    printf("%s -> %7.2f cm; %p\n", buffer, kfnode->GetZ(), kftmpl);
	    sv->mKfNodeWrappers.push_back(KalmanNodeWrapper(kfnode, kftmpl, sv->mLogicalNode->mGeoMtx));
	  }
	} //for nd
    
	dgroup->mSensitiveVolumes[lNode] = sv;

	// Increment sensitive volume and KF node counters;
	dgroup->svCounter++;
	
	break;
      } //for gr
    } //for iq
  } //while
  //exit(0);

  return 0;
} // EicHtcTask::DeclareSensitiveVolumes()

// ---------------------------------------------------------------------------------------

//
// THINK: get rid of this call once debugging is finished?;
//

int HtcKalmanFilter::KalmanFilterMagneticField(TVector3 &xx, TVector3 &B)
{
  double x[3] = {xx[0], xx[1], xx[2]}, BB[3];
  FairField *field = FairRunAna::Instance()->GetField();

  field->GetFieldValue(x, BB);

  for(unsigned iq=0; iq<3; iq++)
    B[iq] = BB[iq];

  return 0;
} // HtcKalmanFilter::KalmanFilterMagneticField()

// ---------------------------------------------------------------------------------------

unsigned EicHtcTask::GetMaxPossibleHitCount()  const
{
  unsigned counter = 0;

  // FIXME: for now assume each location can have at most 1 hit; if ever want to delegate 
  // this call to FwdTrackFinder be aware, that respective gr_counter loop in FwdTrackFinder::Init()
  // at present occurs *after* EicHtcTask::Init(), so will have to split 
  // EicHtcTask::ConfigureKalmanFilter() or something like that;
  for(TrKalmanNodeLocation *location=GetKalmanFilter()->GetLocationHead(); location;
      location=location->GetNext(KalmanFilter::Forward)) {

    // FIXME: yet may want to delegate this call to TrKalmanNodeLocation class (so that 
    // it is not exclusively based on whether it has sensitive volumes or not);
    //if (location->HasSensitiveVolumes()) counter++;
    //printf("%f -> %d\n", location->GetZ(), location->GetSensitiveVolumeNodeWrapperCount());
    counter += location->GetSensitiveVolumeNodeWrapperCount();//HasSensitiveVolumes()) counter++;
  } //for location

  return counter;
} // EicHtcTask::GetMaxPossibleHitCount()

// ---------------------------------------------------------------------------------------

int EicHtcTask::ConfigureKalmanFilter()
{
#if 1
  //mKalmanFilter->SetNodeGapMax(10.0);
#endif

  // NB: want to use GetMaxPossibleHitCount() below -> this call has to happen first;
  GetKalmanFilter()->SetUpLocations();

  // As of 2015/11/07 only a single entry left; yet preserve the scheme like it used to be;
  StringList *str = new StringList();
  bool equal_flag = true;
  // --> FIXME: buffer ~overflow; TString?;
  char buffer[16384] = "fired-node-min";
  // This _OLD_ stuff may become needed if ever want to place individual 
  // group limits on missing hit count; for now focus on a sum over all groups;
#if _OLD_
  for(unsigned gr=0; gr<mIdealTrCode->fGroups.size(); gr++) {
    EicDetectorGroup *group = &mIdealTrCode->fGroups[gr];
    
    if (!group->svCounter) continue;
    
    unsigned len = strlen(buffer);
    snprintf(buffer+len, 16384-len-1, "%c%s:%d", 
	     equal_flag ? '=' : ',', group->dname->Name().Data(), group->ndCounter);
    equal_flag = false;
  } //for it 
#else
  for(unsigned gr=0; gr<mIdealTrCode->fGroups.size(); gr++) {
    EicDetectorGroup *group = &mIdealTrCode->fGroups[gr];
    
    if (!group->svCounter) continue;
    
    unsigned len = strlen(buffer);
    snprintf(buffer+len, 16384-len-1, "%c%s", 
	     equal_flag ? '=' : '+', group->dname->Name().Data());
    equal_flag = false;
  } //for it 
  unsigned len = strlen(buffer);
  snprintf(buffer+len, 16384-len-1, ":%d", GetMaxPossibleHitCount() - GetMissingHitCounterMax());
  printf("%s\n", buffer);
#endif

  str->mString = strdup(buffer);
  if (!str->mString) return -1;

  mKalmanFilter->Configure(mMediaBank, str);
  //exit(0);

  // --> FIXME: make configurable;
  const char *rk_cfg[] = {
    "mode=hermes",
    "small-step-limit=1.8cm",
    "small-step-order=4",
    "cell-width-max=2.0cm",
    "interpolation=3x3x1"
  };
  int rk_dim = sizeof(rk_cfg) / sizeof(rk_cfg[0]);
  char **rk_argv = (char **)malloc(rk_dim * sizeof(char*)); assert(rk_argv);
  for(int ik=0; ik<rk_dim; ik++)
    rk_argv[ik] = strdup(rk_cfg[ik]);
  // Configure Runge-Kutta algorithm; again, XML-ize and feed existing function;
  runge_kutta_fun(rk_dim, rk_argv);

  return 0;
} // EicHtcTask::ConfigureKalmanFilter()

// ---------------------------------------------------------------------------------------

InitStatus EicHtcTask::Init() 
{
  // Call original FairTask Init(); it should be *before* the below GEANT3 calls;
  FairTask::Init();

  G3ZEBRA(GCBANK_SIZE);

  // Want no limit; a million of seconds should be enough :-) 
  //TIMEST(1000000);

  // Initialize GEANT common blocks; 
  G3INIT();

  // Initialize GEANT part of ZEBRA storage; 
  G3ZINIT();

  // Read in CARD file if needed; 
  //if (card_file_name) GFFGO();

  // Initialize particle info in the GEANT internals; 
  //GPART();

  // Call user geometry initialization; 
  //if (!geometry || geometry()) _RETURN_(-1, "geometry() failed\n");

  // GEANT geometry optimisation, etc; 
  //GGCLOS();

  // Calculate and initialize the GEANT 
  // cross-sections tables, etc;      
  G3PHYSI();

  // Import mapping tables which determine sensitive volumes; unify with Calo
  // digitization code later;
  {
    FairRootManager* ioman = FairRootManager::Instance();

    for(unsigned gr=0; gr<mIdealTrCode->fGroups.size(); gr++) {
      EicDetectorGroup *group = &mIdealTrCode->fGroups[gr];
 
      if (!group->mGptr) {
	ioman->GetInFile()->GetObject(group->dname->Name() + "GeoParData", group->mGptr); 

	// Well, missing mapping table is a critical failure;
	if (!group->mGptr) {
	  std::cout << "-E- Eic"<< group->dname->Name() <<" hit producer: no map found!" << std::endl;
	  return kERROR;
	} //if

	group->mGptr->InitializeLookupTables();
      } //if

      // Loop through all friend classes and try to read in digitizer header;
      {
	// NB: if reconstruction.C was started with FairRunAna rather than EicRunReco, no luck
	// (I mean do not bother to dig digitization.C file name from FairRunAna internals);
	EicRunAna *fRun = dynamic_cast<EicRunAna *>(EicRunAna::Instance());

	if (fRun) 
	  //for(unsigned fr=0; fr<fRun->mFriendFiles.size(); fr++) {
	  //TFile fin(fRun->mFriendFiles[fr]);
	  for(unsigned fr=0; fr<fRun->GetFriendFiles().size(); fr++) {
	    TFile fin(fRun->GetFriendFiles()[fr]);

	    if (fin.IsOpen()) {

	      fin.GetObject(group->dname->Name() + "TrackingDigiHitProducer", group->mDigi);
	      fin.Close();

	      if (group->mDigi) break;
	    } //if
	  } //if..for fr

	// Well, missing digitizer info is a critical failure;
	if (!group->mDigi) {
	  std::cout << "-E- Eic"<< group->dname->Name() << 
	    " hit producer: no digi layout found!" << std::endl;
	  return kERROR;
	} //if
      }
    } //for it
  }

  // Initialize detector logical map lookup tables;
#if _OFF_
  for(unsigned gr=0; gr<mIdealTrCode->fGroups.size(); gr++) {
    EicDetectorGroup *group = &mIdealTrCode->fGroups[gr];
  
    group->mGptr->InitializeLookupTables();
  } //for gr
#endif

  // Loop through all gGeoManager nodes and match them against detector 
  // sensitive volumes;
  DeclareSensitiveVolumes();

  // Perform the media scan;
  PerformMediaScan(); 

  ConfigureKalmanFilter(); 

  // Get ROOT Manager;
  {
    FairRootManager* ioman= FairRootManager::Instance();

    if(!ioman) {
      Error("EicRecoKalmanFit::Init","RootManager not instantiated!");
      return kERROR;
    } //if

    ioman->Register(mTrackOutBranchName, "Gen", mFitTrackArray, /*fPersistence*/kTRUE);
  }

  return kSUCCESS;
} // EicHtcTask::Init()

// ---------------------------------------------------------------------------------------

// FIXME: do it better later;
#define _DIM_ 4

int EicHtcTask::ConstructLinearTrackApproximation(KfMatrix *A, KfMatrix *b)
{
  // FIXME: re-consider this code;
  //@@@assert(0);

  static KfMatrix *x0 = new KfVector(_DIM_);

  TrKalmanNode *tail = static_cast <TrKalmanNode *>(mKalmanFilter->GetTail());

  if (mParticleMomentumSeed)
    tail->SetMomentum(mParticleMomentumSeed);
  else {
    // FIXME: pull this out either from MCTrack or from IdealGenTrack;
    assert(0);
  } //if
  
  {
    // Want to set 5-th (momentum) component to 0.0; really needed?;
    double S[5] = {0.0, 0.0, 0.0, 0.0, 0.0};

#if _TODAY_
    // Will depend on selected hit combination -> have to repeat every time new;
    A->invert(_ARBITRARY_);
    kfm_multiply(x0, A, b);

    for(int ip=0; ip<4; ip++)
      S[ip] = x0->KFV(ip);
#endif

    //printf("@@@ORG: L-fit    (tail) -> %7.3f, %7.3f, %7.3f, %7.3f\n", 
    //	   cm2mm(S[0]), cm2mm(S[1]), S[2], S[3]);
    //tail->ResetNode(S, mKalmanFilter->GetFieldMode(), _USE_00_);
    mKalmanFilter->ResetNode(tail, S, _USE_00_);
  }

  return 0;
} // EicHtcTask::ConstructLinearTrackApproximation()

// ---------------------------------------------------------------------------------------

FairTrackParP EicHtcTask::GetFairTrackParP(TrKalmanNode *node)
{
  TrKalmanNode * trknode = static_cast <TrKalmanNode*> (node);

  double S[4], momentum = 1./(trknode->GetInversedMomentum() + 
			      (mKalmanFilter->GetFieldMode() == NoField ? 0.0 : 
			       mKalmanFilter->GetHead()->GetXs(4)));
  // Assume just +/-1 for now;
  int charge = momentum > 0.0 ? 1 : -1;

  for(unsigned iq=0; iq<4; iq++)
    S[iq] = node->GetX0(iq) + node->GetXs(iq);

  t_3d_line line /*= parametrize_straight_line*/(S, node->GetZ());

  TVector3 x(line.x [0], line.x [1], line.x [2]), dummy(0.1, 0.1, 0.1);
  TVector3 n(line.nx[0], line.nx[1], line.nx[2]);

  FairTrackParP tpp(x, fabs(momentum) * n, dummy, dummy, charge, dummy, dummy, dummy);

  return tpp;
} // EicHtcTask::GetFairTrackParP()

// ---------------------------------------------------------------------------------------

void EicHtcTask::Exec(Option_t* opt)
{
  {
    static unsigned evCounter;

    evCounter++;
    if (!(evCounter%10)) printf("-I- EicHtcTask::Exec() -> event %6d\n", evCounter);
  }

  // A separate HTC-specific tree; FIXME: move to Init() later;
  if (!mHtcBranch) {
    TTree *outputTree = new TTree(_EIC_HTC_TREE_, "A tree of HTC-reconstructed tracks");
    
    // The easiest: do not switch pointers all the time; THINK: multi-threading?; 
    mHtcBranch = outputTree->Branch(_EIC_HTC_TRACK_, _EIC_HTC_BRANCH_,
				    &mHtcTrack, 32000, 99);
    
    mHtcTrack = new EicHtcTrack();
  } //if

  TrKalmanNode *head = static_cast <TrKalmanNode *>(mKalmanFilter->GetHead());
  TrKalmanNode *tail = static_cast <TrKalmanNode *>(mKalmanFilter->GetTail());

  if (!mKalmanFilter->SetParticleGroup(mParticleHypothesis.Data()))
    fLogger->Fatal(MESSAGE_ORIGIN, "\033[5m\033[31m Unknown particle hypothesis ('%s')!  \033[0m", 
		   mParticleHypothesis.Data());

  static KfMatrix *A = new KfMatrix(_DIM_, _DIM_), *b = new KfVector(_DIM_);

  A->Reset(); b->Reset();

  unsigned firedNodeCounter = 0;
  mKalmanFilter->ResetFiredFlags();

  //printf("EicHtcTask::Exec() ...\n");
  //
  // FIXME: re-arrange loop later in such a way the code can work on >1 track; 
  //        for ideal case in principle all is needed is to leave only hits 
  //        which were produced by this particular track;
  //

  // Loop through all the detectors and allocate hits in KF internal structures;
  for(unsigned gr=0; gr<mIdealTrCode->fGroups.size(); gr++) {
    EicDetectorGroup *group = &mIdealTrCode->fGroups[gr];

    //printf("%s\n", group->dname->Name().Data()); assert(group->_fHits);
    if (!group->_fHits) continue;

    unsigned hnum = group->_fHits->GetEntriesFast();

    //printf("%s -> %2d\n", group->dname->Name().Data(), hnum); //assert(group->_fHits);
    for(unsigned ih=0; ih<hnum; ih++)
      {
	EicTrackingDigiHitOrth2D *hit = (EicTrackingDigiHitOrth2D*)group->_fHits->At(ih);

	//
	// FIXME: in case of >1 track there should be a hit validity 
	//        check right here (and continue if hit does not belong to this track);
	//

	// Part of the sensitive volumes may be of no interest (scan axis did not 
	// cross them) -> just skip;
	SensitiveVolume *sv = group->GetSensitiveVolume(hit);
	if (!sv) continue;

	TrKalmanNode *trknode = 
	  // NB: yes, in this code do not come into complication of having >1 hit per plane;
	  static_cast <TrKalmanNode*> (sv->mKfNodeWrappers[hit->GetKfNodeID()].GetKfNode(0));
	// FIXME: make it easy for now -> select clean events only;
	if (trknode->IsFired()) return;
	trknode->SetHit(hit);

	EicKfNodeTemplate *kftmpl = group->mDigi->mKfNodeTemplates[hit->GetKfNodeID()];
	trknode->SetMeasurementNoise(kftmpl->GetMeasurementNoise(hit));
	// Explicitely allow this crappy replacement on 1D nodes only;
	if (trknode->GetMdim() == 1)
	  {
	    static KfMatrix V = KfMatrix(1,1);

	    double resolutionByHand = GetResolutionByHand(trknode->GetName());

	    if (resolutionByHand) {
	      V.KFM(0,0) = resolutionByHand * resolutionByHand;
	      trknode->SetMeasurementNoise(&V);
	    } //if
	  } //if

	kftmpl->IncrementLinearTrackFitMatrices(sv, hit, tail->GetZ(), A, b);
           
	//printf("%s -> %7.2f mm\n", trknode->GetName(), 10.*hit->GetCoord(0));

	firedNodeCounter++;
	trknode->SetFiredFlag();
      } //for ih
  } //for gr

  //mKalmanFilter->BuildNodeList();

  //printf("%d\n", firedNodeCounter);
  // Basically means import hit file had fewer tracks than simulated.C;
  if (!firedNodeCounter) return;

  // Think on this, please ...;
  mKalmanFilter->LatchGroupNdfControlFlags();

  ConstructLinearTrackApproximation(A, b);

  mKalmanFilter->FilterPass(tail, head, KalmanFilter::Backward);
  //printf("@@@: K-fit    (tail) -> %7.3f, %7.3f, %7.3f, %7.3f\n", 
  //	 cm2mm(head->getX0(0)), cm2mm(head->getX0(1)), head->getX0(2), head->getX0(3));

  // Do iterations by hand; assign head node now;
  //printf("first  fullChain() call ...\n");
  //head->ResetNode(0, mKalmanFilter->GetFieldMode(), _USE_XF_);
  mKalmanFilter->ResetNode(head, 0, _USE_XF_);
  // Well, if _TRUST_SMOOTHER_FCN_ bit is set, KF-chain will keep removing 
  // high-chi^2 nodes beyond reasonable limits; think on this and later 
  // optimize KF.xml options (and perhaps allow to activate this bit as well);
  mKalmanFilter->FullChain(head, tail, KalmanFilter::Forward, _TRUST_FILTER_FCN_/*|_TRUST_SMOOTHER_FCN_*/);
  //head->ResetNode(0, mKalmanFilter->GetFieldMode(), _USE_XS_);
  mKalmanFilter->ResetNode(head, 0, _USE_XS_);
  mKalmanFilter->FullChain(head, tail, KalmanFilter::Forward, _TRUST_FILTER_FCN_/*|_TRUST_SMOOTHER_FCN_*/);
  //tail->resetNode(0, _USE_XS_);
  //printf("@@@FIT: K-fit    (tail) -> %7.3f, %7.3f, %7.3f, %7.3f\n", 
  //	 cm2mm(tail->getX0(0)), cm2mm(tail->getX0(1)), tail->getX0(2), tail->getX0(3));
#if 0
  head->resetNode(0, _USE_XS_);
  printf("@FIT: K-fit    (head) -> %7.3f, %7.3f, %7.3f, %7.3f\n\n", 
	 cm2mm(head->GetX0(0)), cm2mm(head->GetX0(1)), head->GetX0(2), head->GetX0(3));
  printf(" --> %f\n", 1/head->GetX0(4));
#endif
  printf(" --> %f\n", 1./(head->GetInversedMomentum() + head->GetXs(4)));

  //printf("%f\n", head->GetNext(KalmanFilter::Forward)->GetZ());
  
  // Assign internal HTC structure;
  {
    mHtcTrack->mMomentum = 1./(head->GetInversedMomentum() + head->GetXs(4));

    // Assign generic information;
    mHtcTrack->mNdf                 = mKalmanFilter->GetFilterNdf();
    mHtcTrack->mFilterChiSquare     = mKalmanFilter->GetFilterChiSquare();
    mHtcTrack->mFilterChiSquareCCDF = mKalmanFilter->GetFilterChiSquareCCDF();

    // Beam coordinate and slope estimate at the head node;
    for(unsigned xy=0; xy<2; xy++) {
      mHtcTrack->mBeamCoordXY[xy] = 
	mCoordinateScaleXY * (head->GetX0(xy) + head->GetXs(xy));
      mHtcTrack->mBeamCoordSigmaXY[xy] = mResidualScaleXY * sqrt(head->GetCS(xy,xy));

      mHtcTrack->mBeamSlopeXY[xy] = 
	mSlopeScale * (head->GetX0(2+xy) + head->GetXs(2+xy));
      mHtcTrack->mBeamSlopeSigmaXY[xy] = mSlopeScale * sqrt(head->GetCS(2+xy,2+xy));
    } //for xy

    // Clear hit array;
    mHtcTrack->mHits->clear();

    // Assign hits;
    for(unsigned gr=0; gr<mIdealTrCode->fGroups.size(); gr++) {
      EicDetectorGroup *group = &mIdealTrCode->fGroups[gr];

      if (!group->_fHits) continue;

      unsigned hnum = group->_fHits->GetEntriesFast();

      for(unsigned ih=0; ih<hnum; ih++)
      {
	EicTrackingDigiHit *hit = (EicTrackingDigiHit*)group->_fHits->At(ih);

	ULogicalIndex_t iz = group->mGptr->GeantMultiToLogicalIndex(hit->mMultiIndex);

	SensitiveVolume *sv = group->mSensitiveVolumes[group->mGptr->GetLookupTableNode(iz)];
	TrKalmanNode *trknode = 
	  // Yes, use only 0-th hit slot in this code;
	  static_cast <TrKalmanNode*> (sv->mKfNodeWrappers[hit->GetKfNodeID()].GetKfNode(0));

	// Get the new hit pointer with a proper component number;
	EicHtcHit *htchit = new EicHtcHit(trknode->GetMdim());

	htchit->mSmootherChiSquare   = trknode->GetSmootherChiSquare();
	htchit->mSmootherProbability = 
	  ROOT::Math::chisquared_cdf_c(trknode->GetSmootherChiSquare(), trknode->GetMdim());

	// FIXME: assign later if needed (and perhaps state vector itself)!;
	//assert(trknode->getCS(iq,iq) >= 0.0);
	//htchit->mDiagCS     [iq] = sqrt(trknode->getCS(iq,iq) >= 0.0 ? trknode->getCS(iq,iq) : 0.0);

	// Figure out coordinate and residual scale factors;
	double coordinateScale = mCoordinateScaleXY; 
	double residualScale   = mResidualScaleXY; 
	//printf("%f %f %f\n", mResidualScaleXY, residualScale, radius); 

	for(unsigned iq=0; iq<trknode->GetMdim(); iq++) {
	  EicHtcHitComponent *comp = htchit->mComponents + iq;

	  // Local coordinates;
	  comp->mLocalCoord1D    = coordinateScale * hit->_GetCoord(iq);

	  comp->mXsResidual      = residualScale * trknode->GetRs(iq);
	  comp->mXmResidual      = residualScale * trknode->GetRm(iq);

	  comp->mResolution      = residualScale * sqrt(trknode->GetV(iq,iq));
	  //printf("%s -> %f\n", trknode->getName(), comp->mResolution);

	  // FIXME: perhaps can do it better?;
	  //assert(trknode->getRS(iq,iq) >= 0.0 && trknode->getRM(iq,iq) >= 0.0);
	  if (trknode->GetRS(iq,iq) < 0.0 || trknode->GetRM(iq,iq) < 0.0) goto _next_hit;

	  comp->mSigmaRS         = residualScale * sqrt(trknode->GetRS(iq,iq));
	  //comp->mSigmaRS         = mResidualScale * sqrt(trknode->getCS(iq,iq));
	  comp->mSigmaRM         = residualScale * sqrt(trknode->GetRM(iq,iq));
	  //printf("%d/%d/%d -> %f\n", iz, hit->GetKfNodeID(), iq, comp->mSigmaRM);
	} //for iq

	for(unsigned xy=0; xy<2; xy++)
	  htchit->mGlobalCoordXY[xy] = 
	    // Right, mCoordinateScaleXY here for all detector types;
	    mCoordinateScaleXY * (trknode->GetX0(xy) + trknode->GetXs(xy));

	// Assign map element; 
	{
	  kEntry key = kEntry(group->dname->NAME().Data(), 
			      std::pair<unsigned, unsigned>(iz, hit->GetKfNodeID()));
	  (*mHtcTrack->mHits)[key] = htchit;
	}

      _next_hit:
	continue;
      } //for ih
    } //for gr
  }

  // Recover this stuff later as well, please;
#if _LATER_
  mFitTrackArray->Delete();

  {
    PndTrack *fitTrack = new PndTrack();

    TClonesArray& trkRef = *mFitTrackArray;
    Int_t size = trkRef.GetEntriesFast();

    // Construct coordinates at the head node;
    FairTrackParP first = GetFairTrackParP(head);
    FairTrackParP last  = GetFairTrackParP(tail);

    double value = mKalmanFilter->GetFilterChiSquareCCDF();//Probability();

    // FIXME: a test hack for now;
#if _OLD_
    for(unsigned gr=0; gr<mIdealTrCode->fGroups.size(); gr++) {
      EicDetectorGroup *group = &mIdealTrCode->fGroups[gr];

      if (!group->_fHits) continue;

      unsigned hnum = group->_fHits->GetEntriesFast();
      //printf("%s -> %d hits\n", group->dname->Name().Data(), hnum); 

      for(unsigned ih=0; ih<hnum; ih++)
      {
	EicTrackingDigiHit *hit = (EicTrackingDigiHit*)group->_fHits->At(ih);

	//const EicGeoMap *fmap = group->mGptr->GetMapPtrViaHitMultiIndex(hit->fMultiIndex);
	//assert(fmap);

	ULogicalIndex_t iz = group->mGptr->GeantMultiToLogicalIndex(hit->fMultiIndex);
	printf("%s %d\n", group->dname->Name().Data(), iz);

	// Yes, now want to select UVA tracker;
	//if (group->dname->NAME().EqualTo("UVA")) {
	if (group->dname->NAME().EqualTo("TRK") && iz == 1) {
	  //if (group->dname->NAME().EqualTo("SBS") && iz == 1) {
#ifdef _1D_
	  for(unsigned xy=_X_; xy<=_Y_; xy++) {
	    SensitiveVolume *sv = group->mSensitiveVolumes[iz*2+xy]; 
#else
	    SensitiveVolume *sv = group->mSensitiveVolumes[iz]; 
#endif

	    // FIXME: yes, prefer a clean case now; once re-arrange into track loop, fix this;
	    //if (sv->_fHits.size()) return;
	    
	    //sv->_fHits.push_back(hit);
	    
	    TrKalmanNode *trknode = static_cast <TrKalmanNode*> (sv->firstKfNode);
	    // --> FIXME (multiple hits);
	    //trknode->_ht = 0;
	    
	    {
#ifdef _1D_
	      //static KfMatrix V = KfMatrix(1,1);
	      //V.KFM(0,0) = hit->fCov[xy ? 3 : 0];
#else
	      //printf("%f: %7.5f vs %7.5f + %7.5f\n", trknode->getZ(), 
	      //     hit->fLocalCoord[0], trknode->getX0(0), trknode->getXs(0));
	      // Test purposes anyway: [cm] -> [um];
	      {
		double global[3] = {trknode->getX0(0) + trknode->getXs(0), 
				    trknode->getX0(1) + trknode->getXs(1), 
				    trknode->getZ()}, local[3];
		//double global[3] = {trknode->getX0(0) + trknode->getXm(0), 
		//		    trknode->getX0(1) + trknode->getXm(1), 
		//		    trknode->getZ()}, local[3];
		const LogicalVolumeLookupTableEntry *node = group->mGptr->GetLookupTableNode(iz);

		node->mGeoMtx->MasterToLocal(global, local);

		//value = 1E4*(local[0] - hit->fLocalCoord[0]);
		value = 1E4*(local[1] - hit->fLocalCoord[1]);

		//value = 1E4*trknode->getXm(1));

		//value = 1000.*(trknode->getX0(2) + trknode->getXs(2));
		//value = 1000.*(trknode->getX0(3) + trknode->getXs(3));
	      }
	      // FLYSUB preliminary: want Y-component in the real data;
	      //value = 1E4*(trknode->getX0(1) + trknode->getXs(1) - hit->fLocalCoord[1]);
	      //value = hit->fLocalCoord[1];
	      //value = 1E4*sqrt(trknode->getCS(0,0));

	      //static KfMatrix V = KfMatrix(2,2);
	      //V.KFM(0,1) = V.KFM(1,0) = hit->mCov[1];
	      //V.KFM(0,0) =              hit->mCov[0];
	      //V.KFM(1,1) =              hit->mCov[3];
	      //printf("%f %f %f\n", hit->mCov[1], hit->mCov[0], hit->mCov[3]);
#endif
	      //trknode->setMeasurementNoise(V);
	      
	      // Increment linear track fit matrices; --> FIXME (not only 0-th hit);
	      //incrementLinearTrackFitMatrices(sv, hit, A, b);
	    }
	    
	    //sv->firstKfNode->setFiredFlag();
#ifdef _1D_
	  } //for xy
#endif
	} //if
      } //for ih
    } //for gr
#endif

    //if (first.GetQ() == -1) printf("@@CH@@\n");
    //printf("%f -> %f ... %f\n", mKalmanFilter->getFilterChiSquare(), mKalmanFilter->getFilterProbability(), value);
    PndTrack* pndTrack = new(trkRef[size]) PndTrack(first, last, fitTrack->GetTrackCand(),
						    0, value, mKalmanFilter->GetFilterNdf());
  }
#endif

  mHtcBranch->GetTree()->Fill();
} // EicHtcTask::Exec()

// ---------------------------------------------------------------------------------------

void EicHtcTask::FinishTask()
{
  if (mPersistency && mHtcBranch) {
    mHtcBranch->GetTree()->GetCurrentFile()->cd();
    mHtcBranch->GetTree()->Write();
  } //if

  // Save object itself;
  Write();

  FairTask::FinishTask();
} // EicHtcTask::FinishTask()

// ---------------------------------------------------------------------------------------

void EicHtcTask::Print(Option_t* option) const
{
  TTask::Print();

  printf("Forced particle type: %s; momentum seed: %8.2f [GeV/c]\n", 
	 mParticleHypothesis.Data(), mParticleMomentumSeed);
} // EicHtcTask::Print()

// ---------------------------------------------------------------------------------------

bool HitKeyCompare(kEntry lh, kEntry rh)
{
  int ret = strcmp(lh.first, rh.first);

  // The actual meaning of ">" and "<" is not important; this std::map
  // arrangement is just to speed up the access;
  if (ret)
    return ret < 0;
  else {
    std::pair<unsigned, unsigned> &lhh = lh.second, &rhh = rh.second;

    if (lhh.first == rhh.first)
      return lhh.second < rhh.second;
    else
      return lhh.first  < rhh.first;
  } //if
} // HitKeyCompare()

// ---------------------------------------------------------------------------------------

const EicHtcHit* EicHtcTrack::GetHit(const char *detName, unsigned group, unsigned node) const
{
  kEntry key = kEntry(detName, std::pair<unsigned, unsigned>(group, node));

  if (mHits->find(key) == mHits->end()) return 0;

  return (*mHits)[key];
} // EicHtcTrack::GetHit()

// ---------------------------------------------------------------------------------------

ClassImp(EicHtcTask)
ClassImp(EicHtcHit)
ClassImp(EicHtcHitComponent)
ClassImp(EicHtcTrack)

