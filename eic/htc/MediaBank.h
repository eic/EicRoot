//
// AYK (ayk@bnl.gov), shaped up in Nov'2015;
//
//  Media distribution along Kalman filter node arrangement line;
//

#include <vector>

#ifndef _MEDIA_BANK_
#define _MEDIA_BANK_

#include <ayk.h>
#include <3d.h>

#include <MediaLayer.h>

class MediaBank {
 public:
  // Shape up old request_media_scan() function up as a constructor;
 MediaBank(/*const t_3d_line &scanLine,*/ const t_3d_line &axisLine, double maxLength): 
  mScanLine(axisLine), mAxisLine(axisLine), mMaxLength(maxLength), mOutOfRangeFlag(false),
    mSlopeCff(1.0) {};

  void SetScanLine(const t_3d_line &scanLine) {
    mScanLine = scanLine;

    mSlopeCff = scanLine.nx.Dot(mAxisLine.nx);
    // This would not make any sense;
    if (mSlopeCff <= 0.0) throw;
  };

  // Print out media bank contents;
  void Print();

  int StartNextLayer(TGeoMaterial *material, TVector3 pt);
  void SetCurrentLayerThickness(double thickness) {
    MediaLayer *layer = GetCurrentMediaLayer();
    if (!layer) return;

    // Correct thickness for relative slope of scan axis;
    layer->SetThickness(thickness * mSlopeCff);

    // Check out-of-range condition;
    if (GetMaxLength() && layer->GetZ0() + thickness >  GetMaxLength())
      mOutOfRangeFlag = true;
  };
  
  unsigned GetMediaLayerCount()                   const { return mMediaLayers.size(); };
  const MediaLayer *GetMediaLayer(unsigned iq) const { 
    return (iq < mMediaLayers.size() ? &mMediaLayers[iq] : 0); 
  };
  MediaLayer *GetCurrentMediaLayer()                 { 
    return (mMediaLayers.size() ? &mMediaLayers[GetMediaLayerCount()-1] : 0); 
  };
  double GetMaxLength()                           const { return mMaxLength; };

  bool IsOutOfRange()                             const { return mOutOfRangeFlag; };

  const t_3d_line &GetScanLine()                  const { return mScanLine; };

 private:
  // If non-zero, material will be recorded along "axis" line, but 
  // not further from axis.x[] than this value;
  double mMaxLength;

  bool mOutOfRangeFlag;

  // mAxisLine.nx[] is a vector along which material thickness should be 
  // estimated; in a certain sense it is along node ordering direction; 
  // mAxisLine.x[] determines start of coordinate system; is the line
  // along which ROOT TGeoManager will perform the actual material scan;
  t_3d_line mScanLine, mAxisLine;
  double mSlopeCff;

  std::vector<MediaLayer> mMediaLayers;
};

#endif

