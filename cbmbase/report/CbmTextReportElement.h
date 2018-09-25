/**
 * \file CbmTextReportElement.h
 * \brief Implementation of CbmLitReportElement for text output.
 * \author Semen Lebedev <s.lebedev@gsi.de>
 * \date 2011
 */
#ifndef CBMTEXTREPORTELEMENT_H_
#define CBMTEXTREPORTELEMENT_H_

#include "CbmReportElement.h"
#include "TObject.h"
#include <string>

using std::string;
using std::vector;

/**
 * \class CbmTextReportElement
 * \brief Implementation of CbmLitReportElement for text output.
 * \author Semen Lebedev <s.lebedev@gsi.de>
 * \date 2011
 */
class CbmTextReportElement: public CbmReportElement
{
public:
   /**
    * \brief Constructor.
    */
   CbmTextReportElement();

   /**
    * \brief Destructor.
    */
   virtual ~CbmTextReportElement();

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
         Int_t nofCols,
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
         Int_t size,
         const string& title) const;

private:
   string FormatCell(
         const string& cell) const;

   Int_t fColW; // column width

   ClassDef(CbmTextReportElement, 1)
};

#endif /* CBMTEXTREPORTELEMENT_H_ */
