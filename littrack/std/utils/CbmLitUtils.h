#ifndef CBMLITUTILS_H_
#define CBMLITUTILS_H_

class TCanvas;

#include <vector>
#include <string>
#include <sstream>
#include <iostream>

using std::string;
using std::vector;

namespace lit
{

template <class T>
string ToString(
   const T& value)
{
   std::stringstream ss;
   ss << (T)value;
   return ss.str();
}

template <class T>
string NumberToString(
   const T& value, int precision = 1)
{
   // First determine number of digits in float
   string digis = ToString<int>(value);
   int ndigis = digis.size();

   std::stringstream ss;
   ss.precision(ndigis + precision);
   ss << value;
   return ss.str();
}

/* Returns -1 if x<0, +1 if x>0, 0 if x==0 */
template <class T>
int Sign(
   const T& x)
{
   static const T ZERO = 0;
   return (x > ZERO) ? 1 : ((x < ZERO) ? -1 : 0);
}

void SaveCanvasAsImage(
   TCanvas* c,
   const std::string& dir);

string FindAndReplace(
		const string& name,
		const string& oldSubstr,
		const string& newSubstr);

vector<string> Split(
		const string& name,
		char delimiter);
}

#endif /* CBMLITUTILS_H_ */
