/* viomtggenstyle.h  - viogen configuration for mtc plugin */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006, 2007  Thomas Moor, Klaus Schmidt, Sebastian Perk

*/



#ifndef FAUDES_VIOMTCGENSTYLE_H
#define FAUDES_VIOMTCGENSTYLE_H

#include "libviodes.h"
#include "viogenstyle.h"


// render options parameter for one graph element
class GioMtcRenderOptions : public GioRenderOptions {
public:
  // construct
  GioMtcRenderOptions(void); 
  virtual ~GioMtcRenderOptions(void) {}; 
  // clear to default
  void Clear(void);
  // compile parameters
  void Compile(void);

  // extra mtc compiled parameters
  QList<QPen> mExtraPens;
};




/*
 ************************************************
 ************************************************

 VioMtcGeneratorStyle is deribed from VioGeneratorStyle
 to add multi colored marking support. Currently,
 there are no extra configuration options.

 ************************************************
 ************************************************
 */


class VioMtcGeneratorStyle : public VioGeneratorStyle {

public:

  // construct/destruct
  VioMtcGeneratorStyle(const QString& ftype="Generator");
  ~VioMtcGeneratorStyle(void) {};

  // access to my parameters
  QSet<QString> ColorSet(void) { return mColorSet;};


  // have render options object factory
  virtual GioRenderOptions* NewRenderOptions() const { 
    return new GioMtcRenderOptions(); };

  // map faudes elements to renderoptions (here: base class flags and mtc colors)
  virtual void MapElementOptions(const VioGeneratorModel* pGenModel, const VioElement& rElem, GioRenderOptions* rOptions); 
  
  // draw states
  void AddStatePaths(QList<GioDrawElement>& delements, const QPointF& pointC, const QPointF& pointL, const GioRenderOptions* pOptions);


protected:

  // load hard-coded default
  void Initialise(const QString& ftype);

  // configuration from file
  void ReadFile(const QString& filename="");

  // parameters
  QSet<QString> mColorSet;

};



#endif
