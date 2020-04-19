#ifndef CBMLITCOMPARATORS_H_
#define CBMLITCOMPARATORS_H_

#include "data/CbmLitHit.h"
#include "data/CbmLitStripHit.h"
#include "data/CbmLitPixelHit.h"
#include "data/CbmLitTrackParam.h"
#include "propagation/CbmLitMaterialInfo.h"

#include <functional>
#include <iostream>



//class CompareMaterialInfoZLess:
//   public std::binary_function<
//   const CbmLitMaterialInfo&,
//   const CbmLitMaterialInfo&,
//   bool>
//{
//public:
//   bool operator()(const CbmLitMaterialInfo& mat1, const CbmLitMaterialInfo& mat2) const {
//      return mat1.GetZpos() < mat2.GetZpos();
//   }
//};
//
//
//
//class CompareTrackParamZLess:
//   public std::binary_function<
//   const CbmLitTrackParam&,
//   const CbmLitTrackParam&,
//   bool>
//{
//public:
//   bool operator()(const CbmLitTrackParam& par1, const CbmLitTrackParam& par2) const {
//      return par1.GetZ() < par2.GetZ();
//   }
//};



//class ComparePixelHitPtrYLess :
//   public std::binary_function<
//   const CbmLitPixelHit*,
//   const CbmLitPixelHit*,
//   bool>
//{
//public:
//   bool operator()(const CbmLitPixelHit* hit1, const CbmLitPixelHit* hit2) const {
//      return hit1->GetY() < hit2->GetY();
//   }
//};
//
//
//
//class ComparePixelHitPtrXLess :
//   public std::binary_function<
//   const CbmLitPixelHit*,
//   const CbmLitPixelHit*,
//   bool>
//{
//public:
//   bool operator()(const CbmLitPixelHit* hit1, const CbmLitPixelHit* hit2) const {
//      return hit1->GetX() < hit2->GetX();
//   }
//};
//
//
//
//class CompareHitPtrXULess :
//   public std::binary_function<
//   const CbmLitHit*,
//   const CbmLitHit*,
//   bool>
//{
//public:
//   bool operator()(const CbmLitHit* hit1, const CbmLitHit* hit2) const {
//      if (hit1 == NULL || hit2 == NULL) return false; // Bug fix from 10.12.2012. Protection against NULL hit pointer.
//      if (hit1->GetType() == kLITPIXELHIT) {
//         const CbmLitPixelHit* phit1 = static_cast<const CbmLitPixelHit*>(hit1);
//         const CbmLitPixelHit* phit2 = static_cast<const CbmLitPixelHit*>(hit2);
//         return phit1->GetX() < phit2->GetX();
//      } else if (hit1->GetType() == kLITSTRIPHIT) {
//         const CbmLitStripHit* shit1 = static_cast<const CbmLitStripHit*>(hit1);
//         const CbmLitStripHit* shit2 = static_cast<const CbmLitStripHit*>(hit2);
//         return shit1->GetU() < shit2->GetU();
//      } else {
//         std::cout << "CompareHitPtrXULess: HIT TYPE NOT SUPPORTED" << std::endl;
//         return false;
//      }
//   }
//};
//
//
//
//class CompareStripHitPtrULess :
//   public std::binary_function<
//   const CbmLitStripHit*,
//   const CbmLitStripHit*,
//   bool>
//{
//public:
//   bool operator()(const CbmLitStripHit* hit1, const CbmLitStripHit* hit2) const {
//      return hit1->GetU() < hit2->GetU();
//   }
//};

#endif /*CBMLITCOMPARATORS_H_*/
