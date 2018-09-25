/**
 * \file CbmHtmlReportElement.h
 * \brief Implementation of CbmReportElement for HTML output.
 * \author Semen Lebedev <s.lebedev@gsi.de>
 * \date 2011
 */
#ifndef CBMHTMLREPORTELEMENT_H_
#define CBMHTMLREPORTELEMENT_H_

#include "CbmReportElement.h"
#include <string>

using std::string;
using std::vector;

/**
 * \class CbmHtmlReportElement
 * \brief Implementation of CbmReportElement for text output.
 * \author Semen Lebedev <s.lebedev@gsi.de>
 * \date 2011
 */
class CbmHtmlReportElement: public CbmReportElement
{
public:
   /**
    * \brief Constructor.
    */
   CbmHtmlReportElement();

   /**
    * \brief Destructor.
    */
   virtual ~CbmHtmlReportElement();

   /**
    * \brief Inherited from CbmReportElement.
    */
   virtual string TableBegin(
         const string& caption,
         const vector<string>& colNames) const;

   /**
    * \brief Inherited from CbmReportElement.
    */
   virtual string TableEnd() const;

   /**
    * \brief Inherited from CbmReportElement.
    */
   virtual string TableEmptyRow(
         int nofCols,
         const string& name) const;

   /**
    * \brief Inherited from CbmReportElement.
    */
   virtual string TableRow(
         const vector<string>& row) const;

   /**
    * \brief Inherited from CbmReportElement.
    */
   virtual string Image(
         const string& title,
         const string& file) const;

   /**
    * \brief Inherited from CbmReportElement.
    */
   virtual string DocumentBegin() const;

   /**
    * \brief Inherited from CbmReportElement.
    */
   virtual string DocumentEnd() const;

   /**
    * \brief Inherited from CbmReportElement.
    */
   virtual string Title(
         int size,
         const string& title) const;

   ClassDef(CbmHtmlReportElement, 1)
};

#endif /* CBMHTMLREPORTELEMENT_H_ */
