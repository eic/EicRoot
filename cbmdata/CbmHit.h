// -------------------------------------------------------------------------
// -----                     CbmHit header file                        -----
// -----                 Created 16/11/07  by V. Friese                -----
// -------------------------------------------------------------------------

#ifndef CBMHIT_H
#define CBMHIT_H


#include "FairHit.h"


/**
 ** Abstract base class for hits used for tracking in CBM.
 ** Derives from FairHit. 
 ** Additional member is the covariance of x and y.
 ** Derived classes have to implement the pure virtual method GetStationNr()
 **
 **@author V.Friese <v.friese@gsi.de>
 **@since 16/11/07
 */

class CbmHit : public FairHit 
{

 public:

  /** Default constructor **/
  CbmHit();


  /** Constructor with hit parameters 
  *@param detId  Unique detector identifier
  *@param pos    Position vector [cm]
  *@param dpos   Error of position vector [cm]
  *@param covXY  Covariance of x and y [cm**2]
  *@param index  Reference index
  **/
  CbmHit(Int_t detID, TVector3& pos, TVector3& dpos, 
	    Double_t covXY, Int_t index);


  /** Destructor **/
  virtual ~CbmHit();


  /** Accessors **/
  Double_t GetCovXY() const { return fCovXY; }
  virtual Int_t GetStationNr() const = 0;


  /** Output to screen **/
  virtual void Print(const Option_t* opt = 0) const;



 protected:

  Double32_t fCovXY;          // Covariance of x and y coordinates



ClassDef(CbmHit,1);

};



#endif
