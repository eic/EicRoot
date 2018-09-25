/**
 * \file CbmDrawHist.h
 * \brief Helper functions for drawing 1D and 2D histograms and graphs.
 * \author Andrey Lebedev <andrey.lebedev@gsi.de>
 * \date 2008
 **/
#ifndef CBMDRAWHIST_H_
#define CBMDRAWHIST_H_

#include "TH1.h"
#include "TH1D.h"
#include "TH2.h"
#include "TPad.h"
#include "TLegend.h"
#include "TGraph.h"
#include "TGraph2D.h"

#include <string>
#include <vector>
using std::string;
using std::vector;

/**
 * \class CbmDrawingOptions
 * \brief Default options for drawing.
 * \author Andrey Lebedev <andrey.lebedev@gsi.de>
 * \date 2012
 */
class CbmDrawingOptions
{
public:

   static Int_t Color(Int_t colorIndex) {
      static const Int_t nofColors = 6;
      static Int_t colors[nofColors] = {kRed, kBlue + 1, kGreen + 3, kMagenta + 4, kYellow + 2, kViolet};
      return (colorIndex < nofColors) ? colors[colorIndex] : colorIndex;
   }

   static Int_t LineWidth() {
      return 2;
   }

   static Int_t LineStyle(Int_t lineStyleIndex) {
      return lineStyleIndex + 1;
   }

   static Int_t MarkerSize() {
      return 1;
   }

   static Int_t MarkerStyle(Int_t markerIndex) {
      static const Int_t nofMarkers = 8;
      static Int_t markers[nofMarkers] = {kOpenCircle, kOpenSquare, kOpenTriangleUp, kOpenDiamond,
                                          kFullCircle, kFullSquare, kFullTriangleUp};
      return (markerIndex < nofMarkers) ? markers[markerIndex] : markerIndex;
   }

   static Double_t TextSize() {
      return 0.06;
   }
};

/**
 * \enum HistScale
 * \brief Define linear or logarithmic scale for drawing.
 */
enum HistScale {
   kLog = 0, /**> Linear scale. */
   kLinear = 1 /**> Logarithmic scale */
};


/**
 * \fn SetStyles
 * \brief Set default styles for histograms.
 */
void SetDefaultDrawStyle();

/**
 * \fn DrawH1
 * \brief Draw 1D histogram.
 * \param[in] hist Pointer to histogram.
 * \param[in] logx Specify logarithmic or linear scale for X axis.
 * \param[in] logy Specify logarithmic or linear scale for Y axis.
 * \param[in] drawOpt Other drawing options (see ROOT documentation for details).
 * \param[in] color Color.
 * \param[in] lineWidth Line width.
 * \param[in] lineStyle Line style (see ROOT documentation for details).
 * \param[in] markerSize Marker size.
 * \param[in] markerStyle Marker style (see ROOT documentation for details).
 */
void DrawH1(
   TH1* hist,
   HistScale logx = kLinear,
   HistScale logy = kLinear,
   const string& drawOpt = "",
   Int_t color = CbmDrawingOptions::Color(0),
   Int_t lineWidth = CbmDrawingOptions::LineWidth(),
   Int_t lineStyle = CbmDrawingOptions::LineStyle(0),
   Int_t markerSize = CbmDrawingOptions::MarkerSize(),
   Int_t markerStyle = CbmDrawingOptions::MarkerStyle(0));


/**
 * \fn DrawH2
 * \brief Draw 2D histogram.
 * \param[in] logx Specify logarithmic or linear scale for X axis.
 * \param[in] logy Specify logarithmic or linear scale for Y axis.
 * \param[in] logz Specify logarithmic or linear scale for Z axis.
 * \param[in] drawOpt Other drawing options (see ROOT documentation for details).
 */
void DrawH2(
   TH2* hist,
   HistScale logx = kLinear,
   HistScale logy = kLinear,
   HistScale logz = kLinear,
   const string& drawOpt = "COLZ");


