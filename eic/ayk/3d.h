/* -------------------------------------------------------------------------- */
/*  3d.h                                                                      */ 
/*                                                                            */
/*    Include file for the 3d algebra routines.                               */ 
/*                                                                            */
/*  A.Kisselev, PNPI, St.Petersburg, Russia.                                  */
/*    e-mail: kisselev@hermes.desy.de                                         */
/* -------------------------------------------------------------------------- */

#include <TVector3.h>
#include <TRotation.h>

#ifndef _3D_H
#define _3D_H

#define _X_ 0
#define _Y_ 1
#define _Z_ 2

#define _PARALLEL_ 1
#define _COINSIDE_ 2
#define _BELONG_   3

// Trivial but useful; 
#define VZERO(vec) ((vec).SetXYZ(0.0, 0.0, 0.0))
  
class t_3d_line {
 public:
  t_3d_line() {VZERO(x); VZERO(nx);};
  t_3d_line(TVector3 _x, TVector3 _nx) {
    x  = _x;
    nx = _nx;
    nx.SetMag(1.0);
  }
  t_3d_line(double S[4], double z);
  ~t_3d_line() {};

  TVector3 x, nx;
};

// Plane parametrization; want to keep it 
// separate from the t_line although they 
// are essentially the same; 
class t_3d_plane {
 public:
  t_3d_plane() {VZERO(x); VZERO(nx);};
  t_3d_plane(TVector3 _x, TVector3 _nx) {
    x  = _x;
    nx = _nx;
    nx.SetMag(1.0);
  }
  ~t_3d_plane() {};

  const TVector3 GetCoord() const { return x; };

  TVector3 x, nx;
};

#ifdef  __cplusplus
extern "C" {
#endif
  //int build_line_from_2_points(TVector3 &x1, TVector3 &x2, t_3d_line *line);
  int cross_p_l(const t_3d_plane *pl, const t_3d_line *ll, TVector3 &crs);
  double point_to_line_dist(TVector3 &xx, t_3d_line *ll, TVector3 &bridge);
  double point_to_plane_dist(TVector3 &xx, t_3d_plane *pl);
  int build_plane_from_point_and_line(TVector3 &xx, t_3d_line *ll, t_3d_plane *pl);

  //  FIXME: unify these two cases later; for now just leave this call in such 
  // a way, that it is explicitely useable under Linux (gcc) and Mac OS (Clang);
  //#ifndef __APPLE__
  //t_3d_line parametrize_straight_line(double S[4], double z);
  //#endif

  int cross_l_l(const t_3d_line *l1, const t_3d_line *l2, t_3d_line *crs, double *rx, 
		double *theta, double tt[]);
  double line_to_line_dist(t_3d_line *l1, t_3d_line *l2);

  int deparametrize_straight_line(t_3d_line *line, double z, double S[4]);
#ifdef  __cplusplus
}
#endif

//#ifdef _APPLE_
//t_3d_line parametrize_straight_line(double S[4], double z);
//#endif

#endif
