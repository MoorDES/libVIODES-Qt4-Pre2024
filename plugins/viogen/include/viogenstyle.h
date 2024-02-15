/* viogenstyle.h  - viogen configuration */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006, 2007  Thomas Moor, Klaus Schmidt, Sebastian Perk

*/



#ifndef FAUDES_VIOGENSTYLE_H
#define FAUDES_VIOGENSTYLE_H

#include "libviodes.h"

// win32 dll symbol export/import
#ifndef VIOGEN_BUILD_LIB
#define VIOGEN_API Q_DECL_IMPORT
#else
#define VIOGEN_API Q_DECL_EXPORT
#endif

// debugging: types and plugins
#ifdef FAUDES_DEBUG_VIO_GENERATOR
#define FD_DQG(message) FAUDES_WRITE_CONSOLE("FAUDES_VIO_GENERATOR: " << message)
#else
#define FD_DQG(message) { };
#endif


 /*
   How graph element rendering is implemented ...

   Rendering mechansism, bottom up
   1) drawing primitves as functions eg StatePath(...)
   2) primitives use an argument RenderOptions for control of colors, shapes etc
   3) the argument RenderOptions is retrieved from the faudes attribute by ElementOptions(...)
   4) ElementOptions interprets AttributeFlags by FlagEffects
   5) FlagEffects are read from th config file

   To interfere with the mechanism you can
   A) reimplement drawing primitives in derived style class
   B) reimplement ElementOptions(..) to interpret other aspects that AttributeFlags only
   C) edit the config file to activate other flag effects.
  */
   

// render options parameter for one graph element
class VIOGEN_API GioRenderOptions {
public:
  // construct
  GioRenderOptions(void); 
  virtual ~GioRenderOptions(void) {};

  // clear to default
  virtual void Clear(void);

  // compile parameters
  virtual void Compile(void);

  // defining parameter
  int mLineStyle;
  const QColor* mLineColor;
  int mBodyStyle;    
  const QColor* mBodyColor;
  int mTextStyle;

  // compiled parameter
  static QPen   mNoPen;
  static QBrush mNoBrush;
  QPen   mLinePen;
  QBrush mLineBrush;
  QBrush mBodyBrush;  
  QFont  mFont;  
};


// symbolic graph element render options
// coding rational: each hex digit represents one exclusive sub-option
typedef enum { 
  GioNoLine=0,  
  GioSolid=0x0001, GioDashed=0x0002, GioDotted=0x0003,
} GioLineStyle;

typedef enum { 
  GioNoBody=0,   
  GioShapeMask=0x000f,
  GioInvisible=0x0001, GioCircle=0x0002, GioRectangle=0x0003, GioHexagon=0x0004, 
  GioStrokeMask=0x00f0,
  GioStroke=0x0010, GioDoubleStroke=0x0020, GioDiamond=0x0030, GioDoubleDiamond=0x0040, 
  GioSizeMask=0x0f00,
  GioSmallSize=0x0100, GioNormSize=0x0200, GioAutoSize=0x0400,
  GioOptionMask=0xf000,
  GioInit=0x1000, GioDouble=0x2000,
} GioBodyStyle;  
typedef enum { 
  GioNoText=0, 
  GioDefault=0x0001, GioBold=0x0002, GioItalic=0x0003, GioTypewriter=0x0004,
} GioTextStyle;
  

/*
 ************************************************
 ************************************************

 GioFlagEffect associates a graph element render option 
 with a faudes flag. The flag is specified by a mask 
 and a value. The corresponding effect is a symbolic 
 linestyle, a symbolic bodystyle and a symbolic textstyle. 
 Not all effects are implemented by for all items e.g. 
 GioStroke is only availabe for GioTrans items. 

 GioFlagEffect can be read from TokenReader for
 the purpose of configuration file support. 

 ************************************************
 ************************************************
 */

class VIOGEN_API GioFlagEffect {
public:
  // construct 
  GioFlagEffect(void);
  GioFlagEffect(
    const QString& name, faudes::fType mask, faudes::fType value, 
    GioLineStyle lstyle, int lcolor, 
    GioBodyStyle bstyle, int bcolor, GioTextStyle tstyle);

  // read from tokenreader (true on success)
  bool Read(faudes::TokenReader& rTr);

  // apply this effect if applicable
  void Apply(GioRenderOptions* pRenderOptions, faudes::fType flags) const;
  void Apply(GioRenderOptions* pRenderOptions, const faudes::AttributeVoid& rAttr) const;

  // name of effect
  QString mName;

  // mask that activates effect
  faudes::fType mMask;
  faudes::fType mValue;

  // visual effect
  GioLineStyle  mLineStyle; 
  int           mLineColor;
  GioBodyStyle  mBodyStyle; 
  int           mBodyColor;
  GioTextStyle  mTextStyle;
};


// draw element: actual drawing paths and appearance options
class VIOGEN_API GioDrawElement {
  public:
    QPainterPath   mPath;
    const QPen*   pPen;
    const QBrush* pBrush;
};


/*
 ************************************************
 ************************************************

 VioGeneratorStyle provides configuration for the
 viogen plugin of libviodes. The class is derived from
 VioStyle and adds parameters and shared  helper 
 functions for generator representations.

 On construction, the actual data will be read from 
 the same file as the global configuration VioStyle 
 or set to default values. However, there may exist 
 multiple VioGeneratorStyle instances that differ 
 programmatically. 

 ************************************************
 ************************************************
 */

// forward
class VioGeneratorModel;


