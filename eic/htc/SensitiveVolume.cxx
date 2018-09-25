//
// AYK (ayk@bnl.gov), 2014/09/19
//
//  Re-mastered RegisteringPlane class;
//

// Looks stupid to redefine this stuff; but I really don't want to include 
// htcvtx.h with all the ADAMO stuff in;
#define _SX_ 2
#define _SY_ 3

#include <htclib.h>
#include <SensitiveVolume.h>
#include <MediaBank.h>

// Yes, per default energy losses are accounted; as well as multiple scattering;
//int dE_dx_flag = 1;

// Steps for taking numerical derivatives; tune later!; NB: drv_steps_qp will be 
// calculated in Runge-Kutta code for each track new (depending on momentum)!;
double _drv_steps[4] = {0.001, 0.001, 0.001, 0.001};

/* ========================================================================== */

int SensitiveVolume::TrackToHitDistance(t_3d_line *line, EicTrackingDigiHit *hit, 
					       double qdist[])
{
  KalmanNodeWrapper *kfwrapper = &mKfNodeWrappers[hit->GetKfNodeID()];

  double xgarr[3] = {line->x [0], line->x [1], line->x [2]}, xnarr[3];
  double ngarr[3] = {line->nx[0], line->nx[1], line->nx[2]}, nnarr[3];
  
  // Move 'line' to the node coordinate system (this includes MARS->SV & SV->ND
  // transformations);
  kfwrapper->GetNodeToMasterMtx()->MasterToLocal    (xgarr, xnarr);
  kfwrapper->GetNodeToMasterMtx()->MasterToLocalVect(ngarr, nnarr);

  t_3d_line local_line(TVector3(xnarr[0], xnarr[1], xnarr[2]), 
		       TVector3(nnarr[0], nnarr[1], nnarr[2]));
  
  TVector3 vtx;
  t_3d_plane local_plane(TVector3(0,0,0), TVector3(0,0,1));
  if (cross_p_l(&local_plane, &local_line, vtx)) return -1;

  unsigned dim = kfwrapper->GetKfNode(0)->GetMdim();
  double coord[dim];
  EicKfNodeTemplate *kftmpl = kfwrapper->GetKfNodeTemplate();
  
  kftmpl->ThreeDeeToTemplate(vtx, coord);
  
  for(unsigned ipp=0; ipp<dim; ipp++)
    qdist[ipp] = coord[ipp] - hit->_GetCoord(ipp);
      
  return 0;
} // SensitiveVolume::TrackToHitDistance()

/* ========================================================================== */

