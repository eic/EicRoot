// -------------------------------------------------------------------------
// -----                 PNDMCMATCHCREATORTASK header file             -----
// -----                  Created 20/03/07  by R.Kliemt               -----
// -------------------------------------------------------------------------


/** PNDMCMATCHCREATORTASK.h
 *@author T.Stockmanns <t.stockmanns@fz-juelich.de>
 **
 ** Displays all available informations for a given event
 **/


#ifndef CBMMCMATCHLOADERTASK_H
#define CBMMCMATCHLOADERTASK_H


// framework includes
#include "FairTask.h"

//#include <vector>
//#include <map>

class CbmMCMatch;
class TClonesArray;

class CbmMCMatchLoaderTask : public FairTask
{
 public:

  /** Default constructor **/
	CbmMCMatchLoaderTask();

  /** Destructor **/
  virtual ~CbmMCMatchLoaderTask();


  /** Virtual method Init **/
  virtual void SetParContainers();
  virtual InitStatus Init();


  /** Virtual method Exec **/
  virtual void Exec(Option_t* opt);

  virtual void Finish();



 private:

  TClonesArray* fMCLink;
  
  int fEventNr;
  CbmMCMatch* fMCMatch;

  void Register();

  void Reset();

  void ProduceHits();

  CbmMCMatchLoaderTask(const CbmMCMatchLoaderTask&);
  CbmMCMatchLoaderTask& operator=(const CbmMCMatchLoaderTask&);

  ClassDef(CbmMCMatchLoaderTask,1);

};

#endif
