//
// AYK (ayk@bnl.gov), 2013/06/12
//
//  "Ideal" TPC digitization code
//

#include <math.h>
#include <stdlib.h>
#include <assert.h>

#include <TRandom3.h>

#include <TpcGeoParData.h>
#include <EicTrackingDigiHit.h>
#include <EicTpcDigiHitProducer.h>

using namespace std;

#define _SQR_(arg) ((arg)*(arg))

// -----------------------------------------------------------------------------------------------

int EicTpcDigiHitProducer::HandleHit(const EicMoCaPoint *point)
{
  // Sanity check; digi parameters need to be assigned;
  if (!digi->fTransverseDispersion || !digi->fLongitudinalDispersion || 
      !digi->fLongitudinalIntrinsicResolution || !digi->fTransverseIntrinsicResolution ||
      !digi->fGemVerticalPadSize)
  {
    std::cout << "-E- EicTpcDigiHitProducer::HandleHit(): part of digi parameters not assigned!" << std::endl;
    return -1;
  } //if

  //
  // Figure out how many times this trajectory part crossed vertical pad 
  // radius; yes, it's a hack; do it better later; assume that 1) I have to 
  // create 1 hit per such a crossing, 2) resolutions are constant and given 
  // via command line, 3) may safely generate 3D hits at random locations 
  // along the trajectory line; also assume that there are no pathologic 
  // cases like crossing the same radius twice within a step;
  //

  ULogicalIndex_t xy = mGptr->GeantMultiToLogicalIndex(point->GetMultiIndex());
  const LogicalVolumeLookupTableEntry *node = mGptr->GetLookupTableNode(xy);

  // FIXME: just check these is not screw up here;
  //assert(0);
  //TVector3 lstart = node->MasterToLocal(point->GetPosIn());
  //TVector3 lend   = node->MasterToLocal(point->GetPosOut());
  TVector3 lstart = MasterToLocal(node->mGeoMtx, point->GetPosIn());
  TVector3 lend   = MasterToLocal(node->mGeoMtx, point->GetPosOut());

  // TPC gas volume half-length in [cm]; NB: as of 2014/08/05 this is actually 
  // a quarter-length (so ~50cm);
  double zLen = 0.1 * (dynamic_cast <const TpcGeoParData*>(mGptr))->mTotalGasVolumeLength/4.0;

  double rstart = sqrt(_SQR_(lstart[0]) + _SQR_(lstart[1]));
  double rend   = sqrt(_SQR_(lend  [0]) + _SQR_(lend  [1]));

  //printf("%f %f\n", rstart, digi->fGemVerticalPadSize);

  unsigned nstart = (int)floor(rstart/digi->fGemVerticalPadSize);
  unsigned nend   = (int)floor(rend  /digi->fGemVerticalPadSize);
  unsigned hnum   = nstart >= nend ? nstart - nend : nend - nstart;
  //printf("%d\n", hnum);

  // Well, this procedure is simplistic anyway; so assume, that 1) steps are reasonably
  // small compared to track curvature, 2) pads are arranged in cirlces of radius N*digi->fGemVerticalPadSize, 
  // 3) hits need to be produced only if hnum!=0 and exactly in this quantity;
  if (hnum)
  {
    // Simplify expressions below;
    double x1       = lstart[0];
    double y1       = lstart[1];
    double x2       = lend  [0];
    double y2       = lend  [1];
    // Prepare to solve quadratic equation;
    double A        = _SQR_(x1-x2) + _SQR_(y1-y2);
    double B        = x2*(x1-x2) + y2*(y1-y2);
    unsigned min    = nstart < nend ? nstart : nend;

    for(unsigned iqq=0; iqq<hnum; iqq++)
    {
      double R = (min + iqq + 1)*digi->fGemVerticalPadSize;
      double C = _SQR_(x2) + _SQR_(y2) - _SQR_(R);

      // Find 2 roots of quadratic equation (a*x1+(1-a)*x2)^2 + (a*y1+(1-a)*y2)^2 = R^2;
      double det = _SQR_(B) - A*C;
      assert(A && det);
      double rr[2] = {(-B + sqrt(det))/A, (-B - sqrt(det))/A};

      // I'm interested in root(s) in the range [0..1] (see equation above); at least 
      // one should exist, otherwise "hnum" would be wrong; well, in fact there 
      // should be exactly one "proper" root; check on that later;
      unsigned ok_counter = 0;
      for(unsigned ir=0; ir<2; ir++)
      {
	double alfa = rr[ir];

	if (alfa >= 0 && alfa <= 1)
	{
	  ok_counter++;

	  // Calculate 3D hit point;
	  TVector3 local = alfa * lstart + (1 - alfa) * lend;
   
#if _OLD_
	  // So then smear local coordinates; yes, smear Y-coordinate as well, 
	  // assuming it can be calculated better than N*digi->fGemVerticalPadSize using several 
	  // timing measurements; think later;
	  //TVector3 qResolution(digi->fTransverseIntrinsicResolution, 
	  //		       digi->fTransverseIntrinsicResolution, 
	  //		       digi->fLongitudinalIntrinsicResolution);
	  double qResolution[3] = {digi->fTransverseIntrinsicResolution, 
				   digi->fTransverseIntrinsicResolution, 
				   digi->fLongitudinalIntrinsicResolution};

	  // Calculate drift distance and imitate extra dispersion contribution;
	  {
	    TVector3 driftCff(digi->fTransverseDispersion, digi->fTransverseDispersion, 
			      digi->fLongitudinalDispersion);

	    // As of 2014/08/05 both up- and downstread half-gas volumes are oriented
	    // properly -> max value of local[2] coordinate is at the pads location (endcap);
	    double driftDist = zLen - local[2];

	    //printf("%f\n", driftDist);
	    //assert(driftDist >= 0.0);
	    if (driftDist < 0.0) driftDist = 0.0;

	    for(int iq=0; iq<3; iq++)
	      qResolution[iq] = sqrt(_SQR_(qResolution[iq]) + _SQR_(driftCff[iq]*sqrt(driftDist)));
	  }

	  for(int iq=0; iq<3; iq++)
	  {
	    // Convert [um] -> [cm];
	    qResolution[iq] /= 1E4;
	    local[iq] += gRandom->Gaus(0.0, qResolution[iq]);
	  } /*for iq*/
	  //printf("%f %f %f\n", local[0], local[1], local[2]);

	  // Make FairHit happy; do not want to change the meaning of those variables;
	  //TVector3 global = node->LocalToMaster(local);
	  TVector3 global = LocalToMaster(node->mGeoMtx, local);

	  // Create hit; 
	  //assert(0);
	  //TVector3 u(1,0,0), v(0,1,0);
	  {
	    //double x[3] = {1,0,0}, y[3] = {0,1,0}, result[3];
	    //TVector3 u, v;

	    //node->mGeoMtx->LocalToMasterVect(x, result);
	    //u.SetXYZ(x[0], x[1], x[2]);
    
	    //node->mGeoMtx->LocalToMasterVect(y, result);
	    //v.SetXYZ(result[0], result[1], result[2]);

	    //assert(0);
	    //new((*arr)[arr->GetEntriesFast()])  
	    //EicTrackingDigiHitOrth2D(detName, point, kfNodeID, global, local, mSigma);
	    {
	      double locarr[3] = {local.X(), local.Y(), local.Z()};

	      double qCovariance[3][3];
	      memset(qCovariance, 0x00, sizeof(qCovariance));
	      for(unsigned ip=0; ip<3; ip++)
		qCovariance[ip][ip] = _SQR_(qResolution[ip]);

	      new((*mDigiHitArray)[mDigiHitArray->GetEntriesFast()]) 
		// FIXME: 'nd=0' is dummy here;
		EicTrackingDigiHit3D(mDetName->Name(), point, global, locarr, qCovariance);
	      //qResolution);
	    }
	  }
#else
	  // So then smear local coordinates; coordinate along the pads is smeared uniformly
	  // based on pad height; may want to override this behavior and smear in a gaussian 
	  // way using by-hand provided resolution;
	  double radialResolution = digi->fRadialIntrinsicResolution ? 
	    digi->fRadialIntrinsicResolution : 1E4*digi->fGemVerticalPadSize/sqrt(12.);
	  double qResolution[3] = {radialResolution, 
				   digi->fTransverseIntrinsicResolution, 
				   digi->fLongitudinalIntrinsicResolution};
	  //printf("%f %f %f\n", qResolution[0], qResolution[1], qResolution[2]);
	  
	  // Calculate drift distance and imitate extra dispersion contribution;
	  {
	    TVector3 driftCff(digi->fTransverseDispersion, digi->fTransverseDispersion, 
			      digi->fLongitudinalDispersion);

	    // As of 2014/08/05 both up- and downstread half-gas volumes are oriented
	    // properly -> max value of local[2] coordinate is at the pads location (endcap);
	    double driftDist = zLen - local[2];

	    //printf("%f\n", driftDist);
	    //assert(driftDist >= 0.0);
	    if (driftDist < 0.0) driftDist = 0.0;

	    // So these will be gaussian resolution estimates in phi-aligned system;
	    for(int iq=0; iq<3; iq++)
	      qResolution[iq] = sqrt(_SQR_(qResolution[iq]) + _SQR_(driftCff[iq]*sqrt(driftDist)));

	    // Convert [um] -> [cm];
	    for(int iq=0; iq<3; iq++)
	      qResolution[iq] /= 1E4;
	  }

	  // Cook phi-aligned 3D vector, ...;
	  //double radius = sqrt(_SQR_(local[0]) + _SQR_(local[1]));
	  //double buffer[3] = {radius, 0.0, local[2]}, smeared[3];
	  double buffer[3] = {R, 0.0, local[2]}, smeared[3];

	  // ... smear it, ...;
	  //for(int iq=0; iq<3; iq++)
	  // Well, this is not exactly clean; assume, that instead of having equidistant
	  // hit coordinates in radial direction I can smear uniformly the points where 
	  // trajectory crosses concentric cirlces of N*<pad_size> radii; I suspect one 
	  // can further improve radial coordinate accuracy by comparing amplitudes in 
	  // neighboring rows; consider setRadialIntrinsicResolution() in digitization.C;
	  buffer[0] += digi->fRadialIntrinsicResolution ? 
	    gRandom->Gaus(0.0, qResolution[0]) : digi->fGemVerticalPadSize * gRandom->Uniform(-0.5, 0.5);
	  for(int iq=1; iq<3; iq++)
	    buffer[iq] += gRandom->Gaus(0.0, qResolution[iq]);

	  // ... rotate back to the vicinity of original local[] point;
	  {
	    double phi = atan2(local[1], local[0]);
	    double s = sin(phi), c = cos(phi);
	    double r[2][2] = {{c, -s}, {s, c}};

	    memset(smeared, 0x00, sizeof(smeared));
	    for(unsigned ip=0; ip<2; ip++)
	      for(unsigned iq=0; iq<2; iq++)
		smeared[ip] += r[ip][iq]*buffer[iq];
	    smeared[2] = buffer[2];
	    //printf("%f %f ... %f %f\n", local[0], smeared[0], local[1], smeared[1]);

	    // Make FairHit happy; do not want to change the meaning of those variables;
	    TVector3 global = LocalToMaster(node->mGeoMtx, smeared);

	    double qCovariance[3][3], qBuffer[2][2];
	    memset(qCovariance, 0x00, sizeof(qCovariance));
	    //for(unsigned ip=0; ip<3; ip++)
	    //qCovariance[ip][ip] = _SQR_(qResolution[ip]);
	    //#if _LATER_
	    memset(qBuffer,     0x00, sizeof(qBuffer));
	    for(unsigned ip=0; ip<2; ip++)
	      qBuffer[ip][ip] = _SQR_(qResolution[ip]);
	    
	    for(unsigned ip=0; ip<2; ip++)
	      for(unsigned iq=0; iq<2; iq++)
		for(unsigned it=0; it<2; it++)
		  for(unsigned is=0; is<2; is++)
		    qCovariance[ip][is] += r[ip][iq] * qBuffer[iq][it] * r[is][it];
	    qCovariance[2][2] = _SQR_(qResolution[2]);
	    //#endif

	    TVector3 vCoord(smeared);
	    new((*mDigiHitArray)[mDigiHitArray->GetEntriesFast()])
	      //EicTrackingDigiHit3D(mDetName->Name(), point, global, smeared, qCovariance);
	      EicTrackingDigiHit3D(mDetName->Name(), point, global, vCoord, qCovariance);
	  }
#endif
	} /*if*/
      } /*for ir*/

#if _BACK_      
      assert(ok_counter == 1);
#endif
    } /*for iq*/
  } //if

  return 0;
} // EicTpcDigiHitProducer::HandleHit()

