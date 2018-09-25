//
// AYK (ayk@bnl.gov), 2013/06/13
//
//  Calorimeter digi hit producer classes;
//

//#include <stdlib.h>
#include <assert.h>
#include <cmath>

#include <TGeoNode.h>
#include <TRotation.h>
#include <TParticlePDG.h>
#include <TMath.h>
#include <TRandom.h>
#include <TDatabasePDG.h>

#include <FairRunAna.h>
#include <FairRuntimeDb.h>

#include <EicLibrary.h>
#include <EicGeoPar.h>
//#include <EicFakeMoCaPoint.h>
#include <EicCalorimeterDigiHitProducer.h>

using namespace std;

// FIXME: do it better later;
#define _SQR_(x) ((x)*(x))

// -------------------------------------------------------------------------

EicCalorimeterDigiHitProducer::EicCalorimeterDigiHitProducer(const char *name): 
  //TString inFileName) :
  EicDigiHitProducer(name)
{ 
  ResetVars();

  //mInFileName = inFileName;
} // EicCalorimeterDigiHitProducer::EicCalorimeterDigiHitProducer()

// -------------------------------------------------------------------------
#if _LATER_
int EicCalorimeterDigiHitProducer::UseFakeMoCaPointDatabase(const char *dbFileName, UInt_t energyBinNum)
{
#if _OLD_
  TString expandedFileName(dbFileName);

  // Correct path if needed;
  if (!expandedFileName.BeginsWith("./") && !expandedFileName.BeginsWith("/"))
  {
    TString wrkDir = getenv("VMCWORKDIR");

    expandedFileName = wrkDir + "/input/" + expandedFileName;
  } //if

  TFile dbFile(expandedFileName);
#else
  TFile dbFile("input/", ExpandedFileName(dbFileName));
#endif

  // Declare ROOT tree which will hold entries;
  TTree *dbTree = (TTree*)dbFile.Get(mDetName->Name() + "EicFakeMoCaPointDatabase"); assert(dbTree);

  // Alocate buffer variable; think on split, etc; later;
  EicFakeMoCaPointDbEntry *dbEntry = new EicFakeMoCaPointDbEntry(); 

  TBranch *branch = dbTree->GetBranch("EicFakeMoCaPointDbEntry"); assert(branch);
  branch->SetAddress(&dbEntry);

  mFakeDB = new EicFakeMoCaPointDatabase();

  mFakeDB->mEnergyBinNum = energyBinNum; assert(energyBinNum);

  // Yes, expect object with a predefined name; do a better check later;
  dbFile.GetObject(mDetName->Name() + "EnergyCutOffTable", mFakeDB->mCutOffMap);
  assert(mFakeDB->mCutOffMap); 

  for (std::map<Int_t, std::pair<Double_t,Double_t> >::iterator it=mFakeDB->mCutOffMap->begin(); 
  	 it!=mFakeDB->mCutOffMap->end(); ++it)
    mFakeDB->mDB[abs(it->first)] = new std::vector<EicFakeMoCaPointDbEntry>[mFakeDB->mEnergyBinNum];

  // Read in entries one by one and allocate the whole database;
  unsigned evtNum = dbTree->GetEntries(); 
  printf("  -I- Importing '%s' fake MC hit database (%d DB entries) ...\n", 
	 mDetName->Name().Data(), evtNum);
  for(unsigned evt=0; evt<evtNum; evt++)
  {
    dbTree->GetEntry(evt);

    unsigned hitNum = dbEntry->fHits->GetEntriesFast();
    //printf("   %4d: -> %4d hits\n", evt, hitNum);
 
    // Well, assume it is available;
    TDatabasePDG *pdgTable = TDatabasePDG::Instance(); assert(pdgTable);

    Int_t PDG = dbEntry->getPrimaryParticlePDG();

    //printf("%d %d\n", PDG, hitNum);
    // Check, that dbEntry->PDG is present in the map!; fix!;
    if (mFakeDB->mCutOffMap->find(abs(PDG)) == mFakeDB->mCutOffMap->end())
    {
      printf("\n *** Primary mother PDG is not present in the DB map; "
	     "(must be the wrong input DB file)! ***\n\n");
      exit(0);
    } //if

    double eLogMin = log((*mFakeDB->mCutOffMap)[abs(PDG)].first);
    double eLogMax = log((*mFakeDB->mCutOffMap)[abs(PDG)].second);
    double bWidth  = (eLogMax - eLogMin)/mFakeDB->mEnergyBinNum;

    double mass = ((TParticlePDG*)pdgTable->GetParticle(PDG))->Mass();
    double eKin = sqrt(mass*mass + dbEntry->fP.Mag2()) - mass;
    double eLogKin = log(eKin);
    int bId = (int)floor((eLogKin - eLogMin)/bWidth);

    //printf("%d (%d)\n", bId, mFakeDB->mEnergyBinNum);
    // Check value-at-the-edge condition later, please;
    if (bId < 0 || bId >= mFakeDB->mEnergyBinNum)
     {
       printf("\n *** Primary mother energy out of range; "
	      "(must be the wrong input MC file)! ***\n\n");
       exit(0);
     } //if

    // Do this better, please; suffices for now (just recreate pointer and copy contents over);
    mFakeDB->mDB[abs(PDG)][bId].push_back(*dbEntry);
    mFakeDB->mDB[abs(PDG)][bId][mFakeDB->mDB[abs(PDG)][bId].size()-1].fHits = 
      //new TClonesArray("EicFakeMoCaPointDbHit");
      new TClonesArray(*dbEntry->fHits);
    //printf("%p %p\n", dbEntry->fHits, mFakeDB->mDB[abs(PDG)][bId][mFakeDB->mDB[abs(PDG)][bId].size()-1].fHits);
    //*(mFakeDB->mDB[abs(PDG)][bId][mFakeDB->mDB[abs(PDG)][bId].size()-1].fHits) = *(dbEntry->fHits);
    // Really needed?;
    //dbEntry->fHits->Delete();
  } //for evt

  dbFile.Close();

  return 0;
} // EicCalorimeterDigiHitProducer::UseFakeMoCaPointDatabase()
#endif
// -------------------------------------------------------------------------

