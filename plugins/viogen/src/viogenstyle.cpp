/* viogenstyle.cpp  - viogen configuration */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006, 2007  Thomas Moor, Klaus Schmidt, Sebastian Perk

*/



#include "viogenstyle.h"
#include "viogenerator.h"

/*
 *************************************
 *************************************
 *************************************
 
 Implementation of RenderOptions

 *************************************
 *************************************
 *************************************
 */

// statics
QPen   GioRenderOptions::mNoPen=QPen(Qt::NoPen);
QBrush GioRenderOptions::mNoBrush;

// construct
GioRenderOptions::GioRenderOptions(void) : 
    mLineStyle(GioNoLine),
    mLineColor(&VioStyle::Color("NoColor")),
    mBodyStyle(GioNoBody),
    mBodyColor(&VioStyle::Color("NoColor")),
    mTextStyle(GioNoText) 
{
  Compile();
};

// clear
void GioRenderOptions::Clear(void) {
  mLineStyle= GioNoLine;
  mLineColor= &VioStyle::Color("Black");
  mBodyStyle= GioNoBody;
  mBodyColor= &VioStyle::Color("White");
  mTextStyle= GioNoText; 

  // debugging inject
  /*
  mLineStyle= GioDashed;
  mLineColor= &VioStyle::Color("Red");
  mBodyStyle= 0;
  mBodyColor= &VioStyle::Color("Blue");
  */

  Compile();
};

// compile
void GioRenderOptions::Compile(void) {
  mLinePen=   VioStyle::DefaultPen();
  if(mLineStyle==GioSolid)     mLinePen.setStyle(Qt::SolidLine); 
  if(mLineStyle==GioDashed)    mLinePen=VioStyle::DefaultDashed();
  if(mLineStyle==GioDotted)    mLinePen=VioStyle::DefaultDotted(); 
  if(mBodyStyle==GioInvisible) mLinePen=mNoPen;
  mLinePen.setColor(*mLineColor);
  mLineBrush= VioStyle::DefaultBrush();
  mLineBrush.setColor(*mLineColor);
  mBodyBrush= VioStyle::DefaultBrush();
  mBodyBrush.setColor(*mBodyColor);
}


/*
 *************************************
 *************************************
 *************************************
 
 Implementation of GioFlagEffects

 *************************************
 *************************************
 *************************************
 */

// construct default
GioFlagEffect::GioFlagEffect(void) :
  mName("NoGioFlagEffect"), mMask(0), mValue(0), 
  mLineStyle(GioNoLine), mLineColor(VioNoColor), 
  mBodyStyle(GioNoBody), mBodyColor(VioNoColor),
  mTextStyle(GioNoText) 
{};


// construct from data
GioFlagEffect::GioFlagEffect(
  const QString& name, faudes::fType mask, faudes::fType value, 
  GioLineStyle lstyle, int lcolor, 
  GioBodyStyle bstyle, int bcolor, GioTextStyle tstyle) :
  mName(name), mMask(mask), mValue(value), 
  mLineStyle(lstyle), mLineColor(lcolor), 
  mBodyStyle(bstyle), mBodyColor(bcolor),
  mTextStyle(tstyle) 
{}

