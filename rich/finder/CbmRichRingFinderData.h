/*
 * CbmRichRingFinderData.h
 *
 */

#ifndef CBM_RICH_RING_FINDER_DATA_H_
#define CBM_RICH_RING_FINDER_DATA_H_

#include <functional>


/**
* \class CbmRichHoughHit
*
* \brief Implementation of RICH hit for ring finder algorithm.
*
* \author Semen Lebedev
* \date 2008
**/
class CbmRichHoughHit {
public:
   /**
    * \brief Standard constructor.
    */
   CbmRichHoughHit():
      fHit(),
      fX2plusY2(0.f),
      fId(0),
      fIsUsed(false)
   { }

   virtual ~CbmRichHoughHit(){}

   CbmRichHitLight fHit;
   float fX2plusY2;
   unsigned short fId;
   bool fIsUsed;
};

/**
* \class CbmRichHoughHitCmpUp
*
* \brief CbmRichHoughHit comparator for hits sorting by X coordinate.
*
* \author Semen Lebedev
* \date 2008
**/
class CbmRichHoughHitCmpUp:
       public std::binary_function<
             const CbmRichHoughHit,
             const CbmRichHoughHit,
             bool>
{
public:

   virtual ~CbmRichHoughHitCmpUp(){}

   bool operator()(
         const CbmRichHoughHit &m1,
         const CbmRichHoughHit &m2) const
   {
      return m1.fHit.fX < m2.fHit.fX;
   }
};


/**
* \class CbmRichRingComparatorMore
*
* \brief CbmRichRingLight comparator based on the selection ANN criterion.
*
* \author Semen Lebedev
* \date 2008
**/
class CbmRichRingComparatorMore:
       public std::binary_function<
             const CbmRichRingLight*,
             const CbmRichRingLight*,
             bool>
{
public:

   virtual ~CbmRichRingComparatorMore(){}

   bool operator()(
         const CbmRichRingLight* ring1,
         const CbmRichRingLight* ring2) const
   {
      return ring1->GetSelectionNN() > ring2->GetSelectionNN();
   }
};


#endif /* CBM_RICH_RING_FINDER_DATA_H_ */
