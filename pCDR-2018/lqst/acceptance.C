
#define _THETA_PLOT_

// Assume 10 GeV energy; 2-d column in pCDR table 3.3;

// Beam emittance; converted to [mm];
//#define _EX_         (20.0E-6)
//#define _EY_         ( 1.0E-6)
#define _EX_         (22.0E-6)
#define _EY_         ( 3.3E-6)

// Beam energy spread; 
#define _DPP_        (10E-4)

// Central trajectory offset; must be tuned by hand;
//#define _X0_         ( 0.0)
#define _X0_         ( 207.24)
#define _Y0_         ( 0.00)

// This cut will limit acceptance at small theta; 
#define _SIGMA_CUT_  (10.0)

// Dispersion at LQST location in [mm/10^-3];
#define _DX_         (-0.22)

// Beta functions around Z ~ -30m for this beam tune; converted to [mm];
#define _BETA_X_     (87E3)
#define _BETA_Y_     (56E3)

// FIXME: hardcode to 10 GeV/c;
#define _P0_ (18.0)
// Approximately :-);
#define _ME_ (0.5E-4)

//void acceptance()
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");  

  gStyle->SetOptStat(0);
  gStyle->SetPadBottomMargin(0.13);

  // Input simulated & reconstructed files;
  TFile *ff = new TFile("simulation.root");
  TTree *cbmsim = ff->Get("cbmsim"); 
  cbmsim->AddFriend("cbmsim", "digitization.root");

  TClonesArray *lqstHitArray = new TClonesArray("EicTrackingDigiHitOrth2D"); 
  cbmsim->SetBranchAddress("LqstTrackingDigiHit", &lqstHitArray);
  TClonesArray *mcTrackArray = new TClonesArray("PndMCTrack");
  cbmsim->SetBranchAddress("MCTrack", &mcTrackArray);

  TCanvas *c1 = new TCanvas("c1", "c1", 0, 0, 800, 500);

  TH1D *th0 = new TH1D("th0", "", 250,    0.,  25.);
  th0->SetMinimum(0); th0->SetLineWidth(2); th0->SetLineColor(kBlack);
  TH1D *th1 = new TH1D("th1", "", 250,    0.,  25.); th1->SetLineWidth(2); th1->SetLineColor(kGreen);
  TH1D *th2 = new TH1D("th2", "", 250,    0.,  25.); th2->SetLineWidth(2); th2->SetLineColor(kRed);
  TH1D *th3 = new TH1D("th3", "", 250,    0.,  25.); th3->SetLineWidth(2); th3->SetLineColor(kBlue);


#if _TODAY_
  TH1D *thq3 = new TH1D("thq3", "", 40,    0.,  4.);
  thq3->SetMinimum(0); thq3->SetLineWidth(2); thq3->SetLineColor(kBlue); 
  TH1D *pq3 = new TH1D("pq3", "" ,  80,   60.,  140.0); 
  pq3->SetLineColor(kBlue); pq3->SetMinimum(0); pq3->SetLineWidth(2); 
  TH2D *pthq3 = new TH2D("pthq3", "" ,  80,   60.,  140.0, 40,    0.,  4.); 


  //TH1D *thres = new TH1D("thres", "", 100,  -1.,  1.);
  //TH1D *thyres = new TH1D("thyres", "", 100,  -1.,  1.);
  TH1D *pres  = new TH1D("pres", "",  100, -10., 10.);
  TH1D *dpt = new TH1D("dpt", "dpt", 100, -300., 300.);
#endif
  //TH1D *hxl = new TH1D("hxl", "" ,  200,  206.,  208.0);
  TH1D *hxl = new TH1D("hxl", "" ,  200,  -1.0,  1.0);
  //TH1D *q2h0 = new TH1D("q2h0", "" ,  1000,   0.0,  0.03);
  //TH1D *q2h3 = new TH1D("q2h3", "" ,  1000,   0.0,  0.03);
  TH1D *q2h0 = new TH1D("q2h0", "" ,  100,   -20.0,  5.0);
  q2h0->SetMinimum(0); q2h0->SetLineWidth(3); q2h0->SetLineColor(kBlack);
  TH1D *q2h1 = new TH1D("q2h1", "" ,  100,   -20.0,  5.0);
  q2h1->SetLineWidth(2); q2h1->SetLineColor(kGreen);
  TH1D *q2h3 = new TH1D("q2h3", "" ,  100,   -20.0,  5.0);
  q2h3->SetLineWidth(1); q2h3->SetLineColor(kBlue);
#if _TODAY_
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

  //TGeoRotation *rw = new TGeoRotation();
  //double _22mrad_ = 0.022 * TMath::RadToDeg();
  //rw->RotateY(_22mrad_);

  // FIXME: these are the values tuned for the particular optics of course;
  double aX[2][2] = {{-1.810, 24.070}, {-0.165, 0.240}}, aXinv[2][2], aYinv = 1/7.725;
  double det = aX[0][0]*aX[1][1] - aX[0][1]*aX[1][0]; assert(det);
  aXinv[0][0] =  aX[1][1]/det;
  aXinv[0][1] = -aX[0][1]/det;
  aXinv[1][0] = -aX[1][0]/det;
  aXinv[1][1] =  aX[0][0]/det;
