//
// Shape up and customization of the original rad_length.C script;
//

// May want to produce plot either vs pseudorapidity or vs theta;
#define _ETA_MODE_

// [2..178] degrees (theta mode); [-4..+4] (eta mode);  
#define _THETA_MIN_  2.0
#define _ETA_MAX_    4.0

#define _BIN_NUM_    200

#define _YAXIS_MAX_ 0.12

struct {
  char *name;
  int color;
} dets[] = {
  // Yes, better to have fixed colors;
  {"cave",          kMagenta},
  {"pipe",          kGray},
  {"vst",           kYellow},
  {"fst",           kGreen},
  {"bst",           kGreen},
  {"TPC/TpcIfc",    kBlue},
  {"TPC/TpcGas",    kCyan},
  //{"MM",            kBlue},
  //{"TPC/TpcOfc",    kBlue},
  //{"TPC/TpcEndcap", kRed},
  //{"fgt",           kOrange},
  //{"bgt",           kOrange},
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


void rad_length() {
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
#ifdef _ETA_MODE_
    TH1F* tmp = h[i] = new TH1F(dets[i].name, dets[i].name, _BIN_NUM_, -_ETA_MAX_, _ETA_MAX_);
#else
    TH1F* tmp = h[i] = new TH1F(dets[i].name, dets[i].name, _BIN_NUM_, _THETA_MIN_, 180.0 - _THETA_MIN_);
#endif

    tmp->SetMarkerStyle(21);
    tmp->SetMarkerColor(dets[i].color);
    tmp->SetMarkerSize(1.0);
    tmp->SetFillColor(dets[i].color);
  }

  for (Int_t i = 0; i < _BIN_NUM_; i++) {
    // The trick is that I do not want to account cave air beyond the last tracking element
    // along this theta line; makes sense?;
    double caveAccu = 0.0;

#ifdef _ETA_MODE_
    double eta_bin_width = 2 * _ETA_MAX_/_BIN_NUM_;
    double eta = -_ETA_MAX_ + (i - 0.5)*eta_bin_width;
    double theta = 2 * atan(exp(-eta));
#else
    double theta_bin_width = (180.0 - 2*_THETA_MIN_)/_BIN_NUM_;
    double theta = (_THETA_MIN_ + (i - 0.5)*theta_bin_width)*TMath::Pi()/180.;
#endif 
    printf("bin# %3d (of %3d) ...\n", i, _BIN_NUM_);

    double xx[3] = {0.0, 0.0, 0.1}, nn[3] = {TMath::Sin(theta), 0.0, TMath::Cos(theta)};
    gGeoManager->SetCurrentPoint    (xx);
    gGeoManager->SetCurrentDirection(nn);

    for(TGeoNode *node = gGeoManager->GetCurrentNode(); ; ) {
      TGeoMaterial *material = node->GetVolume()->GetMaterial();
      double radlen = material->GetRadLen();
      
      gGeoManager->FindNextBoundary();
      double thickness = gGeoManager->GetStep();
      
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
	    h[cave]->SetBinContent(i+1, h[cave]->GetBinContent(i+1) + caveAccu);
	    caveAccu = 0.0;
	  } //if

	  h[iii]->SetBinContent(i+1, h[iii]->GetBinContent(i+1) + thickness/radlen);
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
    
#ifdef _ETA_MODE_
  hs->GetXaxis()->SetTitle("Pseudorapidity");
#else
  hs->GetXaxis()->SetTitle("Theta");
#endif
  hs->GetXaxis()->SetLabelSize(0.03);
  hs->GetYaxis()->SetTitle("radiation length, X / X0");
  hs->GetYaxis()->SetLabelSize(0.03);
  hs->SetMaximum(_YAXIS_MAX_);
    
  TLegend *legend=new TLegend(0.38,0.88 - NSUB*.04,0.62,0.88);
  legend->SetTextFont(72);
  legend->SetTextSize(0.03);
  for (int i = 0; i < NSUB; i++) {
    legend->AddEntry(h[ NSUB - i - 1 ], dets[NSUB - i - 1].name,"p");
  }
  legend->Draw();
  
  c1->SaveAs("radlen.gif");
  //exit(0);
}
