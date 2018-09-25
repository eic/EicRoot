//
// AYK (ayk@bnl.gov)
//
//  Hough transform code elementary cell structure;
//
//  Initial port from OLYMPUS sources: Oct'2015;
//

#include <HoughTree.h>
#include <HoughCell.h>

// ---------------------------------------------------------------------------------------

HoughCell::HoughCell(const HoughTree *tree): mDaughters(0) 
{
  unsigned gdim = tree->GetGdim();

  // Allocate cell range arrays and reset elements to the __OUT_OF_RANGE_BIT_ values;
  mFrom = new t_hough_range[gdim];
  mTo   = new t_hough_range[gdim]; 

  ResetRanges(tree);
} // HoughCell::HoughCell()

// ---------------------------------------------------------------------------------------

void HoughCell::ResetRanges(const HoughTree *tree, bool immunity[])
{
  // Yes, it is easier to have both mFrom/mTo limits reset;
  for(unsigned gr=0; gr<tree->GetGdim(); gr++)
    // Some of the entries should NOT be modified -> check on that;
    if (!immunity || !immunity[gr])
      mFrom[gr] = mTo[gr] = __OUT_OF_RANGE_BIT_; 
} // HoughCell::ResetRanges()

// ---------------------------------------------------------------------------------------

void HoughCell::UpdateRanges(const HoughTree *tree, const t_hough_range id[]) 
{
  for(unsigned gr=0; gr<tree->GetGdim(); gr++) {
    // Mask out out-of-range bit, it does not matter here;
    t_hough_range range = id[gr] & ~__OUT_OF_RANGE_BIT_;

    // FIXME: unify the below stuff in one loop?;
    if (mFrom[gr] == __OUT_OF_RANGE_BIT_)
      mFrom[gr] = range;
    else {
      t_hough_range value = 0x0;

      for(unsigned iq=0; iq<tree->GetGroup(gr)->GetCoordDescr().size(); iq++) {
	const CoordinateDescriptor *descr = &tree->GetGroup(gr)->GetCoordDescr()[iq];

	t_hough_range is    = descr->UnpackCoord(mFrom[gr]);
	t_hough_range offer = descr->UnpackCoord(range);

	value |= descr->PackCoord(offer < is ? offer : is);
      } //for iq

      mFrom[gr] = value;
    } //if

    if (mTo[gr] == __OUT_OF_RANGE_BIT_)
      mTo[gr] = range;
    else {
      t_hough_range value = 0x0;

      for(unsigned iq=0; iq<tree->GetGroup(gr)->GetCoordDescr().size(); iq++) {
	const CoordinateDescriptor *descr = &tree->GetGroup(gr)->GetCoordDescr()[iq];

	t_hough_range is    = descr->UnpackCoord(mTo[gr]);
	t_hough_range offer = descr->UnpackCoord(range);

	value |= descr->PackCoord(offer > is ? offer : is);
      } //for iq

      mTo[gr] = value;
    } //if
  } //for gr
} // HoughCell::UpdateRanges()

// ---------------------------------------------------------------------------------------