// attribute style relaxed reading, 
bool GioFlagEffect::Read(faudes::TokenReader& rTr) {
  faudes::Token token;
  rTr.Peek(token);
  if(token.Type()!=faudes::Token::Begin) return false;
  if(token.StringValue()!="VisualFlagEffect") return false;
  // get core data
  rTr.ReadBegin("VisualFlagEffect");
  mName=VioStyle::QStrFromStr(rTr.ReadString());
  mMask=rTr.ReadInteger();
  mValue=rTr.ReadInteger();
  // clear styls
  mLineStyle=GioNoLine;
  mLineColor=VioNoColor;
  mBodyStyle=GioNoBody;
  mBodyColor=VioNoColor;
  mTextStyle=GioNoText;
  while(rTr.Peek(token)) {
    // done
    if(token.Type()==faudes::Token::End) 
    if(token.StringValue()=="VisualFlagEffect") break;
    // get linestyle
    if(token.Type()==faudes::Token::Begin) 
    if(token.StringValue()=="LineStyle") {
      rTr.ReadBegin("LineStyle");
      while(!rTr.Eos("LineStyle")){
        std::string option;
        option=rTr.ReadOption();
        // its a style option
        if(option=="Invisible")  { mBodyStyle=GioInvisible; break; }
        if(option=="Solid")      { mLineStyle=GioSolid; break; }
        if(option=="Dashed")     { mLineStyle=GioDashed; break; }
        if(option=="Dotted")     { mLineStyle=GioDotted; break; }
        // its a color option
        mLineColor=VioStyle::ColorIndex(VioStyle::QStrFromStr(option));
      }
      rTr.ReadEnd("LineStyle");
      continue;
    }
    // get bodystyle
    if(token.Type()==faudes::Token::Begin) 
    if(token.StringValue()=="BodyStyle") {
      rTr.ReadBegin("BodyStyle");
      while(!rTr.Eos("BodyStyle")){
        std::string option;
        option=rTr.ReadOption();
        // its a style option
        if(option=="Invisible")      { mBodyStyle=GioInvisible; break; }
        if(option=="Circle")         { mBodyStyle=GioCircle; break; }
        if(option=="Rectangle")      { mBodyStyle=GioRectangle; break; }
        if(option=="Hexagon")        { mBodyStyle=GioHexagon; break; }
        if(option=="Diamond")        { mBodyStyle=GioDiamond; break; }
        if(option=="SmallSize")      { mBodyStyle=GioSmallSize; break; }
        if(option=="NormSize")       { mBodyStyle=GioNormSize; break; }
        if(option=="AutoSize")       { mBodyStyle=GioAutoSize; break; }
        if(option=="DoubleDiamond")  { mBodyStyle=GioDoubleDiamond; break; }
        if(option=="Stroke")         { mBodyStyle=GioStroke; break; }
        if(option=="DoubleStroke")   { mBodyStyle=GioDoubleStroke; break; }
        if(option=="Double")         { mBodyStyle=GioDouble; break; }
        if(option=="Init")           { mBodyStyle=GioInit; break; }
        // its a color option
        mBodyColor=VioStyle::ColorIndex(VioStyle::QStrFromStr(option));
      }
      rTr.ReadEnd("BodyStyle");
      continue;
    }
    // get textstyle
    if(token.Type()==faudes::Token::Begin) 
    if(token.StringValue()=="TextStyle") {
      rTr.ReadBegin("TextStyle");
      while(!rTr.Eos("TextStyle")){
        std::string option;
        option=rTr.ReadOption();
        // its a style option
        if(option=="Bold")        { mTextStyle=GioBold; break; }
        if(option=="Italic")      { mTextStyle=GioItalic; break; }
        if(option=="Typewriter")  { mTextStyle=GioTypewriter; break; }
      }
      rTr.ReadEnd("LineStyle");
      continue;
    }
    // ignore other tokens
    rTr.Get(token);
  }
  rTr.ReadEnd("VisualFlagEffect");
  FD_DQG("GioFlagEffect::Read(rTr): " << VioStyle::StrFromQStr(mName));
  return true;
}

// apply effect
void GioFlagEffect::Apply(GioRenderOptions* pRenderOptions, faudes::fType flags) const {
  // bail out
  if((flags & mMask) != mValue) return;
  FD_DQG("GioFlagEffect::Apply(): " << VioStyle::StrFromQStr(mName) << " (width bodystyle " << mBodyStyle<< ")");
  // doit: line style (only one sub-option)
  if((mLineStyle & 0x000f) != 0) 
    { pRenderOptions->mLineStyle &= 0xfff0; pRenderOptions->mLineStyle |= (mLineStyle & 0x000f); }
  // doit: body style, A
  if((mBodyStyle & 0x000f) != 0) 
    { pRenderOptions->mBodyStyle &= 0xfff0; pRenderOptions->mBodyStyle |= (mBodyStyle & 0x000f); }
  // doit: body style, B
  if((mBodyStyle & 0x00f0) != 0) 
    { pRenderOptions->mBodyStyle &= 0xff0f; pRenderOptions->mBodyStyle |= (mBodyStyle & 0x00f0); }
  // doit: body style, C
  if((mBodyStyle & 0x0f00) != 0) 
    { pRenderOptions->mBodyStyle &= 0xf0ff; pRenderOptions->mBodyStyle |= (mBodyStyle & 0x0f00); }
  // doit: body style, D (accumulative)
  if((mBodyStyle & 0xf000) != 0) 
    { pRenderOptions->mBodyStyle |= (mBodyStyle & 0xf000); }
  // doit: text style (only one sub-option)
  if((mTextStyle & 0x000f) != 0) 
    { pRenderOptions->mTextStyle &= 0xfff0; pRenderOptions->mTextStyle |= (mTextStyle & 0x000f); }
  // doit colors
  if(mLineColor!=VioNoColor) pRenderOptions->mLineColor=&VioStyle::Color(mLineColor);
  if(mBodyColor!=VioNoColor) pRenderOptions->mBodyColor=&VioStyle::Color(mBodyColor);
}

// apply effect
void GioFlagEffect::Apply(GioRenderOptions* pRenderOptions, const faudes::AttributeVoid& rAttr) const {
  const faudes::AttributeFlags* fattr=dynamic_cast<const faudes::AttributeFlags*>(&rAttr);
  if(!fattr) return;
  Apply(pRenderOptions,fattr->mFlags);
}




/*
 ************************************************
 ************************************************
 ************************************************

 Implementation: VioGeneratorStyle 

 ************************************************
 ************************************************
 ************************************************
 */


