//
// AYK (ayk@bnl.gov), 2013/06/13
//
//  Calorimeter digi hit classes;
//

#include <map>

#include <TObject.h>

#include <EicGeoParData.h>

#ifndef _EIC_CALORIMETER_DIGI_HIT_
#define _EIC_CALORIMETER_DIGI_HIT_

class CalorimeterCellZCoordBin: public TObject {
 public:
 CalorimeterCellZCoordBin(): mHitNum(0), mEnergyDeposit(0), mTime(0), mZ(0) {};
  ~CalorimeterCellZCoordBin() {};
  
  // This counter makes sense only if fOneStepOneHit is set to kFALSE (?);
  UInt_t mHitNum;            // number of hits in this Z-bin

  Double32_t mEnergyDeposit; // energy deposit in this Z-bin
  Double32_t mTime;          // average time when energy deposit happened
  Double32_t mZ;             // average Z coordinate; may differ from Z-bin middle

  ClassDef(CalorimeterCellZCoordBin,1);
};

class EnergyDeposit: public TObject 
{
  friend class EicCalorimeterDigiHitProducer;

 public:
 EnergyDeposit(): mPassive(0.0), mSensitive(0.0), mSensitiveBirk(0.0) {};
  ~EnergyDeposit() {};

 private:
  Double32_t mPassive;       // energy deposit in passive material
  Double32_t mSensitive;     // energy deposit in sensitive material
  Double32_t mSensitiveBirk; // "effective" energy deposit after Birk's correction

  ClassDef(EnergyDeposit,3);
};

class CalorimeterCellParent: public TObject {
 public:
 CalorimeterCellParent(): mSignalPhotonCount(0), mZCoordDim(0), mZCoordBins(0) {};
  ~CalorimeterCellParent() { if (mZCoordBins) delete[] mZCoordBins; };
  
  void InitializeZCoordSpectrum(UInt_t nBins) {
    mZCoordDim  = nBins;
    mZCoordBins = new CalorimeterCellZCoordBin[nBins]; 
  };

  // Well, consider to retain this array in the output even if dim=1 (default);
  // 'mEnergyDeposit' is clearly redundant then, but 'mZ' gives average shower
  // depth estimate, and 'mTime' may also be useful;
  UInt_t mZCoordDim;                       //! {dE,<t>,<z>} vs Z-coord array dimension
  // Figure out why ROOT streaming does not work here;
  CalorimeterCellZCoordBin *mZCoordBins;   //! [mZCoordDim] 

  // Yes, for now just want to count photons; if ever come to something like 
  // smearing due to electronic effects, can introduce a different transient variable
  // in the reconstruction code; NB: want it signed because APD noise may send 
  // this value negative for "too low" cells, whatever that means for now;
  Long64_t mSignalPhotonCount;             // number of photons produced in this cell from this parent

  ClassDef(CalorimeterCellParent,3);
};

class CalorimeterCell: public TObject {
 public:
 CalorimeterCell(): mTimeDim(0), mTimeSpectrum(0)/*, mPhotonCountSum(0)*/, mNoisePhotonCount(0) {};
  ~CalorimeterCell() { 
    if (mTimeSpectrum) delete[] mTimeSpectrum; 
    
    mEnergyDeposits.clear();
  };
  
  void InitializeTimeSpectrum(UInt_t nBins) {
    mTimeDim      = nBins;
    mTimeSpectrum = new UInt_t[nBins]; 
    
    // Clang does not like this memset();
    //memset(mTimeSpectrum, 0x00, sizeof(mTimeSpectrum));
    for(unsigned iq=0; iq<nBins; iq++)
      mTimeSpectrum[iq] = 0;
  };
  
  std::map<std::string, EnergyDeposit> mEnergyDeposits; // energy deposit map in cell passive & sensitive material(s); [GeV] 
  
  // Transient variable used in the reconstruction code;
  mutable Double_t mEstimatedEnergyDeposit; //!
    
  // Basically number of registered photons in [ns] time bins;
  UInt_t mTimeDim;                         // time spectrum array dimension
  UInt_t *mTimeSpectrum;                   //[mTimeDim] time distribution of registered photons 
  
  // Noise contribution; 
  Long64_t mNoisePhotonCount;              // noise photons collected in this cell

  // Sum over all parent's mPhotonCount and noise contribution; transient;
  //Long64_t mPhotonCountSum;                //!

  // Index is <primary mother ID, secondary mother ID>; 
  std::map<std::pair<UInt_t, UInt_t>, CalorimeterCellParent> mCellParents; // inputs from different parent particles

  Long64_t GetPhotonCountSum() const {
    Long64_t sum = 0;

    // There was a clear bug fixed here on 2016/07/14 ('return' in a wrong place); check performance!!!;
    //assert(0);
    
    for (std::map<std::pair<UInt_t, UInt_t>, CalorimeterCellParent>::const_iterator jt=mCellParents.begin(); 
	 jt!=mCellParents.end(); ++jt) {
      const CalorimeterCellParent *parent = &jt->second;

      sum += parent->mSignalPhotonCount;
      
      //return sum + mNoisePhotonCount;
    } //for jt (parents)

    return sum + mNoisePhotonCount;
  };

  ClassDef(CalorimeterCell,42);
};
  
//
// Try to go without FairHit inheritance first; also no big reason 
// to inherit from EicDigiHit (used for tracking); think later;
//
class EicCalorimeterDigiHit: public TObject
{
  friend class EicCalorimeterReconstruction;

 public:
 EicCalorimeterDigiHit(): mUsed(0), mPtrCell(0) {};
 EicCalorimeterDigiHit(ULogicalIndex_t xy, /*UInt_t primaryMother,*/ const CalorimeterCell *cell): mUsed(0) {
    mCoord         = xy;
    //mPrimaryMother = primaryMother;
    mPtrCell       = cell;
  };
  ~EicCalorimeterDigiHit() {};

 private:
  // A working variable used in reconstruction code;
  UInt_t mUsed;                    //!

  ULogicalIndex_t mCoord;          // encoded XY coordinates of the respective calorimeter cell
  //UInt_t mPrimaryMother;         // primary mother ID
  const CalorimeterCell *mPtrCell; //-> pointer to cell frame
  
  ClassDef(EicCalorimeterDigiHit,11);
};

#endif
