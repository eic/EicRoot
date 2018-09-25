//
// AYK (ayk@bnl.gov)
//
//    Kalman filter node and associated structures; ported from 
//  HERMES/OLYMPUS sources; cleaned up 2014/10/13;
//

#include <cstdio>
#include <cassert>

#include <KalmanNode.h>

// ---------------------------------------------------------------------------------------

//
// C++: no sense to check on pointer != NULL, since exception will be thrown anyway, right?;
//

void KalmanNode::AllocateKfMatrices(unsigned sdim)
{ 

  // Magnet-off transport matrices are calculated once --> make them global;
  //printf("allocation %f ...\n", z);
  for(unsigned fb=0; fb<2; fb++) {
    FF[fb]  = new KfMatrix(sdim, sdim); 
    //if (!node->FF[fb]) return NULL;

    // Will be unity matrices per default;
    FF[fb]->Unity();
  } /*for*/

  // There are dummy nodes, which can not provide ea measurement -> no need to 
  // allocate these matrices then;
  if (mDim) {
    m    = new KfVector(mDim); 
  
    V    = new KfMatrix(mDim, mDim); 
    
    H    = new KfMatrix(mDim, sdim);  
    
    K    = new KfMatrix(sdim, mDim); 
    
    rf   = new KfVector(mDim); 
    ep   = new KfVector(mDim); 
    
    RPI  = new KfMatrix(mDim, mDim); 
    RF   = new KfMatrix(mDim, mDim);
    
    rs   = new KfVector(mDim); 
    RS   = new KfMatrix(mDim, mDim);   
    
    MMTX = new KfMatrix(mDim, mDim); 
    MVEC = new KfVector(mDim); 

    rm   = new KfVector(mDim);
    RM   = new KfMatrix(mDim, mDim);  
  } //if
    
  // Will be unity matrix per default;
  FM   = new KfMatrix(sdim, sdim);
  FM->Unity();

  Q    = new KfMatrix(sdim, sdim); 
  
  x0   = new KfVector(sdim);  

  xp   = new KfVector(sdim); 
      
  xf   = new KfVector(sdim);  

  CP   = new KfMatrix(sdim, sdim); 
  CF   = new KfMatrix(sdim, sdim);  

  xs   = new KfVector(sdim); 
  qq   = new KfVector(sdim); 

  CS   = new KfMatrix(sdim, sdim); 

  xm   = new KfVector(sdim); 
  CM   = new KfMatrix(sdim, sdim); 

  LB   = new KfMatrix(sdim, sdim);  
  L    = new KfMatrix(sdim, sdim); 
  QQ   = new KfMatrix(sdim, sdim); 
} // KalmanNode::AllocateKfMatrices()

// ---------------------------------------------------------------------------------------

void KalmanNode::SetFiredFlag()
{
  mFired = true;

  // And increment all respective group counters;
  for(int gr=0; gr<mNodeGroupNum; gr++) {
    NodeGroup *group = mNodeGroups[gr];

    group->mFiredNodeNum++;
    //printf("  SetFiredFlag(): now %2d total!\n", group->mFiredNodeNum);
  } //for gr
} // KalmanNode::SetFiredFlag()

// ---------------------------------------------------------------------------------------

void KalmanNode::ResetFiredFlag()
{
  mFired = false;

  // And decrement all respective group counters;
  for(int gr=0; gr<mNodeGroupNum; gr++) {
    NodeGroup *group = mNodeGroups[gr];

    group->mFiredNodeNum--;
    //printf("ResetFiredFlag(): now %2d total!\n", group->mFiredNodeNum);

    // Remove once debugging finished;
    assert(group->mFiredNodeNum >= 0);
  } //for gr
} // KalmanNode::ResetFiredFlag()

// ---------------------------------------------------------------------------------------

