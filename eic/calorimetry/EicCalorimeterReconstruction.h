//
// AYK (ayk@bnl.gov), 2013/06/13
//
//  Calorimeter reconstruction code classes;
//

#include <TClonesArray.h>

#include <FairTask.h>

#include <EicDetName.h>
#include <EicCalorimeterDigiHitProducer.h>

#ifndef _EIC_CALORIMETER_RECO_CODE_
#define _EIC_CALORIMETER_RECO_CODE_

class CalorimeterCellGroup: public TObject 
{
public:
 CalorimeterCellGroup(): mEnergy(0.0) {};
  ~CalorimeterCellGroup() {};

  // FIXME: may want to optimize ROOT streamer interface; now it's duplicating
  // cell pointers I guess;
  std::vector<const CalorimeterCell*> mCells;                     // calorimeter cell pointers

  Double_t mEnergy;                                               // full estimated energy of this cell group; [GeV]
  std::map<std::pair<UInt_t, UInt_t>, Double_t> mEnergyPerParent; // same, split over contributing parent particles

  ClassDef(CalorimeterCellGroup,15);
};

class CalorimeterRecoParData: public TObject
{
  // Yes, avoid complications; this "class" is a data placeholder;
  friend class EicCalorimeterReconstruction;

 public:
 CalorimeterRecoParData(): mClusterSeedThreshold(0.0), mNeighbourSearchThreshold(0.0), 
    mCellAccountingThreshold(0.0), mPhotonToEnergyConversionFactor(0.0)  {};
  ~CalorimeterRecoParData() {};

 private:
  // All thresholds are 0.0 per default; units are "estimated [GeV] based on light yield
  // provided during digitization; NB: it may be dangerous to set 'mCellAccountingThreshold' to 
  // a value lower than the one matching 'digi->mCleanupThreshold' since part of cells could 
  // have been rejected
  // during digitization phase already (so one should better re-run digitization with lower
  // value of 'digi->mCleanupThreshold'); also, it is more or less clear, that only 
  // "mClusterSeedThreshold > mNeighbourSearchThreshold > mCellAccountingThreshold" make sense; 
  // code will work with any input though, just produce a warning;
  Double_t mClusterSeedThreshold;     // cells above this [GeV] threshold can initiate new clusters
  Double_t mNeighbourSearchThreshold; // cells above this [GeV] threshold can add more neighbors to cluster
  Double_t mCellAccountingThreshold;  // cells below this [GeV] threshold will simply be ignored

  Double_t mPhotonToEnergyConversionFactor; // photon-to-energy scaling factor

  ClassDef(CalorimeterRecoParData,4);
};

#define _DEFAULT_LIGHT_YIELD_PLOT_MAX_   10000
#define _DEFAULT_LIGHT_YIELD_PLOT_NBINS_   100

class EicCalorimeterReconstruction : public FairTask {  
public:
  EicCalorimeterReconstruction() { ResetVars(); };
  EicCalorimeterReconstruction(TString name);

  ~EicCalorimeterReconstruction() {};

  void ResetVars() {
    mDigi = 0; mGptr = 0; mReco = 0;

    mLightYieldPlot          = 0;
    mLightYieldPlotMax       = _DEFAULT_LIGHT_YIELD_PLOT_MAX_;
    mLightYieldPlotNbins     = _DEFAULT_LIGHT_YIELD_PLOT_NBINS_;

    mLightYieldPlotRequested = false;
  };

  InitStatus Init();
  void Exec(Option_t * option);

  void AddNeighbors(CalorimeterCellGroup &group, std::multimap<UInt_t,EicCalorimeterDigiHit*>::reverse_iterator it);
  bool AreNeighbors(EicCalorimeterDigiHit *h1, EicCalorimeterDigiHit *h2);

  void SetClusterAlgorithmThresholds(Double_t clusterSeedThreshold, Double_t neighbourSearchThreshold, 
				     Double_t cellAccountingThreshold) {
    mReco->mClusterSeedThreshold     = clusterSeedThreshold;
    mReco->mNeighbourSearchThreshold = neighbourSearchThreshold;
    mReco->mCellAccountingThreshold  = cellAccountingThreshold;
  };

  void SetPhotonToEnergyConversionFactor(double factor) { 
    mReco->mPhotonToEnergyConversionFactor = factor;
  };

  void RequestLightYieldPlot(unsigned nMax = _DEFAULT_LIGHT_YIELD_PLOT_MAX_, 
			     unsigned nBins = _DEFAULT_LIGHT_YIELD_PLOT_NBINS_) { 
    mLightYieldPlotMax   = nMax;
    mLightYieldPlotNbins = nBins;

    mLightYieldPlotRequested = true;
  };

  virtual void Finish();

 protected:
  EicDetName *mDetName;                              //! 

  EicGeoParData *mGptr;                              //!
  CalorimeterDigiParData *mDigi;                     //!

  TClonesArray *mDigiHits;                           //! array of digi hits (input)

  // Output array of detector-specific cluster groups;
  TClonesArray* mClusterGroupArray;                  //! array of reco hits (output)

  // Yes, multimap, because photon counts can of course be the same for some cells;
  std::multimap<UInt_t,EicCalorimeterDigiHit*> hmap; //! 

  std::vector<CalorimeterCellGroup> mCellGroups;     //!

  TH1F* mLightYieldPlot;                             //!
  unsigned mLightYieldPlotMax, mLightYieldPlotNbins; //!

  // Want to dump this information into the output file;
  CalorimeterRecoParData *mReco;                     //!

  Bool_t mLightYieldPlotRequested;                   //!

  ClassDef(EicCalorimeterReconstruction,10);
};

#endif
