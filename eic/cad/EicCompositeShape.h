//
// AYK (ayk@bnl.gov), 2014/03/11; revamped in Oct'2017;
//
//  EicRoot CompositeShape ROOT shape; basically TGeoCompositeShape with 
//  graphics buffer handling replaced by input from STL representation of
//  the object during half-space cut manipulation routines; 
//

#include <stdlib.h>

#include <TGeoCompositeShape.h>

class TopoDS_Shape;

#ifndef _EIC_COMPOSITE_SHAPE_
#define _EIC_COMPOSITE_SHAPE_

// Well, do not want to depend on how TBuffer3D is arranged, so do not use TBuffer3D as 
// a base class; it looks like TBuffer3D constructor has Init() call which resets everything 
// to a virgin state; even if this observation is wrong, declare a separate class here 
// where mimic TBuffer3D functionality in the required part (data storage);
class EicBuffer3D: public TObject {
  friend class EicCompositeShape;

 public:
 EicBuffer3D(): fNbPnts(0), fNbSegs(0), fNbPols(0), fPnts(0), fSegs(0), fPols(0)  {}; 
  ~EicBuffer3D() {};
  
  UInt_t NbPnts() { return fNbPnts;};
  UInt_t NbSegs() { return fNbSegs;};
  UInt_t NbPols() { return fNbPols;};
  
 private:
  // Well, I could perhaps save just an OpenCascade-created STL binary buffer
  // here; this would require parsing (and perhaps healing) this buffer every time
  // when "eve" started; prefer to store parsed data arrays right away here;
  UInt_t fNbPnts, fNbSegs, fNbPols, fPntsCapacity, fSegsCapacity, fPolsCapacity;

  Double_t xMin, yMin, zMin, xMax, yMax, zMax; // limits defining bounding box

  // NB: format here is different compared to TBuffer3D variables (triangular
  // facets only; color is not stored);
  Double_t *fPnts;              //[fPntsCapacity] x0, y0, z0, x1, y1, z1, ..... ..... ....
  Int_t    *fSegs;              //[fSegsCapacity] p0, q0, p1, q1, ..... ..... ....  
  Int_t    *fPols;              //[fPolsCapacity] s0, t0, u0, s1, t1, u1, ..... ..... ....

  ClassDef(EicBuffer3D,9)
};

class EicCompositeShape: public TGeoCompositeShape {
 public:
 EicCompositeShape(): TGeoCompositeShape(), mLocalBuffer3D(0), mSolid(0) {}; 	
 EicCompositeShape(const char* name, const char* expression, const TopoDS_Shape *solid): 
  TGeoCompositeShape(name, expression) { 
    mLocalBuffer3D = new EicBuffer3D();
    mSolid = solid;
  };
  ~EicCompositeShape() {};

  // Leave only those methods, which I see called; in fact may remove them as 
  // weel, since they simplay call the base class methods;
  Double_t Capacity()         const { return TGeoCompositeShape::Capacity(); };
  void ClearThreadData()      const { TGeoCompositeShape::ClearThreadData(); };
  TGeoBoolNode *GetBoolNode() const { return TGeoCompositeShape::GetBoolNode(); };
  void GetMeshNumbers(Int_t& nvert, Int_t& nsegs, Int_t& npols) const {
    TGeoCompositeShape::GetMeshNumbers(nvert, nsegs, npols);
  };
  Int_t GetNmeshVertices()    const { return TGeoCompositeShape::GetNmeshVertices(); };
  Bool_t GetPointsOnSegments(Int_t iq, Double_t* dq) const { 
    return TGeoCompositeShape::GetPointsOnSegments(iq, dq);
  };
  Bool_t IsComposite()        const { return TGeoCompositeShape::IsComposite(); };
  void SetPoints(Double_t* points) const { TGeoCompositeShape::SetPoints(points); };

  // Well, so these are THE ESSENTIAL CALLS, which replace the original 
  // TGeoCompositeShape functionality;
  Bool_t PaintComposite(Option_t* option = "") const;
  void FillBuffer3D(TBuffer3D & buffer, Int_t reqSections, Bool_t localFrame) const;
  const TBuffer3D &GetBuffer3D(Int_t reqSections, Bool_t localFrame) const;

  // This guy is called from EicCadFile::DumpAsRootSolid() right after creating the 
  // EicCompositeShape instance;
  int LocalFillBuffer3D(double stl_quality_coefficient);

  // THINK: well, should not I in fact replace TGeoCompositeShape SetPoints() and 
  // SetSegsAndPols() calls?;
  void LocalSetPoints(Double_t *points) const;
  void LocalSetSegsAndPols(TBuffer3D &buff) const;
  
 private:
  EicBuffer3D *mLocalBuffer3D;
  const TopoDS_Shape *mSolid; //!
  
  ClassDef(EicCompositeShape,8) 
};

#endif

