/**
 * \file CbmRichGeoTestStudyReport.cxx
 * \author Semen Lebedev <s.lebedev@gsi.de>
 * \date 2012
 */
#include "CbmRichGeoTestStudyReport.h"
#include "CbmReportElement.h"

#include "TSystem.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/assign/list_of.hpp>
using boost::assign::list_of;
using std::endl;
using std::stringstream;

CbmRichGeoTestStudyReport::CbmRichGeoTestStudyReport()
{
	SetReportName("rich_geo_qa");
}

CbmRichGeoTestStudyReport::~CbmRichGeoTestStudyReport()
{
}

void CbmRichGeoTestStudyReport::Create()
{
   Out().precision(3);
   Out() << R()->DocumentBegin();
   Out() << R()->Title(0, GetTitle());

   Out() << R()->TableBegin("Efficiencies", list_of(string("")).range(GetStudyNames()));
   Out() << PrintRow("acc_mean", "ACC");
   Out() << PrintRow("circle_fit_eff_mean", "Circle fit");
   Out() << PrintRow("ellipse_fit_eff_mean", "Ellipse fit");
   Out() << R()->TableEnd() << endl;

   Out() << R()->TableBegin("Nof point and hits in ring", list_of(string("")).range(GetStudyNames()));
   Out() << PrintRow("nof_points_in_ring_mean", "Nof points, mean");
   Out() << PrintRow("nof_points_in_ring_rms", "Nof points, rms");
   Out() << PrintRow("nof_hits_in_ring_mean", "Nof hits, mean");
   Out() << PrintRow("nof_hits_in_ring_rms", "Nof hits, rms");
   Out() << R()->TableEnd() << endl;

   Out() << R()->TableBegin("Point fit", list_of(string("")).range(GetStudyNames()));
   Out() << PrintRow("points_fit_a_axis_mean", "A axis, mean");
   Out() << PrintRow("points_fit_a_axis_rms", "A axis, rms");
   Out() << PrintRow("points_fit_b_axis_mean", "B axis, mean");
   Out() << PrintRow("points_fit_b_axis_rms", "B axis, rms");
   Out() << PrintRow("points_fit_boa_mean", "B/A, mean");
   Out() << PrintRow("points_fit_boa_rms", "B/A, rms");
   Out() << PrintRow("points_fit_r_mean", "Radius, mean");
   Out() << PrintRow("points_fit_r_rms", "Radius, mean");
   Out() << R()->TableEnd() << endl;

   Out() << R()->TableBegin("Hit fit", list_of(string("")).range(GetStudyNames()));
   Out() << PrintRow("hits_fit_a_axis_mean", "A axis, mean");
   Out() << PrintRow("hits_fit_a_axis_rms", "A axis, rms");
   Out() << PrintRow("hits_fit_b_axis_mean", "B axis, mean");
   Out() << PrintRow("hits_fit_b_axis_rms", "B axis, rms");
   Out() << PrintRow("hits_fit_boa_mean", "B/A, mean");
   Out() << PrintRow("hits_fit_boa_rms", "B/A, rms");
   Out() << PrintRow("hits_fit_r_mean", "Radius, mean");
   Out() << PrintRow("hits_fit_r_rms", "Radius, mean");
   Out() << R()->TableEnd() << endl;

   Out() << R()->TableBegin("Point fit vs. hit fit. Ellipse.", list_of(string("")).range(GetStudyNames()));
   Out() << PrintRow("diff_ellipse_da_mean", "dA, mean");
   Out() << PrintRow("diff_ellipse_da_rms", "dA, rms");
   Out() << PrintRow("diff_ellipse_db_mean", "dB, mean");
   Out() << PrintRow("diff_ellipse_db_rms", "dB, rms");
   Out() << PrintRow("diff_ellipse_dx_mean", "dX, mean");
   Out() << PrintRow("diff_ellipse_dx_rms", "dX, rms");
   Out() << PrintRow("diff_ellipse_dy_mean", "dY, mean");
   Out() << PrintRow("diff_ellipse_dy_rms", "dY, rms");
   Out() << R()->TableEnd() << endl;

   Out() << R()->TableBegin("Point fit vs. hit fit. Circle.", list_of(string("")).range(GetStudyNames()));
   Out() << PrintRow("diff_circle_dr_mean", "dR, mean");
   Out() << PrintRow("diff_circle_dr_rms", "dR, rms");
   Out() << PrintRow("diff_circle_dx_mean", "dX, mean");
   Out() << PrintRow("diff_circle_dx_rms", "dX, rms");
   Out() << PrintRow("diff_circle_dy_mean", "dY, mean");
   Out() << PrintRow("diff_circle_dy_rms", "dY, rms");
   Out() << R()->TableEnd() << endl;

   Out() <<  R()->DocumentEnd();
}

string CbmRichGeoTestStudyReport::PrintRow(
      const string& property,
      const string& name)
{
   vector<string> n(GetStudyNames().size(), "");
   for (int i = 0; i < GetStudyNames().size(); i++) {
      n[i] = PrintValue(i, property);
   }
   return R()->TableRow(list_of(name).range(n));
}

string CbmRichGeoTestStudyReport::PrintValue(
      int studyId,
      const string& valueName)
{
   stringstream ss;
   ss.precision(3);
  // ss << fQa[studyId].get(valueName, -1.);
   return ss.str();
}