int EicCalorimeterDigiHitProducer::RequestTimeSpectra(Double_t tRange, UInt_t tDim) 
{
  if (!mDigi || !tRange || !tDim) return -1;

  mDigi->mTimeRange    = tRange; 
  mDigi->mTimeDim      = tDim; 

  mDigi->mTimeBinWidth = tRange / tDim;

  return 0;
} // EicCalorimeterDigiHitProducer::RequestTimeSpectra()

// -------------------------------------------------------------------------

InitStatus EicCalorimeterDigiHitProducer::ExtraInit()
{
#if _MOVED_
  // Loop through all the maps and set sensitive flags for those, whose 
  // top-level volume names match the requested ones;
  for(int iq=0; iq<mGptr->GetMapNum(); iq++)
  {
    EicGeoMap *fmap = mGptr->GetMapPtrViaMapID(iq);

    const std::pair<TString, double> *entry = 
      /*mDigi->*/mSensitiveVolumes.AnyMatch(*fmap->GetInnermostVolumeName());
    
    if (entry) fmap->SetSensitivityFlag(entry->second);
  } //for iq
#endif

  // May want to move this stuff to EicDigiHitProducer::Init();
  FairRootManager* ioman = FairRootManager::Instance();
  //mFakeMoCaPointArray = (TClonesArray*) ioman->GetObject(mDetName->Name() + "FakeMoCaPoint");
  //if ( !mFakeMoCaPointArray ) {
  //std::cout << "-W- "<<mDetName->Name()<<"DigiHitProducer::Init: "
  //	      << "No "<<mDetName->Name()<<"Fake MC point array!" << std::endl;
  //return kERROR;
  //} //if

  mDigiHitArray = new TClonesArray("EicCalorimeterDigiHit");
  //ioman->Register(mDetName->Name() + "DigiHit", mDetName->NAME(), mDigiHitArray, mPersistence);
  ioman->Register(mDetName->Name() + "CalorimeterDigiHit", mDetName->NAME(), mDigiHitArray, mPersistence);

  return kSUCCESS;
} // EicCalorimeterDigiHitProducer::ExtraInit()

// -------------------------------------------------------------------------

int EicCalorimeterDigiHitProducer::PreExec()
{
  // Obviously need to clean cell array before next event;
  mCells.clear();

  return 0;
} // EicCalorimeterDigiHitProducer::PreExec()

// -------------------------------------------------------------------------

int EicCalorimeterDigiHitProducer::HandleHitCore(CalorimeterCell *cell)
{

  return 0;
} // EicCalorimeterDigiHitProducer::HandleHitCore() 

// -------------------------------------------------------------------------

