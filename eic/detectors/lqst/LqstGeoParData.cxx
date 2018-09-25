//
// RMP (rpetti@bnl.gov), 09-10-2014
//
// Low Q^2 tagging detector specific data definitions
//

#include <iostream>

#include "TGeoBBox.h"
#include "EicGeoMap.h"
#include "TMath.h"

#include <LqstGeoParData.h>

//#include <CalorimeterGeoParData.h>

using namespace std;

TGeoVolume *LqstCell::createCell(LqstGeoParData *detector, TString name)
{
  //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // create each individual detector element
  /*
  TGeoTrd1 *element = new TGeoTrd1("detectorElement", 
				   0.1*LqstGeoParData::cellFaceSize/2., 
				   0.1*LqstGeoParData::cellFaceSize/8., 
				   0.1*LqstGeoParData::cellFaceSize/2., 
				   0.1*LqstGeoParData::cellLength/2.); 

  */

  TGeoBBox *element = new TGeoBBox(name,
				   detector->get_cellFaceSize()/2.,
				   detector->get_cellFaceSize()/2., 
				   detector->get_cellFaceLength()/2.); 

  TGeoVolume *velement = new TGeoVolume(name, element, detector->GetMedium(_DIAMOND_));

  velement->SetLineColor(kYellow);
  velement->SetFillColor(kYellow);

  return velement;

}

TGeoVolume *LqstCell::createCell(LqstGeoParData *detector, TString name, double length)
{
  //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // create each individual detector element

  // this is meant to be used for the ecal sector
  
  /*
     
    TGeoTrd1 *element = new TGeoTrd1("detectorElement", 
    0.1*LqstGeoParData::cellFaceSize/2., 
    0.1*LqstGeoParData::cellFaceSize/8., 
    0.1*LqstGeoParData::cellFaceSize/2., 
    0.1*LqstGeoParData::cellLength/2.); 
    
  */
  
  Double_t alveoleThickness    = 0.1*0.2;  // was 0.01*0.2
  Double_t cellEnvelopeWidth   = detector->get_cellFaceSize() + 2.*alveoleThickness;
  Double_t cellEnvelopeLength = length + 2.*alveoleThickness;

  TString nameAlveole = name;
  nameAlveole = nameAlveole.Append("Alveole");

  TGeoBBox *alveole = new TGeoBBox(nameAlveole,
				   cellEnvelopeWidth/2.,
				   cellEnvelopeWidth/2.,
				   cellEnvelopeLength/2.);
  TGeoVolume *valveole = new TGeoVolume(nameAlveole, alveole, detector->GetMedium("CarbonFiber"));

  // crystal volume inside alveole
  TGeoBBox *crystal = new TGeoBBox(name,
				   detector->get_cellFaceSize()/2.,
				   detector->get_cellFaceSize()/2., 
				   length/2.); 

  TGeoVolume *vcrystal = new TGeoVolume(name, crystal, detector->GetMedium(_DIAMOND_));

  vcrystal->SetLineColor(kBlue);
  vcrystal->SetFillColor(kBlue);

  // place crystal into the alveole
  valveole->AddNode(vcrystal, 0, new TGeoCombiTrans(0.0, 0.0, 0.0, new TGeoRotation()));

  valveole->SetLineColor(kBlue-1);
  valveole->SetFillColor(kBlue-1);

  return valveole;

}

void LqstGeoParData::InitializeDetector(LqstGeoParData *det, int nlayers, int nX, int nY, double size, double length, double zoffset, double xoffset, double angle)
{
  lqs = det;

  lqs->set_numCellsX(nX);
  lqs->set_numCellsY(nY);

  lqs->set_cellFaceSize(size);
  lqs->set_cellFaceLength(length);

  lqs->set_beamLineOffset(zoffset);
  lqs->set_xOffset(xoffset);

  lqs->set_angle(angle);

  lqs->set_nGroups(nlayers);


  mcell = new LqstCell();

  //  makeMap();


  /*
  double volumeWidth = get_detectorWidth();
  double volumeLength = get_detectorLength();
  double volumeHeight = get_detectorHeight();
  */

  // holder volume for entire detector...filled with air
  //  holder = new TGeoBBox("HolderVolume", 0.1*volumeLength/2., 0.1*volumeHeight/2., 0.1*volumeWidth/2.);


  // individual cells
  // createCell();

  // create map
  //makeMap();
  
}


