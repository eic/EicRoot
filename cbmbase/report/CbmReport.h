/**
 * \file CbmLitReport.h
 * \brief Base class for reports.
 * \author Andrey Lebedev <andrey.lebedev@gsi.de>
 * \date 2011
 */

#ifndef CBMREPORT_H_
#define CBMREPORT_H_

#include "TObject.h"
#include <iostream>
#include <string>
#include <vector>
class CbmReportElement;
class TCanvas;
using std::string;
using std::vector;
using std::ostream;

/**
 * \enumeration ReportType
 * \brief Enumeration defines different report types.
 * \author Andrey Lebedev <andrey.lebedev@gsi.de>
 * \date 2011
 */
enum ReportType {kCoutReport, kHtmlReport, kLatexReport, kTextReport};

/**
 * \class CbmReport
 * \brief Base class for reports.
 * \author Andrey Lebedev <andrey.lebedev@gsi.de>
 * \date 2011
 */
class CbmReport : public TObject {
public:
   /**
    * \brief Constructor.
    */
   CbmReport();

   /**
    * \brief Destructor.
    */
   virtual ~CbmReport();

   /**
    * \brief Accessor to CbmReportElement object.
    * User has to write the report using available tags from CbmReportElement class.
    */
   const CbmReportElement* R() const { return fR; }

   /**
    * \brief All text output goes to this stream.
    */
   ostream& Out() const { return *fOut; }

   /* Setters */
   void SetReportName(const string& name) { fReportName = name; }
   void SetReportTitle(const string& title) { fReportTitle = title; }
   void SetOutputDir(const string& outputDir) { fOutputDir = outputDir; }

   /* Accessors */
   const string& GetReportName() const { return fReportName; }
   const string& GetReportTitle() const { return fReportTitle; }
   const string& GetOutputDir() const { return fOutputDir; }

protected:
   /**
    * \brief Pure abstract function which is called from public Create() function.
    * This function has to write report using Out() for output stream and
    * R() for report elements. This function is called for each report type.
    */
   virtual void Create() = 0;

   /**
    * \brief Pure abstract function which is called from public Create() function.
    * This function has to draw all necessary histograms and graphs.
    * It is called only once before report creation.
    */
   virtual void Draw() = 0;

   /**
    * \brief Create all available report types.
    */
   void CreateReports();

   /**
    * \brief Create canvas and put it to vector of TCanvases.
    * Canvases created with this function will be automatically saved to image and
    * printed in the report.
    */
   TCanvas* CreateCanvas(
		   const char* name,
		   const char* title,
		   Int_t ww,
		   Int_t wh);

   /**
    * \brief Save all canvases to images.
    */
   void SaveCanvasesAsImages() const;

   /**
    * \brief Write canvases to file.
    */
   void WriteCanvases() const;

   /**
    * \brief Print images created from canvases in the report.
    */
   void PrintCanvases() const;

private:
   /**
    * \brief Create concrete CbmReportElement instance based on report type.
    * \param[in] reportType Type of the report to be produced.
    */
   void CreateReportElement(
         ReportType reportType);

   /**
    * \brief Delete report element. Normally should be called at the end of Create function.
    */
   void DeleteReportElement();

   string fReportName; // Name of report
   string fReportTitle; // Title of report
   string fOutputDir; // Output directory for the report files
   ReportType fReportType; // Current report type
   CbmReportElement* fR; // Report element tool
   mutable ostream* fOut; // Output stream

   // Storage for TCanvas. All Canvases in this vector will be automatically saved
   // to image and printed in the report.
   // User can use CreateCanvas function which automatically push created canvas in this vector.
   vector<TCanvas*> fCanvases;

//private:

   CbmReport(const CbmReport&);
   CbmReport& operator=(const CbmReport&);

   ClassDef(CbmReport, 1)
};

#endif /* CBMREPORT_H_ */