int EicCalorimeterDigiHitProducer::HandleHit(const EicMoCaPoint *point)
{
  // Obtain map pointer;
  const EicGeoMap *fmap = mGptr->GetMapPtrViaHitMultiIndex(point->GetMultiIndex());

  // If map is not declared as "sensitive" (so its top-level volume is not sensitive)
  // and energy deposit accouting is not requested, no reason to allocate a new cell;
  if ((!fmap || !fmap->IsSensitive()) && !EnergyDepositAccountingRequested()) return 0;

  ULogicalIndex_t xy = mGptr->GeantMultiToLogicalIndex(point->GetMultiIndex());

  //#ifdef _OLD_STYLE_
  CalorimeterCell *cell = &mCells[xy];
  //#else
  //ULogicalIndex_t xy_screwed = ((xy & 0x0003) << 16) | ((xy & 0x000C) >> 2);
  //CalorimeterCell *cell = &mCells[xy_screwed];
  //#endif
  CalorimeterCellParent *parent = 
    &cell->mCellParents[std::pair<UInt_t, UInt_t>(point->GetPrimaryMotherID(), point->GetSecondaryMotherID())];

  // If timing spectrum is needed on output (and unless this cell was initialized
  // already), do it now;
  if (mDigi->mTimeDim && !cell->mTimeSpectrum) cell->InitializeTimeSpectrum    (mDigi->mTimeDim);
  if (                   !parent->mZCoordBins) parent->InitializeZCoordSpectrum(mDigi->mZCoordDim);

  // Default is the full GEANT volume name;
  const char *volumePlottingName = fmap->GetInnermostVolumeName()->Data();

  // If energy deposit accounting requested, do it now; do NOT account Birk's law here;
  if (EnergyDepositAccountingRequested()) {
    // Figure out which name to use to fill out energy deposit entries; the logic is: if
    // exact name match exists, use it; otherwise use pattern name;
    {
      // FIXME: well, even that this stuff is useful for test purposes only, may want 
      // to optimize it a bit later (look-up table, etc);
      const std::pair<TString, double> *entry = 
	/*mDigi->*/mSensitiveVolumes.ExactMatch(volumePlottingName);

      if (!entry) {
	entry = /*mDigi->*/mSensitiveVolumes.PrefixMatch(volumePlottingName);

	if (entry) volumePlottingName = entry->first.Data();
      } //if
    } 

    if (fmap->IsSensitive())
      cell->mEnergyDeposits[volumePlottingName].mSensitive += point->GetEnergyLoss();
    else
      cell->mEnergyDeposits[volumePlottingName].mPassive   += point->GetEnergyLoss();
  } //if

  // If map is not "sensitive", just return; nothing else to do;
  if (!fmap->IsSensitive()) return 0;

  {
    // Figure out whether have access to cell length;
    const CalorimeterGeoParData* cGptr = dynamic_cast <const CalorimeterGeoParData*>(mGptr);
    double towerLength = cGptr ? 0.1 * cGptr->mCellLength : 0.0, local[3];
    //, towerLength = 0.1 * (dynamic_cast <const CalorimeterGeoParData*>(mGptr))->mCellLength;

    // In principle mGptr can be a pointer to basic EicGeoParData rather than 
    // CalorimeterGeoParData (see two-in-one.C in tutorials); in this case mGptr->mCellLength
    // is not available (and also I do not want to give it by hand in digitization.C as an extra
    // parameter in order to avoid inconsistencies); the code should still work though, without
    // Z-binning (so ignore attenuation length);
    if (!towerLength && (mDigi->mZCoordDim != 1 || mDigi->mAttenuationLength)) {
	printf("\n *** either mDigi->mZCoordDim != 1 or mDigi->mAttenuationLength != 0.0 ***\n"
	       " *** but tower length unknown, sorry! ***\n\n");
      exit(0);
    } //if

    {
      // Figure out local Z-coordinate *with*respect*to*TOWER*middle; yes, not with respect to 
      // EmCal fiber piece or HCal sintillator plate (because attenuation is defined in tower
      // coordinates); also calculate respective Z-bin; NB: ;
      TVector3 middle( 0.5 * (point->GetPosIn() + point->GetPosOut()) );

      double midarr[3] = {middle.X(), middle.Y(), middle.Z()};

      const LogicalVolumeLookupTableEntry *node = mGptr->GetLookupTableNode(xy);

      //mGeometryHub->mLookup + mGeometryHub->GetGptr()->GetX(xy)*mGeometryHub->mRealDim[1]*mGeometryHub->mRealDim[2] + 
      //mGeometryHub->GetGptr()->GetY(xy)*mGeometryHub->mRealDim[2] + mGeometryHub->GetGptr()->GetZ(xy);
      assert(node->mGeoNode);

      // Transformation from MARS to tower coordinate system;
      node->mGeoMtx->MasterToLocal(midarr, local);
    }


    // Z-coordinate in the range [0..fzLen] rather than +/-fzLen/2; reset to 0.0 if tower
    // length unknown, just in order to avoid confusions in all other places;
    double zCoord = towerLength ? local[2] + towerLength/2. : 0.0;

    //printf("%f %f %d\n", zCoord, towerLength, mDigi->mZCoordDim); exit(0);
    // Respective bin in Z-plot;
    int nz = towerLength ? (int)floor(zCoord/(towerLength/mDigi->mZCoordDim)) : 0;
    //assert(nz >= 0 && nz < mDigi->mZDim);
    if (nz < 0 || nz >= mDigi->mZCoordDim) {
      // Consider to regularize?; hard to control boundary conditions; think later;
      if (nz < 0) 
	nz = 0;
      else
	if (nz >= mDigi->mZCoordDim) 
	  nz = mDigi->mZCoordDim - 1;
    } //if	     
    
    CalorimeterCellZCoordBin *zbin = parent->mZCoordBins + nz;
    
    //return 0;

    double qTime = point->GetTime();
    // Should not happen, see EicDetector::ProcessHits();
    assert(/*point->GetEnergyLoss() &&*/ point->GetStep());
    // Rescale by Birk's law right here;
    double dE    = point->GetEnergyLoss()/
      (1. + fmap->GetBirkConstant()*point->GetEnergyLoss()/point->GetStep());
    
    // Birk's law corrected energy deposit for sensitive volumes only;
    if (EnergyDepositAccountingRequested()) 
      cell->mEnergyDeposits[volumePlottingName].mSensitiveBirk += dE;
    
    zbin->mHitNum++;
    zbin->mEnergyDeposit += dE;
    zbin->mTime          += dE*qTime;
    zbin->mZ             += dE*zCoord;

    //#ifdef _OLD_STYLE_
    //printf("@@QQ@@ %0X %0X %f\n", point->GetMultiIndex(), xy, dE);
    //#else
    //printf("@@QQ@@ %0X %0X %f\n", point->GetMultiIndex(), xy_screwed, dE);
    //#endif
  }

  return HandleHitCore(cell);
} // EicCalorimeterDigiHitProducer::HandleHit()

// -------------------------------------------------------------------------

