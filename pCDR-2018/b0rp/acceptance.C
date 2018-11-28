
#define _41_GEV_
//#define _100_GEV_
//#define _275_GEV_
//#define _THETA_PLOT_

// Assume 41 x 5 GeV case in table 3.3;
#ifdef _41_GEV_
// Beam emittance; converted to [mm];
#define _EX_         (45.9E-6)
#define _EY_         ( 2.6E-6)

// Beam energy spread; 
#define _DPP_        (10.3E-4)
#endif
// Assume 100 x 10 GeV case in table 3.3;
#ifdef _100_GEV_
#define _EX_         (33.8E-6)
#define _EY_         ( 1.8E-6)

#define _DPP_        (9.1E-4)
#endif
// Assume 275 x 18 GeV case in table 3.3;
#ifdef _275_GEV_
#define _EX_         (12.4E-6)
#define _EY_         ( 5.5E-6)

#define _DPP_        (4.6E-4)
#endif

//
// Assume the rest is the same for all three cases;
//

// Central trajectory offset; must be tuned by hand;
#define _X0_         (-0.73)
#define _Y0_         ( 0.00)
#define _SX0_        (38.76)
#define _SY0_        ( 0.00)

// This cut will limit acceptance at small theta; 
#define _SIGMA_CUT_  (10.0)

// Dispersion at RP location in [mm/10^-3];
#define _DX_         (-0.19)

// Beta functions around Z~30m for this beam tune; converted to [mm];
#define _BETA_X_     (475E3)
#define _BETA_Y_     (520E3)

//void acceptance()
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");  

  gStyle->SetOptStat(0);
  gStyle->SetPadLeftMargin(0.13);
  gStyle->SetPadBottomMargin(0.13);

  // Input simulated & reconstructed files;
  TFile *ff = new TFile("simulation.root");
  TTree *cbmsim = ff->Get("cbmsim"); 
  cbmsim->AddFriend("cbmsim", "digitization.root");

  TClonesArray *rpHitArray = new TClonesArray("EicTrackingDigiHitOrth2D"); 
  cbmsim->SetBranchAddress("RpTrackingDigiHit", &rpHitArray);
  TClonesArray *b0HitArray = new TClonesArray("EicTrackingDigiHitOrth2D"); 
  cbmsim->SetBranchAddress("B0trackerTrackingDigiHit", &b0HitArray);
  TClonesArray *mcTrackArray = new TClonesArray("PndMCTrack");
  cbmsim->SetBranchAddress("MCTrack", &mcTrackArray);

  TCanvas *c1 = new TCanvas("c1", "c1", 0, 0, 800, 500);

  TH1D *th0 = new TH1D("th0", "", 250,    0.,  25.);
  th0->SetMinimum(0); th0->SetLineWidth(2); th0->SetLineColor(kBlack);
  TH1D *th1 = new TH1D("th1", "", 250,    0.,  25.); th1->SetLineWidth(2); th1->SetLineColor(kGreen);
  TH1D *th2 = new TH1D("th2", "", 250,    0.,  25.); th2->SetLineWidth(2); th2->SetLineColor(kRed);
  TH1D *th3 = new TH1D("th3", "", 250,    0.,  25.); th3->SetLineWidth(2); th3->SetLineColor(kBlue);


#if 1
  TH1D *thq3 = new TH1D("thq3", "", 100,    0.,  10.);
  thq3->SetMinimum(0); thq3->SetLineWidth(2); thq3->SetLineColor(kBlue); 
  TH1D *pq3 = new TH1D("pq3", "" ,  80,   42.-20.,  42.+20.); 
  pq3->SetLineColor(kBlue); pq3->SetMinimum(0); pq3->SetLineWidth(2); 
  TH2D *pthq3 = new TH2D("pthq3", "" ,  80,   42.-20.,  42.+20., 40,    0.,  10.); 
#else
  TH1D *thq3 = new TH1D("thq3", "", 100,    0.,  4.);
  thq3->SetMinimum(0); thq3->SetLineWidth(2); thq3->SetLineColor(kBlue); 
  TH1D *pq3 = new TH1D("pq3", "" ,  80,   60.,  140.0); 
  pq3->SetLineColor(kBlue); pq3->SetMinimum(0); pq3->SetLineWidth(2); 
  TH2D *pthq3 = new TH2D("pthq3", "" ,  80,   60.,  140.0, 40,    0.,  4.); 
