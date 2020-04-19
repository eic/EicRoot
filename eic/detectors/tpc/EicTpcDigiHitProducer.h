//
// AYK (ayk@bnl.gov), 2013/06/12
//
//  "Ideal" TPC digitization code
//

#ifndef _EIC_TPC_DIGI_HIT_PRODUCER_
#define _EIC_TPC_DIGI_HIT_PRODUCER_

#include "EicTrackingDigiHitProducer.h"
#include "EicDigiParData.h"

class TpcDigiParData: public EicDigiParData
{
  // Yes, avoid complications; this "class" is a data placeholder;
  friend class EicTpcDigiHitProducer;
  //friend class EicCalorimeterReconstruction;

 public:
 TpcDigiParData(): EicDigiParData(), fTransverseDispersion(0.0), fLongitudinalDispersion(0.0), 
    fRadialIntrinsicResolution(0.0),
    fTransverseIntrinsicResolution(0.0), fLongitudinalIntrinsicResolution(0.0), fGemVerticalPadSize(0.0) {};
  ~TpcDigiParData() {};

  void Print();

 private:
  Double_t fTransverseDispersion;            // transverse   dispersion in [um]/sqrt(D[cm]);
  Double_t fLongitudinalDispersion;          // longitudinal dispersion in [um]/sqrt(D[cm]);

  // An effective resolution degradation, caused by clustering, electronics, etc; comes in 
  // quadrature with respective [um]/sqrt(D[cm]) term;
  Double_t fTransverseIntrinsicResolution;   // contribution, not associated with transverse   dispersion; [um]
  Double_t fRadialIntrinsicResolution;       // may want to override 1/sqrt(12) radial resolution estimate; [um]
  Double_t fLongitudinalIntrinsicResolution; // contribution, not associated with longitudinal dispersion; [um]

  Double_t fGemVerticalPadSize;              // GEM pad height; sort of determines number of hits; [cm]

  ClassDef(TpcDigiParData,7);
};

class EicTpcDigiHitProducer : public EicTrackingDigiHitProducer
{
 public:
 EicTpcDigiHitProducer(): EicTrackingDigiHitProducer("TPC", 
						     EicDigiHitProducer::Calculate) {
    digi = new TpcDigiParData();
    
    AssignDigiHitClassName("EicTrackingDigiHit3D");
  };

  ~EicTpcDigiHitProducer() {};

  int HandleHit(const EicMoCaPoint *point);

  // May want either to save assigned parameters or import them all at once;
  int exportTpcDigiParameters(const char *fileName);
  int importTpcDigiParameters(const char *fileName);

  // May want also to change by hand some of the parameters;
  void setTransverseDispersion(Double_t tDispersion) { 
    digi->fTransverseDispersion = tDispersion; 
  };
  void setLongitudinalDispersion(Double_t lDispersion) { 
    digi->fLongitudinalDispersion = lDispersion; 
  };
  void setLongitudinalIntrinsicResolution(Double_t lResolution) { 
    digi->fLongitudinalIntrinsicResolution = lResolution; 
  };
  void setTransverseIntrinsicResolution(Double_t tResolution) { 
    digi->fTransverseIntrinsicResolution = tResolution; 
  };
  void setRadialIntrinsicResolution(Double_t tResolution) { 
    digi->fRadialIntrinsicResolution = tResolution; 
  };
  void setGemVerticalPadSize(Double_t pitch) { digi->fGemVerticalPadSize = pitch; };

  void Print() { digi->Print(); };

 private:
  TpcDigiParData *digi;

  // Let EicTrackingDigiHitProducer know digi frame pointer;
  EicDigiParData *getEicDigiParDataPtr() { return digi; };
  //virtual void Finish();

  ClassDef(EicTpcDigiHitProducer,1);
};

#endif