// constructor
VioGeneratorStyle::VioGeneratorStyle(const QString& ftype) : VioStyle()
{
  FD_DQT("VioGeneratorStyle::VioGeneratorStyle(): ftype \"" << VioStyle::StrFromQStr(ftype) << "\"");
  // fix my pointers
  mGlobalAttribute=0;
  mTransAttribute=0;
  mStateAttribute=0;
  mEventAttribute=0;
  // configure 
  Initialise(ftype);
  ReadFile();
  FD_DQG("VioGeneratorStyle::VioGeneratorStyle(): ftype \"" << VioStyle::StrFromQStr(mFaudesType) << "\"");
  FD_DQG("VioGeneratorStyle::VioGeneratorStyle(): effects " << mTransEffects.size() << " " 
    << mStateEffects.size() << " " << mEventEffects.size() );
};

// set defaults
void VioGeneratorStyle::Initialise(const QString& ftype){
  // record type
  mFaudesType=ftype;
  // attribute prototypes
  mGlobalAttribute=qobject_cast<VioAttributeModel*>(VioTypeRegistry::NewModel("AttributeGlobalFlags"));  
  mTransAttribute=qobject_cast<VioAttributeModel*>(VioTypeRegistry::NewModel("AttributeTransFlags"));  
  mStateAttribute=qobject_cast<VioAttributeModel*>(VioTypeRegistry::NewModel("AttributeStateFlags"));  
  mEventAttribute=qobject_cast<VioAttributeModel*>(VioTypeRegistry::NewModel("AttributeEventFlags"));  
  // test and throw exception
  if(!mGlobalAttribute || !mTransAttribute || !mStateAttribute || !mEventAttribute) {
    std::stringstream errstr;
    errstr << "VioDes RTI lookup error when defining " << VioStyle::StrFromQStr(ftype) << " generator style";
    throw faudes::Exception("VioGeneratorStyle::Initialise", errstr.str(), 48);
  }
  // layout
  mLayoutFlags= Generator | Properties;
  // built-in effects: states
  InsStateEffect(GioFlagEffect("Initial State", 0x80000000,0x80000000,
    GioNoLine, VioNoColor, GioInit, VioNoColor, GioNoText));
  InsStateEffect(GioFlagEffect("Marked State", 0x40000000,0x40000000,
    GioNoLine, VioNoColor, GioDouble, VioNoColor, GioNoText));
  // parameters
  mStateNormalSize=60;
  mStateSmallSize=20;
  mStateMarkGap=3;
  mImportMeshWidthX=180;
  mImportMeshWidthY=120;
};

// load from file
void VioGeneratorStyle::ReadFile(const QString& filename){
  // figure my section and get token reader
  if(mFaudesType=="") return;
  faudes::TokenReader* trp=NewStyleReader(mFaudesType+"Style", filename);
  std::string fsection=VioStyle::StrFromQStr(mFaudesType+"Style");
  if(!trp) return;
  // loop my section
  while(!trp->Eos(fsection)) {
    faudes::Token token;
    trp->Peek(token);
    if(token.Type()!=faudes::Token::Begin) { trp->Get(token); continue; }
    // read my subsections: attributes
    if(token.StringValue()=="Attributes") {
      FD_DQG("VioGeneratorStyle::Config: setting up attribute prototypes ");
      trp->ReadBegin("Attributes");
      mGlobalAttribute=qobject_cast<VioAttributeModel*>(VioTypeRegistry::NewModel(VioStyle::QStrFromStr(trp->ReadString())));  
      mTransAttribute=qobject_cast<VioAttributeModel*>(VioTypeRegistry::NewModel(VioStyle::QStrFromStr(trp->ReadString())));  
      mStateAttribute=qobject_cast<VioAttributeModel*>(VioTypeRegistry::NewModel(VioStyle::QStrFromStr(trp->ReadString())));  
      mEventAttribute=qobject_cast<VioAttributeModel*>(VioTypeRegistry::NewModel(VioStyle::QStrFromStr(trp->ReadString())));  
      // test and throw exception
      if(!mGlobalAttribute || !mTransAttribute || !mStateAttribute || !mEventAttribute) {
        std::stringstream errstr;
        errstr << "VioDes RTI lookup error when defining " << VioStyle::StrFromQStr(mFaudesType) << " generator style";
        throw faudes::Exception("VioGeneratorStyle::Initialise", errstr.str(), 48);
      }     
      trp->ReadEnd("Attributes");
    }
    // read my subsections: transition flags 
    if(token.StringValue()=="VisualTransitions") {
      mTransEffects.clear();
      trp->ReadBegin("VisualTransitions");
      GioFlagEffect effect;
      while(effect.Read(*trp)) InsTransEffect(effect);
      trp->ReadEnd("VisualTransitions");
      continue;
    }
    // read my subsections: state flags 
    if(token.StringValue()=="VisualStates") {
      mStateEffects.clear();
      trp->ReadBegin("VisualStates");
      GioFlagEffect effect;
      while(effect.Read(*trp)) InsStateEffect(effect);
      trp->ReadEnd("VisualStates");
      continue;
    }
    // read my subsections: event flags 
    if(token.StringValue()=="VisualEvents") {
      mEventEffects.clear();
      trp->ReadBegin("VisualEvents");
      GioFlagEffect effect;
      while(effect.Read(*trp)) InsEventEffect(effect);
      trp->ReadEnd("VisualEvents");
      continue;
    }
    // read my subsections: edit functions
    if(token.StringValue()=="EditFunctions") {
      trp->ReadBegin("EditFunctions");
      VioEditFunction vfnct;
      while(vfnct.Read(*trp)) InsEditFunction(vfnct);
      trp->ReadEnd("EditFunctions");
      continue;
    }
    // skip unkown subsection
    trp->ReadBegin(token.StringValue());
    trp->ReadEnd(token.StringValue());
  }
  // done
  trp->ReadEnd(VioStyle::StrFromQStr(mFaudesType)+"Style");
  delete trp;
  FD_DQT("VioGeneratorStyle::Config:done ");
}


