//
// AYK (ayk@bnl.gov), 2013/06/13
//
//  Calorimeter reconstruction code classes;
//

#include <assert.h>
#include <iostream>
#include <cstdlib>

#include <EicRunAna.h>
#include <EicCalorimeterReconstruction.h>

// ---------------------------------------------------------------------------------------

EicCalorimeterReconstruction::EicCalorimeterReconstruction(TString name):
  FairTask("Test version of " + name + " reconstruction code")
{
  ResetVars();

  mDetName = new EicDetName(name);

  mReco = new CalorimeterRecoParData();
} // EicCalorimeterReconstruction::EicCalorimeterReconstruction()

// ---------------------------------------------------------------------------------------

InitStatus EicCalorimeterReconstruction::Init() 
{
  FairRootManager *fManager = FairRootManager::Instance();

  // As of 2015/07/17 require that reconstruction.C is started with EicRunAna rather
  // than FairRunAna, sorry;
  EicRunAna *fRun = EicRunAna::Instance();
  if (!fRun) {
    std::cout << "-E- EicCalorimeterReconstruction::Init(): no EicRunAna instance found!" << std::endl;
    std::cout << "-E- EicCalorimeterReconstruction::Init(): please use EicRunAna instead of FairRunAna!" << std::endl;
    return kERROR;
  } //if

  // Read mapping table; missing mapping table is a critical failure;
  {
    TFile fgeo(fRun->GetInputFileName());
    fgeo.GetObject(mDetName->Name() + "GeoParData", mGptr);

    fgeo.Close();

    if (!mGptr) {
      std::cout << "-E- EicCalorimeterReconstruction::Init(): no mapping info found!" << std::endl;
      return kERROR;
    } //if
  }

  // One of the friends must be digitization file; find it;
  {
    for(unsigned fr=0; fr<fRun->GetFriendFiles().size(); fr++) {
      TFile fin(fRun->GetFriendFiles()[fr]);
    
      if (fin.IsOpen()) {
	fin.GetObject(mDetName->Name() + "CalorimeterDigiParData", mDigi);
	fin.Close();
      
	if (mDigi) break;
      } //if
    } //for fr 

    if (!mDigi) {
      std::cout << "-E- EicCalorimeterReconstruction::Init(): no digi info found!" << std::endl;
      return kERROR;
    } //if
  }

  // Check threshold settings consistency;
  if (mReco->mClusterSeedThreshold     < mReco->mNeighbourSearchThreshold || 
      mReco->mNeighbourSearchThreshold < mReco->mCellAccountingThreshold)
    std::cout << "-W- " << mDetName->NAME() << " clustering algorithm: strange threshold settings!" << 
      std::endl;

  // Find digitized hit array;
  //mDigiHits = (TClonesArray *)fManager->GetObject(mDetName->Name() + "DigiHit");
  mDigiHits = (TClonesArray *)fManager->GetObject(mDetName->Name() + "CalorimeterDigiHit");
  if ( ! mDigiHits ) {
    std::cout << "-W-  EicIdealTracker::Init: No " << mDetName->Name() << "Hit array!" << std::endl;
    return kERROR;
  } //if

  mClusterGroupArray = new TClonesArray("CalorimeterCellGroup");
  //mClusterGroupArray = new TClonesArray("EicMoCaPoint");
  fManager->Register(mDetName->Name() + "ClusterGroup", mDetName->NAME(), mClusterGroupArray, kTRUE);

  // Allocate a single 1D histogram with the "registered" light yield;
  if (mLightYieldPlotRequested)
    mLightYieldPlot = new TH1F(mDetName->Name() + "LightYield", mDetName->NAME() + " Light Yield", 
			       mLightYieldPlotNbins, 0.0, 1.0 * mLightYieldPlotMax);

  return kSUCCESS;
} // EicCalorimeterReconstruction::Init()

// ---------------------------------------------------------------------------------------

