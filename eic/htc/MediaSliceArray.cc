//
// AYK (ayk@bnl.gov), shaped up in Nov'2015;
//
//  Array of media slices;
//

#include <cassert>
#include <cstdlib>
#include <cstring>

#include <htclib.h>

#include <MediaBank.h>
#include <MediaSliceArray.h>

// ---------------------------------------------------------------------------------------

MediaSliceArray::MediaSliceArray(MediaBank *bank, double z1, double z2):
  mZ0(0.0), mThickness(0.0)
{
  // Does this make sense; well, if "bank" was not initialized 
  // for some reason, I will have an empty slice array, fine;
  if (!bank) return;

  // Well, this is not too much efficient, but done only once and 
  // does not consume too much CPU anyway;                        
  for(int iq=0; iq<bank->GetMediaLayerCount(); iq++) {
    const MediaLayer *mlayer = bank->GetMediaLayer(iq);

    // Media layer does not belong to the searched [z1,z2] range; 
    if (mlayer->GetZ0() + mlayer->GetThickness() < z1) continue;
    if (mlayer->GetZ0()                          > z2) break;

    double z0 = mlayer->GetZ0() < z1 ? z1 : mlayer->GetZ0();
    double thickness = (mlayer->GetZ0() + mlayer->GetThickness() < z2 ? 
			mlayer->GetZ0() + mlayer->GetThickness() : z2) - z0;
    mMediaSlices.push_back(MediaSlice(mlayer, z0, thickness));
  } //for iq

  // Sanity check; well, allow nodes outside of media scan area; typically
  // these are nodes at negative Z (for vertex building); print a warning and assume
  // that these nodes are at pure vacuum;
  if (!mMediaSlices.size()) {
    //printf("Warning: Z range [%7.1f .. %7.1f]cm - no media scan data, assume vacuum!\n",
    //   z1, z2);

    return;
  } //if

  // Array's 'mZ0' is inherited from the very first slice; 
  mZ0 = mMediaSlices[0].GetZ0(); 

  // Now when all slices in the array are known, calculate effective A and Z; 
  for(int iq=0; iq<mMediaSlices.size(); iq++) {
    MediaSlice *mslice = &mMediaSlices[iq]; 

    // Yes, I'm interested in "effective" thickness, namely the area
    // where material slices are present;
    mThickness += mslice->GetThickness();
  } //for iq
} // MediaSliceArray::MediaSliceArray()

// ---------------------------------------------------------------------------------------

double MediaSliceArray::GetDE(t_particle_group *pgroup, int charge, double e0_kin)
{
  double de_sum = 0.;

  // Do it more efficiently if CPU ever becomes a problem (at least
  // merge same {A,Z} elements in all slices; or even create 
  // some sort of a look-up table;
  for(int iq=0; iq<mMediaSlices.size(); iq++) {
    MediaSlice *mslice = &mMediaSlices[iq]; 
    TGeoMaterial *material = mslice->GetMediaLayer()->GetMaterial();

    // Really that strong?; relieve later;
    //assert(material);

    switch (pgroup->dE_dx_model) {
    case _DEDX_HADRON_:
      for(int el=0; el<material->GetNelements(); el++) {
	float de_dx;
	double ad, zd, wd;
	material->GetElementProp(ad, zd, wd, el);

	// Basically skip pure vacuum elements;
	if (!zd) continue;

	// Consider 'e0' to be constant; so in a certain sense losses
	// along a given media slice array (read: between 2 neighboring
	// Kalman filter nodes) are assumed to be small;
	G3DRELX(ad, zd, wd*material->GetDensity(), e0_kin, pgroup->mass, de_dx);

	// Add respective fraction to the overall losses; 
	de_sum += wd * de_dx * material->GetDensity() * mslice->GetThickness();
      } //for el
      break;
      // Think on electron energy losses later;
#if _LATER_
    case _DEDX_ELECTRON_:
      // Skip pure vacuum slices;
      if (material->nlmat > 1 || material->A[0])
      {
	double de_dx_ionization, de_dx_bremsstrahlung;

	GDRELE(e0_kin, charge, material->POTL, material->FAC, 
	       material->C, material->X0, material->X1, material->AA, 
	       de_dx_ionization);

	// This fraction appears to be suspiciously small; check later;
	GBRELA(material->nlmat, material->wmat, material->A, material->Z,
	       // Convert density to g/cm3 here; @@@MM@@
	       ///material->AAVG, material->ZAVG, material->_density * 1000., e0_kin,
	       material->AAVG, material->ZAVG, material->_density, e0_kin,
	       charge, de_dx_bremsstrahlung);
	//printf("--> %f %f %f\n", e0_kin, de_dx_ionization, de_dx_bremsstrahlung);

	// Add respective fraction to the overall losses;
	de_sum += (de_dx_ionization + de_dx_bremsstrahlung)*
	  material->_density*mslice->thickness;
      } /*if*/
      break;
#endif
    default:
      assert(0);
    } /*switch*/
  } /*for iq*/

  return de_sum;
} // MediaSliceArray::GetDE()

// ---------------------------------------------------------------------------------------
