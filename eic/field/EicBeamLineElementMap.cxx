// AYK (ayk@bnl.gov), 2014/09/03
//
//  EIC IR Beam line element magnetic field map handler;
//

//
// CHECK: XY-orientation (well, and Z-sign as well); A[][] -> transpose or not?;
//

#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include <TGeoBBox.h>
#include <TGeoVolume.h>

// FIXME: clean up these dependencies later; 
//#include <htclib.h>
//extern char XYZ[3];
static char XYZ[3] = {'X', 'Y', 'Z'};
#include <Mgrid.h>

#include <EicLibrary.h>
#include <EicBeamLineElementMap.h>

// =======================================================================================

//
// FIXME: check against the latest EicHtcTask codes and modify accordingly;
//

int EicBeamLineElementMap::Initialize() 
{
  // Figure out the actual file name;
  TString fileName = ExpandedFileName("input/", GetFileName());

  FILE *fin = fopen(fileName.Data(), "r");
  if (!fin) {
    printf("-E- EicBeamLineElementMap::Initialize() -> fail to open '%s' file!\n", fileName.Data());
    return -1;
  } //if

  char buffer[1024];

  // Skip 1-st line (a comment);
  fgets(buffer, 1024-1, fin);

  // Import A[3][3] and b[3] 3D transformation parameters; prefer to encode them in 
  // mTransformation rather than in Mgrid internally;
  {
    double A[9], b[3];

    // Files have comma-separated field (and no spaces); and LF they do have too -> "\n"; rely on this;
    if (fscanf(fin, "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf\n", 
	       A+0, A+1, A+2, A+3, A+4, A+5, A+6, A+7, A+8, b+0, b+1, b+2) != 12) {
      printf("-E- EicBeamLineElementMap::Initialize() -> file '%s': wrong input format!\n", fileName.Data());
      return -1;
    } //if   

    printf("%15.10f %15.10f %15.10f %15.10f %15.10f %15.10f %15.10f %15.10f %15.10f ... %15.10f %15.10f %15.10f\n", 
    	 A[0], A[1], A[2], A[3], A[4], A[5], A[6], A[7], A[8], b[0], b[1], b[2]);  

    // Assume matrix A[][] does not require transposition here; FIXME: check on that!;
    TGeoRotation *rw = new TGeoRotation();
    rw->SetMatrix(A);
    // Convert [m] -> [cm];
    mTransformation = new TGeoCombiTrans(100.0 * b[0], 100.0 * b[1], 100.0 * b[2], rw);
  }  

  // Skip 3-rd line (a comment);
  fgets(buffer, 1024-1, fin);

  // Import grid parameters;
  {
    // Follow Stephen's notation;
    unsigned Nx, Ny, Nz;
    double x0, x1, dx, y0, y1, dy, z0, z1, dz;

    if (fscanf(fin, "%lf,%lf,%d,%lf,%lf,%lf,%d,%lf,%lf,%lf,%d,%lf\n",
	       &x0, &x1, &Nx, &dx, &y0, &y1, &Ny, &dy, &z0, &z1, &Nz, &dz) != 12) {
      printf("-E- EicBeamLineElementMap::Initialize() -> file '%s': wrong input format!\n", fileName.Data());
      return -1;
    } //if   

    // Well, in this particular case have to resort to using fOrigin[] in TGeoBBox definition;
    // also convert [m] to [cm] units here; NB: consider to cut the box right at the edge point
    // location in TGeoBBox declaration (so that Contains() call selects this area only); however
    // the imported grid will be extended by half-cell in all directions according to HTC-style map
    // conventions (so defined grid points will become cubic cell centers); FIXME: clarify the 
    // idea with Stephen at some point;
    double origin[3] = {100.0 * (x0+x1)/2, 100.0 * (y0+y1)/2, 100.0 * (z0+z1)/2}; 
    mShape = dynamic_cast<TGeoShape*>(TGeoBBox(100.0 * (x1-x0)/2, 100.0 * (y1-y0)/2, 
					       100.0 * (z1-z0)/2, origin).Clone());

    // Now can define HTC-style grid array; could have probably used import_ascii_field_map() 
    // with some modifications; prefer to do everything by hand though, especially since step
    // is declared in the ASCII file header anyway; follow the logic of 
    // EicHtcTask::initializeMgridSlice() call;
    {
      // Create empty mgrid; 
      int dim[3] = {Nx, Ny, Nz};
      // So add half-cell on all sides;
      double min[3] = {x0 - dx/2, y0 - dy/2, z0 - dz/2}, max[3] = {x1 + dx/2, y1 + dy/2, z1 + dz/2};

      MgridDirection *fdir[3];
      CoordSystem *csystem = new CoordSystem(_CARTESIAN_, 3, XYZ);
      CoordSystem *fsystem = new CoordSystem(_CARTESIAN_, 3, XYZ);
      assert(csystem && fsystem);

      // Create direction frames; NB: NO fake Z!;
      for(int ik=0; ik<3; ik++) {
	// Rescale [m] -> [cm];
	min[ik] *= 100.0; 
	max[ik] *= 100.0;

	fdir[ik] = new MgridDirection(dim[ik], min[ik], max[ik]);
      } //for ik

      // Eventually create and initialize mgrid;
      mGrid = create_single_mgrid_header((char*)"DUMMY", csystem, fsystem, 
					  fdir, _FIELD_COMPONENT_VALUES_);
      if (!mGrid || mGrid->initializeAsSingleMgrid(0)) {
	printf("-E- EicBeamLineElementMap::Initialize(): !!! most likely you ran out of memory !!!\n\n");
	return -1;
      } /*if*/

      // Set desired interpolation mode; FIXME: no interpolation for now;
      //if (slice->mgrid->setHtcInterpolationMode(&RK_htci)) return NULL;
      assert(!mGrid->turnInterpolationOff());
    }

    // Skip 5-th line (a comment);
    fgets(buffer, 1024-1, fin);

    // Import all the expected Nx*Ny*Nz lines with the real data; 
    {
      unsigned dim3D = Nx*Ny*Nz;

      //
      // FIXME: check these codes after HTC re-import;
      //

      for(unsigned lc=0; lc<dim3D; lc++) {
	//t_3d_vector xx, BB;
	double xx[3], BB[3];

	if (fscanf(fin, "%lf,%lf,%lf,%lf,%lf,%lf\n",
		   xx+0, xx+1, xx+2, BB+0, BB+1, BB+2) != 6) {
	  printf("-E- EicBeamLineElementMap::Initialize() -> file '%s': wrong input format!\n", fileName.Data());
	  return -1;
	} //if   

	// Convert [m] -> [cm];
	for(unsigned iq=0; iq<3; iq++) 
	  xx[iq] *= 100.;

	MgridCell *cell = mGrid->coordToCellPtr(xx); assert(cell);
	for(unsigned iq=0; iq<3; iq++) 
	  // Convert [T] -> [kGs];
	  cell->B[iq] = 10.0 * BB[iq] * mScale;

	unsigned linear = mGrid->cellPtrToLinear(cell);
	//assert(linear < dim3D);
	mGrid->markCellAsSafe(linear);
      } //for iq

      //for(unsigned lc=0; lc<dim3D; lc++) 
      //assert(mGrid->GetCellProperty(lc) == _SAFE_CELL_);
    }
  }

  // Import bore parameters;
  if (fscanf(fin, "Length,%lf\n", &mLength) != 1 ||
      fscanf(fin, "Angle,%lf\n",  &mAngle)  != 1 ||
      fscanf(fin, "Bore,%lf\n",   &mBore)   != 1) {
    printf("-E- EicBeamLineElementMap::Initialize() -> file '%s': bore information missing!\n", fileName.Data());
    return -1;
  } //if   
  //printf("%f %f %f\n", mLength, mAngle, mBore); exit(0);

  fclose(fin);

  return EicMagneticFieldMap::Initialize();
} // EicBeamLineElementMap::Initialize()