// define flag and visual effects (todo? doublets?)
void VioGeneratorStyle::InsStateEffect(const GioFlagEffect& effect) {
  mStateEffects.append(effect);
}  
void VioGeneratorStyle::InsEventEffect(const GioFlagEffect& effect) {
  mEventEffects.append(effect);
}  
void VioGeneratorStyle::InsTransEffect(const GioFlagEffect& effect) {
  mTransEffects.append(effect);
}  

// get ref of flag definitions
const QList<GioFlagEffect>& VioGeneratorStyle::StateEffects(void) {
  return mStateEffects; }
const QList<GioFlagEffect>& VioGeneratorStyle::EventEffects(void) {
  return mEventEffects; }
const QList<GioFlagEffect>& VioGeneratorStyle::TransEffects(void) {
  return mTransEffects; }


// map faudes elements to renderoptions (here: from flageffects)
void VioGeneratorStyle::MapElementOptions(const VioGeneratorModel* pGenModel, const VioElement& rElem, GioRenderOptions* pOptions) {
  FD_DQG("VioGeneratorStyle::MapElementOptions() for " << rElem.Str());
  pOptions->Clear();
  // bail out
  if(!pGenModel->ElementExists(rElem)) return;
  // have faudes gen ref
  const faudes::vGenerator* fgen = pGenModel->Generator(); 
  // its a state
  if(rElem.Type()==VioElement::EState) {
    // universal defaults
    pOptions->mLineColor= &VioStyle::Color("Black");
    pOptions->mBodyColor= &VioStyle::Color("White");
    // get flags (incl fake init/marked)
    faudes::fType flags = 0;
    const faudes::AttributeVoid* attr= &fgen->StateAttribute(rElem.State());
    const faudes::AttributeFlags* fattr= dynamic_cast<const faudes::AttributeFlags*>(attr);
    if(fattr) flags=fattr->mFlags;
    if(fgen->ExistsInitState(rElem.State())) flags |= 0x80000000;
    if(fgen->ExistsMarkedState(rElem.State())) flags |= 0x40000000;
    FD_DQG("VioGeneratorStyle::MapElementOptions() for " << rElem.Str() << " flags " << flags);
    // apply all effects
    for(int i=0; i<mStateEffects.size(); i++) 
      mStateEffects[i].Apply(pOptions,flags);
    // set up brushes etc
    pOptions->Compile();
  }
  // its a transition
  if(rElem.Type()==VioElement::ETrans) {
    // general defaults
    pOptions->mLineColor= &VioStyle::Color("Black");
    pOptions->mBodyColor= &VioStyle::Color("Black");
    // bail out
    if(!fgen->ExistsTransition(rElem.Trans())) return;
    // get trans flags 
    faudes::fType flags = 0;
    const faudes::AttributeVoid* attr= &fgen->TransAttribute(rElem.Trans());
    const faudes::AttributeFlags* fattr= dynamic_cast<const faudes::AttributeFlags*>(attr);
    if(fattr) flags=fattr->mFlags;
    FD_DQG("VioGeneratorStyle::MapElementOptions() for " << rElem.Str() << " with t-flags " << flags);
    // apply all effects
    for(int i=0; i<mTransEffects.size(); i++) 
      mTransEffects[i].Apply(pOptions,flags);
    // get event flags 
    if(fgen->ExistsEvent(rElem.Trans().Ev)) {
      attr= &fgen->EventAttribute(rElem.Trans().Ev);
      fattr= dynamic_cast<const faudes::AttributeFlags*>(attr);
      if(fattr) flags=fattr->mFlags;
    }
    FD_DQG("VioGeneratorStyle::MapElementOptions() for " << rElem.Str() << " with te-flags " << flags);
    // apply all effects
    for(int i=0; i<mEventEffects.size(); i++) 
      mEventEffects[i].Apply(pOptions,flags);
    // set up brushes etc
    pOptions->Compile();
  }

}

/*
*****************************

State render helper functions

*****************************
*/



// shape of state: fixed size? set pointL, return true
bool VioGeneratorStyle::StatePointL(QPointF& pointL, const QPointF& pointC, const GioRenderOptions* pOptions) {
  // prepare result
  bool fixed = false;
  // check size effect
  if((pOptions->mBodyStyle & GioSizeMask) == GioSmallSize) {
    pointL=pointC+QPointF(StateSmallSize()/2,StateSmallSize()/2);
    fixed=true;
  };
  if((pOptions->mBodyStyle & GioSizeMask) == GioNormSize) {
    pointL=pointC+QPointF(StateNormalSize()/2,StateNormalSize()/2);
    fixed=true;
  };
  return fixed;
}