#if _LATER_
LogicalVolumeLookupTableEntry *EicCalorimeterDigiHitProducer::findHitNode(EicFakeMoCaPoint *point,
									double master[3], 
									double local[3])
{
#if _LATER_
  // Return same node as EicFakeMoCaPoint; no optimization can go faster than this
  // branch (which is of course fake);
#if 0
  UGeo_t xyPoint = GetGptr()->remapGeantMultiIndex(point->fMultiIndex) & 0xFFFFFFFF;
  CalorimeterLookupTableEntry *node = 
    mLookup + GetGptr()->getX(xyPoint)*mRealDim[1]*mRealDim[2] + GetGptr()->getY(xyPoint)*mRealDim[2] + GetGptr()->getZ(xyPoint);
  assert(node->gNode);

  //printf("  -> trying %s\n", node->first->GetName());
  node->gMtx->MasterToLocal(master, local);
  return node;
#endif

  // Dumb linear search through all the calorimeter cells; prohibitively CPU expensive, but 
  // safe (debugging purposes only -> to check, that optimized version gives identical result);
#if 0
  for(unsigned nd=0; nd<mDim3D; nd++)
  {
    CalorimeterLookupTableEntry *node = mLookup + nd;

    if (!node->gNode) continue;

    //printf("  -> trying %s\n", node->first->GetName());
    node->gMtx->MasterToLocal(master, local);
    
    //node->second->Print();
    //printf("%f %f %f\n", master[0], master[1], master[2]);
    //printf("  %f %f %f\n", local[0], local[1], local[2]);
    
    if (node->gNode->GetVolume()->GetShape()->Contains(local)) return node;
  } //for nd
#endif

  //
  // The below code checks hit-to-cell match in rectangular slices starting 
  // off EicFakeMoCaPoint cell; perhaps can be optimized further;
  //
#if 1
  // Check last node (perhaps there is a match); does it work actually?;
  if (mLastOkNode)
  {
    mLastOkNode->mGeoMtx->MasterToLocal(master, local);

    if (mLastOkNode->mGeoNode->GetVolume()->GetShape()->Contains(local)) 
      return mLastOkNode;
    else
      mLastOkNode = 0;
  } //if

  // So: move outwards starting off the node which contained FakeMoCaPoint;
  UGeo_t xyPoint = mGeometryHub->GetGptr()->RemapMultiIndex(point->fMultiIndex) & 0xFFFFFFFF;
  unsigned ixyzPoint[3] = {mGeometryHub->GetGptr()->GetX(xyPoint), 
			   mGeometryHub->GetGptr()->GetY(xyPoint), 
			   mGeometryHub->GetGptr()->GetZ(xyPoint)};
  unsigned dMax[3], lrDist[2][3], d3Max = 0;

  // Figure out absolute max 1D distances to map edges from this point;
  for(unsigned iq=0; iq<3; iq++)
  {
    lrDist[0][iq] =               ixyzPoint[iq];
    lrDist[1][iq] = mGeometryHub->GetRealDim(iq) - ixyzPoint[iq] - 1;
    dMax [iq] = lrDist[0][iq] > lrDist[1][iq] ? lrDist[0][iq] : lrDist[1][iq];

    d3Max += dMax[iq];
  } //for iq

  double dMinOverall = 0.0;

  // Loop through all possible Chebyshev 3D distances, starting off the actually hit node;
  for(unsigned dst=0; dst<d3Max; dst++)
  {
    double dMin = 0.0;

    for(unsigned ix=0; ix<=dst; ix++)
	for(unsigned iy=0; iy<=dst-ix; iy++)
	{
	  // So that ix+iy+iz=dst, right?;
	  unsigned iz = dst - ix - iy;

	  // And loop through all 3 LR-combinations;
	  for(unsigned ixlr=0; ixlr<2; ixlr++)
	  {
	    if (ix > lrDist[ixlr][0]) continue;

	    unsigned idX = ixyzPoint[0] + (ixlr ? ix : -ix);

	    for(unsigned iylr=0; iylr<2; iylr++)
	    {
	      if (iy > lrDist[iylr][1]) continue;

	      unsigned idY = ixyzPoint[1] + (iylr ? iy : -iy);

	      for(unsigned izlr=0; izlr<2; izlr++)
	      {
		if (iz > lrDist[izlr][2]) continue;

		unsigned idZ = ixyzPoint[2] + (izlr ? iz : -iz);

		// Eventually get node pointer and check whether 3D point is 
		// inside this volume or not;
		CalorimeterLookupTableEntry *node = 
		  mGeometryHub->mLookup + idX*mGeometryHub->mRealDim[1]*mGeometryHub->mRealDim[2] + 
		  idY*mGeometryHub->mRealDim[2] + idZ;
		
		if (!node->mGeoNode) continue;

		//printf("  -> trying %s\n", node->first->GetName());
		node->mGeoMtx->MasterToLocal(master, local);

		// Calculate point-to-volume-center distance in XY-projection; since this 
		// appens in the *local* cell coordinate system, should be correct for
		// both FEMC (XY) and CEMC (XZ) calorimeters;
		double dd = sqrt(_SQR_(local[0]) + _SQR_(local[1])/* + _SQR_(local[2])*/);
		
		// If 'dd' is not the smallest of the ones checked so far, no 
		// sense to call this expensive function; this is not really true
		// for arbitrary calorimeter, but seems to be so if blocks are 
		// packed in a "regular" fashion; perhaps allow to disable for 
		// test purposes?; think later;
		if ((!dMinOverall || dd <= dMinOverall) && 
		    node->mGeoNode->GetVolume()->GetShape()->Contains(local)) 
		{
		  mLastOkNode = node;
		  return node;
		} //if

		if (!dMin        || dd < dMin)        dMin        = dd;
		if (!dMinOverall || dd < dMinOverall) dMinOverall = dd;
	      } //for izlr
	    } //for iylr
	  } //for ixlr
	} //for ix..iz

    // Well, if true 3D distances from cell centers at this 'dst' to point 
    // transfered to local coordinates started to get bigger, I'm definitely 
    // out of luck (would most likely mean 3D point fits into a gap or perhaps
    // is outside of calorimeter rad.length reach); may actually want to 
    // still account this point taking the cell with *smallest* distance if 
    // Z-coordinate is Ok (so then it's an intercell gap); again, may want 
    // to disable this check (though it should work for "regular" volume 
    // collection); think later;
    if (dMinOverall && dMin > dMinOverall) break;
  } //for dst
#endif
#endif

  // Missed any calorimeter cell -> return NULL pointer;
  return 0;
} // EicCalorimeterDigiHitProducer::findHitNode
#endif
// -------------------------------------------------------------------------