void EicBeamLineElementMap::SetFieldScale(const double fieldScaler)
{
  mScale = fieldScaler;
}


// ---------------------------------------------------------------------------------------

//
// NB: can not move to the header file because then would need to include Mgrid headers;
//

int EicBeamLineElementMap::GetFieldValue(const double xx[], double B[]) const 
{
#if 0
  {
    double global[3] = {-4.9, -4.6, 950}, local[3];
    
    mTransformation->MasterToLocal(global, local);

    mGrid->getCartesianFieldValue(local, B);
    printf("local[]: %f %f %f -> B[] = %f %f %f -> (%d)\n", local[0], local[1], local[2], B[0], B[1], B[2], Contains(global)); exit(0);
  }
#endif

  //
  // Convert xx[] to local coordinate system -> calculate field -> convert to global B[];
  //

  //
  // FIXME: check these codes (after HTC re-import);
  //

  double xl[3];//, BL[3];

  // NB: it looks like this->Contains() call will also have to perform mTransformation->MasterToLocal()
  // before doing anything else; so no gain to call it first; THINK: the only thing I can do to speed 
  // processing up is to call mShape->Contains() after mTransformation->MasterToLocal() here; hmm?;
  mTransformation->MasterToLocal(xx, xl);

  // If ever care about performance, consider to use mGrid->directFieldValue();
  {
    TVector3 xlv(xl[0], xl[1], xl[2]), BLV;

    if (mGrid->getCartesianFieldValue(xlv, BLV)) {
      for(unsigned iq=0; iq<3; iq++)
	B[iq] = 0.0;
      
      return -1;
    } //if

    double BL[3] = {BLV[0], BLV[1], BLV[2]};
    mTransformation->LocalToMasterVect(BL, B);
  }

  return 0;
} // EicBeamLineElementMap::GetFieldValue()

