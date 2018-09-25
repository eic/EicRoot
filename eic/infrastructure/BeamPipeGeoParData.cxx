//
// AYK (ayk@bnl.gov), 2014/08/13
//
//  Beam pipe geometry description around the IP;
//

#include <assert.h>

#include <TGeoTube.h>
#include <TGeoCone.h>
#include <TGeoPcon.h>

#include <BeamPipeGeoParData.h>

// ---------------------------------------------------------------------------------------

bool BeamPipeGeoParData::CheckGeometry()
{
  //int ipElementID = -1;

  // Loop through all the elements and check, that they match each other
  // (no gaps or diameter mismatch), IP piece is present, etc;
  for(unsigned el=0; el<mElements.size(); el++) {
    BeamPipeElement *element = mElements[el];

    // At least 2 sections must be present;
    if (element->mSections.size() < 2) {
      printf("\n  Less than 2 sections per piece!\n");
      return false;
    } //if

    // Check whether it is an IP-area element;
    if (element->mIpElement) {
      if (mIpElementID != -1) {
	printf("\n  Duplicate IP area beam pipe element!\n");
	return false;
      } //if

      mIpElementID = el;
    } //if

    // Check end section match;
    if (el) {
      const BeamPipeSection *lh = mElements[el-1]->GetLastOrientedSection();
      const BeamPipeSection *rh = element->GetFirstOrientedSection();
      
      // Require an overlap (sort of vacuum tight);
      if ((lh->mOuterDiameter/2                               < rh->mOuterDiameter/2 - element->mThickness) ||
	  (lh->mOuterDiameter/2 - mElements[el-1]->mThickness > rh->mOuterDiameter/2)) {
	printf("\n  Neighboring piece diameter mismatch (vacuum gap)!\n");
	return false;
      } //if
    } //if

    // Check section order;
    for(unsigned sc=0; sc<element->mSections.size()-1; sc++) {
      // NB: 'lh' & 'rh' assignment does not necessarily reflect "swap" flag here;
      const BeamPipeSection *lh = element->mSections[sc+0];
      const BeamPipeSection *rh = element->mSections[sc+1];

      if (((  element->mSwapped  == BeamPipeElement::Swap) && (-rh->mOffset > -lh->mOffset)) || 
	  (((!element->mSwapped) != BeamPipeElement::Swap) && ( lh->mOffset >  rh->mOffset))) {
	printf("\n  Neighboring sections are out of order!\n");
	return false;
      } //if
    } //for sc
  } //for el

  if (mIpElementID == -1) {
    printf("\n  No IP area beam pipe element defined!\n");
    return false;
  } //if

  // Loop once again and adjust offsets of non-IP pieces;
  if (mIpElementID != 0)
  {
    double accuOffset = mElements[mIpElementID]->GetFirstSection()->mOffset;

    for(int el=mIpElementID-1; el>=0; el--) {
      BeamPipeElement *element = mElements[el];

      accuOffset          -= element->GetLength();
      element->mAccuOffset = accuOffset;
    } //for el
  } //if
  if (mIpElementID != mElements.size()-1)
  {
    double accuOffset = mElements[mIpElementID]->GetLastSection()->mOffset;

    for(unsigned el=mIpElementID+1; el<mElements.size(); el++) {
      BeamPipeElement *element = mElements[el];

      element->mAccuOffset = accuOffset;
      accuOffset          += element->GetLength();
    } //for el
  } //if

  return true;
} // BeamPipeGeoParData::CheckGeometry()

// ---------------------------------------------------------------------------------------

int BeamPipeGeoParData::ConstructGeometry()
{
  // Error message will be printed out there, don't worry;
  if (!CheckGeometry()) return false;

  for(unsigned el=0; el<mElements.size(); el++) {
    const BeamPipeElement *element = mElements[el];

    char name[128];
    // FIXME: do it better later; will hopefully never need more than 100 pieces?;
    assert(mElements.size() <= 100);
    snprintf(name, 128-1, "%s%02d", mDetName->Name().Data(), el);

    TGeoRotation *rY180 = new TGeoRotation();
    rY180->RotateY(180);

    TGeoVolume *vwpipe;

    // Check which representation would fit best;
    if (element->mSections.size() == 2) {
      const BeamPipeSection *lh = element->GetFirstSection();
      const BeamPipeSection *rh = element->GetLastSection();

      // Yet it can be either a cylinder or a cone;
      if (lh->mOuterDiameter == rh->mOuterDiameter) {
	// Fine, cook a TGeoTube;
	TGeoTube *tpipe = new TGeoTube(name,
				       0.1 * (lh->mOuterDiameter/2 - element->mThickness),
				       0.1 * lh->mOuterDiameter/2,
				       0.1 * element->GetLength()/2);
	vwpipe = new TGeoVolume(name, tpipe, GetMedium(element->mMaterial));
      }
      else {
	// Fine, cook a TGeoCone;
	TGeoCone *cpipe = new TGeoCone(name,
				       0.1 * element->GetLength()/2,
				       0.1 * (lh->mOuterDiameter/2 - element->mThickness),
				       0.1 * lh->mOuterDiameter/2,
				       0.1 * (rh->mOuterDiameter/2 - element->mThickness),
				       0.1 * rh->mOuterDiameter/2);
	vwpipe = new TGeoVolume(name, cpipe, GetMedium(element->mMaterial));
      } //if
    }
    else
    {
      // Then it's a TGeoPcon;
      TGeoPcon *ppipe = new TGeoPcon(name, 0.0, 360.0, element->mSections.size());

      // Simplify th esituation: place element middle to z=0 (unless it is an IP one);
      double zLocalOffset = element->mIpElement ? 0.0 : 
	(element->mSections[0]->mOffset + element->mSections[element->mSections.size()-1]->mOffset)/2;

      for(unsigned sc=0; sc<element->mSections.size(); sc++) {
	const BeamPipeSection *section = element->mSections[sc];

	ppipe->DefineSection(sc, 
			     0.1 * (section->mOffset - zLocalOffset), 
			     0.1 * (section->mOuterDiameter/2 - element->mThickness),
			     0.1 * section->mOuterDiameter/2);
	vwpipe = new TGeoVolume(name, ppipe, GetMedium(element->mMaterial));
      } //for sc
    } //if

    // Either acknowledge local offsets (absolute placement) or ignore them;
    double zOffset = element->mIpElement ? /*zOffset =*/ 0.0 : element->mAccuOffset + element->GetLength()/2;
    GetTopVolume()->AddNode(vwpipe, 0, 
			    new TGeoCombiTrans(0.0, 0.0, 0.1 * zOffset, 
					       element->mSwapped == BeamPipeElement::Swap ? rY180 : 0));
    
  } //for el
	
  // And put this stuff as a whole into the top volume; 
  FinalizeOutput();

  return 0;
} // BeamPipeGeoParData::ConstructGeometry()

// ---------------------------------------------------------------------------------------

ClassImp(BeamPipeSection)
ClassImp(BeamPipeElement)
ClassImp(BeamPipeGeoParData)
