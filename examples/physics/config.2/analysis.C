
//
//  This script is specifically tuned to be run on the PC farm 
//  production output; may want to optimize all the logic for speed later; for 
//  now just loop through directories one by one; 
//
//  If run scripts by hand (and produce single reconstruction.root file), then just:
//
//  root -l 'analysis.C(".", 1)'
//

// For production mode running; otherwise expect to find files right in 
// the "dir" directory (see parameters);
//#define _EXPECT_SUBDIRECTORIES_

//#define _SHOW_PURITY_
//#define _SHOW_KINEMATICS_

#define _X_MIN_   (1E-5)
#define _X_MAX_   (1)
#if defined(_SHOW_PURITY_) || defined(_SHOW_KINEMATICS_)
#define _X_BNUM_  (5*5*1)
#else
#define _X_BNUM_  (200)
#endif
#define _LOGX_BWID_  ((log(_X_MAX_)-log(_X_MIN_))/_X_BNUM_)

#define _Q2_MIN_  (1)
#define _Q2_MAX_  (1E3)
#if defined(_SHOW_PURITY_) || defined(_SHOW_KINEMATICS_)
#define _Q2_BNUM_ (3*4)
#else
#define _Q2_BNUM_ (200)
#endif
#define _LOGQ2_BWID_  ((log(_Q2_MAX_)-log(_Q2_MIN_))/_Q2_BNUM_)

#define _Y_BNUM_  (200)

