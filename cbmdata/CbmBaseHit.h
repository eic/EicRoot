/**
 * \file CbmBaseHit.h
 * \author Andrey Lebedev <andrey.lebedev@gsi.de>
 * \date 2009
 *
 * Base class for hits used for tracking in CBM.
 * Derives from TObject.
 * Each hit has its unique identifier of type HitType,
 * which can be later use for safe type casting.
 **/
#ifndef CBMBASEHIT_H_
#define CBMBASEHIT_H_

enum HitType {
	kHIT,
	kPIXELHIT,
	kSTRIPHIT,
	kSTSHIT,
	kMVDHIT,
	kRICHHIT,
	kMUCHPIXELHIT,
	kMUCHSTRAWHIT,
	kTRDHIT,
	kTOFHIT,
	kECALHIT
};

#include "TObject.h"
#include <string>
using std::string;
class FairMultiLinkedData;

class CbmBaseHit : public TObject
{
public:
	/**
	 * \brief Default constructor.
	 */
	CbmBaseHit();

	/**
	 * \brief Destructor.
	 */
	virtual ~CbmBaseHit();

	/* Accessors */
	HitType GetType() const { return fType; }
	Double_t GetZ() const { return fZ; }
	Double_t GetDz() const { return fDz; }
	Int_t GetRefId() const { return fRefId; }
	Int_t GetAddress() const { return fAddress; }
	FairMultiLinkedData* GetLinks() const { return fLinks; }

	/* Setters */
	void SetZ(Double_t z) { fZ = z; }
	void SetDz(Double_t dz) { fDz = dz; }
	void SetRefId(Int_t refId) { fRefId = refId; }
	void SetAddress(Int_t address) { fAddress = address; }
	 void SetLinks(FairMultiLinkedData* links) { fLinks = links; }

	/**
	 * Pure virtual function. Must be implemented in derived class.
	 * Should return plane identifier of the hit. Usually this is station or layer
	 * number of the detector. Can be calculated using unique detector identifier
	 * or can use additional class member from the derived class to store the plane identifier.
	 **/
	virtual Int_t GetPlaneId() const = 0;

	/**
	 * \brief Pure virtual function. Must be implemented in derived class.
	 * Has to return string representation of the object.
	 **/
	virtual string ToString() const = 0;

protected:
	/**
     * \brief Sets hit type.
     * \param type hit type
     **/
	void SetType(HitType type) { fType = type; }

        CbmBaseHit(const CbmBaseHit&);
        CbmBaseHit& operator=(const CbmBaseHit&);   

private:
	HitType fType; ///< hit type
	Double_t fZ; ///< Z position of hit [cm]
	Double_t fDz; ///< Z position error [cm]
	Int_t fRefId; ///< some reference id (usually to cluster, digi or MC point)
	Int_t fAddress; ///< detector unique identifier
	FairMultiLinkedData* fLinks; ///< Monte-Carlo link collection

  
	ClassDef(CbmBaseHit, 1);
};

#endif /* CBMBASEHIT_H_ */
