// -------------------------------------------------------------------------
// -----                 PNDMCMATCHCREATORTASK header file             -----
// -----                  Created 20/03/07  by R.Kliemt               -----
// -------------------------------------------------------------------------


/** PNDMCMATCHCREATORTASK.h
 *@author T.Stockmanns <t.stockmanns@fz-juelich.de>
 **
 ** Displays all available informations for a given event
 **/


#ifndef PNDMCMATCHCREATORTASK_H
#define PNDMCMATCHCREATORTASK_H


// framework includes
#include "FairTask.h"

#include <map>

class CbmMCMatch;
class TClonesArray;

class CbmMCMatchCreatorTask : public FairTask
{
 public:

  /** Default constructor **/
	CbmMCMatchCreatorTask();

  /** Destructor **/
  virtual ~CbmMCMatchCreatorTask();


  /** Virtual method Init **/
  virtual void SetParContainers();
  virtual InitStatus Init();


  /** Virtual method Exec **/
  virtual void Exec(Option_t* opt);

  virtual void Finish();



 private:
  InitStatus InitBranches();
  std::map<std::string, TClonesArray*> fBranches;

  TClonesArray* fMCLink;//->

  int fEventNr;
  CbmMCMatch* fMCMatch;

  void Register();

  void Reset();

  void ProduceHits();

  CbmMCMatchCreatorTask(const CbmMCMatchCreatorTask&);
  CbmMCMatchCreatorTask& operator=(const CbmMCMatchCreatorTask&);

  ClassDef(CbmMCMatchCreatorTask,1);

};

#endif
