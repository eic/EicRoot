//
// AYK (ayk@bnl.gov), 2014/01/08; revamped in Oct'2017;
//
//  EicRoot CAD manipulation routines; STL facet class;
//

#include <assert.h>

#include <EicStlFacet.h>

// =======================================================================================

EicStlFacet::EicStlFacet(EicStlVertex * const v1, EicStlVertex * const v2, EicStlVertex * const v3):
  mKey(NULL), mVertices(NULL), mCounter(0)
{
  mVbuffer.push_back(v1); mVbuffer.push_back(v2); mVbuffer.push_back(v3);
} // EicStlFacet::EicStlFacet()

// ---------------------------------------------------------------------------------------

int EicStlFacet::Calculate( void )
{ 
  // Move vertices to the ordered map;
  mVertices = new vEntry(EicStlKeyCompare);

  for(unsigned iq=0; iq<3; iq++) {
    EicStlVertex *vtx = mVbuffer[iq];

    // Potentially switch to the new (merged) vertices;
    if (vtx->GetMergedVertex()) vtx = vtx->GetMergedVertex();

    (*mVertices)[vtx->key()] = vtx;
  } //for iq

  // Do not need intermediate array any longer;
  //mVbuffer.clear();

  // Check, that there are no identical vertices;
  if (mVertices->size() != 3) return -1;

  // And eventually compose the 3*3-double key;
  mKey = GetVertexArrayKey(mVertices);

  // Now that all 3 vertices are "fixed", calculate normal vector; NB: it does not bear 
  // any orientation meaning here, just an "unsigned" direction;
  {
    TVector3 vv[3];

    unsigned counter = 0;
    for(vEntry::iterator it=mVertices->begin(); it!=mVertices->end() ; it++) 
      vv[counter++] = TVector3(it->second->key()->GetData());

    TVector3 dv21 = vv[1] - vv[0], dv32 = vv[2] - vv[1];
    mNormal = dv21.Cross(dv32).Unit();
  }
  
  return 0;
} // EicStlFacet::Calculate()

// =======================================================================================
