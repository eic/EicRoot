/*
 * PndVertex.h
 *
 *  Created on: Aug 25, 2009
 *      Author: stockman
 */

#ifndef PNDVERTEX_H_
#define PNDVERTEX_H_

#include "FairHit.h"

class PndVertex: public FairHit {
public:
	PndVertex();
	PndVertex(double x, double y, double z){
		SetX(x); SetY(y); SetZ(z);
	}
	virtual ~PndVertex();

	ClassDef(PndVertex, 0);
};

#endif /* PNDVERTEX_H_ */
