/**
 * \file CbmRichGeoTestStudyReport.h
 * \brief Creates study report for RICH geometry testing.
 * \author Semen Lebedev <s.lebedev@gsi.de>
 * \date 2012
 */
#ifndef CBM_RICH_GEO_TEST_STUDY_REPORT
#define CBM_RICH_GEO_TEST_STUDY_REPORT

#include "CbmStudyReport.h"
#include <string>
#include "TSystem.h"
using std::string;

/**
 * \class CbmRichGeoTestStudyReport
 * \brief Creates study report for RICH geometry testing.
 *
 * Generates report from a number of results of different
 * simulations and reconstructions. Creates comparison tables.
 * Very useful for studies since all numbers are automatically
 * put in the comparison tables.
 *
 * \author Semen Lebedev <s.lebedev@gsi.de>
 * \date 2012
 *
 */
class CbmRichGeoTestStudyReport : public CbmStudyReport
{
public:
   /**
    * \brief Constructor.
    */
   CbmRichGeoTestStudyReport();

   /**
    * \brief Destructor.
    */
   virtual ~CbmRichGeoTestStudyReport();

protected:
    /**
    * \brief Inherited from CbmLitStudyReport.
    */
   void Create();

   /**
   * \brief Inherited from CbmLitStudyReport.
   */
  void Draw() {};

   /**
    * \brief Print one row in a table.
    * \param[in] property Name of the property in property tree.
    * \param[in] name Name of the row.
    * \return String with table row.
    */
   virtual string PrintRow(
      const string& property,
      const string& name);

   /**
    * \brief
    */
   virtual string PrintValue(
         int studyId,
         const string& valueTitle);
};

#endif
