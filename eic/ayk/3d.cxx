/* -------------------------------------------------------------------------- */
/*  3d.c                                                                      */ 
/*                                                                            */
/*    A useful collection of 3d algebra routines. One day should be           */
/* assembly-level optimized (SSE usage). BLAS?                                */ 
/*                                                                            */
/*  A.Kisselev, PNPI, St.Petersburg, Russia.                                  */
/*    e-mail: kisselev@hermes.desy.de                                         */
/* -------------------------------------------------------------------------- */

#include <cmath>
#include <cstring>

#include <ayk.h>
#include <3d.h>

/* ========================================================================== */
/*  Calculates crossing parameters of 2 straight lines; assumes, that         */
/* n1,n2 are normalized to '1'; assumes that lines are 'measured'             */
/* with the same accuracy; 'crs' is the line parametrized in a way            */
/* that '*crs->x' is a point inbetween 2 lines and '*crs->nx' is it's         */
/* normalized vector (NB: it is guaranteed to point from l1 to l2);           */
/* then '*rx' is the length of the 'bridge';                                  */

int cross_l_l(const t_3d_line *l1, const t_3d_line *l2, t_3d_line *crs, double *rx, 
  double *theta, double tt[])
{
  double t1, t2, a, b, c = l1->nx.Dot(l2->nx);//smul_v_v(l1->nx, l2->nx);
  TVector3 xy, q1, q2, _dd_;
  int config = 0, exact_crossing_flag = 0;

    /* Check 2 line configuration; */
  if (l1->nx == l2->nx) config = _PARALLEL_;

  if (crs || rx || tt)
  {
    xy = l1->x - l2->x;//sub_v_v(xy, l1->x, l2->x);

    a = xy.Dot(l1->nx);//smul_v_v(xy, l1->nx);
    b = xy.Dot(l2->nx);//smul_v_v(xy, l2->nx);

      /* Just take any definite 't2' in case if lines are parallel; */
      /* Since such occasions must be rare don't want to arrange    */
      /* special branch;                                            */
    t2 = config ? 0. : (b - a*c)/(1 - SQR(c)); 
    t1 = t2*c - a;

    if (tt)
    {
      tt[0] = t1;
      tt[1] = t2;
    } /*if*/

      /* Calculate closest to each other points on both straight lines; */
    q1 = t1 * l1->nx + l1->x;//mul_s_v(q1, t1, l1->nx); add_v_v(q1, q1, l1->x);
    q2 = t2 * l2->nx + l2->x;//mul_s_v(q2, t2, l2->nx); add_v_v(q2, q2, l2->x);
    
      /* Vector connecting 2 straight lines; */
    _dd_ = q2 - q1;//sub_v_v(_dd_, q2, q1);

      /* Point between 2 lines; */
    if (crs)
    {
      if (config) 
  	  /* Anyway makes no sense --> make it definite; */
        VZERO(crs->x);//vzero(crs->x);
      else
	crs->x = 0.5 * (q1 + q2);
      //{
      //add_v_v(crs->x, q1, q2); 
      //mul_s_v(crs->x, .5, crs->x);
      //} /*if*/

      crs->nx = _dd_;//vcopy(crs->nx, _dd_);

        /* If normalization fails (crossing lines), */
        /* prefer to set to the defined values;     */
      //if (normalize_v(crs->nx))
      if (!crs->nx.Mag())
      {
	exact_crossing_flag = 1;
	VZERO(crs->nx);//vzero(crs->nx);
	crs->nx[_Z_] = 1.;
      } 
      else
	crs->nx.SetMag(1.0);
    } /*if*/  

      /* Length of the 'bridging' vector; */
    //if (rx) *rx = exact_crossing_flag ? 0. : len_v(_dd_); 
    if (rx) *rx = exact_crossing_flag ? 0. : _dd_.Mag(); 
  } /*if*/

    /* Angle between 2 lines; */
  if (theta) *theta = rad2deg(acos(c));

  return config;
} /* cross_l_l */  

/* ========================================================================== */
/*  Calculates crossing point of the plane and straight line;                 */
/* assumes, that np,nl are normalized to '1';                                 */

int cross_p_l(const t_3d_plane *pl, const t_3d_line *ll, TVector3 &crs)
{
  int config = 0;
  double a, c = pl->nx.Dot(ll->nx);//smul_v_v(pl->nx, ll->nx);
  TVector3 qq;//, save;

  //printf("c: %f\n", c);

  qq = pl->x - ll->x;//sub_v_v(qq, pl->x, ll->x);
  a = qq.Dot(pl->nx);//smul_v_v(qq, pl->nx);

    /* Check configuration; */
  if (!c)
  {
    config = _PARALLEL_;
    if (!a) config = _BELONG_;

    //assert(0);
      /* Anyway makes no sense --> make it definite; */
    VZERO(crs);//vzero(crs);
  } 
  else
    crs = ll->x + (a/c) * ll->nx;
  //{
  //  /* May want to reparametrize 'll' => 'crs' may be just */
  //  /* 'll->x' => need to save it;                         */
  //vcopy(save, ll->x);
  //mul_s_v(crs, a/c, ll->nx);
  //add_v_v(crs, crs, save);  
  //} /*if*/

  return config;
} /* cross_p_l */

