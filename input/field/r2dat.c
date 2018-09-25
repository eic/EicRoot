
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct frec {
  float B[3];
};

static void usage(char *exe)
{
  printf("\n usage: %s <Rick's ASCII map name> <1|2|3|4|5>\n\n", exe);
  printf("    -> converts Rick's ASCII file into the SolenoidMap{12345}.dat\n");
  printf("       file which can be later imported by PandaRoot\n\n");

  exit(-1);
} /* usage */

int main(int argc, char **argv)
{
  if (argc != 3) usage(argv[0]);

  {
    unsigned id = atoi(argv[2]);
    if (!id || id > 5) usage(argv[0]);

    {
      char *fname = argv[1], qname[FILENAME_MAX];

      snprintf(qname, FILENAME_MAX-1, "SolenoidMap%d.dat", id);

      {
	FILE *fin = fopen(fname, "r"), *fout = fopen(qname, "w");

	if (!fin)
	{
	  printf("Failed to open '%s' for reading!\n", fname);
	  exit(-1);
	} /*if*/

	if (!fout)
	{
	  printf("Failed to open '%s' for writing!\n", qname);
	  exit(-1);
	} /*if*/

	{
	  // Assume, that XYZ coordinates are always in ascending order, 
	  // so it suffices to count number of changes;
	  unsigned dummy, nodes[3] = {0, 0, 0};
	
	  // Assume Rick's format will never change; first line should define 
	  // number of nodes per XYZ coordinate;
	  if (fscanf(fin, "%d %d %d %d\n", nodes+0, nodes+1, nodes+2, &dummy) != 4)
	  {
	    printf("File '%s' has wrong format!\n", fname);
	    exit(-2);
	  } /*if*/

	  // Loop through all the data lines;
	  {
	    float x[3], B[3];
	    unsigned lnum = nodes[0]*nodes[1]*nodes[2];

	    // Skip 7 meaningless lines;
	    {
	      size_t count;
	      char *ptr = 0;

	      for(unsigned ip=0; ip<7; ip++)
		getline(&ptr, &count, fin);
	    }

	    {
	      float xmin[3], xmax[3];
	      // NB: stack variable will not work (array may get too large)!;
	      frec *frecs = new frec[lnum];
     
	      // Read exactly the declared number of data lines;
	      for(unsigned ip=0; ip<lnum; ip++)
	      {
		if (fscanf(fin, "%f %f %f %f %f %f\n", x+0, x+1, x+2, B+0, B+1, B+2) != 6)
		{
		  printf("File '%s' has wrong format!\n", fname);
		  exit(-2);
		} /*if*/

		// Store data records;
		memcpy(frecs[ip].B, B, 3*sizeof(float));

		// Update min/max values;
		for(unsigned iq=0; iq<3; iq++)
		{
		  if (!ip || x[iq] < xmin[iq]) xmin[iq] = x[iq]; 
		  if (!ip || x[iq] > xmax[iq]) xmax[iq] = x[iq]; 
		} /*for iq*/
	      } /*for ip*/

	      // Dump file header; units are [cm] in both cases; 
	      fprintf(fout, "Solenoid\n");
	      fprintf(fout, "G\n");
	      for(unsigned iq=0; iq<3; iq++)
		fprintf(fout, "%6.1f %6.1f %3d\n", xmin[iq], xmax[iq], nodes[iq]);

	      // Dump the data records;
	      for(unsigned ip=0; ip<lnum; ip++)
	      {
		struct frec *fptr = frecs + ip;

		fprintf(fout, "%10.3f %10.3f %10.3f\n", fptr->B[0], fptr->B[1], fptr->B[2]);
	      } /*for ip*/
	    } 
	  }
	}
      }
    }
  }

  exit(0);
}  