#endif

  // Beam 1 sigma size at the LQST location; 
  double sigx = sqrt(_EX_*_BETA_X_), sigy = sqrt(_EY_*_BETA_Y_); //printf("%f %f\n", sigx, sigy); exit(0);
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
    PndMCTrack *mctrack = (PndMCTrack *)mcTrackArray->At(0);

    if (mctrack->GetPdgCode() == 11 && mctrack->GetMotherID() == -1) {
      TVector3 pin = mctrack->GetMomentum(); 

      double out[3]; out[0] = pin.x(); out[1] = pin.y(); out[2] = pin.z();

      //rw->MasterToLocalVect(arr, out);
      TVector3 psim(out);//, prec;
      // FIXME: do by hand for now;
      double p40[4], p41[4], dp4[4]; 
      p40[0] = sqrt(_ME_*_ME_ + _P0_*_P0_); p40[1] = p40[2] = 0.0; p40[3] = -_P0_;
      p41[0] = sqrt(_ME_*_ME_ + pin.Mag2()); p41[1] = pin.x(); p41[2] = pin.y(); p41[3] = pin.z();
      //printf("%f\n", pin.Z());
      for(unsigned iq=0; iq<4; iq++)
	dp4[iq] = p41[iq] - p40[iq];
      double q2 = dp4[0]*dp4[0];
      for(unsigned iq=1; iq<4; iq++)
	q2 -= dp4[iq]*dp4[iq];
      double theta = 1000*(180.0 - psim.Theta() * TMath::RadToDeg())*TMath::DegToRad();//, ptsim = psim.Pt();
      printf("%d -> %7.2f %7.2f  %f\n", lqstHitArray->GetEntriesFast(), theta, psim.Mag(), q2);
      //#if _TODAY_
      //printf("%d %d -> %7.2f\n", b0HitArray->GetEntriesFast(), lqstHitArray->GetEntriesFast(), theta);
      th0->Fill(theta); //pq3->Fill(psim.Mag()); pt0->Fill(ptsim);//pt0->Fill(_P0_*psim.Theta());
      //q2h0->Fill(-q2);
      q2h0->Fill(log(-q2));
      //#endif
      if (lqstHitArray->GetEntriesFast() == 2) {
	//#if _TODAY_
	th1->Fill(theta);
	q2h1->Fill(log(-q2));
	//pt1->Fill(_P0_*psim.Theta());
	//pt1->Fill(ptsim);
	//#endif
	
	// NB: will use the same _X0_ & _Y0_ for both; does not matter as long as [0] is 
	// used for slope calculation only;
	//double XL[2], YL[2], base = 200.0;
	//for(unsigned iq=0; iq<2; iq++) {
	EicTrackingDigiHitOrth2D *lqsthit = lqstHitArray->At(0);
	
	double xl = 10*lqsthit->GetCoord(0) - _X0_, yl = 10*lqsthit->GetCoord(1) - _Y0_;
	hxl->Fill(xl);

	//printf("XL = %7.2f, YL = %7.2f [mm]\n", xl, yl);
	double qx = xl/sigx, qy = yl/sigy, q2d = sqrt(qx*qx+qy*qy);
	  
	// 10 sigma in 2D;
	if (q2d > _SIGMA_CUT_) {
	  th3->Fill(theta);
	  //q2h3->Fill(-q2);
	  q2h3->Fill(log(-q2));
#if _TODAY_
	  //pt3->Fill(_P0_*psim.Theta());
	  pt3->Fill(ptsim);
	  
	  thq3->Fill(theta);
	  pq3->Fill(psim.Mag());
	  //printf("--> %f %f\n", 
	  pthq3->Fill(psim.Mag(), theta);
#endif
	} //if
	    
	hxl->Fill(xl);
	//x1->Fill(xl);
	//y1->Fill(yl);
	//if (fabs(theta)< 1.) xx->Fill(xl);
	//if (fabs(theta)< 1.) yy->Fill(yl);
	//xy->Fill(xl, yl);

#if _TODAY_
	double Xlqst[2], Xip[2]; Xlqst[0] = XL[1]; Xlqst[1] = 1000*(XL[1] - XL[0])/base - _SX0_;
	double Ylqst = YL[1], Yip = aYinv * Ylqst; 
	Xip[0] = Xip[1] = 0.0;
	for(unsigned i0=0; i0<2; i0++)
	  for(unsigned i1=0; i1<2; i1++)
	    Xip[i0] += aXinv[i0][i1]*Xlqst[i1];
	
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
#endif
      } //if
    } //if
  } //for ev

  q2h0->GetXaxis()->SetTitle("log(Q^{2})");
  q2h0->GetXaxis()->SetTitleOffset(0.9);
  q2h0->GetXaxis()->SetLabelFont(52);
  q2h0->GetXaxis()->SetLabelSize(0.050);
  q2h0->GetXaxis()->SetTitleFont(52);
  q2h0->GetXaxis()->SetTitleSize(0.060);

  q2h0->GetYaxis()->SetTitle("Events");
  q2h0->GetYaxis()->SetTitleOffset(0.8);
  q2h0->GetYaxis()->SetLabelFont(52);
  q2h0->GetYaxis()->SetLabelSize(0.050);
  q2h0->GetYaxis()->SetTitleFont(52);
  q2h0->GetYaxis()->SetTitleSize(0.060);

  q2h0->Draw();
  q2h1->Draw("SAME");
  q2h3->Draw("SAME");
  gPad->SetGrid();

#if _TODAY_
  //phi->Draw();
  //acc->Draw("COLZ");
  //xy->Draw("COLZ");
  pq3->GetXaxis()->SetTitle("Leading proton momentum [GeV/c]");
  pq3->GetXaxis()->SetTitleOffset(0.9);
  pq3->GetXaxis()->SetLabelFont(52);
  pq3->GetXaxis()->SetLabelSize(0.050);
  pq3->GetXaxis()->SetTitleFont(52);
  pq3->GetXaxis()->SetTitleSize(0.060);

  pq3->GetYaxis()->SetTitle("Events");
  pq3->GetYaxis()->SetTitleOffset(0.8);
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
  thq3->GetYaxis()->SetTitleOffset(0.8);
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

#endif
} // acceptance()
