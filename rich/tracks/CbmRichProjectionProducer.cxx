/**
* \file CbmRichProjectionProducer.cxx
*
* \author P.Stolpovsky
* \date 2005
**/

#include "CbmRichProjectionProducer.h"
#include "CbmRichHitProducer.h"

#include "FairRootManager.h"
#include "CbmMCTrack.h"
#include "FairTrackParam.h"
#include "FairRuntimeDb.h"
#include "FairRun.h"
#include "FairGeoNode.h"
#include "CbmGeoRichPar.h"
#include "FairGeoTransform.h"
#include "FairGeoVector.h"
#include "FairRunAna.h"

#include "TVector3.h"
#include "TClonesArray.h"
#include "TMatrixFSym.h"

#include <iostream>

using std::cout;
using std::endl;


CbmRichProjectionProducer::CbmRichProjectionProducer(
      Int_t zflag):
   CbmRichProjectionProducerBase(zflag),
   fListRICHImPlanePoint(NULL),

   fNHits(0),
   fEvent(0),

   fDetX(0.0),
   fDetY(0.0),
   fDetZ(0.0),
   fDetWidthX(0.0),
   fDetWidthY(0.0),
   fThetaDet(0.0),
   fPhiDet(0.0),

   fDetXTransf(0.0),
   fDetYTransf(0.0),
   fDetZTransf(0.0),

   fZm(0.0),
   fYm(0.0),
   fXm(0.0),
   fR(0.0),

   fMaxXTrackExtr(0.0),
   fMaxYTrackExtr(0.0),

   fSensNodes(NULL),
   fPassNodes(NULL),
   fPar(NULL)
{
}

CbmRichProjectionProducer::~CbmRichProjectionProducer()
{
   FairRootManager *fManager =FairRootManager::Instance();
   fManager->Write();
}

void CbmRichProjectionProducer::SetParContainers()
{
   FairRunAna* sim = FairRunAna::Instance();
   FairRuntimeDb* rtdb=sim->GetRuntimeDb();
   fPar = (CbmGeoRichPar*)(rtdb->getContainer("CbmGeoRichPar"));
}