/* ========================================================================== */
/*   Constructs line using 2 space points; always uses x1 for the             */
/* parametrization; 'nx' is guaranteed to point from x1 to x2;                */
    
#if _OLD_
//
// --> FIXME: has never been checked since conversion to ROOT, etc;
//
t_3d_line::t_3d_line(TVector3 &x1, TVector3 &x2)
{
  if (x1 == x2)
  {
    VZERO(x); VZERO(nx);
    //return _COINSIDE_;
  } 
  else
  {
    x = x1;//vcopy(line->x, x1);
    nx = x2 - x1;//sub_v_v(line->nx, x2, x1);
    nx.SetMag(1.0);//normalize_v(line->nx);
  } //if
} // t_3d_line::t_3d_line()
#endif

/* ========================================================================== */
/*   Conctructs plane using line and point in space; always uses xx           */
/* for the plane parametrization;                                             */ 

int build_plane_from_point_and_line(TVector3 &xx, t_3d_line *ll, t_3d_plane *pl)
{
  TVector3 vv;
  double t0;

    /* Find minimal distance point on the line; */
  vv = xx - ll->x;//sub_v_v(vv, xx, ll->x);
  t0 = vv.Dot(ll->nx);//smul_v_v(vv, ll->nx);
  vv = ll->x + t0 * ll->nx;
  //mul_s_v(vv, t0, ll->nx);
  //add_v_v(vv, vv, ll->x);

    /* Check that original point is not sitting on the line; */
  if (vv == xx) return _BELONG_;

    /* Build normalized vector along this bridge; */
  vv -= xx;//sub_v_v(vv, vv, xx);

    /* And assign the resulting plane structure; */
  pl->x = xx;//vcopy(pl->x, xx);
  pl->nx = ll->nx.Cross(vv);//vmul_v_v(pl->nx, ll->nx, vv);
  pl->nx.SetMag(1.0);//normalize_v(pl->nx);

  return 0;
} /* build_plane_from_point_and_line */

/* ========================================================================== */
/*   NB: Modified in June'2008; perhaps older version was correct as well;    */

double point_to_line_dist(TVector3 &xx, t_3d_line *ll, TVector3 &bridge)
{
  TVector3 diff, pro, orth;

  diff = xx - ll->x;//sub_v_v(diff, xx, ll->x);
  //pro = smul_v_v(diff, ll->nx) * ll->nx;//mul_s_v(pro, smul_v_v(diff, ll->nx), ll->nx);
  pro = diff.Dot(ll->nx) * ll->nx;//mul_s_v(pro, smul_v_v(diff, ll->nx), ll->nx);

  orth = diff - pro;//sub_v_v(orth, diff, pro);

  //if (bridge) 
  bridge = orth;//VCOPY(bridge, orth);

  return orth.Mag();//return len_v(orth);
} /* point_to_line_dist */

/* ========================================================================== */
/*  Well, this is sort of trivial; still useful;                              */

double point_to_plane_dist(TVector3 &xx, t_3d_plane *pl)
{
  TVector3 vv;

  vv = xx - pl->x;//sub_v_v(vv, xx, pl->x);

  return fabs(vv.Dot(pl->nx));//smul_v_v(vv, pl->nx));
} /* point_to_plane_dist */

/* ========================================================================== */

double line_to_line_dist(t_3d_line *l1, t_3d_line *l2)
{
  double dist;

  cross_l_l(l1, l2, NULL, &dist, NULL, NULL);

  return dist;
} /* line_to_line_dist */

/* ========================================================================== */
#if _OLD_
t_3d_line parametrize_straight_line(double S[4], double z)
{
  double norm = sqrt(1. + SQR(S[2]) + SQR(S[3]));

  return t_3d_line(TVector3(S[0], S[1], z), TVector3(S[2]/norm, S[3]/norm, 1./norm));
} /* parametrize_straight_line */
#else
t_3d_line::t_3d_line(double S[4], double z)
{
  double norm = sqrt(1. + SQR(S[2]) + SQR(S[3]));

  x  = TVector3(S[0], S[1], z);
  nx = TVector3(S[2]/norm, S[3]/norm, 1./norm);
} // t_3d_line::t_3d_line() 
#endif

/* ========================================================================== */

int deparametrize_straight_line(t_3d_line *line, double z, double S[4])
{
  TVector3 crs;
  t_3d_plane parametrization_plane(TVector3(0., 0., z), TVector3(0., 0., 1.));

  if (cross_p_l(&parametrization_plane, line, crs)) return -1;

  S[0] = crs[_X_];
  S[1] = crs[_Y_];

  if (!line->nx[_Z_]) return -1;

  S[2] = line->nx[_X_]/line->nx[_Z_];
  S[3] = line->nx[_Y_]/line->nx[_Z_];

  return 0;
} /* deparametrize_straight_line */

/* ========================================================================== */