// -----------------------------------------------------------------------------------------------

int EicTpcDigiHitProducer::exportTpcDigiParameters(char *fileName)
{
  // Yes, export always happens precisely to the path given via 'fileName' (no VMCWORKDIR
  // expansion like in importTpcDigiParameters());
  TFile fout(fileName, "RECREATE");

  if (!fout.IsOpen())
  {    
    std::cout << "-E- EicTpcDigiHitProducer::exportTpcDigiParameters(): failed to open '" << 
      fileName << "'!" << std::endl;
    return -1;
  } //if

  fout.WriteObject(digi, mDetName->Name() + "DigiParData");
  fout.Close();

  return 0;
} // EicTpcDigiHitProducer::exportTpcDigiParameters()

// -----------------------------------------------------------------------------------------------

int EicTpcDigiHitProducer::importTpcDigiParameters(char *fileName)
{
  TString expandedFileName(fileName);

  // Correct path if needed;
  if (!expandedFileName.BeginsWith("./") && !expandedFileName.BeginsWith("/"))
  {
    TString wrkDir = getenv("VMCWORKDIR");

    expandedFileName = wrkDir + "/input/" + expandedFileName;
  } //if

  TFile fin(expandedFileName);

  if (!fin.IsOpen())
  {    
    std::cout << "-E- EicTpcDigiHitProducer::importTpcDigiParameters(): failed to open '" << 
      fileName << "'!" << std::endl;
    return -1;
  } //if

  //fout.WriteObject(digi, mDetName->Name() + "DigiParData");
  fin.GetObject(mDetName->Name() + "DigiParData", digi); assert(digi);
  fin.Close();

  return 0;
} // EicTpcDigiHitProducer::importTpcDigiParameters()

// -----------------------------------------------------------------------------------------------

void TpcDigiParData::Print()
{
  // Damn std::cout, long live printf!;
  printf("\n-I- TPC 'naive' digitization parameters:\n\n");
  printf("   transverse dispersion            : %7.2f [um]/sqrt(D[cm])\n", fTransverseDispersion);
  printf("   longitudinal dispersion          : %7.2f [um]/sqrt(D[cm])\n", fLongitudinalDispersion);
  printf("   transverse intrinsic resolution  : %7.2f [um]\n", fTransverseIntrinsicResolution);
  printf("   longitudinal intrinsic resolution: %7.2f [um]\n", fLongitudinalIntrinsicResolution);
  printf("   radial intrinsic resolution      : %7.2f [um]\n", fRadialIntrinsicResolution);
  printf("   GEM vertical pad size            : %7.2f [cm]\n\n", fGemVerticalPadSize);
} // TpcDigiParData::Print()

// -----------------------------------------------------------------------------------------------

ClassImp(EicTpcDigiHitProducer)