// shape of state: circle
void VioGeneratorStyle::StateCirclePath(QPainterPath& state, const QPointF& pointC, const QPointF& pointL) {
  state=QPainterPath();
  QPointF diag=pointL-pointC;
  state.addEllipse(pointC.x()-diag.x(),pointC.y()-diag.y(), 
    2*diag.x(), 2*diag.y());
  state.closeSubpath();
}

// shape of state: rectangle
void VioGeneratorStyle::StateRectanglePath(QPainterPath& state, const QPointF& pointC, const QPointF& pointL) {
  state=QPainterPath();
  QPointF diag=pointL-pointC;
  state.addRect(pointC.x()-diag.x(),pointC.y()-diag.y(), 
    2*diag.x(), 2*diag.y());
  state.closeSubpath();
}


// shape of state: hexagon
void VioGeneratorStyle::StateHexagonPath(QPainterPath& state, const QPointF& pointC, const QPointF& pointL) {
  state=QPainterPath();
  QPointF diag=pointL-pointC;
  state.moveTo(pointC + QPointF( +diag.x()*(0.42),+diag.y()) );
  state.lineTo(pointC + QPointF( -diag.x()*(0.42),+diag.y()) );
  state.lineTo(pointC + QPointF( -diag.x(),       +diag.y()*(0.42)) );
  state.lineTo(pointC + QPointF( -diag.x(),       -diag.y()*(0.42)) );
  state.lineTo(pointC + QPointF( -diag.x()*(0.42),-diag.y()) );
  state.lineTo(pointC + QPointF( +diag.x()*(0.42),-diag.y()) );
  state.lineTo(pointC + QPointF( +diag.x(),       -diag.y()* (0.42)) );
  state.lineTo(pointC + QPointF( +diag.x(),       +diag.y()* (0.42)) );
  state.lineTo(pointC + QPointF( +diag.x()*(0.42),+diag.y()) );
  state.closeSubpath();
}


// shape of state, single
void VioGeneratorStyle::StateSinglePath(QPainterPath& state, const QPointF& pointC, const QPointF& pointL, const GioRenderOptions* pOptions) {
  // check size;
  QPointF pl=pointL;
  state=QPainterPath();
  // invisible
  if((pOptions->mBodyStyle & GioShapeMask) == GioInvisible) return;
  // sensible default
  StatePointL(pl, pointC, pOptions);
  // check actual shape
  StateCirclePath(state,pointC,pl);
  if((pOptions->mBodyStyle & GioShapeMask) == GioRectangle) {
    StateRectanglePath(state,pointC,pl);
  };
  if((pOptions->mBodyStyle & GioShapeMask) == GioHexagon) {
    StateHexagonPath(state,pointC,pl);
  };
  // fix filling
  state.setFillRule(Qt::WindingFill);
}

// append shapes of states (with double option)
void VioGeneratorStyle::AddStatePaths(QList<GioDrawElement>& delements, const QPointF& pointC, const QPointF& pointL, const GioRenderOptions* pOptions) {
  // add a single empty path
  GioDrawElement nelem;
  delements.push_back(nelem);
  GioDrawElement& delem=delements.last();
  // set pen and brush
  delem.pPen=&(pOptions->mLinePen);
  delem.pBrush=&(pOptions->mBodyBrush);
  // set up a single path
  StateSinglePath(delem.mPath,pointC,pointL,pOptions);
  // should we mark ?
  if( (pOptions->mBodyStyle & GioDouble) == GioDouble) {
    // add another empty path
    GioDrawElement nelem;
    delements.push_back(nelem);
    GioDrawElement& delem=delements.last();
    // set pen and brush
    delem.pPen=&(pOptions->mLinePen);
    delem.pBrush=&(pOptions->mBodyBrush);
    // set up a path
    QPointF pointL2=QPointF(pointL.x()-StateMarkGap(),pointL.y()-StateMarkGap());
    StateSinglePath(delem.mPath,pointC,pointL2,pOptions);
  }
}


// init path
void VioGeneratorStyle::AddInitPath(QList<GioDrawElement>& delements, const QPointF& pointA, const QPointF& pointB, const GioRenderOptions* pOptions) {  
  // add a single empty path
  GioDrawElement nelem;
  delements.push_back(nelem);
  GioDrawElement& delem=delements.last();
  // set pen and brush
  delem.pPen=&(pOptions->mLinePen);
  delem.pBrush=&(pOptions->mLineBrush);
  // should we init?
  if( (pOptions->mBodyStyle & GioInit) != GioInit) return;
  // doit
  VioGeneratorStyle::ArrowTip(delem.mPath,pointA,pointB);
  delem.mPath.moveTo(pointA);
  delem.mPath.lineTo(pointB);
}
  

