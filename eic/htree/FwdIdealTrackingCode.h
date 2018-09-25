//
// AYK (ayk@bnl.gov)
//
//  STAR-related EicIdealTrackingCode wrapper; used only as 
//  detector declaration and sensitive volume handler code;
//

#include <EicIdealTrackingCode.h>

#ifndef _FWD_IDEAL_TRACKING_CODE_
#define _FWD_IDEAL_TRACKING_CODE_

class FwdIdealTrackingCode : public EicIdealTrackingCode 
{
 public:
  FwdIdealTrackingCode() {};
  ~FwdIdealTrackingCode() {};  
        
  // Intercept few EicIdealTrackingCode virtual calls; yes, do not want
  // any activity from this code rather than whatever happens in Init() 
  // call and hit TCloneArrays which are filled out by the main code;
  void Register() {}; 
  void Exec(Option_t * option) {};              
  void Finish() {}; 

  ClassDef(FwdIdealTrackingCode,1);
};

#endif
