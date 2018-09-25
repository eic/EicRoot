/******************************************************************************
*  $Id: CbmRichRingSelect.h,v 1.1 2006/09/13 14:53:31 hoehne Exp $
*
*  Class  : CbmRichRingSelect
*  Description : Abstract base class for concrete RICH ring selection algorithm:
*                to be run after ring-track assign for fake-ring rejection
*
*  Author : Simeon Lebedev
*  E-mail : salebedev@jinr.ru
*
*******************************************************************************
*  $Log: CbmRichRingSelect.h,v $
*  Revision 1.1  2006/09/13 14:53:31  hoehne
*  initial version
*
*  
*
*******************************************************************************/


#ifndef CBM_RICH_RING_SELECT
#define CBM_RICH_RING_SELECT 1

#include "TObject.h"
//#include "CbmRichRing.h"
#include "TClonesArray.h"

class CbmRichRing;

class CbmRichRingSelect : public TObject
{

 public:

  /** Default constructor **/
 CbmRichRingSelect() : TObject(), fVerbose(0), fHitsArray(NULL) { };
  
  /** Destructor **/
  virtual ~CbmRichRingSelect() { };

  virtual void Init() { };

  virtual void DoSelect(CbmRichRing* ring) = 0;

   void SetVerbose(Int_t verbose) { fVerbose = verbose; };
   
  Int_t GetNofHitsOnRing(CbmRichRing* ring) { return -42; };
  Double_t GetAngle(CbmRichRing* ring) { return -42.0; };   
 protected:

  Int_t fVerbose;      // Verbosity level
  TClonesArray* fHitsArray;

 private:
 
  CbmRichRingSelect(const CbmRichRingSelect&);
  CbmRichRingSelect& operator=(const CbmRichRingSelect&);

  ClassDef(CbmRichRingSelect,1);

};

#endif