void analysis(const char *dir, unsigned nn = 1)
{
#if defined(_SHOW_PURITY_) && defined(_SHOW_KINEMATICS_)
  printf("defined(_SHOW_PURITY_) && defined(_SHOW_KINEMATICS_): one at a time, please!\n");
  exit(0);
#endif

  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");  
  gStyle->SetOptStat(0);
  gStyle->SetPalette(55, 0);
  gStyle->SetLabelSize(0.04, "xy");
  gStyle->SetPadLeftMargin(0.12);
  gStyle->SetPadBottomMargin(0.13);
  gStyle->SetTextFont(52);

  unsigned ok [_X_BNUM_][_Q2_BNUM_];
  unsigned in [_X_BNUM_][_Q2_BNUM_];
  memset(ok,  0x00, sizeof(ok));
  memset(in,  0x00, sizeof(in));

  double xb[_X_BNUM_+1], q2b[_Q2_BNUM_+1];
  
  for(int ip=0; ip<=_X_BNUM_; ip++)
  {
    xb [ip] = exp(log(_X_MIN_) + ip*_LOGX_BWID_);
    //printf("%2d -> %f\n", ip, xb[ip]);
  }
  for(int iq=0; iq<=_Q2_BNUM_; iq++)
  {
    q2b[iq] = exp(log(_Q2_MIN_) + iq*_LOGQ2_BWID_);
    //printf("%2d -> %f\n", iq, q2b[iq]);
  }
  
  TH2D *xx  = new TH2D("xx",  "",    _X_BNUM_,  xb,  _X_BNUM_,  xb);
  TH2D *yy  = new TH2D("yy",  "",    _Y_BNUM_,  0.01, 1.0, _Y_BNUM_,  0.01, 1.0);
  TH1D *y1d = new TH1D("y1d", "",    _Y_BNUM_,  0.01, 1.0);
  TH2D *qq  = new TH2D("qq",  "",    _Q2_BNUM_, q2b, _Q2_BNUM_, q2b);
  TH1D *q1d = new TH1D("q1d", "q1d", _Q2_BNUM_, q2b);
  TH2D *xq  = new TH2D("xq",  "xq",  _X_BNUM_,  xb,  _Q2_BNUM_, q2b);
  TH2D *mg  = new TH2D("mg",  "",    _X_BNUM_,  xb,  _Q2_BNUM_, q2b);
  
#ifndef _EXPECT_SUBDIRECTORIES_
  nn = 1;
#endif

  // Input simulated & reconstructed files;
  for(unsigned iqn=0; iqn<nn; iqn++) {
    char qdir[128] = "/";
#ifdef _EXPECT_SUBDIRECTORIES_
    snprintf(qdir, 128-1, "/%05d/", iqn);
#endif
    TString mcInFile  = TString(dir) + TString(qdir) + "simulation.root";
    TString rcInFile  = TString(dir) + TString(qdir) + "reconstruction.root";
    
    EicRootManager *io = new EicRootManager(mcInFile, 0, rcInFile);
    if (io->GetEicRcEventBranch()) {
      printf("chunk# %4d, %6d entries ...\n", iqn, io->GetEicRcTreeEntries());

      for(unsigned ev=0; ev<io->GetEicRcTreeEntries(); ev++) {
	io->GetEicRcTreeEntry(ev);

	const EicRcEvent *rcEvent = io->GetEicRcEvent();
	erhic::EventMC   *mcEvent = rcEvent->GetGenMcEvent();

	if (!rcEvent->GetNTracks()) continue;
	  
	{
	  erhic::LeptonKinematicsComputer rcDis(*rcEvent);
	  erhic::DisKinematics *rcKin = rcDis.Calculate();
	  //printf("X: %7.5f; Q^2: %8.1f\n", rcKin->mX, rcKin->mQ2);
	  unsigned rcIx = mg->GetXaxis()->FindBin(rcKin->mX)  - 1;
	  unsigned rcIq = mg->GetYaxis()->FindBin(rcKin->mQ2) - 1; 
	    
	  erhic::LeptonKinematicsComputer mcDis(*mcEvent);
	  erhic::DisKinematics *mcKin = mcDis.Calculate();
	  //printf("X: %7.5f; Q^2: %8.1f\n\n", mcKin->mX, mcKin->mQ2); 
	  unsigned mcIx = mg->GetXaxis()->FindBin(mcKin->mX)  - 1;
	  unsigned mcIq = mg->GetYaxis()->FindBin(mcKin->mQ2) - 1;
	  
	  if (mcIx >=0 && mcIx < _X_BNUM_ && 
	      mcIq >=0 && mcIq < _Q2_BNUM_) {
	    if (mcIx == rcIx && mcIq == rcIq) 
	      ok[mcIx][mcIq]++;
	    else {
	      if (rcIx >=0 && rcIx < _X_BNUM_ && 
		  rcIq >=0 && rcIq < _Q2_BNUM_)
		in[rcIx][rcIq]++;
	    } //if
	    
	      // Yes, in particular want Y-smearing plot to have Q^2 < 1 cut applied; 
	    xx->Fill (mcKin->mX,  rcKin->mX);
	    yy->Fill (mcKin->mY,  rcKin->mY);
	    y1d->Fill(mcKin->mY);
	    qq->Fill (mcKin->mQ2, rcKin->mQ2);
	    
	    q1d->Fill(mcKin->mQ2);
	    xq->Fill (mcKin->mX, mcKin->mQ2);
	  } //if
	}     
      } //for ev
    } 
    else
      printf("chunk# %4d: corrupted (or missing)\n", iqn);
    
    delete io;
  } //for iqn

  for(int ix=0; ix<_X_BNUM_; ix++) 
    for(int iq=0; iq<_Q2_BNUM_; iq++) {
      double value = ok[ix][iq] ? 1.0*ok[ix][iq]/(ok[ix][iq]+in[ix][iq]) : 0.0;
      mg->SetBinContent(ix+1, iq+1, value);
    } //for ix..iq

#if !defined(_SHOW_PURITY_) && !defined(_SHOW_KINEMATICS_)
  TCanvas *c11 = new TCanvas("c11", "c11", 0, 0, 1400, 400);
  c11->Divide(3,1);

  c11->cd(1);
  gPad->SetLogz();
  
  yy->SetMinimum(2);
  yy->GetXaxis()->SetTitle("Simulated Y");
  yy->GetXaxis()->SetTitleSize(0.05);
  yy->GetXaxis()->SetTitleFont(52);
  yy->GetXaxis()->SetLabelSize(0.04);
  yy->GetXaxis()->SetLabelFont(52);
  yy->GetXaxis()->SetTitleOffset(1.2);
  yy->GetYaxis()->SetTitle("Reconstructed Y");
  yy->GetYaxis()->SetTitleSize(0.05);
  yy->GetYaxis()->SetTitleFont(52);
  yy->GetYaxis()->SetLabelSize(0.04);
  yy->GetYaxis()->SetLabelFont(52);
  yy->Draw("COLZ");

  c11->cd(2);
  gPad->SetLogx();
  gPad->SetLogy();
  gPad->SetLogz();
  xx->SetMinimum(2);
  xx->GetXaxis()->SetTitle("Simulated X_{Bj}");
  xx->GetXaxis()->SetTitleSize(0.05);
  xx->GetXaxis()->SetTitleFont(52);
  xx->GetXaxis()->SetLabelSize(0.04);
  xx->GetXaxis()->SetLabelFont(52);
  xx->GetXaxis()->SetTitleOffset(1.2);
  xx->GetYaxis()->SetTitle("Reconstructed X_{Bj}");
  xx->GetYaxis()->SetTitleSize(0.05);
  xx->GetYaxis()->SetTitleFont(52);
  xx->GetYaxis()->SetLabelSize(0.04);
  xx->GetYaxis()->SetLabelFont(52);
  xx->Draw("COLZ");

  c11->cd(3);
  gPad->SetLogx();
  gPad->SetLogy();
  gPad->SetLogz();
  qq->SetMinimum(2);
  qq->GetXaxis()->SetTitle("Simulated Q^{2}, [GeV^{2}]");
  qq->GetXaxis()->SetTitleSize(0.05);
  qq->GetXaxis()->SetTitleFont(52);
  qq->GetXaxis()->SetLabelSize(0.04);
  qq->GetXaxis()->SetLabelFont(52);
  qq->GetXaxis()->SetTitleOffset(1.2);
  qq->GetYaxis()->SetTitle("Reconstructed Q^{2}, [GeV^{2}]");
  qq->GetYaxis()->SetTitleSize(0.05);
  qq->GetYaxis()->SetTitleFont(52);
  qq->GetYaxis()->SetLabelSize(0.04);
  qq->GetYaxis()->SetLabelFont(52);
  qq->Draw("COLZ");
#else
  TCanvas *c11 = new TCanvas("c11", "c11", 0, 0, 700, 450);
#ifdef _SHOW_PURITY_
  TH2D *hh = mg;
#else
  TH2D *hh = xq;
  c11->SetLogz();
#endif
  c11->SetLogx();
  c11->SetLogy();

  hh->GetXaxis()->SetTitle("Simulated X_{Bj}");
  hh->GetXaxis()->SetTitleSize(0.05);
  hh->GetXaxis()->SetTitleFont(52);
  hh->GetXaxis()->SetLabelSize(0.04);
  hh->GetXaxis()->SetLabelFont(52);
  hh->GetXaxis()->SetTitleOffset(1.2);
  hh->GetYaxis()->SetTitle("Simulated Q^{2}, [GeV^{2}]");
  hh->GetYaxis()->SetTitleSize(0.05);
  hh->GetYaxis()->SetTitleFont(52);
  hh->GetYaxis()->SetLabelSize(0.04);
  hh->GetYaxis()->SetLabelFont(52);
  hh->SetMaximum(1.0);
  hh->Draw("COLZ");
  //q1d->Draw();

  TF1 *y01 = new TF1("y01", "0.01*140*140*x");
  y01->SetLineColor(kBlack);
  y01->Draw("SAME");
  TText *t01 = new TText(0.025, 2.5, "y = 0.01");
  t01->SetTextAngle(45);
  t01->Draw();

  TF1 *y10 = new TF1("y10", "0.10*140*140*x");
  y10->SetLineColor(kBlack);
  y10->Draw("SAME");
  TText *t10 = new TText(0.0025, 2.5, "y = 0.10");
  t10->SetTextAngle(45);
  t10->Draw();

  TF1 *y95 = new TF1("y95", "0.95*140*140*x");
  y95->SetLineColor(kBlack);
  y95->Draw("SAME");
  TText *t95 = new TText(0.00025, 2.5, "y = 0.95");
  t95->SetTextAngle(45);
  t95->Draw();
#endif
} // analysis()
