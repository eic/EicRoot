/**
 * \file CbmPixelHit.h
 * \author Andrey Lebedev <andrey.lebedev@gsi.de>
 * \date 2009
 *
 * Base class for pixel hits used for tracking in CBM.
 * Derives from CbmBaseHit.
 * Additional members are x, y coordinates and x, y, dxy covariances.
 **/
#ifndef CBMPIXELHIT_H_
#define CBMPIXELHIT_H_

#include "CbmBaseHit.h"

#include "TVector3.h"

class CbmPixelHit : public CbmBaseHit
{
public:
	/**
	 * \brief Default constructor.
	 */
	CbmPixelHit();

	/**
	 * \brief Standard constructor.
	 * \param address Detector unique identifier.
	 * \param x X position of the hit [cm].
	 * \param y Y position of the hit [cm].
	 * \param z Z position of the hit [cm].
	 * \param dx X position error of the hit [cm].
	 * \param dy Y position error of the hit [cm].
	 * \param dz Z position error of the hit [cm].
	 * \param dxy XY correlation of the hit.
	 * \param refId Some reference ID.
	 **/
	CbmPixelHit(
			Int_t address,
			Double_t x,
			Double_t y,
			Double_t z,
			Double_t dx,
			Double_t dy,
			Double_t dz,
			Double_t dxy,
			Int_t refId);

	/**
	 * \breif Standard constructor.
	 * \param address Detector unique identifier.
	 * \param pos Position of the hit as TVector3 [cm].
	 * \param err Position errors of the hit as TVector3 [cm].
	 * \param dxy XY correlation of the hit.
	 * \param refId Some reference ID.
	 **/
	CbmPixelHit(
			Int_t address,
			const TVector3& pos,
			const TVector3& err,
			Double_t dxy,
			Int_t refId);

	/**
	 * brief Destructor.
	 */
	virtual ~CbmPixelHit();

	/**
	 * \brief Inherited from CbmBaseHit.
	 **/
	virtual string ToString() const;

	/* Accessors */
	Double_t GetX() const { return fX; }
	Double_t GetY() const { return fY; }
	Double_t GetDx() const { return fDx; }
	Double_t GetDy() const { return fDy; }
	Double_t GetDxy() const { return fDxy; }

	/**
	 * \brief Copies hit position to pos.
	 * \param pos Output hit position.
	 */
	void Position(TVector3& pos) const;

	/**
	 * \brief Copies hit position error to pos.
	 * \param pos Output hit position error.
	 */
	void PositionError(TVector3& dpos) const;

	/* Setters */
	void SetX(Double_t x) { fX = x; }
	void SetY(Double_t y) {fY = y;}
	void SetDx(Double_t dx) {fDx = dx;}
	void SetDy(Double_t dy) {fDy = dy;}
	void SetDxy(Double_t dxy) {fDxy = dxy;}

	/**
	 * \brief Sets position of the hit.
	 * \param pos new hit position.
	 **/
	void SetPosition(const TVector3& pos);

	/**
	 * \breif Sets position error of the hit.
	 * \param dpos new hit position error
	 **/
	void SetPositionError(const TVector3& dpos);

private:
	Double_t fX, fY; ///< X, Y positions of hit [cm]
	Double_t fDx, fDy; ///< X, Y errors [cm]
	Double_t fDxy; ///< XY correlation

	ClassDef(CbmPixelHit, 1);
};

#endif /* CBMPIXELHIT_H_ */
