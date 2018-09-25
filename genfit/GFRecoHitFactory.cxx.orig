/* Copyright 2008-2010, Technische Universitaet Muenchen,
   Authors: Christian Hoeppner & Sebastian Neubert

   This file is part of GENFIT.

   GENFIT is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published
   by the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   GENFIT is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with GENFIT.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "GFRecoHitFactory.h"

#include<iostream>


GFRecoHitFactory::GFRecoHitFactory(){
}

GFRecoHitFactory::~GFRecoHitFactory(){
  clear();
}

void GFRecoHitFactory::addProducer(int detID, GFAbsRecoHitProducer* hitProd, void *ptr) {
  if(fHitProdMap[detID].first != NULL) {
	GFException exc("GFRecoHitFactory: detID already in use",__LINE__,__FILE__);
	exc.setFatal();
	std::vector<double> numbers;
	numbers.push_back(detID);
	exc.setNumbers("detID",numbers);
	throw exc;
  }
  else {
    fHitProdMap[detID] = std::pair<GFAbsRecoHitProducer*, void*>(hitProd, ptr);
  }
}

void GFRecoHitFactory::clear(){
  std::map<int, std::pair<GFAbsRecoHitProducer*, void*> >::iterator it=fHitProdMap.begin();
  while(it!=fHitProdMap.end()){
    delete it->second.first;
    ++it;
  }
  fHitProdMap.clear();
}

GFAbsRecoHit* GFRecoHitFactory::createOne(int detID, int index) {
  if(fHitProdMap[detID].first != NULL) {
    return fHitProdMap[detID].first->produce(index, fHitProdMap[detID].second);
  }


  else {
	GFException exc("GFRecoHitFactory: no hitProducer for this detID available",__LINE__,__FILE__);
	exc.setFatal();
	std::vector<double> numbers;
	numbers.push_back(detID);
	exc.setNumbers("detID",numbers);
	throw exc;
  }
}

std::vector<GFAbsRecoHit*> GFRecoHitFactory::createMany(const GFTrackCand& cand){
  std::vector<GFAbsRecoHit*> hitVec;
  unsigned int nHits=cand.getNHits();
  for(unsigned int i=0;i<nHits;i++) {
    unsigned int detID;
    unsigned int index;
    cand.getHit(i,detID,index);
    hitVec.push_back( createOne(detID,index) );
  }
  return hitVec;
}

