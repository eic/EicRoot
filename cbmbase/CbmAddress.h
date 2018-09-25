/** @file CbmAddress.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 23.05.2013
 **/

#ifndef CBMADDRESS_H
#define CBMADDRESS_H 1

#include "Rtypes.h"
#include "CbmModuleList.h"











/** @class CbmAddress
 ** @brief Base class for interfaces to the unique address
 ** @author V.Friese <v.friese@gsi.de>
 ** @version 1.0
 **
 ** CbmAddress is the base class for the concrete interfaces to the
 ** unique address, which is encoded in a 32-bit field (Int_t).
 ** The definition of this bit field is different for the various
 ** detector systems; common for all is that the first four bits are
 ** reserved for the system identifier.
 **
 **/
class CbmAddress
{
  public:


    /** Constructor  **/
    CbmAddress() { };


    /** Destructor  **/
    virtual ~CbmAddress() { };


    /** Number of bits for system Id in the address field
     ** @return Number of bits
     **/
    static Int_t GetNofSystemBits() { return fgkSystemBits; }


    /** Get the system Id from the address
     ** @param address  Unique address
     ** @return  systemId
     **/
    static Int_t GetSystemId(UInt_t address) {
      return address & ( ( 1 << fgkSystemBits ) - 1 );
    }


  protected:

    /** Number of bits for system Id in the address field  **/
    static const Int_t fgkSystemBits = 4;

};

#endif /* CBMADDRESS_H */