#endif

  //TH1D *thres = new TH1D("thres", "", 100,  -1.,  1.);
  //TH1D *thyres = new TH1D("thyres", "", 100,  -1.,  1.);
  TH1D *pres  = new TH1D("pres", "",  100, -10., 10.);
  TH1D *dpt = new TH1D("dpt", "dpt", 100, -300., 300.);

  TH1D *hxl = new TH1D("hxl", "" ,  300,  -300.,  300.0);

  TH1D *pt0 = new TH1D("pt0", "" ,150,    0.,  1.5); 
  pt0->SetLineColor(kBlack); pt0->SetMinimum(0); pt0->SetLineWidth(1); 
  TH1D *pt1 = new TH1D("pt1", "", 150,    0.,  1.5); pt1->SetLineWidth(3); pt1->SetLineColor(kGreen);
  TH1D *pt2 = new TH1D("pt2", "", 150,    0.,  1.5); pt2->SetLineWidth(3); pt2->SetLineColor(kRed);
  TH1D *pt3 = new TH1D("pt3", "", 150,    0.,  1.5); pt3->SetLineWidth(3); pt3->SetLineColor(kBlue);

  TH1D *hq2d = new TH1D("hq2d", "hq2d", 100,    0.,  100.);
  TH1D *phi= new TH1D("phi","phi",100,  -3.14159, 3.14159);
  TH2D *acc= new TH2D("acc","acc",100,  -10.,  10., 100,  -10.,  10.);
  //TH1D *x0 = new TH1D("x0", "x0", 1000, -10., 10.);
  TH1D *x1 = new TH1D("x1", "x1", 3000, -30., 30.);
  TH1D *sx = new TH1D("sx", "sx", 1000, -10., 10.);
  //TH1D *xx = new TH1D("xx", "xx", 100, -650., -450.);
  TH1D *y1 = new TH1D("y1", "y1", 2000, -20., 20.);
  TH1D *sy = new TH1D("sy", "sy", 1000, -10., 10.);
  TH2D *xy = new TH2D("xy", "xy",  50, -120., 120.,  50, -120., 120.);
  //TH2D *xy = new TH2D("xy", "xy",  50, -650., -450.,  50, -100., 100.);

  TGeoRotation *rw = new TGeoRotation();
  double _22mrad_ = 0.022 * TMath::RadToDeg();
  rw->RotateY(_22mrad_);

  // FIXME: these are the values tuned for the particular optics of course;
  double aX[2][2] = {{-1.810, 24.070}, {-0.165, 0.240}}, aXinv[2][2], aYinv = 1/7.725;
  double det = aX[0][0]*aX[1][1] - aX[0][1]*aX[1][0]; assert(det);
  aXinv[0][0] =  aX[1][1]/det;
  aXinv[0][1] = -aX[0][1]/det;
  aXinv[1][0] = -aX[1][0]/det;
  aXinv[1][1] =  aX[0][0]/det;

  // Beam 1 sigma size at the RP location; 
  double sigx = sqrt(_EX_*_BETA_X_), sigy = sqrt(_EY_*_BETA_Y_);
  double sigp = _DX_ * _DPP_ * 1E3; //printf("%f\n", sigp); exit(0);
  sigx = sqrt(sigx*sigx + sigp*sigp);
  //printf("%f %f\n", sqrt(_EX_*_BETA_X_), sqrt(_EY_*_BETA_Y_)); exit(0);
  //printf("%f %f\n", _EX_*_BETA_X_, _EY_*_BETA_Y_); exit(0);

  // Loop through all events; NB: for box-generated events without secondaries 
  // could simply use cbmsim->Project() as well; in general EicEventAssembler 
  // should be used for "true" physics events;
  int nEvents = cbmsim->GetEntries();
  for(unsigned ev=0; ev<nEvents; ev++) {
    cbmsim->GetEntry(ev);

    assert(mcTrackArray->GetEntriesFast() == 1);
    PndMCTrack *mctrack = mcTrackArray->At(0);

    if (mctrack->GetPdgCode() == 2212 && mctrack->GetMotherID() == -1) {
      TVector3 pin = mctrack->GetMomentum(); 
      //+if (pin.Mag() < 38) continue;
      double arr[3], out[3]; arr[0] = pin.x(); arr[1] = pin.y(); arr[2] = pin.z();

      rw->MasterToLocalVect(arr, out);
      TVector3 psim(out), prec;
      double theta = 1000*psim.Theta(), ptsim = psim.Pt();//Mag()*psim.Theta();
      printf("%d %d -> %7.2f %7.2f\n", b0HitArray->GetEntriesFast(), rpHitArray->GetEntriesFast(), theta, psim.Mag());
      //printf("%d %d -> %7.2f\n", b0HitArray->GetEntriesFast(), rpHitArray->GetEntriesFast(), theta);
      th0->Fill(theta); pq3->Fill(psim.Mag()); pt0->Fill(ptsim);//pt0->Fill(_P0_*psim.Theta());
#if _LATER_
      if (!b0HitArray->GetEntriesFast() && rpHitArray->GetEntriesFast() == 2) {
	th1->Fill(theta);
	//pt1->Fill(_P0_*psim.Theta());
	pt1->Fill(ptsim);
	
	// NB: will use the same _X0_ & _Y0_ for both; does not matter as long as [0] is 
	// used for slope calculation only;
	double XL[2], YL[2], base = 200.0;
	for(unsigned iq=0; iq<2; iq++) {
	  EicTrackingDigiHitOrth2D *rphit = rpHitArray->At(iq);
	
	  double xl = 10*rphit->GetCoord(0) - _X0_, yl = 10*rphit->GetCoord(1) - _Y0_;

	  XL[iq] = xl; YL[iq] = yl;

	  // NB: prefer to work with hit#1, for historic reasons;
	  if (iq == 1) {
	    //printf("XL = %7.2f, YL = %7.2f [mm]\n", xl, yl);
	    double qx = xl/sigx, qy = yl/sigy, q2d = sqrt(qx*qx+qy*qy);
	  
	    // 10 sigma in 2D;
	    if (q2d > _SIGMA_CUT_) {
	      th3->Fill(theta);
	      //pt3->Fill(_P0_*psim.Theta());
	      pt3->Fill(ptsim);

	      thq3->Fill(theta);
	      pq3->Fill(psim.Mag());
	      //printf("--> %f %f\n", 
	      pthq3->Fill(psim.Mag(), theta);
	    } //if
	    
	    hxl->Fill(xl);
	    x1->Fill(xl);
	    y1->Fill(yl);
	    //if (fabs(theta)< 1.) xx->Fill(xl);
	    //if (fabs(theta)< 1.) yy->Fill(yl);
	    xy->Fill(xl, yl);
	  } //if
	} //for iq

	double Xrp[2], Xip[2]; Xrp[0] = XL[1]; Xrp[1] = 1000*(XL[1] - XL[0])/base - _SX0_;
	double Yrp = YL[1], Yip = aYinv * Yrp; 
	Xip[0] = Xip[1] = 0.0;
	for(unsigned i0=0; i0<2; i0++)
	  for(unsigned i1=0; i1<2; i1++)
	    Xip[i0] += aXinv[i0][i1]*Xrp[i1];
	
	//printf("%7.3f %7.3f -> %7.3f\n", theta, Xip[1], Xip[1] - theta);
	double perc = 100*(psim.Mag()-275.)/275.;
	{
	  //double sx = htctrack->BeamSX(), sy = htctrack->BeamSY(), p = htctrack->mMomentum;
	  double rsx = Xip[1]/1000., rsy = Yip/1000., p = 275.*(1 + 0.01*Xip[0]);
	  double norm = sqrt(1.0 + rsx*rsx + rsy*rsy);
	  double arr[3], out[3]; arr[0] = p*rsx/norm; arr[1] = p*rsy/norm; arr[2] = p/norm;
	  
	  //rw->MasterToLocalVect(arr, out);
	  prec = TVector3(arr);//out); 
	  //printf("%f\n", prec.Mag());
	}
	//printf("%7.3f %7.3f -> %7.3f\n", perc, Xip[0], Xip[0] - perc);
	//thres->Fill(Xip[1] - theta);
	//thres->Fill(Yip - theta);
	//pres->Fill( Xip[0] - perc);
	pres->Fill(100.*(prec.Mag() - psim.Mag())/psim.Mag());
	//printf("%f\n", 1000*(prec.Pt() - psim.Pt()));
	dpt->Fill(1000*(prec.Pt() - psim.Pt()));

	//printf("%f\n", 1000*(XL[1] - XL[0])/base - _SX0_);
	sx->Fill(1000*(XL[1] - XL[0])/base - _SX0_);
	sy->Fill(1000*(YL[1] - YL[0])/base - _SY0_);
      } //if
#endif
      if (b0HitArray->GetEntriesFast() == 4) {
	th2->Fill(theta);
	//pt2->Fill(_P0_*psim.Theta());
	pt2->Fill(ptsim);
      } //if
    } //if
  } //for ev

  //phi->Draw();
  //acc->Draw("COLZ");
  //xy->Draw("COLZ");
  pq3->GetXaxis()->SetTitle("Spectator proton momentum [GeV/c]");
  pq3->GetXaxis()->SetTitleOffset(0.9);
  pq3->GetXaxis()->SetLabelFont(52);
  pq3->GetXaxis()->SetLabelSize(0.050);
  pq3->GetXaxis()->SetTitleFont(52);
  pq3->GetXaxis()->SetTitleSize(0.060);

  pq3->GetYaxis()->SetTitle("Events");
  pq3->GetYaxis()->SetTitleOffset(1.0);
  pq3->GetYaxis()->SetLabelFont(52);
  pq3->GetYaxis()->SetLabelSize(0.050);
  pq3->GetYaxis()->SetTitleFont(52);
  pq3->GetYaxis()->SetTitleSize(0.060);

  thq3->GetXaxis()->SetTitle("Spectator proton theta [mrad]");
  thq3->GetXaxis()->SetTitleOffset(0.9);
  thq3->GetXaxis()->SetLabelFont(52);
  thq3->GetXaxis()->SetLabelSize(0.050);
  thq3->GetXaxis()->SetTitleFont(52);
  thq3->GetXaxis()->SetTitleSize(0.060);

  thq3->GetYaxis()->SetTitle("Events");
  thq3->GetYaxis()->SetTitleOffset(1.0);
  thq3->GetYaxis()->SetLabelFont(52);
  thq3->GetYaxis()->SetLabelSize(0.050);
  thq3->GetYaxis()->SetTitleFont(52);
  thq3->GetYaxis()->SetTitleSize(0.060);

  pthq3->GetXaxis()->SetTitle("Spectator proton momentum [GeV/c]");
  pthq3->GetXaxis()->SetTitleOffset(0.9);
  pthq3->GetXaxis()->SetLabelFont(52);
  pthq3->GetXaxis()->SetLabelSize(0.050);
  pthq3->GetXaxis()->SetTitleFont(52);
  pthq3->GetXaxis()->SetTitleSize(0.060);

  //pthq3->GetYaxis()->SetTitle("Events");
  pthq3->GetYaxis()->SetTitle("Spectator proton theta [mrad]");
  pthq3->GetYaxis()->SetTitleOffset(0.8);
  pthq3->GetYaxis()->SetLabelFont(52);
  pthq3->GetYaxis()->SetLabelSize(0.050);
  pthq3->GetYaxis()->SetTitleFont(52);
  pthq3->GetYaxis()->SetTitleSize(0.060);

