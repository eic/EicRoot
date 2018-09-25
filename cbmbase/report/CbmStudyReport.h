/**
 * \file CbmStudyReport.h
 * \brief Base class for study reports.
 * \author Semen Lebedev <s.lebedev@gsi.de>
 * \date 2011
 */
#ifndef CBMSTUDYREPORT_H_
#define CBMSTUDYREPORT_H_

#include "CbmReport.h"
#include <string>
#include <vector>

using std::vector;
using std::string;
using std::ostream;

class CbmHistManager;

/**
 * \class CbmStudyReport
 * \brief Base class for study reports.
 * \author Semen Lebedev <s.lebedev@gsi.de>
 * \date 2011
 */
class CbmStudyReport : public CbmReport
{
public:
   /**
    * \brief Constructor.
    */
   CbmStudyReport();

   /**
    * \brief Destructor.
    */
   virtual ~CbmStudyReport();

   /**
    * \brief Main function which creates report data.
    *
    * Non virtual interface pattern is used here.
    * User always creates simulation report using this public non virtual method.
    * In order to configure concrete report generation class
    * user has to implement protected Create() method
    * and getters for the file names.
    *
    * \param[in] histManagers Array of histogram managers for which report is created.
    * \param[in] studyNames Names of studies.
    * \param[in] outputDir name of the output directory.
    */
   void Create(
         const vector<CbmHistManager*>& histManagers,
         const vector<string>& studyNames,
         const string& outputDir);

   /**
    * \brief Main function which creates report data.
    *
    * Same pattern is used here.
    *
    * \param[in] fileNames Array of file names for which report is created.
    * \param[in] studyNames Names of studies.
    * \param[in] outputDir name of the output directory.
    */
   void Create(
         const vector<string>& fileNames,
         const vector<string>& studyNames,
         const string& outputDir);

   /* Accessors */
   const vector<CbmHistManager*>& HM() const { return fHM; }
   CbmHistManager* HM(Int_t index) const { return fHM[index]; }
   const vector<string>& GetStudyNames() const { return fStudyNames; }
   const string& GetStudyName(Int_t index) const { return fStudyNames[index]; }

private:
   vector<CbmHistManager*> fHM; // Histogram managers for each study
   vector<string> fStudyNames; // Names of studies

   ClassDef(CbmStudyReport, 1)
};

#endif /* CBMSTUDYREPORT_H_ */
