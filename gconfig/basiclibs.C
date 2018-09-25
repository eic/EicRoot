
void basiclibs()
{
  gSystem->Load("libRIO");
  //gSystem->Load("libCore");
  gSystem->Load("libGeom");
  gSystem->Load("libGeomPainter");
  gSystem->Load("libVMC");
  gSystem->Load("libEG");
  //@@@gSystem->Load("libEGPythia6");
  //@@@gSystem->Load("libPythia6");  
  gSystem->Load("libPhysics"); 
  gSystem->Load("libNet");  
  gSystem->Load("libTree");
  gSystem->Load("libMinuit");
  gSystem->Load("libMathMore");

  gSystem->Load("libProof");
  gSystem->Load("libProofPlayer");

  // 2016/07/14 -> FIXME: figure out why this stuff is not compiled under Mac OS;
#ifndef __APPLE__
  gSystem->Load("libGX11TTF");
  gSystem->Load("libGX11");
#endif
}  