void LqstGeoParData::constructTrackerLayers()
{

  unsigned tcounter = 0;

 
  TGeoVolume *cell = (TGeoVolume*)mcell->createCell(lqs, "trackerCell");

  EicGeoMap *gmapT = lqs->CreateNewMap();
  gmapT->AddGeantVolumeLevel("trackerCell", lqs->get_nGroups()*lqs->get_totalNumCells());
  gmapT->SetSingleSensorContainerVolume("trackerCell");
 

  // for tracking layers (group 0)
  lqs->AddLogicalVolumeGroup(nCellsX, nCellsY, lqs->get_nGroups());
 
  for(int ilay = 0; ilay<lqs->get_nGroups(); ilay++)
    {
      
      double offset = ilay*10. + 1. + 25./2.;

      for(unsigned ix=0; ix<nCellsX; ix++)
	{
	  // double xx = cellFaceSize*(ix-1./2.);
	  
	  double xx = cellFaceSize*(ix - (nCellsX-1)/2.);	 
	  
	  for(unsigned iy=0; iy<nCellsY; iy++)
	    {
	      double yy = cellFaceSize*(iy - (nCellsY-1)/2.);
	      
	      //=-=-==-=-
	      // used to create the mapping function
	      //UInt_t geant[1] = {ilay}, lgroup = 0, logical[3] = {ix, iy, ilay};
	      UInt_t geant[1] = {tcounter}, group=0, logical[3] = {ix, iy, ilay};
	      
	      if(lqs->SetMappingTableEntry(gmapT, geant, group, logical))
		{
		  cout << "Failed to set mapping table entry!" << endl;
		  exit(0);
		}
	      
	      //lqs->GetTopVolume()->AddNode( mcell->createCell(lqs, "trackerCell"), tcounter++, new TGeoCombiTrans( xx, yy, offset, new TGeoRotation()) );
	      lqs->GetTopVolume()->AddNode( cell, tcounter++, new TGeoCombiTrans( xx, yy, offset, new TGeoRotation()) );
	      //lqs->AddNode( mcell->createCell(lqs), tcounter++, new TGeoCombiTrans( xx, yy, offset, new TGeoRotation()) );
	    }  // for (iy)
	  
	}  // for (ix)
      
      //      tcounter = 0;
    }


}

void LqstGeoParData::constructEcalLayers()
{

  // for now assume the same cell size for emcal as for tracking layers

  unsigned tcounter = 0;


  // for ecal layers (group 1)
  UInt_t lgroup = lqs->AddLogicalVolumeGroup(nCellsX, nCellsY);

  TGeoVolume *cell = (TGeoVolume*)mcell->createCell(lqs, "ecalCell", ecalTowerLength);

  // create a CalorimeterGeoParData object here so that
  //  geometry parameters can be stored and retrieved
  //  as other calorimeter systems (for use in digitization step)
  //mEcalTowerLength = lqs->GetEcalTowerLength();   // was hard set to 100

  double offset = -(30. + 1. - ecalTowerLength/2.);
  //double offset = -mCellLength/2.;

  EicGeoMap *gmapCr = lqs->CreateNewMap();
  //gmapE->AddGeantVolumeLevel("ecal", 1);
  gmapCr->AddGeantVolumeLevel("ecalCell", 1);
  gmapCr->AddGeantVolumeLevel("ecalCellAlveole", lqs->get_totalNumCells());
  gmapCr->SetSingleSensorContainerVolume("ecalCellAlveole");

  /*
  // Perhaps want to make alveoles GEANT sensitive volumes as well (say, to check 
  // energy losses there) -> create a separate map; 
  EicGeoMap *gmapAl = lqs->CreateNewMap();
  // N*N alveole packs;
  gmapAl->AddGeantVolumeLevel("ecalCellAlveole", lqs->get_totalNumCells());
  gmapAl->SetSingleSensorContainerVolume("ecalCellAlveole");
  */

  for(unsigned ix=0; ix<nCellsX; ix++)
    {
      
      // enter the cell envelope, make room for aveole enclosure of size 0.2mm
      double xx = (cellFaceSize + 2.*0.1*0.2)*(ix - (nCellsX-1)/2.); 
      
      for(unsigned iy=0; iy<nCellsY; iy++)
	{
	  double yy = (cellFaceSize + 2.*0.1*0.2)*(iy - (nCellsY-1)/2.);

	  //=-=-==-=-
	  // used to create the mapping function
	  //UInt_t geant[1] = {tcounter}, lgroup = 1, logical[3] = {ix, iy};
	  UInt_t geant[2] = {0, tcounter}, logical[2] = {ix, iy};
	  
	  //if(lqs->SetMappingTableEntry(gmapCr, geant+0, lgroup, logical) ||
	  // lqs->SetMappingTableEntry(gmapAl, geant+1, lgroup, logical))
	  if(lqs->SetMappingTableEntry(gmapCr, geant+0, lgroup, logical) )
	    {
	      cout << "Failed to set mapping table entry!" << endl;
	      exit(0);
	    }
	  
	  lqs->GetTopVolume()->AddNode(  cell, tcounter++, new TGeoCombiTrans( xx, yy, offset, new TGeoRotation()) );
	  //lqs->GetTopVolume()->AddNode(  mcell->createCell(lqs, "ecalCell", ecalTowerLength), tcounter++, new TGeoCombiTrans( xx, yy, offset, new TGeoRotation()) );
	  
	}  // end iy loop
    }  // end ix loop

}  // end constructEcalLayers function

void LqstGeoParData::constructDetector()
{  

  constructTrackerLayers();
  constructEcalLayers();

  TGeoRotation *rotator = new TGeoRotation();
  rotator->RotateY(totalAngle*TMath::RadToDeg());
  
  lqs->SetTopVolumeTransformation(new TGeoCombiTrans(xOffset, 0., beamLineOffset, rotator));

  lqs->FinalizeOutput();

}

ClassImp(LqstGeoParData)