#if _LATER_
int EicCalorimeterDigiHitProducer::ProcessFakeMoCaPoints( void )
{
#if _LATER_
  // All this eKin/mass stuff needs to be optimized later, please;
  TDatabasePDG *pdgTable = TDatabasePDG::Instance(); assert(pdgTable);

  unsigned nEntries = fFakeMoCaPointArray->GetEntriesFast();
  //cout << nEntries << endl;

  for(unsigned iq=0; iq<nEntries; iq++)
  {
    EicFakeMoCaPoint *point = (EicFakeMoCaPoint*) fFakeMoCaPointArray->At(iq);

    // Figure out PDG and kin.energy bin;
    int PDG = point->getPDG();
    TParticlePDG *particle = pdgTable->GetParticle(PDG);
    double mass = particle->Mass(), eKin = sqrt(_SQR_(mass) + point->p().Mag2()) - mass;

    // Get random shower database entry matching PDG & eKin bin;
    EicFakeMoCaPointDbEntry *dbEntry = mFakeDB->getDbEntry(PDG, point->p());

    // Either PDG unknown or no entries in this energy bin;
    if (!dbEntry)
    {
      std::cout << "-E- EicCalorimeterDigiHitProducer::ProcessFakeMoCaPoints(): mFakeDB->getDbEntry() returned NULL!" << std::endl;
      return -1;
    } //if

    // Figure out energy scaling coefficient; based on what?;
    double eKinDb = sqrt(_SQR_(mass) + dbEntry->fP.Mag2()) - mass;
    double scale = eKin / eKinDb;

    unsigned nHits = dbEntry->fHits->GetEntriesFast();

    // Construct 3D rotation matrix, which moves unit vector along dbEntry->fP to
    // a unit vector along point->p(); first calculate a vector along 
    // [dbEntry->fP x point->p()]; NB: no need to normalize it (rr.Rotate() works 
    // fine without that); use TVector3 and TMatrix machinery here and below;
    TVector3 axis = dbEntry->fP.Cross(point->p());
    // Rotation angle to align shower with point-p(); yeat another random 
    // angle [0 .. 2pi] around point->p(); 
    double angle = dbEntry->fP.Angle(point->p()), yaangle = gRandom->Uniform(2.*TMath::Pi());
    // Whatever dummy frame to be used with rr.Rotate(); C++ is funny sometimes;
    TRotation rr;

    // Loop through all energy deposit hits in this shower;
    for(unsigned ip=0; ip<nHits; ip++)
    {
      EicFakeMoCaPointDbHit *dbHit = (EicFakeMoCaPointDbHit*)dbEntry->fHits->At(ip);

      // Figure out energy deposit vertex; do in steps for clarity; original offset 
      // is just 3D vector difference between database shower vertex and point of 
      // energy deposit;
      TVector3 origOffset = dbHit->getHitCoord() - dbEntry->getPrimaryVtx();
      // Rotate to align with point->p();
      TVector3 rotOffset  = rr.Rotate(angle, axis) * origOffset;
      // Rotate by random angle along point->p() in addition and add to MoCaPoint vertex;
      TVector3 vtx = point->vtx() + rr.Rotate(yaangle, point->p()) * rotOffset;
      double master[3] = {vtx.x(), vtx.y(), vtx.z()}, local[3];

      // Figure out, which calorimeter cell fiducial volume this 3D point belongs to;
      CalorimeterLookupTableEntry *node = findHitNode(point, master, local);

      if (!node) continue;
      
      UGeo_t xy = node->gGeo;

      //
      // -> these codes should be unified with HandleHit(); later;
      //

      CalorimeterCell *cell = 
	&mCells[std::pair<UInt_t, UInt_t>(xy, mDigi->mAcknowledgePrimaryMother ? point->fPrimaryMotherId : 0)];
	  
      // Energy deposit accounting does not make sense here?;
      
      // If timing spectrum is needed on output (and unless this cell was initialized
      // already), do it now;
      if (mDigi->mTDim && !cell->fTimeSpectrum) cell->initializeTimeSpectrum(mDigi->mTDim);
      if (               !cell->mZCoordBins)        cell->initializeZSpectrum   (mDigi->mZDim);

      // Well, could use node->gNode here to assign cell->fzLen; assume, that calorimeter 
      // cells are all of the same length -> use same procedure as in HandleHit();
      if (!cell->fzLen) cell->fzLen = 2*fGeoH->GetSensorDimensionsPath(point->fVolumePath)[2];
      // Z-coordinate in the range [0..fzLen] rather than +/-fzLen/2; 
      double zCoord = local[2] + cell->fzLen/2.;

      // Respective bin in Z-plot;
      int nz = (int)floor(zCoord/(cell->fzLen/mDigi->mZDim));
      //assert(nz >= 0 && nz < mDigi->mZDim);
      if (nz < 0 || nz >= mDigi->mZDim) 
      {
	// Consider to regularize?; hard to control boundary conditions; think later;
	if (nz == -1) 
	  nz = 0;
	else
	  if (nz == mDigi->mZDim) 
	    nz = mDigi->mZDim - 1;
	  else
	    // Think on this condition later;
	    break;
      }			     

      CalorimeterCellZCoordBin *zbin = cell->mZCoordBins + nz;
	  
      double qTime = point->GetTime();
      // Take care to implemenet Birk's law here as well;
      assert(0);
      double dE    = scale * dbHit->getEnergyLoss();
      
      zbin->fHitNum++;
      zbin->mEnergyDeposit += dE;
      zbin->fTime          += dE*qTime;
      zbin->mZ             += dE*zCoord;
      
      // Check condition later, 
      if (HandleHitCore(cell)) 
      {
	std::cout << "-E- EicCalorimeterDigiHitProducer::HandleHitCore(): faled!" << std::endl;
	return -1;
      } //if
    } //for ip
  } //for iq
#endif

  return 0;
} // EicCalorimeterDigiHitProducer::ProcessFakeMoCaPoints()
#endif
// -------------------------------------------------------------------------

