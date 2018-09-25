/** @file CbmModuleList.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 13.06.2013
 **/

#include "FairLogger.h"

#include "CbmModuleList.h"



// -----   Initialisation of the list of modules   -------------------------
map<Int_t, TString> CbmModuleList::DefineModules() {
  map<Int_t, TString> data;

  data[kRef]    = "ref";
  data[kMvd]    = "mvd";
  data[kSts]    = "sts";
  data[kRich]   = "rich";
  data[kMuch]   = "much";
  data[kTrd]    = "trd";
  data[kTof]    = "tof";
  data[kEcal]   = "ecal";
  data[kPsd]    = "psd";
  data[kDummy]  = "dummy";
  data[kMagnet] = "magnet";
  data[kTarget] = "target";
  data[kPipe]   = "pipe";

  return data;
}
// -------------------------------------------------------------------------



// -----   Initialise static map -------------------------- ----------------
// This is done by using the copy constructor of std::map, calling the
// method DefineModules, which actually fills the static map.
map<Int_t, TString> CbmModuleList::fModules(CbmModuleList::DefineModules());
// -------------------------------------------------------------------------



// ------  Get module Id from module name  ---------------------------------
Int_t CbmModuleList::GetModuleId(const char* moduleName) {

  map<Int_t, TString>::iterator it = fModules.begin();
  while ( it != fModules.end() ) {
    if ( ! (it->second).CompareTo(moduleName, TString::kIgnoreCase) )
      return it->first;
  it++;
  }
  return -1;

}
// -------------------------------------------------------------------------



// ------   Get module name from module Id   --------------------------------
TString CbmModuleList::GetModuleName(Int_t moduleId) {
  if ( fModules.find(moduleId) == fModules.end() ) {
    LOG(ERROR) << "Illegal module Id " << moduleId << FairLogger::endl;
    return "";
  }
  return fModules.find(moduleId)->second;
}
// -------------------------------------------------------------------------




