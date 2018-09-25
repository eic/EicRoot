//////////////////////////////////////////////////////////////////////////
//                                                                      //
// VAbsMicroCandidate	                                                //
//                                                                      //
// Definition of an abstract interface to a micro candidate.	        //
//                                                                      //
// Author: Sascha Berger and Marcel Kunze, RUB, March 1999		//
// Copyright (C) 1999-2001, Ruhr-University Bochum.			//
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TVector3.h"
#include "FairRecoCandidate.h"

ClassImp(FairRecoCandidate);

TBuffer &operator>>(TBuffer &buf, FairRecoCandidate *&obj)
{
   obj = (FairRecoCandidate *) buf.ReadObject(FairRecoCandidate::Class());
   return buf;
}

#include <iostream>
using namespace std;

void FairRecoCandidate::PrintOn(std::ostream &o) const 
{
    o << " ======= Fit Params ======= " << endl; 
    TVector3 r = GetPosition();
    TVector3 p = GetMomentum();
	o << " Position              : (" << r.X() << ";" << r.Y() << ";" << r.Z() << ")" << endl;
	o << " Momentum              : (" << p.X() << ";" << p.Y() << ";" << p.Z() << ")" << endl;
	o << " Momentum.mag          : " << p.Mag() << endl;
	o << " Charge                : " << GetCharge() << endl;
	o << " Energy                : " << GetEnergy() << endl;
    
    if (GetCharge() != 0) {
	o << " ======= Track Quality ======= " 
	  << "\n Fit quality         : Ndof " << GetDegreesOfFreedom()<< " chi2 " << GetChiSquared() 
	  << endl;//"\n track length        : " << GetTrackLength()<<endl; 
    }
          
} 

std::ostream&  operator << (std::ostream& o, const FairRecoCandidate& a) { a.PrintOn(o); return o; }
