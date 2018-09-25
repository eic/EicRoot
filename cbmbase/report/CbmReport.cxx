/**
 * \file CbmReport.cxx
 * \brief Base class for reports.
 * \author Andrey Lebedev <andrey.lebedev@gsi.de>
 * \date 2011
 */
#include "CbmReport.h"
#include "CbmLatexReportElement.h"
#include "CbmHtmlReportElement.h"
#include "CbmTextReportElement.h"
#include "TCanvas.h"

#include <fstream>
#include <string>
using std::ofstream;
using std::string;

CbmReport::CbmReport():
   fReportName("qa_report"),
   fReportTitle("QA report"),
   fOutputDir("./"),
   fR(NULL),
   fOut(NULL),
   fReportType(kCoutReport),
   fCanvases()
{
}

CbmReport::~CbmReport()
{
}

void CbmReport::CreateReportElement(
      ReportType reportType)
{
   fReportType = reportType;
   if (NULL != fR) delete fR;
   if (NULL != fOut && fReportType != kCoutReport) delete fOut;
   if (reportType == kLatexReport) {
	   fR = new CbmLatexReportElement();
	   fOut = new ofstream(string(GetOutputDir() + fReportName + ".tex").c_str());
   } else if (reportType == kHtmlReport) {
	   fR = new CbmHtmlReportElement();
	   fOut = new ofstream(string(GetOutputDir() + fReportName + ".html").c_str());
   } else if (reportType == kTextReport) {
	   fR = new CbmTextReportElement();
	   fOut = new ofstream(string(GetOutputDir() + fReportName + ".txt").c_str());
   } else if (reportType == kCoutReport) {
	   fR = new CbmTextReportElement();
	   fOut = &std::cout;
   }
}

void CbmReport::DeleteReportElement()
{
 //  if (NULL != fR) delete fR;
 //  if (NULL != fOut && fReportType != kCoutReport) delete fOut;
}

void CbmReport::CreateReports()
{
   Draw(); // User has to implement this function!
   SaveCanvasesAsImages();
//   WriteCanvases();

   CreateReportElement(kHtmlReport);
   Create(); // User has to implement this function!
   DeleteReportElement();

   CreateReportElement(kLatexReport);
   Create(); // User has to implement this function!
   DeleteReportElement();

   CreateReportElement(kTextReport);
   Create(); // User has to implement this function!
   DeleteReportElement();

   CreateReportElement(kCoutReport);
   Create(); // User has to implement this function!
   DeleteReportElement();
}

TCanvas* CbmReport::CreateCanvas(
		const char* name,
		const char* title,
		Int_t ww,
		Int_t wh)
{
	TCanvas* canvas = new TCanvas(name, title, ww, wh);
	fCanvases.push_back(canvas);
	return canvas;
}

void CbmReport::SaveCanvasesAsImages() const
{
	if (GetOutputDir() == "") return;
	Int_t nofCanvases = fCanvases.size();
	for (Int_t i = 0; i < nofCanvases; i++) {
		TCanvas* canvas = fCanvases[i];
		//canvas->SaveAs(string(GetOutputDir() + string(canvas->GetTitle()) + ".eps").c_str());
		canvas->SaveAs(string(GetOutputDir() + string(canvas->GetTitle()) + ".png").c_str());
		canvas->SaveAs(string(GetOutputDir() + string(canvas->GetTitle()) + ".gif").c_str());
	}
}

void CbmReport::WriteCanvases() const
{
   if (GetOutputDir() == "") return;
   Int_t nofCanvases = fCanvases.size();
   for (Int_t i = 0; i < nofCanvases; i++) {
      fCanvases[i]->Write();
   }
}

void CbmReport::PrintCanvases() const
{
	Int_t nofCanvases = fCanvases.size();
	for (Int_t i = 0; i < nofCanvases; i++) {
		TCanvas* canvas = fCanvases[i];
		Out() << R()->Image(canvas->GetName(), canvas->GetName());
	}
}

ClassImp(CbmReport)
