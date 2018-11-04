/* ----------------------------------------------------------------- */
/*  geant.c                                                          */
/*                                                                   */
/*    It looks like GEANT pass initialization/run is always the same.*/
/*  Therefore it makes sense to pack it into a standard call with    */
/*  several parameters describing wanted behaviour.                  */
/*                                                                   */
/*  A.Kisselev, PNPI, St.Petersburg, Russia.                         */
/*    e-mail: kisselev@hermes.desy.de                                */
/* ----------------------------------------------------------------- */

#include <cassert>
//#include <cunistd>
#include <cstring>

#include <htclib.h>
//#include <geant.h>

// NB: if e+/e- included make sure that energy losses in the Kalman 
// filter are taken into account properly; if neutral particles in 
// decay chains are used later, will need to restructure; but since
// GEANT particle ID is encoded in htcTrack.status[1], it will be 
// backwards compatible anyway;
t_particle_group particle_groups[] = {
  {(char*)"pion",     0.1395700,      _DEDX_HADRON_,   
   // Changed to string letters in order to match GEANT4;
   {{(char*)"pi+",      8,  211}, {(char*)"pi-",        9,  -211}}},
  {(char*)"kaon",     0.493677,       _DEDX_HADRON_,   
   {{(char*)"K+",      11,  321}, {(char*)"K-",        12,  -321}}},
  {(char*)"proton",   0.93827231,     _DEDX_HADRON_,   
   {{(char*)"Proton",  14, 2212}, {(char*)"Antiproton",15, -2212}}},
  {(char*)"electron", 5.109990615E-4, _DEDX_ELECTRON_, 
   {{(char*)"Positron", 2,  -11}, {(char*)"Electron",   3,    11}}}};
int particle_group_num = sizeof(particle_groups)/sizeof(particle_groups[0]);

/* ================================================================= */
/* Particle names are compared ignoring case;                        */

t_particle *get_particle_by_name(const char *name)
{
  for(int gr=0; gr<particle_group_num; gr++)
    for(int ch=0; ch<2; ch++)
    {
      t_particle *particle = particle_groups[gr].members + ch;

      if (!strcasecmp(particle->name, name))
	return particle;
    } /*for gr..ch*/
    
  return NULL;
} /* get_particle_by_name */

/* ----------------------------------------------------------------- */

t_particle_group *get_particle_group_by_name(const char *grname)
{
  for(int gr=0; gr<particle_group_num; gr++)
  {
    t_particle_group *pgroup = particle_groups + gr;

    if (!strcmp(pgroup->grname, grname)) return pgroup;
  } /*for gr*/    

  return NULL;
} /* get_particle_group_by_name */

/* ================================================================= */
