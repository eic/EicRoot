//
//  On eic000* nodes execute something like 
//
//   source ~fisyak/.cshrc
//
// and then under ROOT:
//
// root.exe [0] gSystem->Load("StarMagField");
// root.exe [1] .x star.C+
//

#include <TROOT.h>
#include <TSystem.h>
#include <TClassTable.h>

#include <StarMagField.h>

#include <cstdio>

// Set this field be SolenoidMap6.dat in PandaRoot notation; NB: will need 
// to modify the codes in order to be able to use it in the reconstruction;
#define _MAP_ID_       6

// Cell size of the output XYZ map in [cm]; would be better to use 2cm cell, 
// but PandaRoot interface does not allow RZ-fields while 3D map file becomes
// prohibitively large;
#define _CELL_SIZE_  (float(4.0))

void star() 
{
  new StarMagField();

  char fname[FILENAME_MAX];
  snprintf(fname, FILENAME_MAX-1, "SolenoidMap%d.dat", _MAP_ID_);
  FILE *fout = fopen(fname, "w");
  if (!fout) {
    printf("Failed to open '%s' for writing!\n", fname);
    exit(-1);
  } //if

  unsigned nodes[3] = {101, 101, 401};
  float xmin[3] = {      0.0,       0.0, -800.0};
  float xmax[3] = {    400.0,     400.0,  800.0};

  // Dump file header;
  fprintf(fout, "Solenoid\n");
  fprintf(fout, "G\n");
  for(unsigned iq=0; iq<3; iq++)
    fprintf(fout, "%6.1f %6.1f %3d\n", xmin[iq], xmax[iq], nodes[iq]);
    
  // Loop through all the XYZ nodes and dump them to file;
  for(unsigned ix=0; ix<nodes[0]; ix++)
    for(unsigned iy=0; iy<nodes[1]; iy++)
      for(unsigned iz=0; iz<nodes[2]; iz++) {
	Float_t xx[3] = {xmin[0] + ix*_CELL_SIZE_, 
			 xmin[1] + iy*_CELL_SIZE_,
			 xmin[2] + iz*_CELL_SIZE_}, B[3];
	
	StarMagField::Instance()->BField(xx,B);
	
	// Convert [Gs];
	for(unsigned iq=0; iq<3; iq++)
	  B[iq] *= 1000.;
	
	fprintf(fout, "%10.3f %10.3f %10.3f\n", B[0], B[1], B[2]);
      } //for ix..iz

  fclose(fout);
}
  