// name of state
void VioGeneratorStyle::AddStateName(QList<GioDrawElement>& delements, const QPointF& pointC, const QPointF& pointL, const QString& text, const GioRenderOptions* pOptions) {
  // add a single empty path
  GioDrawElement nelem;
  delements.push_back(nelem);
  GioDrawElement& delem=delements.last();
  // set pen and brush
  delem.pPen=&(pOptions->mLinePen);
  delem.pBrush=&(pOptions->mLineBrush);
  // draw the name
  QPointF diag=pointL-pointC;
  QRectF rect=QRectF(pointC.x()-diag.x(),pointC.y()-diag.y(), 2*diag.x(), 2*diag.y());
  TextCR(delem.mPath,rect,text);
}


/*
 *************************************
 drawing: transition

 *************************************
 */


// pen for lines, shapes etc
void VioGeneratorStyle::TransLinePen(QPen& pen, const GioRenderOptions* pOptions){
  FD_DQ("VioGeneratorStyle::TransLinePen(...)");
  (void) pOptions;
  pen=DefaultPen();
  pen.setStyle(Qt::SolidLine);
  /*
  for(int i=0; i<mTransEventEffects.size(); i++) {
    const GioFlagEffect& effect= mTransEventEffects[i];
    if((effect.mMask & flags) == effect.mValue) {
      if(effect.mLcolor!=GioNoColor)
        pen.setColor(color(effect.mLcolor));
      if(effect.mLstyle==GioSolid) 
         pen.setStyle(Qt::SolidLine); 
      if(effect.mLstyle==GioDotted) 
         pen.setStyle(Qt::DotLine); 
      if(effect.mLstyle==GioDashed) 
         pen.setStyle(Qt::DashLine); 
    };
  };
  */
}

// brush for line attributes aka arrow tips
void VioGeneratorStyle::TransLineBrush(QBrush& brush, const GioRenderOptions* pOptions){
  brush=DefaultBrush();
  (void) pOptions;
  /*
  for(int i=0; i<mTransEventEffects.size(); i++) {
    const GioFlagEffect& effect= mTransEventEffects[i];
    if(effect.mMask & flags) {
      if(effect.mLcolor!=GioNoColor)
        brush.setColor(color(effect.mLcolor));
    };
  };
  */  
}

// brush for tick
void VioGeneratorStyle::TransTickBrush(QBrush& brush, const GioRenderOptions* pOptions){
  (void) pOptions;
  brush = QBrush();
  brush.setStyle(Qt::SolidPattern);
  brush.setColor(Qt::black);
  /*
  for(int i=0; i<mTransEventEffects.size(); i++) {
    const GioFlagEffect& effect= mTransEventEffects[i];
    if((effect.mMask & flags)== effect.mValue) {
      if(effect.mScolor!=GioNoColor)
        brush.setColor(color(effect.mScolor));
    };
  };
  */  
}

#define TICK_RATIO 0.2
#define TICK_SIZE  8

// find tick point and orientation (in bezier curve segment)
// todo: look up and use bezier evaluation formula
void VioGeneratorStyle::TransTickPos(
  QPointF& pointC, QPointF& pointD, 
  const QPointF& pointA, const QPointF& pointCA,
  const QPointF& pointCB, const QPointF& pointB)
{
  // find coefficients of bezier curve, aka cubic polynomial 
  // s(t) = c3 t^3 + c2 t^2 + c1 t + c0
  // with s(0)=A; s(1)=B; s'(0)=CA-A s'(1)=B-CB;
  QPointF c0 = pointA;
  QPointF c1 = -3*pointA+3*pointCA;
  QPointF c2 = 3*pointA - 6*pointCA + 3*pointCB;
  QPointF c3 = -pointA+3*pointCA-3*pointCB + pointB;
  // evaluate s(t) for t=0.5
  pointC = .125 * c3 + .25 * c2 + .5*c1 +c0;
  // evaluate s'(t) for t=0.5
  pointD = .25 * 3 * c3 + .5 * 2 * c2 + c1;
}


// tick stroke
void VioGeneratorStyle::TransTickStrokePath(QPainterPath& tick, 
  const QPointF& pointA, const QPointF& pointCA,
  const QPointF& pointCB, const QPointF& pointB) 
{
  tick=QPainterPath();
  QPointF pointC, pointD, pointO;
  TransTickPos(pointC, pointD, pointA, pointCA, pointCB, pointB);
  pointO=TICK_SIZE*NormalF(pointD);
  pointD=TICK_SIZE*TICK_RATIO*NormalizeF(pointD);
  tick.setFillRule(Qt::WindingFill);
  tick.moveTo(pointC-pointD-pointO);
  tick.lineTo(pointC-pointD+pointO);
  tick.lineTo(pointC+pointD+pointO);
  tick.lineTo(pointC+pointD-pointO);
}

