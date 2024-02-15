/* viodiaggenstyle.cpp  - viogen configuration for diagnosis plugin */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2010  Thomas Moor

*/



#include "viogenerator.h"
#include "viodiagstateattr.h"
#include "viodiaggenstyle.h"

/*
 *************************************
 *************************************
 *************************************
 
 Implementation of RenderOptions

 *************************************
 *************************************
 *************************************
 */

// construct
GioDiagRenderOptions::GioDiagRenderOptions(void) :
  GioRenderOptions()
{
  Compile();
};

// clear
void GioDiagRenderOptions::Clear(void) {
  GioRenderOptions::Clear();
  mStateLabel="";
  Compile();
};

// compile
void GioDiagRenderOptions::Compile(void) {
  GioRenderOptions::Compile();
}





/*
 ************************************************
 ************************************************
 ************************************************

 Implementation: VioDiagGeneratorStyle 

 ************************************************
 ************************************************
 ************************************************
 */


// constructor
VioDiagGeneratorStyle::VioDiagGeneratorStyle(const QString& ftype) : 
  VioGeneratorStyle(ftype)
{
  FD_DQD("VioDiagGeneratorStyle::VioDiagGeneratorStyle(): ftype \"" << VioStyle::StrFromQStr(ftype) << "\"");

  // in a constrcutor virtual functions are not virtual
  Initialise(ftype);
};

// set defaults
void VioDiagGeneratorStyle::Initialise(const QString& ftype){
  FD_DQD("VioDiagGeneratorStyle::Initialise()");
  // call base ( is done in constructor)
  //VioGeneratorStyle::Initialise(ftype);
  // adjust diag attribute prototypes
  mStateAttribute=qobject_cast<VioAttributeModel*>(VioTypeRegistry::NewModel("AttributeDiagnoserState"));  
  mEventAttribute=qobject_cast<VioAttributeModel*>(VioTypeRegistry::NewModel("AttributeEventFlags"));  
  // test and throw exception
  if(!mGlobalAttribute || !mTransAttribute || !mStateAttribute || !mEventAttribute) {
    std::stringstream errstr;
    errstr << "VioDes RTI lookup error when defining " << VioStyle::StrFromQStr(ftype) << " generator style";
    throw faudes::Exception("VioDiagGeneratorStyle::Initialise", errstr.str(), 48);
  }
};

// load from file
void VioDiagGeneratorStyle::ReadFile(const QString& filename){
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
    // read my subsections: no such
    // skip unkown subsection
    trp->ReadBegin(token.StringValue());
    trp->ReadEnd(token.StringValue());
  }
  // done
  trp->ReadEnd(VioStyle::StrFromQStr(mFaudesType)+"Style");
  delete trp;
  FD_DQD("VioDiagGeneratorStyle::Config:done ");
}


// map faudes elements to renderoptions (here: my label)
void VioDiagGeneratorStyle::MapElementOptions(const VioGeneratorModel* pGenModel, const VioElement& rElem, GioRenderOptions* pOptions) {
  FD_DQD("VioDiagGeneratorStyle::MapElementOptions() for " << rElem.Str());
  pOptions->Clear();
  // call base for flags
  VioGeneratorStyle::MapElementOptions(pGenModel, rElem, pOptions);
  // we're done if its not a state
  if(rElem.Type()!=VioElement::EState) return;
  // have my types
  GioDiagRenderOptions* pxopt = dynamic_cast<GioDiagRenderOptions*>(pOptions);
  if(!pxopt) {
    FD_DQD("VioDiagGeneratorStyle::MapElementOptions(): invalid render options");
    return;
  }
  FD_DQD("VioDiagGeneratorStyle::MapElementOptions(): diagnoser render options");
  // get my attribute (again: inefficient TODO)
  bool exs= pGenModel->Generator()->ExistsState(rElem.State());
  FD_DQD("VioDiagGeneratorStyle::MapElementOptions(): state exists " << exs);
  if(!exs) return;
  const faudes::AttributeVoid* attr= & pGenModel->Generator()->StateAttribute(rElem.State());
  const faudes::AttributeDiagnoserState* dsattr= dynamic_cast<const faudes::AttributeDiagnoserState*>(attr);
  if(!dsattr) return;
  // figure my label
  FD_DQD("VioDiagGeneratorStyle::MapElementOptions() with state label " << dsattr->Str());
  pxopt->mStateLabel=VioStyle::QStrFromStr(dsattr->Str());
}



/*
*****************************
Actual drawing
*****************************
*/




// shape of state: implement autosize (should go to std gio style)
bool VioDiagGeneratorStyle::StatePointL(QPointF& pointL, const QPointF& pointC, const GioRenderOptions* pOptions) {
  // get my extra options
  const GioDiagRenderOptions* pxopt = dynamic_cast<const GioDiagRenderOptions*>(pOptions);
  if(!pxopt) return false;
  // figure my label text
  QRectF trect;
  TextLL(trect,QPointF(0,0),pxopt->mStateLabel); 
  qreal w=trect.width() + 10;
  qreal h=trect.height() + 10;
  // adjust to minvalues
  if(w< StateNormalSize()) w= StateNormalSize();
  if(h< 0.66* StateNormalSize()) h= 0.66*StateNormalSize();
  // figure pointL
  pointL=pointC+QPointF(w/2+4,h/2+4); 
  return true; // true<>fixed
}


// append state name (using the diagnoser label)
void VioDiagGeneratorStyle::AddStateName(QList<GioDrawElement>& delements, const QPointF& pointC, const QPointF& pointL, const QString& text, const GioRenderOptions* pOptions) {
  FD_DQD("VioDiagGeneratorStyle::AddStateName() with text " << VioStyle::StrFromQStr(text));
  // figure my label text
  QString label=text;
  // get my extra options
  const GioDiagRenderOptions* pxopt = dynamic_cast<const GioDiagRenderOptions*>(pOptions);
  if(pxopt) label=pxopt->mStateLabel; 
  // let base do the job
  VioGeneratorStyle::AddStateName(delements,pointC,pointL,label,pOptions);
}

