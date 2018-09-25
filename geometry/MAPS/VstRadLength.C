  
// Hard to match ALICE TDR scale; 
#define _PHI_MIN_     2.00
#define _PHI_MAX_    62.00

#define _BIN_NUM_     1000

// Try to match ALICE TDR scale;
#define _YAXIS_MAX_   0.80

struct {
  char *name;
  int color;
} dets[] = {
  // Colors a-la ALICE TDR;
  {"VST/VstStove00/VstChipAssembly00/VstMimosaShell",     kGreen+1},

  {"VST/VstStove00/VstChipAssembly00/VstWaterPipe",       kCyan+1},
  {"VST/VstStove00/VstChipAssembly00/VstWater",           kBlue},

  {"VST/VstStove00/VstChipAssembly00/VstColdPlate",       kBlack},

  {"VST/VstStove00/VstChipAssembly00/VstSideWall",        kGray+2},
  {"VST/VstStove00/VstChipAssembly00/VstFlatRoof",        kGray+2},
  {"VST/VstStove00/VstChipAssembly00/VstRoofSideBeam",    kGray+2},

  {"VST/VstStove00/VstChipAssembly00/VstEnforcementBeam", kBlack},

  {"VST/VstStove00/VstChipAssembly00/VstCellFlexLayer",   kRed+1},
  {"VST/VstStove00/VstChipAssembly00/VstAluStrips",       kRed+1},
};

TString RemoveCrap(TString &path)
{
  TString ret;
  unsigned len = path.Length();
  bool skip = true;

  // Admittedly this will NOT work for volume names containing underscores;
  for(unsigned iq=0; iq<len; iq++) {
    if (path[iq] == '/') 
      skip = false;
    else if (path[iq] == '_') 
      skip = true;

    if (!skip) ret += path[iq];
  } //for iq
  
  return ret;
} // RemoveCrap()


void VstRadLength() {
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  TFile* f = new TFile("simparams.root");
  f->Get("FairBaseParSet");

  // NB: CAVE material is not really accounted (so only subdetectors);
  TString cavePrefix = "/cave";
  int cave = -1;

  vector <TString> subs;
  for(int i=0; i<sizeof(dets)/sizeof(dets[0]); i++)
    if (!strcmp(dets[i].name, "cave")) {
      subs.push_back(cavePrefix);
      cave = i;
    }
    else
      subs.push_back(cavePrefix + "/" + dets[i].name);
  
  // Yes, this way it works in CINT (but can not use sizeof(dets) trick directly);
  const int NSUB = subs.size();

  TH1F* h[NSUB];
  for (int i = 0; i < NSUB; i++) {
    TH1F* tmp = h[i] = new TH1F(dets[i].name, dets[i].name, _BIN_NUM_, _PHI_MIN_, _PHI_MAX_);

    tmp->SetMarkerStyle(21);
    tmp->SetMarkerColor(dets[i].color);
    tmp->SetMarkerSize(1.0);
    tmp->SetFillColor(dets[i].color);
  }

  double phi_bin_width = (_PHI_MAX_ - _PHI_MIN_)/_BIN_NUM_;

  for (Int_t i = 0; i < _BIN_NUM_; i++) {
    // The trick is that I do not want to account cave air beyond the last tracking element
    // along this theta line; makes sense?;
    double caveAccu = 0.0;

    double phi = (_PHI_MIN_ + (i - 0.5)*phi_bin_width)*TMath::Pi()/180.;
    printf("bin# %3d (of %3d) ...\n", i, _BIN_NUM_);

    //double xx[3] = {0.0, 0.0, 0.1}, nn[3] = {TMath::Sin(theta), 0.0, TMath::Cos(theta)};
    double xx[3] = {0.0, 0.0, 0.1}, nn[3] = {TMath::Cos(phi), TMath::Sin(phi)};
    gGeoManager->SetCurrentPoint    (xx);
    gGeoManager->SetCurrentDirection(nn);

    for(TGeoNode *node = gGeoManager->GetCurrentNode(); ; ) {
      TGeoMaterial *material = node->GetVolume()->GetMaterial();
      double radlen = material->GetRadLen();
      
      gGeoManager->FindNextBoundary();
      double thickness = gGeoManager->GetStep();
      
      //cout << gGeoManager->GetPath() << endl;
      // Well, basically want to convert /cave_1/fstAir06_77/fst@06_6 to 
      // /cave/fstAir06/fst@06, etc;
      TString path = RemoveCrap(gGeoManager->GetPath());

      // See whether I'm interested in such a part; NB: first match wins!;
      for (int iii = 0; iii < NSUB; iii++) 
	if (iii == cave && path.EqualTo( cavePrefix )) {
	  // Pending approval ...;
	  caveAccu += thickness/radlen; 
	  break;
	}
	else if (iii != cave && path.BeginsWith( subs[iii] )) {
	  if (caveAccu) {
	    h[cave]->SetBinContent(i+1, h[cave]->GetBinContent(i+1) + 100. * caveAccu);
	    caveAccu = 0.0;
	  } //if

	  h[iii]->SetBinContent(i+1, h[iii]->GetBinContent(i+1) + 100. * thickness/radlen);
	  break;
	} //for..if

      // Switch to next node along {xx, nn[]} 3D line;
      node = gGeoManager->Step();
      assert(gGeoManager->IsEntering());
      
      // Once out of volume, break;
      if (gGeoManager->IsOutside()) break; 
    } //for inf
  } //for i

  THStack *hs = new THStack("hs","EIC Detector Geometry: Radiation Length Scan");
  for (int i = 0; i < NSUB; i++) {
    hs->Add( h[i] );
  }

  TCanvas* c1 = new TCanvas("c1","c1", 800, 600);
  c1->SetFrameBorderSize(0);
  hs->Draw();
    
  hs->GetXaxis()->SetTitle("Phi");
  hs->GetXaxis()->SetLabelSize(0.03);
  hs->GetYaxis()->SetTitle("radiation length X/X0, [%]");
  hs->GetYaxis()->SetLabelSize(0.03);
  hs->SetMaximum(_YAXIS_MAX_);
    
  TLegend *legend=new TLegend(0.38,0.88 - NSUB*.04,0.62,0.88);
  legend->SetTextFont(72);
  legend->SetTextSize(0.03);
  for (int i = 0; i < NSUB; i++) {
    legend->AddEntry(h[ NSUB - i - 1 ], dets[NSUB - i - 1].name,"p");
  }
  //legend->Draw();
  
  // Calculate average rad.length;
  {
    double averageRadLength = 0.0;

    for (int iii = 0; iii < NSUB; iii++) 
      for(unsigned iq=0; iq<_BIN_NUM_; iq++)
	averageRadLength += h[iii]->GetBinContent(iq+1);

    printf("Average rad.length: %5.3f\n", averageRadLength/_BIN_NUM_);
  }

  c1->SaveAs("radlen.gif");
  //exit(0);
}