void CbmRichProjectionProducer::Init()
{
   FairRootManager* fManager = FairRootManager::Instance();

   fSensNodes = fPar->GetGeoSensitiveNodes();
   fPassNodes = fPar->GetGeoPassiveNodes();

   // get detector position:
   FairGeoNode *det= (FairGeoNode *) fSensNodes->FindObject("rich1d#1");
   FairGeoTransform* detTr=det->getLabTransform();  // detector position in labsystem
   FairGeoVector detPosLab=detTr->getTranslation(); // ... in cm
   FairGeoTransform detCen=det->getCenterPosition();  // center in Detector system
   FairGeoVector detPosCen=detCen.getTranslation();
   fDetZ = detPosLab.Z() + detPosCen.Z(); // z coordinate of photodetector (Labsystem, cm)
   fDetY = detPosLab.Y() + detPosCen.Y(); // y coordinate of photodetector (Labsystem, cm)
   fDetX = detPosLab.X() + detPosCen.X(); // x coordinate of photodetector (Labsystem, cm)

   TArrayD *fdetA=det->getParameters();
   fDetWidthX = fdetA->At(0);
   fDetWidthY = fdetA->At(1);
//  for(Int_t i=0;i<fdetA->GetSize();i++) cout << "Array detector " << fdetA->At(i)<< endl;
   // detector might be rotated by theta around x-axis:
   FairGeoRotation fdetR=detTr->getRotMatrix();

   // possible tilting around x-axis (theta) and y-axis (phi)
   // fdetR(0) = cos(phi)
   // fdetR(1) = 0
   // fdetR(2) = -sin(phi)
   // fdetR(3) = -sin(theta)sin(phi)
   // fdetR(4) = cos(theta) 
   // fdetR(5) = -sin(theta)cos(phi)
   // fdetR(6) = cos(theta)sin(phi)
   // fdetR(7) = sin(theta)
   // fdetR(8) = cos(theta)cos(phi)
   
   // theta = tilting angle around x-axis
   fThetaDet = TMath::ASin(fdetR(7));
   // phi = tilting angle around y-axis
   fPhiDet = -1.*TMath::ASin(fdetR(2));

   cout << "---------------------- RICH Projection Producer ---------------------------------------" << endl;
   cout << "   detector position in (x,y,z): " << fDetX << "  " << fDetY << "  " << fDetZ << endl;
   cout << "   detector size in x and y: " << fDetWidthX << "  " << fDetWidthY << endl;
   cout << "   detector tilting angle (around x): " << fThetaDet*180./TMath::Pi() << " degrees" << endl;
   cout << "   detector tilting angle (around y): " << fPhiDet*180./TMath::Pi() << " degrees" << endl;
  
   // transform nominal detector position (for tilted photodetector):
   // shift x back by fDetZ_org*TMath::Sin(phi) in order to avoid overlap
   fDetXTransf = fDetX*TMath::Cos(fPhiDet)+fDetZ*TMath::Sin(fPhiDet)-fDetZ*TMath::Sin(fPhiDet);
   fDetYTransf = -fDetX*TMath::Sin(fThetaDet)*TMath::Sin(fPhiDet) + fDetY*TMath::Cos(fThetaDet) + fDetZ*TMath::Sin(fThetaDet)*TMath::Cos(fPhiDet);
   fDetZTransf = -fDetX*TMath::Cos(fThetaDet)*TMath::Sin(fPhiDet) - fDetY*TMath::Sin(fThetaDet) + fDetZ*TMath::Cos(fThetaDet)*TMath::Cos(fPhiDet);
  
   // get mirror position:
   //FairGeoNode *mir= (FairGeoNode *) fPassNodes->FindObject("rich1mgl#1");
   FairGeoNode *mir= (FairGeoNode *) fSensNodes->FindObject("rich1mgl#1");
   FairGeoTransform* mirTr=mir->getLabTransform();  // position of mirror center in labsystem
   FairGeoVector mirPosLab=mirTr->getTranslation(); // ... in cm
   fZm = mirPosLab.Z();
   fYm = mirPosLab.Y();
   fXm = mirPosLab.X();

   TArrayD *fmirA=mir->getParameters();  // get other geometry parameters: radius,
   fR = fmirA->At(0);                    // mirror radius
   Double_t spheTheta = TMath::Abs(90. - fmirA->At(2));   // opening angle for SPHERE in theta (90 degree +- theta)
   Double_t sphePhi = TMath::Abs(90. - fmirA->At(4));   // opening angle for SPHERE in phi (90 degree +- phi)
   // from that calculate (with safety factor 1.3) maximum x-y positions for track extrapolation:
   fMaxXTrackExtr = 1.3*(fR*TMath::Tan(sphePhi*TMath::Pi()/180.));
   fMaxYTrackExtr = 1.3*(TMath::Abs(fYm) + fR*TMath::Tan(spheTheta*TMath::Pi()/180.));
  
   // mirror might be rotated by theta around x-axis:
   FairGeoRotation fmirR=mirTr->getRotMatrix();
   Double_t thetaM = -1.*TMath::ASin(fmirR(5)) - TMath::Pi()/2 ;

   // note that mirror is by default tilted by 90 degrees in order to get the necessary shape in GEANT
   // the "extra" tilting angle is then: fThetaM =  -1.*TMath::ASin(fmirR(5)) - TMath::Pi()/2.
   cout << "Mirror center (x,y,z): " << fXm << " " << fYm << " " << fZm << endl;
   cout << "Mirror radius: " << fR << endl;
   cout << "Mirror tilting angle: " << thetaM*180./TMath::Pi() << " degrees" << endl;

   fEvent = 0;

   fListRICHImPlanePoint = (TClonesArray*)fManager->GetObject("RichTrackParamZ");
   if (fZflag == 1) cout << "   use tracks in imaginary plane for projection to photodetector plane" << endl;
   if (fZflag == 2) cout << "   use tracks in RICH mirror for projection to photodetector plane" << endl;
   if ( NULL == fListRICHImPlanePoint) {
      cout << "-W- CbmRichProjectionProducer::Init: No Rich Z-Point array!" << endl;
	}
}

