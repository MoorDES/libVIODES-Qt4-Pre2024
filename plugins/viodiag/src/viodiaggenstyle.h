/* viodiaggenstyle.h  - viogen configuration for diagnosis plugin */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2010 Thomas Moor

*/



#ifndef FAUDES_VIODIAGGENSTYLE_H
#define FAUDES_VIODIAGGENSTYLE_H

#include "libviodes.h"
#include "viogenstyle.h"


// render options parameter for one graph element
class GioDiagRenderOptions : public GioRenderOptions {
public:
  // construct
  GioDiagRenderOptions(void); 
  virtual ~GioDiagRenderOptions(void) {}; 
  // clear to default
  void Clear(void);
  // compile parameters
  void Compile(void);

  // extra diag compiled parameters
  QString mStateLabel;
};




/*
 ************************************************
 ************************************************

 VioDiagGeneratorStyle is derived from VioGeneratorStyle
 to add display of the diagnoser state attribute

 ************************************************
 ************************************************
 */


class VioDiagGeneratorStyle : public VioGeneratorStyle {

public:

  // construct/destruct
  VioDiagGeneratorStyle(const QString& ftype="Diagnoser");
  ~VioDiagGeneratorStyle(void) {};

  // have render options object factory
  virtual GioRenderOptions* NewRenderOptions() const { 
    return new GioDiagRenderOptions(); };

  // map faudes elements to renderoptions (here: base class flags and diagnoser state)
  virtual void MapElementOptions(const VioGeneratorModel* pGenModel, const VioElement& rElem, GioRenderOptions* rOptions); 
  
  // draw state labels
  void AddStateName(QList<GioDrawElement>& delements, const QPointF& pointC, const QPointF& pointL, const QString& text, const GioRenderOptions* pOptions);
  bool StatePointL(QPointF& pointL, const QPointF& pointC, const GioRenderOptions* pOp);

protected:

  // load hard-coded default
  void Initialise(const QString& ftype);

  // configuration from file
  void ReadFile(const QString& filename="");

};



#endif
