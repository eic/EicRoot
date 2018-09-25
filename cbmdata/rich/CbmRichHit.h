/**
 * \file CbmRichHit.h
 * \author B. Polichtchouk
 *
 * Hits of MC tracks in Rich Photodetector including
 * detector geometry and efficiency
 **/

#ifndef CBMRICHHIT_H_
#define CBMRICHHIT_H_

#include "CbmPixelHit.h"

class CbmRichHit : public CbmPixelHit {

public:

  /**
   * \brief Default constructor.
   **/
  CbmRichHit();

  /**
   * \brief Destructor.
   **/
  virtual ~CbmRichHit();

  /**
   * \brief Inherited from CbmBaseHit.
   */
  virtual string ToString() const;

  /**
   * \brief Inherited from CbmBaseHit.
   */
  virtual Int_t GetPlaneId() const { return 0; }

  /** Modifiers **/
  virtual void SetPmtId (Int_t det) { fPmtId = det; }
  virtual void SetNPhotons (Int_t n) { fNPhotons = n; }
  virtual void SetAmplitude(Double_t amp) { fAmplitude = amp; }

  /** Accessors **/
  virtual Int_t GetPmtId() const { return fPmtId; }
  virtual Int_t GetNPhotons() const { return fNPhotons; }
  virtual Double_t GetAmplitude() const { return fAmplitude; }

private:

  Int_t fPmtId; // photomultiplier number
  Int_t fNPhotons; // number of photons in this hit
  Double32_t fAmplitude; // hit amplitude

  ClassDef(CbmRichHit, 1)
};

#endif // CBMRICHHIT_H_
