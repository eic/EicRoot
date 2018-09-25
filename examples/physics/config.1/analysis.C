
// Bjorken X smearing 2D plot parameters;
#define _X_MIN_   (1E-5)
#define _X_MAX_   (1)
#define _X_BNUM_  (100)
#define _LOGX_BWID_  ((log(_X_MAX_)-log(_X_MIN_))/_X_BNUM_)

void analysis()
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");  

  // Force some beautification;
  gStyle->SetOptStat(0);
  gStyle->SetPalette(55, 0);
  gStyle->SetLabelSize(0.04, "xy");
  gStyle->SetPadLeftMargin(0.12);
  gStyle->SetPadBottomMargin(0.13);
  gStyle->SetTextFont(52);

  // Calculate binning in log units;
  double xb[_X_BNUM_+1];
  for(int ip=0; ip<=_X_BNUM_; ip++)
    xb [ip] = exp(log(_X_MIN_) + ip*_LOGX_BWID_);
  
  // Declare 2D smearing plot;
  TH2D *xx  = new TH2D("xx",  "",    _X_BNUM_,  xb,  _X_BNUM_,  xb);
  
  // Input MC & reconstructed event file;
  TString mcInFile  = "simulation.root";
  TString rcInFile  = "reconstruction.root";

  // Well, a clear hack for now; have to use this wrapper rather
  // than ROOT friend declaration and TTree-based GetEntry() calls;
  EicRootManager *io = new EicRootManager(mcInFile, 0, rcInFile);

  // Loop through all events;
  for(unsigned ev=0; ev<io->GetEicRcTreeEntries(); ev++) {
    // Get event entry (TTree GetEntry() wrapper which takes care 
    // to establish all the links between different event components);
    io->GetEicRcTreeEntry(ev);

    // Get pointers to the reconstructed and respective simulated event parts;
    const EicRcEvent *rcEvent = io->GetEicRcEvent();
    erhic::EventMC   *mcEvent = rcEvent->GetGenMcEvent();
    
    // If record has no reconstructed tracks, nothing to talk about -> skip;
    // in fact expect exactly one track (leading electron); 
    if (!rcEvent->GetNTracks()) continue;
            
    // Use reconstructed quantities to calculate scattered lepton kinematics;
    erhic::LeptonKinematicsComputer rcDis(*rcEvent);
    erhic::DisKinematics *rcKin = rcDis.Calculate();
    
    // Use simulated quantities to calculate scattered lepton kinematics;
    erhic::LeptonKinematicsComputer mcDis(*mcEvent);
    erhic::DisKinematics *mcKin = mcDis.Calculate();
    
    // Fill 2D plot;
    xx->Fill(mcKin->mX,  rcKin->mX);
    
    // May want to loop through all the tracks later (if pass all tracks
    // through the Monte-Carlo rather than just leading lepton);
    //printf("  -> %2d\n", event->GetNTracks());
    //for(unsigned tr=0; tr<rcEvent->GetNTracks(); tr++) {
    //const EicRcParticle *rcTrack = rcEvent->GetTrack(tr);
    //} //for tr
  } //for ev

  // Plotting part (trivial);
  TCanvas *c11 = new TCanvas("c11", "c11", 0, 0, 400, 400);
  gPad->SetLogx();
  gPad->SetLogy();
  gPad->SetLogz();
  //xx->SetMinimum(2);
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
} // analysis()