void CbmRichProjectionProducer::DoProjection(
      TClonesArray* richProj)
{
   fEvent++;
   cout << "CbmRichProjectionProducer: event " << fEvent << endl;

   richProj->Clear();
   TMatrixFSym covMat(5);
   for(Int_t i = 0; i < 5; i++){
      for(Int_t j=0; j<=i; j++){
         covMat(i,j) = 0;
      }
   }
   covMat(0,0) = covMat(1,1) = covMat(2,2) = covMat(3,3) = covMat(4,4) = 1.e-4; 

   for(Int_t j = 0; j < fListRICHImPlanePoint->GetEntriesFast(); j++) {
      FairTrackParam* point = (FairTrackParam*)fListRICHImPlanePoint->At(j);
      new((*richProj)[j]) FairTrackParam(0., 0., 0., 0., 0., 0., covMat);
    
      // check if Array was filled
      if (point->GetX() == 0 && point->GetY() == 0 && point->GetZ() == 0 &&
            point->GetTx() == 0 && point->GetTy() ==0) continue;
      if (point->GetQp()==0) continue;
    
      // check that x and y value make sense (sometimes strange extrapolations may appear)
      //if (TMath::Abs(point->GetX()) > fMaxXTrackExtr || TMath::Abs(point->GetY()) > fMaxYTrackExtr){
      //   cout << " -W- RichProjectionProducer: strange (x,y) values for track extrapolation: " <<
      //         point->GetX() << " " << point->GetY() << endl;
		//   continue;
      //}

      Double_t rho1 = 0.;
      Double_t rho2 = 0.;
      TVector3 startP, momP, crossP, centerP;

      // operate on ImPlane point
      if (fZflag ==1) {
         Double_t p = 1./TMath::Abs(point->GetQp());
         Double_t pz;
         if  ((1+point->GetTx()*point->GetTx()+point->GetTy()*point->GetTy()) > 0. )
            pz = p/TMath::Sqrt(1+point->GetTx()*point->GetTx()+point->GetTy()*point->GetTy());
         else {
            cout << " -E- RichProjectionProducer: strange value for calculating pz: " <<
                  (1+point->GetTx()*point->GetTx()+point->GetTy()*point->GetTy()) << endl;
            pz = 0.;
         }
         Double_t px = pz*point->GetTx();
         Double_t py = pz*point->GetTy();
         momP.SetXYZ(px,py,pz);
         point->Position(startP);
         if ((fYm*startP.y())<0) fYm = -fYm; // check that mirror center and startP are in same hemisphere

         // calculation of intersection of track with selected mirror
         // corresponds to calculation of intersection between a straight line and a sphere:
         // vector: r = startP - mirrorCenter
         // RxP = r*momP
         // normP2 = momP^2
         // dist = r^2 - fR^2
         // -> rho1 = (-RxP+sqrt(RxP^2-normP2*dist))/normP2  extrapolation factor for:
         // intersection point crossP = startP + rho1 * momP
         Double_t RxP=(momP.x()*(startP.x()-fXm)+momP.y()*(startP.y()-fYm)+momP.z()*(startP.z()-fZm));
         Double_t normP2=(momP.x()*momP.x()+momP.y()*momP.y()+momP.z()*momP.z());
         Double_t dist=(startP.x()*startP.x()+fXm*fXm+startP.y()*startP.y()+fYm*fYm+startP.z()*startP.z()+fZm*fZm-2*startP.x()*fXm-2*startP.y()*fYm-2*startP.z()*fZm-fR*fR);

         if ((RxP*RxP-normP2*dist) > 0.) {
            if (normP2!=0.)  rho1=(-RxP+TMath::Sqrt(RxP*RxP-normP2*dist))/normP2;
            if (normP2 == 0) cout << " Error in track extrapolation: momentum = 0 " << endl;
         } else {
            cout << " -E- RichProjectionProducer:  RxP*RxP-normP2*dist = " << RxP*RxP-normP2*dist << endl;
         }

         Double_t crossPx = startP.x() + rho1*momP.x();
         Double_t crossPy = startP.y() + rho1*momP.y();
         Double_t crossPz = startP.z() + rho1*momP.z();
         crossP.SetXYZ(crossPx, crossPy, crossPz);

         // check if crosspoint with mirror and chosen mirrorcenter (y) are in same hemisphere
         // if not recalculate crossing point
         if ((fYm*crossP.y())<0) {
            fYm = -fYm;
            RxP=(momP.x()*(startP.x()-fXm)+momP.y()*(startP.y()-fYm)+momP.z()*(startP.z()-fZm));
            normP2=(momP.x()*momP.x()+momP.y()*momP.y()+momP.z()*momP.z());
            dist=(startP.x()*startP.x()+fXm*fXm+startP.y()*startP.y()+fYm*fYm+startP.z()*startP.z()+fZm*fZm-2*startP.x()*fXm-2*startP.y()*fYm-2*startP.z()*fZm-fR*fR);

            if ((RxP*RxP-normP2*dist) > 0.) {
               if (normP2!=0.)  rho1=(-RxP+TMath::Sqrt(RxP*RxP-normP2*dist))/normP2;
               if (normP2 == 0) cout << " Error in track extrapolation: momentum = 0 " << endl;
            } else{
               cout << " -E- RichProjectionProducer:  RxP*RxP-normP2*dist = " << RxP*RxP-normP2*dist << endl;
            }

            crossPx=startP.x()+rho1*momP.x();
            crossPy=startP.y()+rho1*momP.y();
            crossPz=startP.z()+rho1*momP.z();
            crossP.SetXYZ(crossPx,crossPy,crossPz);
         }

         centerP.SetXYZ(fXm,fYm,fZm);    // mirror center
      }// if (fZflag ==1)

      // operate on Rich Mirror point
      if (fZflag ==2) {
         Double_t p = 1./TMath::Abs(point->GetQp());
         Double_t pz;
         if  ((1+point->GetTx()*point->GetTx()+point->GetTy()*point->GetTy()) > 0. ){
            pz = p/TMath::Sqrt(1+point->GetTx()*point->GetTx()+point->GetTy()*point->GetTy());
         } else {
            cout << " -E- RichProjectionProducer: strange value for calculating pz: " <<
                  (1+point->GetTx()*point->GetTx()+point->GetTy()*point->GetTy()) << endl;
            pz = 0.;
         }
         Double_t px = pz*point->GetTx();
         Double_t py = pz*point->GetTy();
         momP.SetXYZ(px,py,pz);
         point->Position(crossP);
         if ((fYm*crossP.y())<0) fYm = -fYm; // check that mirror center and crossP are in same hemisphere

         centerP.SetXYZ(fXm,fYm,fZm); // mirror center
      } // if (fZflag ==2)

      //   calculate normal on crosspoint with mirror
      TVector3 normP(crossP.x()-centerP.x(),crossP.y()-centerP.y(),crossP.z()-centerP.z());
      normP=normP.Unit();
      // check that normal has same z-direction as momentum
      if ((normP.z()*momP.z())<0.) normP = TVector3(-1.*normP.x(),-1.*normP.y(),-1.*normP.z());

      // reflect track
      Double_t np=normP.x()*momP.x()+normP.y()*momP.y()+normP.z()*momP.z();

      Double_t refX = 2*np*normP.x()-momP.x();
      Double_t refY = 2*np*normP.y()-momP.y();
      Double_t refZ = 2*np*normP.z()-momP.z();

      // crosspoint whith photodetector plane:
      // calculate intersection between straight line and (tilted) plane:
      // normal on plane tilted by theta around x-axis: (0,-sin(theta),cos(theta)) = n
      // normal on plane tilted by phi around y-axis: (-sin(phi),0,cos(phi)) = n
      // normal on plane tilted by theta around x-axis and phi around y-axis: (-sin(phi),-sin(theta)cos(phi),cos(theta)cos(phi)) = n
      // point on plane is (fDetX,fDetY,fDetZ) = p as photodetector is tiled around its center
      // equation of plane for r being point in plane: n(r-p) = 0
      // calculate intersection point of reflected track with plane: r=intersection point
      // intersection point = crossP + rho2 * refl_track
      // take care for all 4 cases:
      //        -> first calculate for case x>0, then check
      if (refZ!=0.) {
         if (centerP.y() > 0){
            rho2 = (-TMath::Sin(fPhiDet)*(fDetX-crossP.x())
                  -TMath::Sin(fThetaDet)*TMath::Cos(fPhiDet)*(fDetY-crossP.y())
                  + TMath::Cos(fThetaDet)*TMath::Cos(fPhiDet)*(fDetZ-crossP.z()))/
                  (-TMath::Sin(fPhiDet)*refX-TMath::Sin(fThetaDet)*TMath::Cos(fPhiDet)*refY + TMath::Cos(fThetaDet)*TMath::Cos(fPhiDet)*refZ);
         }
         if (centerP.y() < 0){
            rho2 = (-TMath::Sin(fPhiDet)*(fDetX-crossP.x())
                  -TMath::Sin(-fThetaDet)*TMath::Cos(fPhiDet)*(-fDetY-crossP.y())
                  + TMath::Cos(-fThetaDet)*TMath::Cos(fPhiDet)*(fDetZ-crossP.z()))/
                  (-TMath::Sin(fPhiDet)*refX-TMath::Sin(-fThetaDet)*TMath::Cos(fPhiDet)*refY + TMath::Cos(-fThetaDet)*TMath::Cos(fPhiDet)*refZ);
         }
	      
         //rho2 = -1*(crossP.z() - fDetZ)/refZ;    // only for theta = 0, phi=0
         Double_t xX = crossP.x() + refX * rho2;
         Double_t yY = crossP.y() + refY * rho2;
         Double_t zZ = crossP.z() + refZ * rho2;
      
         if (xX < 0) {
            if (centerP.y() > 0){
               rho2 = (-TMath::Sin(-fPhiDet)*(-fDetX-crossP.x())
                     -TMath::Sin(fThetaDet)*TMath::Cos(-fPhiDet)*(fDetY-crossP.y())
                     + TMath::Cos(fThetaDet)*TMath::Cos(-fPhiDet)*(fDetZ-crossP.z()))/
                     (-TMath::Sin(-fPhiDet)*refX-TMath::Sin(fThetaDet)*TMath::Cos(-fPhiDet)*refY + TMath::Cos(fThetaDet)*TMath::Cos(-fPhiDet)*refZ);
            }
            if (centerP.y() < 0){
               rho2 = (-TMath::Sin(-fPhiDet)*(-fDetX-crossP.x())
                     -TMath::Sin(-fThetaDet)*TMath::Cos(-fPhiDet)*(-fDetY-crossP.y())
                     + TMath::Cos(-fThetaDet)*TMath::Cos(-fPhiDet)*(fDetZ-crossP.z()))/
                     (-TMath::Sin(-fPhiDet)*refX-TMath::Sin(-fThetaDet)*TMath::Cos(-fPhiDet)*refY + TMath::Cos(-fThetaDet)*TMath::Cos(-fPhiDet)*refZ);
            }
       
            xX = crossP.x() + refX * rho2;
            yY = crossP.y() + refY * rho2;
            zZ = crossP.z() + refZ * rho2;
         }
      
         // Transform intersection point in same way as MCPoints were
         // transformed in HitProducer before stored as Hit:
         TVector3 inPos(xX, yY, zZ);
         TVector3 outPos;
         CbmRichHitProducer::TiltPoint(&inPos, &outPos, fPhiDet, fThetaDet, fDetZ);
         Double_t xDet = outPos.X();
         Double_t yDet = outPos.Y();
         Double_t zDet = outPos.Z();
	 //printf("%f %f %f\n", xDet, yDet, zDet);

         //check that crosspoint inside the plane
         if( xDet > (-fDetX-fDetWidthX) && xDet < (fDetX+fDetWidthX)){
            if(TMath::Abs(yDet) > (fDetYTransf-fDetWidthY) && TMath::Abs(yDet) < (fDetYTransf+fDetWidthY)){
	                 FairTrackParam richtrack(xDet,yDet,zDet,0.,0.,0.,covMat);
	                 * (FairTrackParam*)(richProj->At(j)) = richtrack;
            }
         }
      }// if (refZ!=0.)
   }// j
}
