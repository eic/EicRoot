//
// AYK (ayk@bnl.gov), 2013/06/13
//
//  Calorimeter digi hit producer classes;
//

#include <TH1F.h>

//#include <EicFakeMoCaPoint.h>
//#include <EicFakeMoCaPointDatabase.h>
//#include <EicFakeMoCaPointDbEntry.h>
#include <EicCalorimeterDigiHit.h>
#include <EicDigiHitProducer.h>
#include <CalorimeterGeoParData.h>

#ifndef _EIC_CALORIMETER_DIGI_HIT_PRODUCER_
#define _EIC_CALORIMETER_DIGI_HIT_PRODUCER_

class CalorimeterSensorGroup: public TObject
{
 public:
  enum SensorGroupType {Dead, Reflection, Sensor};

 CalorimeterSensorGroup(): mGroupType(Dead), mReflectivity(1.0) {};
  ~CalorimeterSensorGroup() {};

  SensorGroupType mGroupType;   // either dead end (default) or reflective or actual sensor
  Double_t mReflectivity;       // fraction of reflected light if qREFLECTION end; 1. by default

  ClassDef(CalorimeterSensorGroup,3);
}; 

class CalorimeterDigiParData: public TObject
{
  // Yes, avoid complications; this "class" is a data placeholder;
  friend class EicCalorimeterDigiHitProducer;
  friend class EicCalorimeterReconstruction;

 public:
  enum SensorType {Off, SiPM, APD, PMT};

 CalorimeterDigiParData(): mTimeDim(0), mTimeRange(0.0), mTimeBinWidth(0.0), mCleanupThreshold(0), 
    mPrimaryLightYield(0.0), mTimingGateOffset(0.0), mTimingGateWidth(0.0),  
    mAttenuationLength(0.0), mDecayConstant(0.0), mLightPropagationVelocity(0.0), mZCoordDim(1),
    mSensorEfficiency(1.0), mSensorType(Off), mSipmSingleCellNoiseLevel(0.0), 
    mApdGainFactor(1.0), mApdEquivalentNoiseCharge(0.0), mApdExcessNoiseFactor(1.0),
    mSipmSingleCellDynamicRange(0) {};
  ~CalorimeterDigiParData() {};

 private:
  // Time array dimension; default is 0 (not needed);
  UInt_t mTimeDim;                    // number of bins in cell timing plot
  // Full scale in [ns] and bin width;
  Double_t mTimeRange;                // cell timing plot range in [ns]
  Double_t mTimeBinWidth;             //! 
  
  // Cells below this threshold will not be present in digi file output;
  unsigned mCleanupThreshold;         // single cell threshold in [photon]; default is 0

  // This is the number of photons produced per GeV deposited energy (after Birk's 
  // correction); overall "capture rate" and "inefficiency" is accounted here; the 
  // actual detected photon count will naturally be smaller because of poor reflection 
  // on one of the ends and attenuation length; this value should be tuned by hand in 
  // order to match *registered* photon count (see respective 1D histogram) and the 
  // measured rate; can not be done vice versa (setting *measured* average light yield)
  // because of an interplay of shower depth and attenuation length which is hard to 
  // "reverse engineer" into the primary yield without having processed the MC sample
  // on-the-fly prior to the actual digitization; in other words: for a config with 
  // 1) 100% sampling fraction, 2) one end having 100% reflection mirror, 3) other end 100% 
  // efficient SiPM, 4) very long attenuation length -> this number will match the 
  // "measured" number of photons in 1D plot;
  Double_t mPrimaryLightYield;        // expected max photon yield per [GeV]; no default

  // Timing gate parameters; default: accept all;
  Double_t mTimingGateOffset;         // timing gate offset in [ns]
  Double_t mTimingGateWidth;          // timing gate width  in [ns]; NB: it is unrelated to fTRange

  SensorType mSensorType;             // SiPM/APD; digitization differs a bit (see below)

