
TTree *htc;// = ff->Get("htc");

analysis()
{
  // Load FairRoot & EicRoot basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C"); 

  // Open ROOT file and find "htc" tree;
  TFile *ff = new TFile("reconstruction.root");
  htc = (TTree*)ff->Get("htc");

  htc->Draw("track.ChiSquareCCDF()");
} // analysis()