// tick double stroke
void VioGeneratorStyle::TransTickDoubleStrokePath(QPainterPath& tick, 
  const QPointF& pointA, const QPointF& pointCA,
  const QPointF& pointCB, const QPointF& pointB) 
{
  tick=QPainterPath();
  QPointF pointC, pointD, pointO;
  TransTickPos(pointC, pointD, pointA, pointCA, pointCB, pointB);
  pointO=TICK_SIZE*NormalF(pointD);
  pointD=TICK_SIZE*TICK_RATIO*NormalizeF(pointD);
  tick.setFillRule(Qt::WindingFill);
  tick.moveTo(pointC-2*pointD-pointD-pointO);
  tick.lineTo(pointC-2*pointD-pointD+pointO);
  tick.lineTo(pointC-2*pointD+pointD+pointO);
  tick.lineTo(pointC-2*pointD+pointD-pointO);
  tick.lineTo(pointC-2*pointD-pointD-pointO);
  tick.moveTo(pointC+2*pointD-pointD-pointO);
  tick.lineTo(pointC+2*pointD-pointD+pointO);
  tick.lineTo(pointC+2*pointD+pointD+pointO);
  tick.lineTo(pointC+2*pointD+pointD-pointO);
  tick.lineTo(pointC+2*pointD-pointD-pointO);
}

// tick diamond
void VioGeneratorStyle::TransTickDiamondPath(QPainterPath& tick, 
  const QPointF& pointA, const QPointF& pointCA,
  const QPointF& pointCB, const QPointF& pointB) 
{
  tick=QPainterPath();
  QPointF pointC, pointD, pointO;
  TransTickPos(pointC, pointD, pointA, pointCA, pointCB, pointB);
  pointO=TICK_SIZE*NormalF(pointD);
  pointD=TICK_SIZE*TICK_RATIO*NormalizeF(pointD);
  tick=QPainterPath();
  tick.setFillRule(Qt::WindingFill);
  tick.moveTo(pointC-pointD);
  tick.lineTo(pointC-pointD-0.5*pointO);
  tick.lineTo(pointC-pointO);
  tick.lineTo(pointC+pointD-0.5*pointO);
  tick.lineTo(pointC+pointD+0.5*pointO);
  tick.lineTo(pointC+pointO);
  tick.lineTo(pointC-pointD+0.5*pointO);
}

// tick diamond
void VioGeneratorStyle::TransTickDoubleDiamondPath(QPainterPath& tick, 
  const QPointF& pointA, const QPointF& pointCA,
  const QPointF& pointCB, const QPointF& pointB) 
{
  tick=QPainterPath();
  QPointF pointC, pointD, pointO;
  TransTickPos(pointC, pointD, pointA, pointCA, pointCB, pointB);
  pointO=TICK_SIZE*NormalF(pointD);
  pointD=TICK_SIZE*TICK_RATIO*NormalizeF(pointD);
  tick=QPainterPath();
  tick.setFillRule(Qt::WindingFill);
  tick.moveTo(pointC-2*pointD-pointD);
  tick.lineTo(pointC-2*pointD-pointD-0.5*pointO);
  tick.lineTo(pointC-2*pointD-pointO);
  tick.lineTo(pointC-2*pointD+pointD-0.5*pointO);
  tick.lineTo(pointC-2*pointD+pointD+0.5*pointO);
  tick.lineTo(pointC-2*pointD+pointO);
  tick.lineTo(pointC-2*pointD-pointD+0.5*pointO);
  tick.lineTo(pointC-2*pointD-pointD);
  tick.moveTo(pointC+2*pointD-pointD);
  tick.lineTo(pointC+2*pointD-pointD-0.5*pointO);
  tick.lineTo(pointC+2*pointD-pointO);
  tick.lineTo(pointC+2*pointD+pointD-0.5*pointO);
  tick.lineTo(pointC+2*pointD+pointD+0.5*pointO);
  tick.lineTo(pointC+2*pointD+pointO);
  tick.lineTo(pointC+2*pointD-pointD+0.5*pointO);
  tick.lineTo(pointC+2*pointD-pointD);
}

// tick
void VioGeneratorStyle::TransTickPath(QPainterPath& tick, 
  const QPointF& pointA, const QPointF& pointCA,
  const QPointF& pointCB, const QPointF& pointB, 
  const GioRenderOptions* pOptions) 
{
  tick=QPainterPath();
  // invisible overrules other
  if((pOptions->mBodyStyle & GioShapeMask) == GioInvisible) return;
  // various ticks
  if((pOptions->mBodyStyle & GioStrokeMask) == GioStroke) 
        TransTickStrokePath(tick,pointA,pointCA,pointCB,pointB);
  if((pOptions->mBodyStyle & GioStrokeMask) == GioDoubleStroke) 
        TransTickDoubleStrokePath(tick,pointA,pointCA,pointCB,pointB);
  if((pOptions->mBodyStyle & GioStrokeMask) == GioDiamond) 
        TransTickDiamondPath(tick,pointA,pointCA,pointCB,pointB);
  if((pOptions->mBodyStyle & GioStrokeMask) == GioDoubleDiamond) 
        TransTickDoubleDiamondPath(tick,pointA,pointCA,pointCB,pointB);
}

