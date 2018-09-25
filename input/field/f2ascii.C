
void f2ascii(TString fname)
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  // Import ${VMCWORKDIR}/input/${fname}.root & initialize;
  PndSolenoidMap *fField = new PndSolenoidMap(fname, "R");
  fField->Init();

  // Convert and dump ASCII version to ./${fname}.dat;
  fField->WriteAsciiFile(fname + ".dat");
  
  exit(0);
} // f2ascii()
  
