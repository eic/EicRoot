//
// AYK (ayk@bnl.gov)
//
//    Extracted from KalmanNode in Oct'2015;
//

#include <assert.h>

#include <htclib.h>

#include <MediaBank.h>
#include <TrKalmanNode.h>
#include <TrKalmanNodeLocation.h>

// =======================================================================================

static void fill_lower_triangle(double **mtx, int dim)
{
  int ip, iq;

  for(ip=1; ip<dim; ip++)
    for(iq=0; iq<ip; iq++)
      mtx[ip][iq] = mtx[iq][ip];
} /* fill_lower_triangle */

//
// -> move this stuff to some common place later!;
//

// ---------------------------------------------------------------------------------------

// FIXME: this crap should go at some point;

static void *allocate_2dim_array(int dim1, int dim2, int element_size)
{
  int ip;
  void **arr = (void**)malloc(dim1*sizeof(void*));

  if (!arr) return NULL;

  for(ip=0; ip<dim1; ip++)
  {
    arr[ip] = (void*)calloc(dim2, element_size);
    if (!arr[ip]) return NULL;
  } /*for*/

  return arr;
} /* allocate_2dim_array */

// ---------------------------------------------------------------------------------------

static void *allocate_2dim_double_array(int dim1, int dim2)
{
  return allocate_2dim_array(dim1, dim2, sizeof(double));
} /* allocate_2dim_double_array */

// =======================================================================================

ProcessNoise *TrKalmanNodeLocation::InitializeProcessNoiseMatrices(KalmanFilter::Direction fb)
{
  MediaSliceArray *array;
  double F[4][4], Cms, D;
  ProcessNoise *noise = new ProcessNoise();
  //if (!noise) return NULL;

  // FIXME: should go to the ProcessNoise constructor at some point;
  noise->mCxx = (double**)allocate_2dim_double_array(4, 4);
  noise->mCyy = (double**)allocate_2dim_double_array(4, 4);
  noise->mCxy = (double**)allocate_2dim_double_array(4, 4);
  if (!noise->mCxx || !noise->mCyy || !noise->mCxy) return NULL;

  // Initialize field-free transport matrix to unity; 
  memset(F, 0x00, sizeof(F));
  for(int iq=0; iq<4; iq++)
    F[iq][iq] = 1.0;

  if (fb == KalmanFilter::Forward) {
    array = mMediaSliceArray;
    D =  1.;
  } 
  else {
    array = GetPrev(KalmanFilter::Forward)->mMediaSliceArray;
    D = -1.;
  } //if

  // I know that it is in general a bad idea to handle layers          
  // separately (one should in principle calculate total rad.length    
  // first); but for thick layers it is otherwise hard to take [x,sx]  
  // correlations into account; anyway, later may try to do it better; 
  for(int ik=0; ik<array->GetMediaSliceCount(); ik++) {
    MediaSlice *mslice = array->GetMediaSlice(ik); 

    // Material properties unknown --> skip;
    assert(mslice->GetMediaLayer()->GetMaterial());

    // Calculate transport matrix for this slice to reach array back                 
    // or front end; apparently this is strictly valid for the field-free case only; 
    if (fb == KalmanFilter::Forward)
      F[0][2] = F[1][3] = array->GetThickness() - (mslice->GetZ0() - array->GetZ0()) - mslice->GetThickness();
    else
      F[0][2] = F[1][3] =                       - (mslice->GetZ0() - array->GetZ0());

    // Calculate process noise covariance matrix contributions; 
    // NB: memset() resetted all other fields to '0' already;   
    mslice->_RCxx[fb][0][0] =                           SQR(mslice->GetThickness())/3.;
    mslice->_RCxx[fb][0][2] = mslice->_RCxx[fb][2][0] =   D*mslice->GetThickness() /2.;
    mslice->_RCxx[fb][2][2] =                                                  1.;

    mslice->_RCyy[fb][1][1] =                           SQR(mslice->GetThickness())/3.;
    mslice->_RCyy[fb][1][3] = mslice->_RCyy[fb][3][1] =   D*mslice->GetThickness() /2.;
    mslice->_RCyy[fb][3][3] =                                                  1.;

    mslice->_RCxy[fb][0][1] = mslice->_RCxy[fb][1][0] = SQR(mslice->GetThickness())/3.;
    mslice->_RCxy[fb][0][3] = mslice->_RCxy[fb][3][0] =   D*mslice->GetThickness() /2.;
    mslice->_RCxy[fb][1][2] = mslice->_RCxy[fb][2][1] =   D*mslice->GetThickness() /2.;
    mslice->_RCxy[fb][2][3] = mslice->_RCxy[fb][3][2] =                        1.;

    // Well, keep both options in the source code; Moliere theory is not 
    // available in GEANT4;
#ifdef _USE_GEANT3_MOLIERE_CHC_
    // This is according to the gaussian model and should perfectly reproduce 
    // MULS=3 multiple scattering model in GEANT; 1./(E*beta**2) will be      
    // added in initialize_process_noise();  
    Cms = SQR(2.557*mslice->GetMediaLayer()->GetMoliereChc())*mslice->GetThickness();
#else
    // This approach indeed ignores slope coefficient under log() term; 
    Cms = SQR(13.6E-3)*(mslice->GetReducedRadiationLength())*
      SQR(1. + 0.038*log(mslice->GetReducedRadiationLength()));
#endif

    for(int ip=0; ip<4; ip++)
      for(int iq=0; iq<4; iq++)
        for(int ir=0; ir<4; ir++)
          for(int is=ip; is<4; is++)
          {
	    noise->mCxx[ip][is] += Cms*F[ip][iq]*mslice->_RCxx[fb][iq][ir]*F[is][ir];
	    noise->mCyy[ip][is] += Cms*F[ip][iq]*mslice->_RCyy[fb][iq][ir]*F[is][ir];
	    noise->mCxy[ip][is] += Cms*F[ip][iq]*mslice->_RCxy[fb][iq][ir]*F[is][ir];
          } /*for..for*/
  } /*for*/
  fill_lower_triangle(noise->mCxx, 4);
  fill_lower_triangle(noise->mCyy, 4);
  fill_lower_triangle(noise->mCxy, 4);

  return noise;
} // TrKalmanNodeLocation::InitializeProcessNoiseMatrices()

// ---------------------------------------------------------------------------------------

unsigned TrKalmanNodeLocation::GetFiredNodeCount()
{
  unsigned counter = 0;

  for(unsigned nd=0; nd<mNodes.size(); nd++) {
    TrKalmanNode *node = mNodes[nd];
      
    if (node->IsActive() && node->IsFired()) counter++;
  } //for nd

  return counter;
} // TrKalmanNodeLocation::GetFiredNodeCount()

// ---------------------------------------------------------------------------------------
#if 0
bool TrKalmanNodeLocation::HasSensitiveVolumes()
{
  for(unsigned nd=0; nd<mNodes.size(); nd++) {
    TrKalmanNode *node = mNodes[nd];
      
    if (node->GetSensitiveVolume()) return true;
  } //for nd

  return false;
} // TrKalmanNodeLocation::HasSensitiveVolumes()
#endif
// ---------------------------------------------------------------------------------------
