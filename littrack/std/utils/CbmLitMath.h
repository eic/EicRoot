#ifndef CBMLITMATH_H_
#define CBMLITMATH_H_

#include "data/CbmLitTrackParam.h"

class CbmLitHit;
class CbmLitStripHit;
class CbmLitPixelHit;
class CbmLitTrack;

namespace lit
{

/* Calculates chi square
 * @param par Pointer to the updated or smoothed track parameter
 * @param hit Pointer to the hit
 * @return chi square contribution for this hit */
litfloat ChiSq(
   const CbmLitTrackParam* par,
   const CbmLitHit* hit);

/* Calculates chi square for strip hits
 * @param par Pointer to the updated or smoothed track parameter
 * @param hit Pointer to the strip hit
 * @return chi square contribution for this strip hit */
litfloat ChiSq(
   const CbmLitTrackParam* par,
   const CbmLitStripHit* hit);

/* Calculates chi square for pixel hits
 * @param par Pointer to the updated or smoothed track parameter
 * @param hit Pointer to the pixel hit
 * @return chi square contribution for this pixel hit */
litfloat ChiSq(
   const CbmLitTrackParam* par,
   const CbmLitPixelHit* hit);

/* Calculates number of degrees of freedom
 * @param track Pointer to the track */
int NDF(
   const CbmLitTrack* track);

}
#endif /*CBMLITMATH_H_*/
