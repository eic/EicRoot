//
//  Implementation of a roman pot detector
//
//  Written by R. Petti (09-29-2014)
//


romanPot_v2()
{
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  // create a geometry object
  EicGeoParData *romanpot = new EicGeoParData("FWDST", 0, 0);   // for some reason the expected name is FWDST, so put that here

  // set some basic parameteres and wafer sizes/absolute position
  UInt_t waferNum = 1;
  Double_t waferThickness = 0.2;
  Double_t waferWidth = 2500.0;
  Double_t waferSpacing = 100.0;
  Double_t beamLineOffset = 18000.0;

  Double_t containerVolumeLength = (waferNum-1)*waferSpacing + waferThickness + 10.0;
  Double_t containerVolumeWidth = waferWidth + 10.0;


  // create the container volume
  TGeoBBox *container = new TGeoBBox("ContainerVolume",0.1*containerVolumeWidth/2., 0.1*containerVolumeWidth/2., 0.1*containerVolumeLength/2.);
  // probably should fill container volume with vacuum, but leave out for now
  //   this will probably form the vessel in which the sensors reside
  TGeoVolume *vcontainer = new TGeoVolume("ContainerVolume", container, romanpot->GetMedium("vacuum"));

  // create the silicon wafers, first the shape, then fill the shape with material
  TGeoBBox *wafer = new TGeoBBox("SiliconWafer", 0.1*waferWidth/2., 0.1*waferWidth/2., 0.1*waferThickness/2.);
  TGeoVolume *vwafer = new TGeoVolume("SiliconWafer", wafer, romanpot->GetMedium("silicon"));

  // create the map for local to global coordinates for analysis later
  
  EicGeoMap *fgmap = romanpot->CreateNewMap();
  fgmap->AddGeantVolumeLevel("SiliconWafer", waferNum);
  fgmap->SetSingleSensorContainerVolume("SiliconWafer");

  romanpot->AddLogicalVolumeGroup(0, 0, waferNum);
  
  double offset = 0;
  
  // place the detector element(s)
  for(unsigned int wf=0; wf<waferNum; wf++)
    {
      //double offset = 0.1*(wf - (waferNum-1)/2.)*waferSpacing;
      
      offset = wf*600.;

      UInt_t geant[1] = {wf}, group = 0, logical[3] = {0, 0, wf};

      if(romanpot->SetMappingTableEntry(fgmap, geant, group, logical))
	{
	  cout << "Failed to set mapping table entry!" << endl;
	  exit(0);
	}
      
      cout << offset << endl;

    
      romanpot->GetTopVolume()->AddNode(vwafer, wf, new TGeoCombiTrans((0.1*waferWidth/2.+12.47)+1.2, 0.0, offset, new TGeoRotation()));
      romanpot->GetTopVolume()->AddNode(vwafer, wf, new TGeoCombiTrans(-((0.1*waferWidth/2.-12.47))-1.2, 0.0, offset, new TGeoRotation()));
   
	
    }
 
  double displacementInX = 1. + 0.1*waferWidth/2.;
  //  displacementInX = 12.5 - 0.1*waferWidth/2. - 0.1);  //12.5 cm is where beam particles hit
  displacementInX = 0.;

  TGeoRotation *rotator = new TGeoRotation();
  //rotator->RotateY(TMath::RadToDeg()*10.e-3);
  
  romanpot->SetTopVolumeTransformation(new TGeoCombiTrans(displacementInX, 0.0, 0.1*beamLineOffset, rotator));

  romanpot->FinalizeOutput();

  exit(0);
}
