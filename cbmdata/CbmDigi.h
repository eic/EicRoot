/** @file CbmDigi.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 30.04.2013
 **/


#ifndef CBMDIGI_H
#define CBMDIGI_H 1


#include "TObject.h"


class FairMultiLinkedData;



/** @class CbmDigi
 ** @brief Base class for persistent representation of digital information.
 ** @author V.Friese <v.friese@gsi.de>
 ** @version 2.0
 **
 ** CbmDigi is an abstract base class for the ROOT representation of
 ** the smallest information unit delivered by the detector front-ends.
 ** It is equivalent to the message of a single electronics channel.
 ** The information content is the channel address (unique identifier),
 ** the time stamp and (optionally) the charge (ADC).
 **
 ** Unlike the data class used to actually transport the information
 ** through the data acquisition ("message"), the digi contains
 ** context-free information, i.e. absolute time and unique address.
 **
 ** The class contains a pointer to an object of type FairMultiLinkedData,
 ** where the link information to MCPoints can be stored and retrieved.
 ** If there is no such information (as in case of real data), the pointer
 ** will be NULL.
 **
 ** Note that the Digi class is not responsible for the validity of the
 ** pointer. Copy constructor and assignment operator will perform a
 ** shallow copy only. It is the user's responsibility to ensure the validity
 ** of the pointer and to delete the FairMultiLinkedData wherever appropriate.
 **/
class CbmDigi : public TObject
{

 public:

  /** Default constructor  **/
  CbmDigi();


  /** Destructor  **/
  virtual ~CbmDigi() { };


  /** Unique channel address  **/
  virtual Int_t    GetAddress() const = 0;


  /** Charge (optional)  **/
  virtual Double_t GetCharge()  const { return 0.; }


  /** Get pointer to link collection **/
  FairMultiLinkedData* GetLinks() const { return fLinks; }


  /** System (enum DetectorId) **/
  virtual Int_t    GetSystemId() const = 0;


  /** Absolute time [ns]  **/
  virtual Double_t GetTime()    const = 0;


  /** Set pointer to link collection **/
 void SetLinks(FairMultiLinkedData* links) { fLinks = links; }

  

 protected:

  FairMultiLinkedData* fLinks; ///< Monte-Carlo link collection

  CbmDigi(const CbmDigi&);
  CbmDigi& operator=(const CbmDigi&);

  ClassDef(CbmDigi,2);

};

#endif
