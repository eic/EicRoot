/** @file CbmDetectorList.cxx
 ** @author V.Friese  <V.Friese@Gsi.De>
 ** @date 29.04.2010
 **/

#include "CbmDetectorList.h"

#include "TString.h"


// -----   Constructor   ---------------------------------------------------
CbmDetectorList::CbmDetectorList() {
}
// -------------------------------------------------------------------------



// -----   GetSystemName   -------------------------------------------------
void CbmDetectorList::GetSystemName(DetectorId det, TString& name) {

  switch(det) {

  case kREF:  name = "ref";  break;
  case kMVD:  name = "mvd";  break;
  case kSTS:  name = "sts";  break;
  case kRICH: name = "rich"; break;
  case kMUCH: name = "much"; break;
  case kTRD:  name = "trd";  break;
  case kTOF:  name = "tof";  break;
  case kECAL: name = "ecal"; break;
  case kPSD:  name = "psd";  break;
  default:    name = "unknown"; break;

  }

}
// -------------------------------------------------------------------------



// -----   GetSystemName   -------------------------------------------------
void CbmDetectorList::GetSystemName(Int_t det, TString& name) {

  if ( det < kNOFDETS ) GetSystemName(DetectorId(det), name);
  else name = "unknown";

}
// -------------------------------------------------------------------------



// -----   GetSystemNameCaps   ---------------------------------------------
void CbmDetectorList::GetSystemNameCaps(DetectorId det, TString& name) {

  switch(det) {

  case kREF:  name = "REF";  break;
  case kMVD:  name = "MVD";  break;
  case kSTS:  name = "STS";  break;
  case kRICH: name = "RICH"; break;
  case kMUCH: name = "MUCH"; break;
  case kTRD:  name = "TRD";  break;
  case kTOF:  name = "TOF";  break;
  case kECAL: name = "ECAL"; break;
  case kPSD:  name = "PSD";  break;
  default:    name = "UNKNOWN"; break;

  }

}
// -------------------------------------------------------------------------



// -----   GetSystemNameCaps   ---------------------------------------------
void CbmDetectorList::GetSystemNameCaps(Int_t det, TString& name) {

  if ( det < kNOFDETS ) GetSystemNameCaps(DetectorId(det), name);
  else name = "unknown";

}
// -------------------------------------------------------------------------


ClassImp(CbmDetectorList)