// arc
void VioGeneratorStyle::TransArcPath(QPainterPath& arc, const QList<QPointF>& points, const GioRenderOptions* pOptions) 
{
  (void) pOptions;
  arc=QPainterPath();
  if(points.size()<6) return;
  // invisible overrules other
  if((pOptions->mBodyStyle & GioShapeMask) == GioInvisible) return;
  // prepare: first segment with straight line to root
  QPointF fakebegin= points[1] + ArrowSize()*NormalizeF(points[2]-points[1]);
  // prepare: last segment with straight line for arrow tip
  QPointF fakeend= points[points.size()-2] - ArrowSize()*NormalizeF(points[points.size()-2]-points[points.size()-3]);
  // do the arc: line from A to root R
  arc.moveTo(0,0); 
  arc.lineTo(points[1]); 
  arc.lineTo(fakebegin); 
  // do the arc: all cubic segments 
  int i;
  for(i=1; i+3 < points.size()-3; i+=3) 
    arc.cubicTo(points[i+1],points[i+2],points[i+3]);
  arc.cubicTo(points[i+1],points[i+2],fakeend);
  arc.lineTo(points[i+3]); 
  // do the arc: straight line to B
  arc.lineTo(points[i+4]); 
}

// arrow head
void VioGeneratorStyle::TransArrPath(QPainterPath& arr, const QList<QPointF>& points,const GioRenderOptions* pOptions) 
{
  if(points.size()<2) return;
  (void) pOptions;
  arr=QPainterPath();
  // invisible overrules other
  if((pOptions->mBodyStyle & GioShapeMask) == GioInvisible) return;
  // figure points a and b
  const QPointF& pointB=points.back();
  int idxA;
  for(idxA=points.size()-2; idxA>=0; idxA--) {
    if(VioStyle::NormF(points.at(idxA)-pointB)>1) break;
  }
  if(idxA<0) return;
  // doit
  ArrowTip(arr,points.at(idxA),pointB);
}


/*
 *************************************
 register styles

 *************************************
 */



// register all generator styles from config file
void VioRegisterGeneratorStyles(const QString& filename) {
  // figure my section and get token reader
  faudes::TokenReader* trp=VioStyle::NewStyleReader("GeneratorStyles", filename);
  std::string fsection="GeneratorStyles";
  if(!trp) return;
  // make sure, at least Generator/System are faudes registerd
  if(!faudes::TypeRegistry::G()->Exists("Generator")) 
    faudes::TypeRegistry::G()->Insert<faudes::Generator>("Generator");
  if(!faudes::TypeRegistry::G()->Exists("System")) 
    faudes::TypeRegistry::G()->Insert<faudes::System>("System");
  // read my section
  while(!trp->Eos(fsection)) {
    // name of style
    QString ftype=VioStyle::QStrFromStr(trp->ReadString());
    FD_DQT("VioRegisterGeneratorStyle(): register " << VioStyle::StrFromQStr(ftype)); 
    // get base faudes type (defaults to name of style)
    QString rftype=ftype;
    faudes::Token token;
    trp->Peek(token);
    if(token.Type()==faudes::Token::Option) {
      rftype=VioStyle::QStrFromStr(trp->ReadOption());
      // remove name space faudes
      if(rftype.startsWith("faudes::")) rftype.remove(0,QString("faudes::").length());
    }
    // register style as faudes type
    bool err=false;
    if(!faudes::TypeRegistry::G()->Exists(VioStyle::StrFromQStr(ftype))) {
      try{
        faudes::Type* fproto=faudes::TypeRegistry::G()->NewObject(
          VioStyle::StrFromQStr(rftype));        // exception on non ex
        faudes::TypeRegistry::G()->Insert(
          fproto, VioStyle::StrFromQStr(ftype)); // exception on doublets
      } catch(faudes::Exception& fexception) {
        std::stringstream errstr;
        errstr << "RTI lookup error when defining " << VioStyle::StrFromQStr(ftype) << " from " << VioStyle::StrFromQStr(rftype);
        FD_WARN(errstr.str());
        //throw faudes::Exception("VioRegisterGeneratorStyle", errstr.str(), 48);
        err=true;
      }
    }
    // skip on error
    if(err) continue;
    // continue if it exists (i.e. hardcoded)
    if(VioTypeRegistry::Exists(ftype)) continue;
    // try to create and register
    VioGeneratorStyle* astyle;
    VioGeneratorModel* aproto;
    try{
      // construct generator style (incl read actual configuration)
      astyle = new VioGeneratorStyle(ftype);
      // construct vio prototype
      aproto = new VioGeneratorModel(0,astyle); 
    } catch(faudes::Exception& fexception) {
      std::stringstream errstr;
      errstr << "RTI prototype error when defining " << VioStyle::StrFromQStr(ftype) << " from " << VioStyle::StrFromQStr(rftype);
        FD_WARN(errstr.str());
        //throw faudes::Exception("VioRegisterGeneratorStyle", errstr.str(), 48);
        err=true;
    }
    // skip on error
    if(err) continue;
    // register
    VioTypeRegistry::Insert(aproto);
    FD_DQT("VioRegisterGeneratorStyle(): " << VioStyle::StrFromQStr(ftype) << ": done"); 
  }
  // done
  trp->ReadEnd("GeneratorStyles");
  delete trp;
}

