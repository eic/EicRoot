//
// grep -v Bmod DipoleScanOutput.txt | awk -F"," '{ print $1 $2 $3 $5 $6 $7}' > DipoleScanOutput.essential.txt
//
// cc -I../../ayk/include -o dipole dipole.c ../../ayk/libayk.a -lm
// ./dipole
//

#include <stdio.h>
#include <assert.h>

#include <mgsystem.h>

// Set this field be DipoleMap1.dat in PandaRoot notation;
#define _MAP_ID_       1

// Cell size of the output XYZ map in [cm];
#define _CELL_SIZE_  2.5

static double tesla2gs_fun(double val)
{
  return val*1E4;
} // tesla2kgs_fun()

void main( void )
{
  // No conversion -> units will stay [cm] & [G] as in the original file;
  t_ascii_coord xc = {_CARTESIAN_, 3, {'X', 'Y', 'Z'}, m2cm_fun};
  t_ascii_coord fc = {_CARTESIAN_, 3, {'X', 'Y', 'Z'}, tesla2gs_fun};
  char fname[FILENAME_MAX];

  snprintf(fname, FILENAME_MAX-1, "DipoleMap%d.dat", _MAP_ID_);

  {
    FILE *fout = fopen(fname, "w");

    if (!fout)
    {
      printf("Failed to open '%s' for writing!\n", fname);
      exit(-1);
    } /*if*/

    // Import Brett's ASCII file;
    t_mgrid *mgrid = import_ascii_field_map("./DipoleScanOutput.essential.txt",
					    "DID", &xc, &fc, 0);
    assert(mgrid);

    {
      t_3d_vector xx = {-37.5, 107.5, -10.0}, B;

      int ret = get_cartesian_field_value(mgrid, xx, B);
      //printf("%3d: %f %f %f\n", ret, B[0], B[1], B[2]);
      
      // -0.375, 1.075, -0.1, 1.13853, 0.4719783, -1.322822, -2.209778, 2.618347
    }

    {
      unsigned iq, nodes[3] = {101, 101, 201};
      float xmin[3] = {   -125.0,    -125.0, -250.0};
      float xmax[3] = {    125.0,     125.0,  250.0};

	// Dump file header;
	fprintf(fout, "Dipole\n");
	fprintf(fout, "G\n");
	for(iq=0; iq<3; iq++)
	  fprintf(fout, "%6.1f %6.1f %3d\n", xmin[iq], xmax[iq], nodes[iq]);

	// Loop through all the XYZ nodes and dump them to file;
	{
	  int ret;
	  unsigned ix, iy, iz;

	  for(ix=0; ix<nodes[_X_]; ix++)
	    for(iy=0; iy<nodes[_Y_]; iy++)
	      for(iz=0; iz<nodes[_Z_]; iz++)
	      {
		t_3d_vector xx = {xmin[_X_] + ix*_CELL_SIZE_, 
				  xmin[_Y_] + iy*_CELL_SIZE_,
				  xmin[_Z_] + iz*_CELL_SIZE_}, B;
		
		ret = get_cartesian_field_value(mgrid, xx, B);

		fprintf(fout, "%10.3f %10.3f %10.3f\n", B[0], B[1], B[2]);
	      } //for ix..iz
	}
    }
  }
} // main()
