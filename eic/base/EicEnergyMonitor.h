//
// AYK (ayk@bnl.gov), 2015/08/06
//
//  Energy monitor class;
//

#include <set>

#include <TObject.h>
#include <TString.h>
#include <TH1D.h>

#ifndef _EIC_ENERGY_MONITOR_
#define _EIC_ENERGY_MONITOR_

class EicEnergyMonitor: public TObject {
  friend class EicDetector;

 public:
  // FIXME: any initialization?;
  EicEnergyMonitor() {};
  // FIXME: parameter check?;
 EicEnergyMonitor(const char *volumeName, Int_t PDG, 
		  char *histogramName, double histogramMin,
		  double histogramMax, unsigned histogramBinNum):
  mName(volumeName), mPDG(PDG), 
    mHistogramName(histogramName), mPrimaryOnly(false), mAtEntrance(true),
    mHistogramMin(histogramMin), mHistogramMax(histogramMax), 
    mHistogramBinNum(histogramBinNum) {
      mHistogram = new TH1D(histogramName, histogramName, histogramBinNum, histogramMin, histogramMax);
  };
  ~EicEnergyMonitor() {};

  void PrimaryOnly()  { mPrimaryOnly = true; };
  void AtVolumeExit() { mAtEntrance = false; };

 private:
  // Well, do not want to create a name pattern hub since in this case will have to handle 
  // double counting of the same particle in potentially different volumes; keep things easy;
  TString mName;

  // Deal with specific particle type only; FIXME: make any type possible?;
  Int_t mPDG;
  // Record particle energy upon either entrance or exit;
  bool mPrimaryOnly;
  bool mAtEntrance;

  // Histogram parameters;
  TString mHistogramName;
  double mHistogramMin;
  double mHistogramMax;
  unsigned mHistogramBinNum;

  TH1D *mHistogram;

  // Protection against double entries on the same track; say primary particle exits 
  // TPC gas volume, hits outer field cage loosing most of its energy, starts curling 
  // in magnetic field and pretends to be registered in the histogram several times;
  // prohibit this: register only once per ID;
  std::set<unsigned> mRegisteredTracks;

  ClassDef(EicEnergyMonitor,3)  
};

#endif
