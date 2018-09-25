
// -------------------------------------------------------------------------
// -----                    CbmDigiManager header file                 -----
// -----                  Created 08/05/07  by V. Friese               -----
// -------------------------------------------------------------------------


/** CbmDigiManager
 *@author Volker Friese <v.friese@gsi.de>
 *@since 08.05.07
 *@version 1.0
 **
 ** CBM task class for random access to digi objects
 ** Task level RECO
 ** Access to digi by channel number
 **/


#ifndef CBMDIGIMANAGER_H
#define CBMDIGIMANAGER_H 1

#include "FairTask.h"

#include "TStopwatch.h"

#include <map>
#include <list>

class TClonesArray;
class CbmDigi;


enum System {MVD=1, STS, RICH, MUCH, TRD, TOF, ECAL, PSD};

class CbmDigiManager : public FairTask
{

 public:

  /**  Default constructor  **/
  CbmDigiManager();


  /**  Destructor  **/
  virtual ~CbmDigiManager();


  /**  Fill maps from detector / channel number to digi index **/
  virtual void Exec(Option_t* opt);


  /**  Access to digi 
   @param  iDetector  detector unique identifier
   *@param iChannel   channel number
   *@value            Pointer to digi object
   **/
  CbmDigi* GetDigi(Int_t iDetector, Int_t iChannel);




 private:

  CbmDigiManager(const CbmDigiManager&);
  CbmDigiManager& operator=(const CbmDigiManager&);

  /**  Initialisation: Get pointers to arrays.  **/
  virtual InitStatus Init();


  /**  Private data members  **/
  const char* fSystem[16];                  //  Name of systems
  TClonesArray* fDigis[16];                 //  Arrays of digis
  std::map<std::pair<Int_t, Int_t>, CbmDigi*> fDigiMap;  //! Digi map
  TStopwatch fTimer;                        //  Timer


  ClassDef(CbmDigiManager,1);

};

#endif


  

