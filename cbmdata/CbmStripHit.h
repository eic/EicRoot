/**
 * \file CbmStripHit.h
 * \author Andrey Lebedev <andrey.lebedev@gsi.de>
 * \date 2009
 *
 * Base class for strip-like hits used for tracking in CBM.
 * Derives from CbmBaseHit.
 * Additional members are u coordinate, phi angle and du, dphi measurement errors.
 **/
#ifndef CBMSTRIPHIT_H_
#define CBMSTRIPHIT_H_

#include "CbmBaseHit.h"

class TVector3;

class CbmStripHit :public CbmBaseHit
{
public:
	/**
	 * \brief Default constructor.
	 */
	CbmStripHit();

	/**
	 * \brief Standard constructor.
	 * \param address detector unique identifier
	 * \param u coordinate in the rotated c.s. [cm]
	 * \param phi strip rotation angle [rad]
	 * \param z Z position of the hit [cm]
	 * \param du U measurement error [cm]
	 * \param dphi PHI measurement error [rad]
	 * \param z Z position of the hit [cm]
	 * \param refId some reference ID
	 **/
	CbmStripHit(
			Int_t address,
			Double_t u,
			Double_t phi,
			Double_t z,
			Double_t du,
			Double_t dphi,
			Double_t dz,
			Int_t refId);

	/**
	 * \brief Standard constructor.
	 * \param address Detector unique identifier.
	 * \param pos Position of the hit as TVector3 (u, phi, z) [cm].
	 * \param err Position errors of the hit as TVector3 (du, dphi, dz) [cm].
	 * \param refId Some reference ID.
	 **/
	CbmStripHit(
			Int_t address,
			const TVector3& pos,
			const TVector3& err,
			Int_t refId);

	/**
	 * \brief Destructor.
	 */
	virtual ~CbmStripHit();

	/**
	 * \brief Inherited from CbmBaseHit.
	 **/
	virtual string ToString() const;

	/* Accessors */
	Double_t GetU() const { return fU; }
	Double_t GetPhi() const { return fPhi; }
	Double_t GetDu() const { return fDu; }
	Double_t GetDphi() const { return fDphi; }

	/* Setters */
	void SetU(Double_t u) { fU = u; }
	void SetPhi(Double_t phi) { fPhi = phi; }
	void SetDu(Double_t du) { fDu = du; }
	void SetDphi(Double_t dphi) { fDphi = dphi; }

private:
	Double_t fU; ///< U coordinate in the rotated c.s [cm]
	Double_t fDu; ///< U error [cm]
	Double_t fPhi; ///< strip rotation angle [rad]
	Double_t fDphi; ///< strip rotation error [rad]

	ClassDef(CbmStripHit, 1);
};

#endif /* CBMSTRIPHIT_H_ */