void EicCalorimeterReconstruction::Exec(Option_t * option) 
{
  mCellGroups.clear();
  hmap.clear();

  mClusterGroupArray->Clear();

  // Reset all "mUsed" flags first; can not do this in the main loop because of recursion;
  // also use this loop to build energy-ordered list of pointers;
  for (Int_t it=0; it<mDigiHits->GetEntriesFast(); it++) 
  {
    EicCalorimeterDigiHit *ghit = (EicCalorimeterDigiHit*)mDigiHits->At(it);

    ghit->mUsed = 0;

    {
      const CalorimeterCell *cell = ghit->mPtrCell;

      // Used twice here -> create an intermediate variable;
      Long64_t cellPhotonCount = cell->GetPhotonCountSum();
      
      // This value will be used in a few places (threshold checks);
      cell->mEstimatedEnergyDeposit = mReco->mPhotonToEnergyConversionFactor * cellPhotonCount;

      // No reason to deal with "too low" cells at all; just do not include them
      // in the ordered cell map;
      if (cell->mEstimatedEnergyDeposit >= mReco->mCellAccountingThreshold)
	hmap.insert(std::pair<UInt_t,EicCalorimeterDigiHit*>(cellPhotonCount, ghit));
    }
  } //for it (hits)

  // Loop through all hits and arrange cluster groups;
  for (std::multimap<UInt_t,EicCalorimeterDigiHit*>::reverse_iterator it=hmap.rbegin(); it!=hmap.rend(); it++)
  {
    EicCalorimeterDigiHit *ghit = it->second;
    const CalorimeterCell *cell = ghit->mPtrCell;

    //printf("%3d %3d -> %5d (%d)\n", (ghit->mCoord >> 16) & 0xFFFF, ghit->mCoord & 0xFFFF, 
    //	   cell->GetPhotonCountSum(), ghit->mUsed);
    {
      printf("@@QQ@@  %8llX -> %3d %3d -> %5d (%d)\n", ghit->mCoord, 
	     mGptr->GetX(ghit->mCoord), mGptr->GetY(ghit->mCoord),
	     (unsigned)cell->GetPhotonCountSum(), ghit->mUsed);
    }      

    // Check that this cell is above cluster seed threshold; actually once a cell is 
    // found which does not satisfy this condition, the following ones can be ignored as 
    // well (multimap is ordered in cell photon counts); so perhaps "break" here would 
    // be more appropriate; fix later;
    if (cell->mEstimatedEnergyDeposit < mReco->mClusterSeedThreshold) continue;

    // Skip cells which already belong to other cluster groups;
    if (ghit->mUsed) continue;

    // Since this cell was not accounted so far in any cluster group, start a new one;
    ghit->mUsed = 1;

    CalorimeterCellGroup group;
    group.mCells.push_back(cell);

    AddNeighbors(group, it);

    mCellGroups.push_back(group);
  } //for it (hits)

  // Loop through all cell groups.;
  {
    Long64_t eventPhotonCountSum = 0;

    printf("%d group(s)\n", (unsigned)mCellGroups.size());
    
    for (unsigned cg=0; cg<mCellGroups.size(); cg++) {
      CalorimeterCellGroup *cgroup = &mCellGroups[cg];

      Long64_t groupPhotonCountSum = 0; 

      for (unsigned iq=0; iq<cgroup->mCells.size(); iq++) {
	const CalorimeterCell *cell = cgroup->mCells[iq];

	groupPhotonCountSum += cell->GetPhotonCountSum();
      } //for iq (mCells)

      // Consider to account for the simulated average noise level here; subtract expectation value;
      switch (mDigi->mSensorType) {
      case CalorimeterDigiParData::SiPM:
	groupPhotonCountSum -= int(cgroup->mCells.size()*((int)mDigi->mTimingGateWidth*mDigi->mSipmSingleCellNoiseLevel));
	break;
      case CalorimeterDigiParData::APD:
	// Seems nothing to do here (digitization added gaussian with a 0.0 mean);
	break;
      default:
	// Put something here if ever want to implement say PMTs;
	assert(0);
      } //switch
      
      cgroup->mEnergy = groupPhotonCountSum * mReco->mPhotonToEnergyConversionFactor;
      printf("  -> %8.5f GeV\n", cgroup->mEnergy);

      eventPhotonCountSum += groupPhotonCountSum;

      // And eventually estimate per-parent energy share; at this point I should assume 
      // that I know exact photon distribution over parents, which pretty much gives 
      // respective fractions; arrange a separate loop for clarity;
      {
	Long64_t groupSignalPhotonCountSum = 0; 
	std::map<std::pair<UInt_t, UInt_t>, Long64_t> perParentSignalPhotonCountSum;
	
	for (unsigned iq=0; iq<cgroup->mCells.size(); iq++) {
	  const CalorimeterCell *cell = cgroup->mCells[iq];

	  for (std::map<std::pair<UInt_t, UInt_t>, CalorimeterCellParent>::const_iterator jt=cell->mCellParents.begin(); 
	       jt!=cell->mCellParents.end(); ++jt) {
	    const CalorimeterCellParent *parent = &jt->second;

	    perParentSignalPhotonCountSum[jt->first] += parent->mSignalPhotonCount;
	    groupSignalPhotonCountSum                += parent->mSignalPhotonCount;
	  } //for jt (parents)
	} //for iq (mCells)

	// Eventually may fill out guessed energy share per contributing parent particle;
	for (std::map<std::pair<UInt_t, UInt_t>, Long64_t>::iterator jt=perParentSignalPhotonCountSum.begin(); 
	       jt!=perParentSignalPhotonCountSum.end(); ++jt) 
	  cgroup->mEnergyPerParent[jt->first] = cgroup->mEnergy * (1.0*jt->second/groupSignalPhotonCountSum);
      }
    } //for it (groups)
    
    // Well, the meaning of this (test) plot is to show estimated overall photon yield;
    // indeed a single particle simulation mode expected; so sum up everything (and better
    // take care to set all the thresholds in SetClusterAlgorithmThresholds() to 0.0;
    if (mLightYieldPlot) mLightYieldPlot->Fill(eventPhotonCountSum);
  }

  // Do this more efficienctly later (remove intermediate "groups" vector);
  for (std::vector<CalorimeterCellGroup>::iterator it=mCellGroups.begin(); it!=mCellGroups.end(); ++it)
    new((*mClusterGroupArray)[mClusterGroupArray->GetEntriesFast()]) 
      CalorimeterCellGroup(*it);
} // EicCalorimeterReconstruction::Exec()