  // NB: in order not to complicate things, even if sensors are installed on both ends, 
  // noise is accounted only once (it is anyway not specified, how many APD/SiPMs are installed 
  // where, right?); 
  Double_t mSipmSingleCellNoiseLevel; // SiPM noise level [counts/ns]; default: 0.0; gate setting required! 
  // So if every cell is equipped with 4x 25um 3x3mm^2 sensors, this number should be 4*14400;
  // ignored for now; later on may want to simulate saturation effects;
  UInt_t mSipmSingleCellDynamicRange; // max number of pixels per cell (and assume uniform light collection)

  Double_t mApdEquivalentNoiseCharge; // APD ENC [e-] for selected operating conditions; default: 0.0
  Double_t mApdGainFactor;            // APD gain factor; default: 1.0
  Double_t mApdExcessNoiseFactor;     // APD ENF; default: 1.0

  Double_t mAttenuationLength;        // material attenuation length in [cm]; default: infinity

  // Want to have some simplified yet realistic sensor model; at the very least account
  // for possible reflection and extra attenuation on the way back; both ends can be 
  // equipped with (identical) sensors, but photon counts will be added; later may want 
  // to consider split between sensors (but at that point will have to implement fiber-to-sensor
  // routing logic); will very likely not be needed, so skip for now;
  CalorimeterSensorGroup mSensors[2]; // upstream and downstream sensor groups
  // Of no use for now; assume, that it suffices to set light yield and 
  // attenuation length to some reasonable number;
  Double_t mSensorEfficiency;         // something like overall QE & fill factor; 1.0 by default

  Double_t mDecayConstant;            // scintillating material decay constant in [ns]; default is 0

  // Default value is "infinity";
  Double_t mLightPropagationVelocity; // light propagation velocity in the material in [cm/ns]

  UInt_t mZCoordDim;                  // number of bins in {dE,<t>,<z>} vs Z-coord plot; default is 1

  //EicNamePatternHub mSensitiveVolumes;// sensitive volumes with their respective Birks' constants

  ClassDef(CalorimeterDigiParData,20);
};

class EicCalorimeterDigiHitProducer: public EicDigiHitProducer
{
 public:
  EicCalorimeterDigiHitProducer() { ResetVars();};
  EicCalorimeterDigiHitProducer(const char *name/*, TString inFileName*/);
  ~EicCalorimeterDigiHitProducer() {};

  // Have to maintain default constructor as well -> split variable reset
  // call in a separate routine;
  void ResetVars() { 
    /*mFakeDB = 0; mFakeMoCaPointArray = 0;*/ mLastOkNode = 0; 
    mEnergyDepositAccounting = false; mDigi = new CalorimeterDigiParData(); 
    mEnergyDepositAccountingPlotDeMax = 0.0;
  };

  //int UseFakeMoCaPointDatabase(const char *dbFileName, UInt_t energyBinNum);

  // Per default cell distributions will have no timing info on output;
  int RequestTimeSpectra(Double_t tRange, UInt_t tDim);
  void RequestEnergyDepositAccounting(double dEmax = 1.0, unsigned nBins = 100) { 
    mEnergyDepositAccountingPlotDeMax = dEmax;
    mEnergyDepositAccountingPlotNbins = nBins;
    mEnergyDepositAccounting = true;
  };
  Bool_t EnergyDepositAccountingRequested() const { return mEnergyDepositAccounting; };
  // NB: "const" modifier here and below is ~fake (pointer contents is obviously affected);
  void SetZBinning(UInt_t zDim) const { mDigi->mZCoordDim = zDim; };
  void SetCleanupThreshold(unsigned threshold) const { mDigi->mCleanupThreshold = threshold; };
  void SetPrimaryLightYield(Double_t yield) const { mDigi->mPrimaryLightYield = yield; };
  void SetTimingGate(Double_t gateMin, Double_t gateMax) const { 
    mDigi->mTimingGateOffset = gateMin; 
    mDigi->mTimingGateWidth  = gateMax - gateMin;
  };

  void SetSensorType(CalorimeterDigiParData::SensorType sType) const { mDigi->mSensorType = sType;};

