
//
//  resolutions are specified at a later stage (see digitization.C); 
//
//  Prefer to declare dimensions in [mm]; convert to [cm] when calling ROOT shape 
//  definition routines only;
//

void b0tracker()
{
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  EicGeoParData *b0tracker = new EicGeoParData("B0Tracker", 0, 0);
  b0tracker->SetFileName("b0tracker.root");

  UInt_t waferNum                =      4;
  // 300um per layer: should be fine?;
  Double_t waferThickness        =    0.3;
  Double_t waferWidth            =  350.0;
  Double_t waferHeight           =  220.0;
  // NB: weird things start happening in the reconstruction code if two outer 
  // silicon layer locations match exactly field region boundaries; so, displace a bit;
  //Double_t waferSpacing          =  420.0;
  // FIXME: for now this should be in sync with B0 length, location & 
  // lambda setting in EicBeamLineElementGrad::GetFieldValue() call;
  Double_t waferSpacing          =  (1200.+2*100.)/(waferNum-1);
  Double_t beamLineOffsetX       =  235.0;
  Double_t beamLineOffsetZ       = 5600.0;

  // Holes in the silicon plates will be done such that they mimic that many [mm] 
  // away from the nominal cone of that many [rad] of the outgoing hadron beam;
  Double_t beam_pipe_clearance       =   10.0; 
  Double_t nominal_proton_cone_size  =    0.005;
  // IR coordinate system is aligned with electron beam; same is true for B0 assembly;
  // therefore proton beam pipe will be "moving towards +X" with increasing Z; 
  Double_t nominal_proton_direction  =    0.0215;

  // Silicon wafer; use the same at all 4 locations; holes will be made differently 
  // though -> composite shapes and respective volumes will be unique;
  TGeoBBox *wafer = new TGeoBBox("B0SiliconWafer", 
				  0.1 * waferWidth/2,
				  0.1 * waferHeight/2,
				  0.1 * waferThickness/2);
  
  for(unsigned wf=0; wf<waferNum; wf++)
  {
    char hname[128];
    sprintf(hname, "B0SiliconHole%02d", wf);

    double dz = (wf - (waferNum-1)/2.)*waferSpacing, z0 = beamLineOffsetZ + dz;

    // Calculate hole radius; the further away from the IP the more; there is 
    // no real gain in this complication, could have taken same r0 for all 4 plates;
    double r0 = z0*tan(nominal_proton_cone_size) + beam_pipe_clearance;
    TGeoTube *hole = new TGeoTube(hname, 
				  0.0,
				  0.1 * r0,
				  0.1 * waferThickness/2 + 0.1);

    // Calculate hole offset in the local wafer coordinate system;
    double dx = z0*tan(nominal_proton_direction) - beamLineOffsetX; 
    printf("%f %f\n", r0, dx);
    
    char tname[128];
    sprintf(tname, "combi%02d", wf);
    TGeoCombiTrans *combi = new TGeoCombiTrans(tname, 0.1 * dx, 0, 0, 0);
    combi->RegisterYourself();

    char vname[128], cname[128];
    sprintf(vname, "B0SiliconPlate%02d", wf);
    sprintf(cname, "B0SiliconWafer-%s:%s", hname, tname);
    TGeoCompositeShape *comp = new TGeoCompositeShape(vname, cname);

    TGeoVolume *vwafer = new TGeoVolume(vname, comp, b0tracker->GetMedium("silicon"));

    EicGeoMap *fgmap = b0tracker->CreateNewMap();
    fgmap->AddGeantVolumeLevel(vname, waferNum);
    fgmap->SetSingleSensorContainerVolume(vname);

    b0tracker->AddLogicalVolumeGroup(0, 0, 1);//waferNum);

    {
      UInt_t geant[1] = {0}, group = wf, logical[3] = {0, 0, 0};
    
      if (b0tracker->SetMappingTableEntry(fgmap, geant, group, logical)) {
	cout << "Failed to set mapping table entry!" << endl;
	exit(0);
      } //if
    }

    b0tracker->GetTopVolume()->AddNode(vwafer, 0, 
				       new TGeoCombiTrans(0.1 * beamLineOffsetX, 0.0, 0.1 * (beamLineOffsetZ + dz), 0));
  } // for wf

  b0tracker->GetColorTable()->AddPatternMatch       ("Silicon", kYellow);
  b0tracker->GetTransparencyTable()->AddPatternMatch("Silicon", 50);

  b0tracker->FinalizeOutput();

  // Yes, always exit;
  exit(0);
} // b0tracker()