// ---------------------------------------------------------------------------------------

void EicCalorimeterReconstruction::AddNeighbors(CalorimeterCellGroup &group, 
						std::multimap<UInt_t,EicCalorimeterDigiHit*>::reverse_iterator it)
{
  for (std::multimap<UInt_t,EicCalorimeterDigiHit*>::reverse_iterator iq=hmap.rbegin(); iq!=hmap.rend(); iq++)
  {
    EicCalorimeterDigiHit *ghit = iq->second;
    const CalorimeterCell *cell = ghit->mPtrCell;

    if (ghit->mUsed) continue;

    // Will add all the neighbours recursively; 
    if (AreNeighbors(it->second, ghit)) 
    {
      ghit->mUsed = 1;
      group.mCells.push_back(cell);

      // Add neighbors of this cell only if it is above certain threshold in energy; this 
      // should in principle help to decouple cluster groups which "touch" themselves
      // through the cells which are inbetween "mReco->fCellAccountingThreshold" and 
      // "mReco->fNeighbourSearchThreshold" in estimated energy deposits; check on that, please!;
      if (cell->mEstimatedEnergyDeposit >= mReco->mNeighbourSearchThreshold) AddNeighbors(group, iq);
    } //if
  } //for iq (hits)
} // EicCalorimeterReconstruction::AddNeighbors()

// ---------------------------------------------------------------------------------------

bool EicCalorimeterReconstruction::AreNeighbors(EicCalorimeterDigiHit *h1, EicCalorimeterDigiHit *h2)
{
  // Yes, want to separate cell hits from different primary parents for now;
  //if (h1->mPrimaryMother != h2->mPrimaryMother) return false;

  return mGptr->AreNeighbours(h1->mCoord, h2->mCoord);
} // EicCalorimeterReconstruction::AreNeighbors()

// ---------------------------------------------------------------------------------------

void EicCalorimeterReconstruction::Finish()
{
  FairRun *fRun = FairRun::Instance();

  // I guess there is no need to save/restore current directory here?;
  fRun->GetOutputFile()->cd();

  if (mLightYieldPlot) mLightYieldPlot->Write();

  // Yes, save under detector-specific pre-defined name;
  mReco->Write(mDetName->Name() + "RecoParData");
} // EicCalorimeterReconstruction::Finish()

// ---------------------------------------------------------------------------------------

ClassImp(CalorimeterRecoParData)
ClassImp(CalorimeterCellGroup)
ClassImp(EicCalorimeterReconstruction)
