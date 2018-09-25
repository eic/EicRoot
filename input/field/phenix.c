//
// cc -o phenix -I./include/ phenix.c ./libayk.a -lm
// ./phenix
//

#include <stdio.h>
#include <assert.h>

#include <mgsystem.h>

// Set this field be SolenoidMap3.dat in PandaRoot notation;
//@@@#define _MAP_ID_       3
// Set this field be SolenoidMap5.dat in PandaRoot notation;
#define _MAP_ID_       5

#if _OLD_
// Cell size of the output XYZ map in [cm];
#define _CELL_SIZE_  3.0
#else
#define _CELL_SIZE_  1.0
#endif

void main( void )
{
  // No conversion -> units will stay [cm] & [G] as in the original file;
  t_ascii_coord xc = {_CYLINDRICAL_, 2, {'R', 'Z'}, 0};
  t_ascii_coord fc = {_CYLINDRICAL_, 2, {'R', 'Z'}, 0};
  char fname[FILENAME_MAX];

  snprintf(fname, FILENAME_MAX-1, "SolenoidMap%d.dat", _MAP_ID_);

  {
    FILE *fout = fopen(fname, "w");

    if (!fout)
    {
      printf("Failed to open '%s' for writing!\n", fname);
      exit(-1);
    } /*if*/

    // Import ASCII file;
    t_mgrid *mgrid = import_ascii_field_map("/home/ayk/FairRoot/eicroot/input/BABAR_V11_GridOut_ePHENIX.SF7",
					    // 34: skip that many lines at the beginning;
					    "PHENIX", &xc, &fc, 34);
    assert(mgrid);

    {
      //t_3d_vector xx = {36., 0., -200.}, B;

      //int ret = get_cartesian_field_value(mgrid, xx, B);
      //printf("%3d: %f %f %f\n", ret, B[0], B[1], B[2]);
      
      //   36.0000      -200.000     -1.178013E+03  8.388588E+03  8.470899E+03  1.509369E+05  6.849402E-02  6.849402E-02  2.939451E-04
    }

#if _OLD_
    // Figure out limits and construct field map frame;
    {
      unsigned nodes[3];
      t_mgrid_direction *dirR = mgrid->dir + _R_, *dirZ = mgrid->dir + _Z_;
      float xmin[3] = {      0.0,       0.0, dirZ->min}, step[3];
      float xmax[3] = {dirR->max, dirR->max, dirZ->max};
      printf("%f %f %f %f\n", dirR->min, dirR->max, dirZ->min, dirZ->max);
      
      {
	unsigned iq;

	// Number of nodes (well, in fact cells) and the actual step size;
	for(iq=0; iq<3; iq++) 
	{
	  nodes[iq] = (int)rint((xmax[iq] - xmin[iq])/_CELL_SIZE_);

	  // The actual step size;
	  step[iq] = (xmax[iq] - xmin[iq])/nodes[iq];
	} //for iq

	// Dump file header;
	fprintf(fout, "Solenoid\n");
	fprintf(fout, "G\n");
	for(iq=0; iq<3; iq++)
	  fprintf(fout, "%6.1f %6.1f %3d\n", xmin[iq], xmax[iq], nodes[iq]);

	// Loop through all the XYZ nodes and dump them to file;
	{
	  int ret;
	  unsigned ix, iy, iz;

	  // Do not care about one-off problem (anyway grid was imported with 
	  // half-cell off limits, and also RZ->XYZ will introduce additional 
	  // interpolation errors; may want to do better later;
	  for(ix=0; ix<=nodes[_X_]; ix++)
	    for(iy=0; iy<=nodes[_Y_]; iy++)
	      for(iz=0; iz<=nodes[_Z_]; iz++)
	      {
		t_3d_vector xx = {xmin[_X_] + ix*step[_X_], 
				  xmin[_Y_] + iy*step[_Y_],
				  xmin[_Z_] + iz*step[_Z_]}, B;
		
		ret = get_cartesian_field_value(mgrid, xx, B);

		fprintf(fout, "%10.3f %10.3f %10.3f\n", 2.*B[0], 2.*B[1], 2.*B[2]);
	      } //for ix..iz
	}
      }
    } 
#else
    {
      unsigned iq, nodes[3] = {101, 101, 401};
      //@@@float xmin[3] = {      0.0,       0.0, -50.0};
      //@@@float xmax[3] = {    100.0,     100.0, 350.0};
      float xmin[3] = {      0.0,       0.0, -200.0};
      float xmax[3] = {    100.0,     100.0,  200.0};

	// Dump file header;
	fprintf(fout, "Solenoid\n");
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
#endif
  }
} // main()