/**
 * \fn DrawH1
 * \brief Draw several TH1 histograms.
 * \param[in] histos Array of histograms.
 * \param[in] histLabels Array of histogram labels.
 * \param[in] logx Specify logarithmic or linear scale for X axis.
 * \param[in] logy Specify logarithmic or linear scale for Y axis.
 * \param[in] drawLegend If true than legend will be drawn.
 * \param[in] x1 X coordinate of legend in current pad.
 * \param[in] y1 Y coordinate of legend in current pad.
 * \param[in] x2 X coordinate of legend in current pad.
 * \param[in] y2 Y coordinate of legend in current pad.
 * \param[in] drawOpt Draw options for TH1->Draw method.
 */
void DrawH1(
   const vector<TH1*>& histos,
   const vector<string>& histLabels,
   HistScale logx = kLinear,
   HistScale logy = kLinear,
   Bool_t drawLegend = true,
   Double_t x1 = 0.25,
   Double_t y1 = 0.99,
   Double_t x2 = 0.55,
   Double_t y2 = 0.75,
   const string& drawOpt = "");

/**
 * \fn DrawGraph
 * \brief Draw TGraph.
 * \param[in] graph Pointer to TGraph.
 * \param[in] logx Specify logarithmic or linear scale for X axis.
 * \param[in] logy Specify logarithmic or linear scale for Y axis.
 * \param[in] drawOpt Other drawing options (see ROOT documentation for details).
 * \param[in] color Color.
 * \param[in] lineWidth Line width.
 * \param[in] lineStyle Line style (see ROOT documentation for details).
 * \param[in] markerSize Marker size.
 * \param[in] markerStyle Marker style (see ROOT documentation for details).
 */
void DrawGraph(
   TGraph* graph,
   HistScale logx = kLinear,
   HistScale logy = kLinear,
   const string& drawOpt = "AC",
   Int_t color = CbmDrawingOptions::Color(0),
   Int_t lineWidth = CbmDrawingOptions::LineWidth(),
   Int_t lineStyle = CbmDrawingOptions::LineStyle(0),
   Int_t markerSize = CbmDrawingOptions::MarkerSize(),
   Int_t markerStyle = CbmDrawingOptions::MarkerStyle(0));

/**
 * \fn DrawGraph.
 * \brief Draw upto 3 TGraphs. If graph == NULL than graph will not be drawn.
 * \param[in] graphs Vector of TGraphs.
 * \param[in] graphLabels Vector of graph labels.
 * \param[in] logx Specify logarithmic or linear scale for X axis.
 * \param[in] logy Specify logarithmic or linear scale for Y axis.
 * \param[in] drawLegend If true than legend will be drawn.
 * \param[in] x1 X coordinate of legend in current pad.
 * \param[in] y1 Y coordinate of legend in current pad.
 * \param[in] x2 X coordinate of legend in current pad.
 * \param[in] y2 Y coordinate of legend in current pad.
 */
void DrawGraph(
   const vector<TGraph*>& graphs,
   const vector<string>& graphLabels,
   HistScale logx = kLinear,
   HistScale logy = kLinear,
   Bool_t drawLegend = true,
   Double_t x1 = 0.25,
   Double_t y1 = 0.99,
   Double_t x2 = 0.55,
   Double_t y2 = 0.75);

/**
 * \fn DrawGraph2D
 * \brief Draw 2D graph.
 * \param[in] graph Pointer to TGraph2D.
 * \param[in] logx Specify logarithmic or linear scale for X axis.
 * \param[in] logy Specify logarithmic or linear scale for Y axis.
 * \param[in] logz Specify logarithmic or linear scale for Z axis.
 * \param[in] drawOpt Other drawing options (see ROOT documentation for details).
 */
void DrawGraph2D(
   TGraph2D* graph,
   HistScale logx = kLinear,
   HistScale logy = kLinear,
   HistScale logz = kLinear,
   const string& drawOpt = "colz");

#endif
