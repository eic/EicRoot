/**
 * \file CbmReportElement.h
 * \brief Abstract class for basic report elements (headers, tables, images etc.).
 * \author Semen Lebedev <s.lebedev@gsi.de>
 * \date 2011
 */
#ifndef CBMREPORTELEMENT_H_
#define CBMREPORTELEMENT_H_

#include "TObject.h"
#include <vector>
#include <string>

using std::string;
using std::vector;

/**
 * \class CbmReportElement
 *
 * \brief Abstract class for basic report elements (headers, tables, images etc.).
 *
 * Each concrete implementation for report elements has to implement
 * this interface (e.g. Latex, text, HTML). Report has to be written
 * using functionality of this interface class in order to be able
 * to automatically produce reports in different representations
 * (e.g. Latex, text, HTML).
 *
 * \author Semen Lebedev <s.lebedev@gsi.de>
 * \date 2011
 *
 */
class CbmReportElement : public TObject
{
public:
   /**
    * \brief Constructor.
    */
   CbmReportElement(){}

   /**
    * \brief Destructor.
    */
   virtual ~CbmReportElement(){}

   /**
    * \brief Return string with table open tag.
    * \param[in] caption Table caption.
    * \param[in] colNames Names of the columns in table.
    * \return String with table open tag.
    */
   virtual string TableBegin(
         const string& caption,
         const vector<string>& colNames) const = 0;

   /**
    * \brief Return string with table close tag.
    * \return String with table close tag.
    */
   virtual string TableEnd() const = 0;

   /**
    * \brief Return string with table row which spans over all columns.
    * \param[in] nofCols number of columns in table.
    * \param[in] name Name of the row.
    * \return string with table row which spans over all columns.
    */
   virtual string TableEmptyRow(
         int nofCols,
         const string& name) const = 0;

   /**
    * \brief Return string with table row tags.
    * \param[in] data Array of strings with data for each cell in a row.
    * \return string with table row tags.
    */
   virtual string TableRow(
         const vector<string>& row) const = 0;

   /**
    * \brief Return string with image tags.
    * \param[in] title Title of the image.
    * \param[in] file Name of the image file.
    * \return string with image tags.
    */
   virtual string Image(
         const string& title,
         const string& file) const = 0;

   /**
    * \brief Return string with open tags for document.
    * \return string with open tags for document.
    */
   virtual string DocumentBegin() const = 0;

   /**
    * \brief Return string with close tags of the document.
    * \return string with close tags of the document.
    */
   virtual string DocumentEnd() const = 0;

   /**
    * \brief Return string with title.
    * \param[in] size Size of the title. [0-5]. 0 is the largest size.
    * \param[in] title Title string.
    * \return string with subtitle.
    */
   virtual string Title(
         int size,
         const string& title) const = 0;

   ClassDef(CbmReportElement, 1)
};

#endif /* CBMREPORTELEMENT_H_ */
