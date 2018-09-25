
#define _NDF_MAX_ 1000

void analysis()
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");  

  // Input simulated & reconstructed files;
  TFile *ff = new TFile("simulation.root");
  TTree *cbmsim = ff->Get("cbmsim"); 
  cbmsim->AddFriend("cbmsim", "reconstruction.root");

  // Figure out max and most popular ndf value;
  TClonesArray *rctrk = new TClonesArray("PndPidCandidate");
  cbmsim->SetBranchAddress("PidChargedCand",&rctrk);
  unsigned nEvt = cbmsim->GetEntries(), ndfMax = 0;
  unsigned arr[1000]; memset(arr, 0x00, sizeof(arr));
  for (unsigned evt = 0; evt<nEvt; evt++) {
    cbmsim->GetEntry(evt);
    
    if (rctrk->GetEntriesFast()) {
      PndPidCandidate *rctrack = rctrk->At(0);
      
      int ndf = rctrack->GetDegreesOfFreedom();
      if (ndf > ndfMax) ndfMax = ndf;
      if (ndf < _NDF_MAX_) arr[ndf]++;
    }
  }
  unsigned ndfMostPopular = 0, ndfMostPopularStat = 0;
  for(unsigned iq=0; iq<_NDF_MAX_; iq++)
    if (arr[iq] > ndfMostPopularStat) {
      ndfMostPopular = iq;
      ndfMostPopularStat = arr[iq];
    }

  char *expression = "100.*(PidChargedCand.GetMomentum().Mag()-MCTrack.GetMomentum().Mag())/MCTrack.GetMomentum().Mag()";
  //char *expression = "10000.*(PidChargedCand.GetMomentum().Theta()-MCTrack.GetMomentum().Theta())";
  //char *expression = "1000.*(PidChargedCand.GetMomentum().Phi()-MCTrack.GetMomentum().Phi())";
  char cut[1024];
  // Allow to lose 3 degrees of freedom compared to the max (assume standard) case;
  sprintf(cut, "EicIdealGenTrack.fNDF>=%d&&EicIdealGenTrack.fChiSquareCCDF>.001", ndfMostPopular-2);

  double par[100]; memset(par, 0x00, sizeof(par));

  TH1D *dp1 = new TH1D("dp1", "dp1", 100, -20.0, 20.0);
  cbmsim->Project("dp1", expression, cut);
  TF1 *fq1 = new TF1("fq1", "gaus(0)",    -20.0, 20.0);
  par[0] = 100.0; par[1] = 0.0; par[2] = 1.0;
  fq1->SetParameters(par);
  dp1->Fit("fq1","R");
  fq1->GetParameters(par);

  double sigma2 = fabs(par[2]), min2 = -5*sigma2, max2 = 5*sigma2;

  TH1D *dp2 = new TH1D("dp2", "dp2", 100, min2, max2);
  cbmsim->Project("dp2", expression, cut);
  TF1 *fq2 = new TF1("fq2", "gaus(0)", min2, max2);
  fq2->SetParameters(par);
  dp2->Fit("fq2","R");
  fq2->GetParameters(par);

  double sigma3 = fabs(par[2]), min3 = -5*sigma3, max3 = 5*sigma3;

  TH1D *dp3 = new TH1D("dp3", "dp3", 100, min3, max3);
  cbmsim->Project("dp3", expression, cut);
  TF1 *fq3 = new TF1("fq3", "gaus(0)", min3, max3);
  fq3->SetParameters(par);
  dp3->Fit("fq3","R");
} // analysis()