// ---------------------------------------------------------------------------------------

#include <EicMediaHub.h>

// Make it configurable parameter later; for now just add 100mm to both width and height;
#define _EXTRA_BORE_WIDTH_ (100.0)

#define _IRON_   ("iron")
#define _VACUUM_ ("vacuum")
#define _ALUMINUM_ ("aluminum")

#include <TGeoManager.h>

int EicBeamLineElementMap::ConstructGeometry() 
{
  // FIXME: obviously need an Instance() member there;
  EicMediaHub *mediaHub = new EicMediaHub();

  mediaHub->Init();

  // Ok, basically I need to create something matching yoke volume and put it in a proper 
  // place in the geometry tree;
  char yokeIronName[128], yokeVacuumName[128];

  snprintf(yokeIronName,   128-1, "%s-Yoke",   GetDetectorName().Data());
  snprintf(yokeVacuumName, 128-1, "%s-Vacuum", GetDetectorName().Data());
  
  // Just per Stephen's definition I guess;
  double origin[3] = {0.0, 0.0, 100.0 * mLength/2};
  TGeoBBox *yoke = new TGeoBBox(yokeIronName, 
				100.0 * mBore + 0.1 * _EXTRA_BORE_WIDTH_,
				100.0 * mBore + 0.1 * _EXTRA_BORE_WIDTH_,
				100.0 * mLength/2, origin);
  mYoke = new TGeoVolume(yokeIronName, yoke, mediaHub->GetMedium(_IRON_));
  mYoke->SetFillColor(GetYokeColor());
  // FIXME: this has no effect -> check!;
  mYoke->SetLineColor(GetYokeColor());
  mYoke->RegisterYourself();

  // Should be vacuum, not air, I guess?; NB: as of 2014/09/11 some of the bores
  // are set to 0.0 by Stephen (not known yet) -> just do not insert bore volume at all;
  if (mBore) {
    TGeoTube *vacuum = new TGeoTube(yokeVacuumName, 
				    0.0,
				    100.0 * mBore,
				    100.0 * mLength/2);
    TGeoVolume *vvacuum = new TGeoVolume(yokeVacuumName, vacuum, mediaHub->GetMedium(_VACUUM_));
    vvacuum->RegisterYourself();

    mYoke->AddNode(vvacuum, 0, new TGeoTranslation(origin[0], origin[1], origin[2]));
  } //if


  // if(mAngle)
  //   {
  //     mTransformation->RotateY(mAngle*(180./3.14159265359));
  //     //mTransformation->SetDx(0.1*(origin[2]+mLength/2.)*sin(mAngle));
  //   }
  /*
  if(mAngle && mLength>0.6)
    {
      mTransformation->RotateY(mAngle);
      if(mLength>2. && mLength<3.)
	mTransformation->SetDx(10);    // was 10 in my original mod (just rotating this last aperture)
      if(mLength>1.5 && mLength<2.)
	mTransformation->SetDx(7.5);
      if(mLength>1. && mLength<1.5)
	mTransformation->SetDx(5.2);
    }
  */
  /*
  if(mAngle == -0.573)
    {
      mTransformation->RotateY(mAngle);
      mTransformation->SetDx(33.2);
    }
  */   
  //add the aluminum exit window
  /*
  TGeoBBox *exitWindow= new TGeoBBox("AlExitWindow", 
				     5.,
				     5.,
				     0.99);
  TGeoVolume *vexitWindow = new TGeoVolume("AlExitWindow", exitWindow, mediaHub->GetMedium(_ALUMINUM_));
  vexitWindow->RegisterYourself();
    
  mYoke->AddNode(vexitWindow, 0, new TGeoTranslation(origin[0], origin[1], origin[2]-200.));
  */

  gGeoManager->GetTopVolume()->AddNode(mYoke, 0, mTransformation);
  
  return 0;
  
} // EicBeamLineElementMap::ConstructGeometry() 

// =======================================================================================

ClassImp(EicBeamLineElementMap)