class VIOGEN_API VioGeneratorStyle : public VioStyle {

public:

  // construct/destruct
  VioGeneratorStyle(const QString& ftype="Generator");
  ~VioGeneratorStyle(void) {};

  // attribute prototypes
  VioAttributeModel* mGlobalAttribute;
  VioAttributeModel* mTransAttribute;
  VioAttributeModel* mStateAttribute;
  VioAttributeModel* mEventAttribute;


  // generator view layout options
  typedef enum { 
    LayoutMask=0x00ff,   // exclusive base layouts
    Generator=1,         // std generator layout
    Alphabet=2,          // std alphabet layout (not implemented)
    OptionMask=0xff00,   // options   
    Properties=0x0100    // have my own property view
  } Layout;

  // actual layout
  int mLayoutFlags;

 
  // insert flag effects
  void InsTransEffect(const GioFlagEffect& effect);
  void InsStateEffect(const GioFlagEffect& effect);
  void InsEventEffect(const GioFlagEffect& effect);
  const QList<GioFlagEffect>& TransEffects(void);
  const QList<GioFlagEffect>& StateEffects(void);
  const QList<GioFlagEffect>& EventEffects(void);

  // have render options object factory
  virtual GioRenderOptions* NewRenderOptions() const { 
    return new GioRenderOptions(); };

  // map faudes elements to renderoptions (here: from flageffects)
  virtual void MapElementOptions(const VioGeneratorModel* pGenModel, const VioElement& rElem, GioRenderOptions* rOptions); 
  
  // numerical drawing parameters
  const qreal& StateNormalSize(void) { return mStateNormalSize;};
  const qreal& StateSmallSize(void) { return mStateSmallSize;};
  const qreal& StateMarkGap(void) { return mStateMarkGap;};
  const qreal& ImportMeshWidthX(void) { return mImportMeshWidthX;};
  const qreal& ImportMeshWidthY(void) { return mImportMeshWidthY;};

  // draw states 
  virtual bool StatePointL(QPointF& pointL, const QPointF& pointC, const GioRenderOptions* pOptions);
  virtual void StateCirclePath(QPainterPath& state, const QPointF& pointC, const QPointF& pointL);
  virtual void StateRectanglePath(QPainterPath& state, const QPointF& pointC, const QPointF& pointL);
  virtual void StateHexagonPath(QPainterPath& state, const QPointF& pointC, const QPointF& pointL);
  virtual void StateSinglePath(QPainterPath& state, const QPointF& pointC, const QPointF& pointL, const GioRenderOptions* pOptions);
  virtual void AddStatePaths(QList<GioDrawElement>& delements, const QPointF& pointC, const QPointF& pointL, const GioRenderOptions* pOptions);
  virtual void AddInitPath(QList<GioDrawElement>& delements, const QPointF& pointA, const QPointF& pointB, const GioRenderOptions* pOptions);
  virtual void AddStateName(QList<GioDrawElement>& delements, const QPointF& pointC, const QPointF& pointL, const QString& text, const GioRenderOptions* pOptions);

  // draw transitions.
  virtual void TransLinePen(QPen& pen,const GioRenderOptions* pOptions);
  virtual void TransLineBrush(QBrush& brush,const GioRenderOptions* pOptions);
  virtual void TransTickBrush(QBrush& brush, const GioRenderOptions* pOptions);
  virtual void TransArcPath(QPainterPath& arc, const QList<QPointF>& points, const GioRenderOptions* pOptions); 
  virtual void TransArrPath(QPainterPath& arr, const QList<QPointF>& points, const GioRenderOptions* pOptions); 
  virtual void TransTickPos(QPointF& pointC, QPointF& pointD, 
    const QPointF& pointA, const QPointF& pointCA,const QPointF& pointCB, const QPointF& pointB); 
  virtual void TransTickPath(QPainterPath& tick, 
    const QPointF& pointA, const QPointF& pointCA,const QPointF& pointCB, const QPointF& pointB, 
    const GioRenderOptions* pOptions);
  virtual void TransTickStrokePath(QPainterPath& tick, 
    const QPointF& pointA, const QPointF& pointCA,const QPointF& pointCB, const QPointF& pointB); 
  virtual void TransTickDoubleStrokePath(QPainterPath& tick, 
    const QPointF& pointA, const QPointF& pointCA,const QPointF& pointCB, const QPointF& pointB); 
  virtual void TransTickDiamondPath(QPainterPath& tick, 
    const QPointF& pointA, const QPointF& pointCA,const QPointF& pointCB, const QPointF& pointB); 
  virtual void TransTickDoubleDiamondPath(QPainterPath& tick, 
    const QPointF& pointA, const QPointF& pointCA,const QPointF& pointCB, const QPointF& pointB); 




protected:

  // load hard-coded default
  virtual void Initialise(const QString& ftype);

  // configuration from file
  virtual void ReadFile(const QString& filename="");

  // parameters
  qreal mStateNormalSize;
  qreal mStateSmallSize;
  qreal mStateMarkGap;
  qreal mImportMeshWidthX;
  qreal mImportMeshWidthY;

  // effects
  QList<GioFlagEffect> mTransEffects;
  QList<GioFlagEffect> mStateEffects;
  QList<GioFlagEffect> mEventEffects;

};


/*
 ************************************************
 ************************************************

 Register all faudes::Generator derivates
 found in a config file

 ************************************************
 ************************************************
 */


void VIOGEN_API VioRegisterGeneratorStyles(const QString& filename="");

#endif