#ifdef _THETA_PLOT_
  th0->GetXaxis()->SetTitle("Leading proton theta [mrad]");
  th0->GetXaxis()->SetTitleOffset(0.9);
  th0->GetXaxis()->SetLabelFont(52);
  th0->GetXaxis()->SetLabelSize(0.050);
  th0->GetXaxis()->SetTitleFont(52);
  th0->GetXaxis()->SetTitleSize(0.060);

  th0->GetYaxis()->SetTitle("Events");
  th0->GetYaxis()->SetTitleOffset(0.8);
  th0->GetYaxis()->SetLabelFont(52);
  th0->GetYaxis()->SetLabelSize(0.050);
  th0->GetYaxis()->SetTitleFont(52);
  th0->GetYaxis()->SetTitleSize(0.060);
  th0->Draw();
  th2->Draw("SAME");
  th3->Draw("SAME");
#else
  pt0->GetXaxis()->SetTitle("DVCS proton P_{t}  [GeV/c]");
  pt0->GetXaxis()->SetTitleOffset(0.9);
  pt0->GetXaxis()->SetLabelFont(52);
  pt0->GetXaxis()->SetLabelSize(0.050);
  pt0->GetXaxis()->SetTitleFont(52);
  pt0->GetXaxis()->SetTitleSize(0.060);

  pt0->GetYaxis()->SetTitle("Events");
  pt0->GetYaxis()->SetTitleOffset(0.8);
  pt0->GetYaxis()->SetLabelFont(52);
  pt0->GetYaxis()->SetLabelSize(0.050);
  pt0->GetYaxis()->SetTitleFont(52);
  pt0->GetYaxis()->SetTitleSize(0.060);

  pt0->Draw();
  pt2->Draw("SAME");
  pt3->Draw("SAME");
#endif

  gPad->SetGrid();
} // acceptance()
