/**
 * \file CbmLatexReportElement.h
 * \brief Implementation of CbmReportElement for Latex output.
 * \author Semen Lebedev <s.lebedev@gsi.de>
 * \date 2011
 */
#ifndef CBMLATEXREPORTELEMENT_H_
#define CBMLATEXREPORTELEMENT_H_

#include "CbmReportElement.h"
#include <string>

using std::string;
using std::vector;

/**
 * \class CbmLatexReportElement
 * \brief Implementation of CbmLitReportElement for Latex output.
 * \author Semen Lebedev <s.lebedev@gsi.de>
 * \date 2011
 */
class CbmLatexReportElement: public CbmReportElement
{
public:
   /**
    * \brief Constructor.
    */
   CbmLatexReportElement();

   /**
    * \brief Destructor.
    */
   virtual ~CbmLatexReportElement();

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

   ClassDef(CbmLatexReportElement, 1)
};

#endif /* CBMLATEXREPORTELEMENT_H_ */