//
// FIXME: may want to optimize this stuff for performance later;
// 

void EicCalorimeterDigiHitProducer::FillEnergyDepositPlot(const char *name, double dE)
{
  // Allocate new element if needed; always 1000 channels, scale is configurable;
  if (mEnergyDepositPlots.find(name) == mEnergyDepositPlots.end()) 
    mEnergyDepositPlots[name] = 
      new TH1F(name, name, mEnergyDepositAccountingPlotNbins, 0.0, mEnergyDepositAccountingPlotDeMax * 1E3);

  mEnergyDepositPlots[name]->Fill(dE);
} // EicCalorimeterDigiHitProducer::FillEnergyDepositPlot()

// -------------------------------------------------------------------------

int EicCalorimeterDigiHitProducer::PostExec()
{
  // Require fake hit database to be present if MC file has fFakeMoCaPointArray
  // entries; just be consistent;
#if _LATER_
  if (mFakeMoCaPointArray->GetEntriesFast() && !mFakeDB)
  {
    std::cout << "-E- EicCalorimeterDigiHitProducer::PostExec(): fFakeMoCaPointArray entries present," << std::endl;
    std::cout << "-E- but no fake hit database available -> forgot useFakeMoCaPointDatabase() call?" << std::endl;
    return -1;
  } //if

  // Convert EicFakeMoCaPoint entries to CalorimeterCell ones in a way both branches
  // merge and the remainder of the code "does not notice" that part of the shower
  // was fake;
  if (mFakeDB && ProcessFakeMoCaPoints())
  {
    std::cout << "-E- EicCalorimeterDigiHitProducer::PostExec(): ProcessFakeMoCaPoints() failed!" << std::endl;
    return -1;
  } //if
#endif

  // Energy deposit sums (passive/sensitive/sensitive-after-Birk) over all accounted cells;
  // makes sense for test beam conditions only (like T1018 test run);
  std::map<std::string, EnergyDeposit> EnergyDepositSum;

  // Well, application should call setLightYield();
  if (!mDigi->mPrimaryLightYield)
  {
    std::cout << "-E- EicCalorimeterDigiHitProducer::PostExec(): light yield is not defined!" << std::endl;
    return -1;
  } //if

  switch (mDigi->mSensorType)
  {
  case CalorimeterDigiParData::SiPM:
    // And yet another sanity check; do once, or?;
    if (mDigi->mSipmSingleCellNoiseLevel && !mDigi->mTimingGateWidth)
    {
      std::cout << "-E- EicCalorimeterDigiHitProducer::PostExec(): simulating noise "
	"requires timing gate set!" << std::endl;
      return -1;
    } //if
    break;
  case CalorimeterDigiParData::APD:
    // No checks here, default values are useable;
    break;
  default:
    // Sensort type should be declared, no default;
    std::cout << "-E- EicCalorimeterDigiHitProducer::PostExec(): sensor type is not defined!" << std::endl;
    return -1;
  } //switch

  // And yet another one;
  if ((mDigi->mTimingGateWidth || mDigi->mTimeDim) && !mDigi->mLightPropagationVelocity)
  {
    std::cout << "-E- EicCalorimeterDigiHitProducer::PostExec(): timing control requires "
      "light propagation velocity set!" << std::endl;
    return -1;
  } //if

  // And also check, that at least one qSENSOR group is defined;
  if (mDigi->mSensors[0].mGroupType != CalorimeterSensorGroup::Sensor && 
      mDigi->mSensors[1].mGroupType != CalorimeterSensorGroup::Sensor)
  {
    std::cout << "-E- EicCalorimeterDigiHitProducer::PostExec(): at least one of the upstream/downstream "
      "ends should be equipped with sensors!" << std::endl;
    return -1;
  } //if

  // Just a shortcut; it looks like 'towerLength = 0' (unknown) case will work here; 
  const CalorimeterGeoParData* cGptr = dynamic_cast <const CalorimeterGeoParData*>(mGptr);
  const double towerLength = cGptr ? 0.1 * cGptr->mCellLength : 0.0;
  //const double towerLength = 0.1 * (dynamic_cast <const CalorimeterGeoParData*>(mGptr))->mCellLength;

  // Loop through all the cells and perform the actual digitization;
  for (std::map<ULogicalIndex_t, CalorimeterCell>::iterator it=mCells.begin(); it!=mCells.end(); ++it) {
    // Yes, want to work with 'cell' pointer rather than 'it->second';
    CalorimeterCell *cell = &it->second;

    for (std::map<std::pair<UInt_t, UInt_t>, CalorimeterCellParent>::iterator jt=cell->mCellParents.begin(); 
	 jt!=cell->mCellParents.end(); ++jt) {
      CalorimeterCellParent *parent = &jt->second;
    
      // Loop through all Z-bins independently; in principle could merge input from all 
      // parent particles before simulating the photon yield; yet want to have mPhotonCount
      // separately in th eoutput -> just do everything separately assuming energy deposits 
      // from different parents do not interfere due to something like Birk's saturation
      // effects (and I guess they should not);
      for(unsigned iz=0; iz<mDigi->mZCoordDim; iz++) {
	CalorimeterCellZCoordBin *zbin = parent->mZCoordBins + iz;

	if (!zbin->mEnergyDeposit) continue;
	
	// First normalize times and <z> in Z-bins;
	zbin->mTime /= zbin->mEnergyDeposit; zbin->mZ /= zbin->mEnergyDeposit;

	// Simulate separately contributions from upstream and downstream ends;
	for(unsigned ud=0; ud<2; ud++) {
	  CalorimeterSensorGroup *sgroup = mDigi->mSensors + ud;

	// If this sensor group end is dead, just skip;
	if (sgroup->mGroupType == CalorimeterSensorGroup::Dead) continue;

	// If sensor group is qREFLECTION, but reflectivity is 0.0, also skip;
	if (sgroup->mGroupType == CalorimeterSensorGroup::Reflection && !sgroup->mReflectivity) continue;
	
	//
	// The rest of this calculation is the same for both ::Reflection and ::Sensor
	// types; the only difference is how attenuation factor is calculated; well, and
	// arrival time as well;
	//

	// Estimate attenuation; account only longitudinal coordinate in the crystal, 
	// since (to first order) the rest of the optical path will affect all emitted 
	// photons in the same way; well, real life can be much more complicated of course;
	double attenuation = 1.0, distance = ud ? towerLength - zbin->mZ : zbin->mZ;

	if (mDigi->mAttenuationLength) attenuation *= exp(-distance/mDigi->mAttenuationLength);

	// If sensor group is in fact reflective end, rescale attenuation factor with 
	// a deterministic reflectivity factor and account extra attenuation length on 
	// the full way back to the other end; NB: it was checked during initialization,
	// that at least one qSENSOR group is present;
	if (sgroup->mGroupType == CalorimeterSensorGroup::Reflection)
	{
	  attenuation *= sgroup->mReflectivity;

	  if (mDigi->mAttenuationLength) 
	    attenuation *= exp(-towerLength/mDigi->mAttenuationLength); 
	} //if

	// FIXME: figure out what's wrong with this code;
#if _BACK_
	assert(0);

	// Calculate expected number of photons; ignore wave length, etc for now;
	// assume pure Poisson distribution for now; NB: assume 1/2 average yield 
	// for both ends;
	unsigned NG;
	switch (mDigi->mSensorType)
	{
	case CalorimeterDigiParData::SiPM:
	  //NG = gRandom->Poisson(zbin->mEnergyDeposit*mDigi->mLightYieldRescaled/2.);
	  NG = gRandom->Poisson(zbin->mEnergyDeposit*mDigi->mPrimaryLightYield/2.);
	  break;
#if _THINK_
	  // Since mDigi->mPrimaryLightYield is decoupled from sampling fraction 
	  // as of 2014/07/17, need to rething calculation for APD case one day;
	case CalorimeterDigiParData::APD:
	  // Rescaling back and forth here should take distribution widening in a proper 
	  // way here; may want to cross-check this later;
	  NG = (Int_t)(mDigi->mApdExcessNoiseFactor*gRandom->PoissonD(zbin->mEnergyDeposit*
								     (mDigi->mLightYieldRescaled/2.)/
								     mDigi->mApdExcessNoiseFactor));
	  break;
#endif
	default:
	  // Put something here if ever want to implement say PMTs;
	  assert(0);
	} //switch

	// Loop through all the expected photons; may eventually want to drop 
	// spectrum construction and save CPU time here;
	for(unsigned ng=0; ng<NG; ng++)
	{
	  // Account for attenuation; do it more efficiently later (just reduce 
	  // NG before entering this loop);
	  if (gRandom->Uniform() > attenuation) continue;

	  // Simulate decay time if needed; ignore possible spread in arrival times
	  // at the moment of energy deposit;
	  double dTime = zbin->fTime + (mDigi->mDecayConstant ? 
					gRandom->Exp(mDigi->mDecayConstant) : 0);

	  // Add propagation time if needed; add cell length in case of reflective end;
	  if (mDigi->mLightPropagationVelocity)
	    //dTime += (cell->fzLen - zbin->mZ)/mDigi->mLightPropagationVelocity;
	    dTime += (sgroup->groupType == qREFLECTION ? distance + cell->fzLen : distance)/
	      mDigi->mLightPropagationVelocity;
	  
	  // Increment entry in respective time bin;
	  if (mDigi->mTDim)
	  {
	    int tid = (int)floor(dTime/mDigi->mTBinWidth);
	    if (tid < 0 || tid >= mDigi->mTDim) continue;
	    
	    cell->fTimeSpectrum[tid]++;
	  } //if

	  // Attenuation estimate will require its own normalization;
	  //cell->fAttenuation += attenuation;
	  //attenuationNorm++;
	  
	  // Check gate condition and increment overall amplitude if fit; yes, 
	  // just use original double values here and inclusive limits;
	  if (!mDigi->mTimingGateWidth ||
	      (dTime >= mDigi->mTimingGateOffset && 
	       dTime <= mDigi->mTimingGateOffset + mDigi->mTimingGateWidth))
	    cell->mPhotonCount += 1;
	} //for ng 
#else
	switch (mDigi->mSensorType)
	{
	case CalorimeterDigiParData::SiPM:
	  parent->mSignalPhotonCount += 
	    gRandom->Poisson(attenuation*zbin->mEnergyDeposit*mDigi->mPrimaryLightYield/2.);	
	  break;
	case CalorimeterDigiParData::APD:
	  parent->mSignalPhotonCount += (Int_t)(mDigi->mApdExcessNoiseFactor*
						gRandom->PoissonD(attenuation*
								  zbin->mEnergyDeposit*(mDigi->mPrimaryLightYield/2.)/
								  mDigi->mApdExcessNoiseFactor));
	  break;
	default:
	  // Put something here if ever want to implement say PMTs;
	  assert(0);
	} //switch
#endif
	} //for ud
      } //for iz
    } //for jt

    // Well, eventually may want to put noise contribution in; 
    switch (mDigi->mSensorType)
    {
    case CalorimeterDigiParData::SiPM:
      // Assume for SiPM it is proportional to the GATE width; 
      cell->mNoisePhotonCount = 
	gRandom->Poisson(mDigi->mTimingGateWidth*mDigi->mSipmSingleCellNoiseLevel);
      break;
    case CalorimeterDigiParData::APD:
      // THINK: well, I guess I just need to rescale expected ASIC noise by APD gain factor
      // in order to equivalize it in "units" with the registered photon count;
      cell->mNoisePhotonCount = (Int_t)(gRandom->Gaus(0.0, mDigi->mApdEquivalentNoiseCharge)/
					mDigi->mApdGainFactor);
      break;
    default:
      // Put something here if ever want to implement say PMTs;
      assert(0);
    } //switch

    if (EnergyDepositAccountingRequested()) 
      for(std::map<std::string, EnergyDeposit>::iterator itr=it->second.mEnergyDeposits.begin(); 
	  itr!=it->second.mEnergyDeposits.end(); itr++) {

	// FIXME: '+' operator, please;
	EnergyDepositSum[itr->first].mPassive       += itr->second.mPassive;
	EnergyDepositSum[itr->first].mSensitive     += itr->second.mSensitive;
	EnergyDepositSum[itr->first].mSensitiveBirk += itr->second.mSensitiveBirk;
      } //if..fot it

    // Yes: cut away low cells right here?; NB: 
    //   - no light yield rescaling is needed here since I deal with cell as a whole!; 
    //   - it is assumed that this threshold is yet lower than any of the clustering 
    //     code thresholds; so the only purpose is to reduce digitized file throwing 
    //     away really very low deposits;
#if _THINK_
    //   - yet retain cell on output if energy accounting was requested; otherwise
    //     would through away cells with eg. only absorber dE deposit;
#endif
    if (//energyDepositAccountingRequested() || 
	// NB: small-deposit APD-equipped cells may go negative in photon 
	// count; default threshold is 0.0, so for these cells check is effective;
	// is it good or bad or does not matter?; think later;
	cell->GetPhotonCountSum() >= mDigi->mCleanupThreshold) {
      //#ifndef _OLD_STYLE_
      //ULogicalIndex_t xy_unscrewed = ((it->first >> 16) & 0x0003) | ((it->first << 2) & 0x000C);
      //#endif

      new((*mDigiHitArray)[mDigiHitArray->GetEntriesFast()]) 
	//#ifdef _OLD_STYLE_
	// I believe it is safe to use "cell" pointer here (so it is persistent), or?;
	EicCalorimeterDigiHit(it->first, cell);
      //#else
      //EicCalorimeterDigiHit(xy_unscrewed, cell);
      //#endif

      //printf("@@QQ@@ %8X %d\n", it->first, cell->GetPhotonCountSum());
    }
  } //for cells

  // Produce printout and fill out dE/dx plots;
  if (EnergyDepositAccountingRequested()) 
    for(std::map<std::string, EnergyDeposit>::iterator itr=EnergyDepositSum.begin(); 
	itr!=EnergyDepositSum.end(); itr++) {
      printf("%-20s -> %9.3f MeV (passive) ... %9.3f MeV (sensitive) ->"
	     "  %9.3f MeV (after Birk)\n", itr->first.c_str(), 
	     1E3*itr->second.mPassive, 1E3*itr->second.mSensitive, 1E3*itr->second.mSensitiveBirk);

      // FIXME: well, this stuff should clearly be performance optimized;
      if (itr->second.mPassive)
	FillEnergyDepositPlot((std::string(mDetName->NAME()) + std::string("-") + itr->first + 
			       std::string("-dE-passive")).c_str(), 1E3*itr->second.mPassive);
      
      // Want same statistics here?;
      if (itr->second.mSensitive) {
	FillEnergyDepositPlot((std::string(mDetName->NAME()) + std::string("-") + itr->first + 
			       std::string("-dE-sensitive")).c_str(), 1E3*itr->second.mSensitive);
	
	FillEnergyDepositPlot((std::string(mDetName->NAME()) + std::string("-") + itr->first + 
			       std::string("-dE-sensitive-Birk")).c_str(), 1E3*itr->second.mSensitiveBirk);
      } //
    } //if..for it

  return 0;
} // EicCalorimeterDigiHitProducer::PostExec()

// -------------------------------------------------------------------------

void EicCalorimeterDigiHitProducer::Finish()
{
  FairRun *fRun = FairRun::Instance();

  // I guess there is no need to save/restore current directory here?;
  fRun->GetOutputFile()->cd();

  // Save energy deposit plots if needed;
  for(std::map<std::string, TH1F*>::iterator itr=mEnergyDepositPlots.begin(); 
      itr!=mEnergyDepositPlots.end(); itr++)
    itr->second->Write();

  // Yes, save under detector-specific pre-defined name;
  //mDigi->Write(mDetName->Name() + "DigiParData");
  mDigi->Write(mDetName->Name() + "CalorimeterDigiParData");
} // EicCalorimeterDigiHitProducer::Finish()

// -------------------------------------------------------------------------

ClassImp(CalorimeterSensorGroup)
ClassImp(CalorimeterDigiParData)
ClassImp(EicCalorimeterDigiHitProducer)