  void SetSipmSingleCellNoiseLevel(Double_t noiseLevel) const { 
    mDigi->mSipmSingleCellNoiseLevel = noiseLevel; 
  };
  void SetSipmSingleCellDynamicRange(UInt_t range) const { 
    mDigi->mSipmSingleCellDynamicRange = range; 
  };

  // FIXME: how about validity checks here?; later;
  void SetApdEquivalentNoiseCharge(Double_t ENC) const { mDigi->mApdEquivalentNoiseCharge = ENC; };
  void SetApdGainFactor(Double_t M) const { mDigi->mApdGainFactor= M; };
  void SetApdExcessNoiseFactor(Double_t ENF) const { mDigi->mApdExcessNoiseFactor = ENF; };

  void SetAttenuationLength(Double_t attenuationLength) const { 
    mDigi->mAttenuationLength = attenuationLength; 
  };
  void SetDecayConstant(Double_t decayConstant) const { mDigi->mDecayConstant = decayConstant; };
  void SetLightPropagationVelocity(Double_t velocity) const { 
    mDigi->mLightPropagationVelocity = velocity; 
  };
  //void SetGeantPrimaryMotherAcknowledgementFlag() const { mDigi->mAcknowledgePrimaryMother = true; };

  // Well, prefer to have two separate calls for a better readability; 
  void ConfigureUpstreamSensorGroup  (CalorimeterSensorGroup::SensorGroupType sType, 
				      Double_t reflectivity = 1.0) const { 
    ConfigureSensorGroupCore(mDigi->mSensors + 0, sType, reflectivity);
  }
  void ConfigureDownstreamSensorGroup(CalorimeterSensorGroup::SensorGroupType sType, 
				      Double_t reflectivity = 1.0) const { 
    ConfigureSensorGroupCore(mDigi->mSensors + 1, sType, reflectivity);
  };

  InitStatus ExtraInit();

  // Yes, for now the whole point of this class is to handle hit
  // production in a way totally different from the default method
  // of EicDigiHitProducer class;
  int PreExec();
  int PostExec();
  int HandleHit(const EicMoCaPoint *point);

  // Shared between HandleHit() and ProcessFakeMoCaPoints(); empty call for now;
  int HandleHitCore(CalorimeterCell *cell);

  virtual void Finish();

 private:
  // Just import mapping table from the Monte-Carlo file;
  //TString mInFileName;                                         //!

  // Map of cells which were hit; index is <xy,primary_mother>;
  //std::map<std::pair<UInt_t, UInt_t>, CalorimeterCell> mCells; //!
  // Map of cells which were hit; index is "xy";
  std::map<ULogicalIndex_t, CalorimeterCell> mCells;             //!

  // May want to account true energy deposits associated with all cells;
  Bool_t mEnergyDepositAccounting;                               //!
  double mEnergyDepositAccountingPlotDeMax;                      //!
  unsigned mEnergyDepositAccountingPlotNbins;                    //!

  // Want to dump this information into the output file;
  CalorimeterDigiParData *mDigi;                                 //!

  //EicFakeMoCaPointDatabase *mFakeDB;                             //!

  void FillEnergyDepositPlot(const char *name, double dE);

  std::map<std::string, TH1F*> mEnergyDepositPlots;              //!

  // Input array of detector-specific EicFakeMoCaPoint's;
  //TClonesArray* mFakeMoCaPointArray;                             //!

  // A working variable;
  LogicalVolumeLookupTableEntry *mLastOkNode;                    //!
  //int ProcessFakeMoCaPoints( void );

  //LogicalVolumeLookupTableEntry *findHitNode(EicFakeMoCaPoint *point, double master[3], double local[3]);

  void ConfigureSensorGroupCore(CalorimeterSensorGroup *sgroup, 
				CalorimeterSensorGroup::SensorGroupType sgType, Double_t reflectivity) const {
    sgroup->mGroupType     = sgType;
    // Set for all types, even that it makes sense for qREFLECTION only; well, how about 
    // range [0..1] check?; fix later;
    sgroup->mReflectivity = reflectivity;
  };

  ClassDef(EicCalorimeterDigiHitProducer,47);
};

#endif
