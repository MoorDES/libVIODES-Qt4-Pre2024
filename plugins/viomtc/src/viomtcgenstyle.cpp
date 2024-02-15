/* viomtcgenstyle.cpp  - viogen configuration for mtc plugin */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006, 2007  Thomas Moor, Klaus Schmidt, Sebastian Perk

*/



#include "viomtcgenstyle.h"
#include "viomtcgenerator.h"

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
GioMtcRenderOptions::GioMtcRenderOptions(void) :
  GioRenderOptions()
{
  Compile();
};

// clear
void GioMtcRenderOptions::Clear(void) {
  GioRenderOptions::Clear();
  mExtraPens.clear();
  Compile();
};

// compile
void GioMtcRenderOptions::Compile(void) {
  GioRenderOptions::Compile();
}





/*
 ************************************************
 ************************************************
 ************************************************

 Implementation: VioMtcGeneratorStyle 

 ************************************************
 ************************************************
 ************************************************
 */


// constructor
VioMtcGeneratorStyle::VioMtcGeneratorStyle(const QString& ftype) : VioGeneratorStyle(ftype)
{
  FD_DQT("VioMtcGeneratorStyle::VioMtcGeneratorStyle(): ftype \"" << VioStyle::StrFromQStr(ftype) << "\"");

  // ina constrcutor virtual functions are not resolved??
  Initialise(ftype);
  //ReadFile();
};

// set defaults
void VioMtcGeneratorStyle::Initialise(const QString& ftype){
  FD_DQT("VioMtcGeneratorStyle::Initialise()");
  // call base
  VioGeneratorStyle::Initialise(ftype);
  // adjust mtc attribute prototypes
  mStateAttribute=qobject_cast<VioAttributeModel*>(VioTypeRegistry::NewModel("AttributeColoredState"));  
  mEventAttribute=qobject_cast<VioAttributeModel*>(VioTypeRegistry::NewModel("AttributeEventCFlags"));  
  // test and throw exception
  if(!mGlobalAttribute || !mTransAttribute || !mStateAttribute || !mEventAttribute) {
    std::stringstream errstr;
    errstr << "VioDes RTI lookup error when defining " << VioStyle::StrFromQStr(ftype) << " generator style";
    throw faudes::Exception("VioMtcGeneratorStyle::Initialise", errstr.str(), 48);
  }
  // parameters
  mColorSet.insert("Red");
  mColorSet.insert("Green");
  mColorSet.insert("Blue");
  mColorSet.insert("Yellow");
};

// load from file
void VioMtcGeneratorStyle::ReadFile(const QString& filename){
  // call base
  VioGeneratorStyle::ReadFile(filename);
  // try more (ineffective mechanism of inheritance/fil parameters TODO)

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
  FD_DQT("VioMtcGeneratorStyle::Config:done ");
}


// map faudes elements to renderoptions (here: from flageffects)
void VioMtcGeneratorStyle::MapElementOptions(const VioGeneratorModel* pGenModel, const VioElement& rElem, GioRenderOptions* pOptions) {
  FD_DQT("VioMtcGeneratorStyle::MapElementOptions() for " << rElem.Str());
  pOptions->Clear();
  // call base for flags
  VioGeneratorStyle::MapElementOptions(pGenModel, rElem, pOptions);
  // we're done if its not a state
  if(rElem.Type()!=VioElement::EState) return;
  // have my types
  GioMtcRenderOptions* pxopt = static_cast<GioMtcRenderOptions*>(pOptions);
  const VioMtcGeneratorModel* pmgen = static_cast<const VioMtcGeneratorModel*>(pGenModel);
  // get my attribute (again: inefficient TODO)
  if(!pGenModel->Generator()->ExistsState(rElem.State())) return;
  const faudes::AttributeVoid* attr= & pGenModel->Generator()->StateAttribute(rElem.State());
  const faudes::AttributeColoredState* csattr= dynamic_cast<const faudes::AttributeColoredState*>(attr);
  if(!csattr) return;
  // do my colored markings
  FD_DQT("VioMtcGeneratorStyle::MapElementOptions() with colored markings " << csattr->Colors().ToString());
  // TODO: need index / brush cache
  // tODO: redesign colormap to use indexes
  faudes::ColorSet::Iterator mit=csattr->Colors().Begin();
  for(;mit!=csattr->Colors().End(); mit++) {
    QString markname=VioStyle::QStrFromStr(csattr->Colors().SymbolicName(*mit));
    if(!pmgen->Layout().ColorMap().contains(markname)) continue;
    const QColor& color = VioStyle::Color(pmgen->Layout().ColorMap()[markname]).light(200);
    QPen cp = VioStyle::DefaultPen();
    cp.setColor(color);
    cp.setWidthF(0.75*StateMarkGap());
    pxopt->mExtraPens.append(cp);
  }
  // set up xtra brushes etc
  //pxopt->Compile();
}

/*
*****************************

*****************************
*/




// append shapes of states (with double option)
void VioMtcGeneratorStyle::AddStatePaths(QList<GioDrawElement>& delements, const QPointF& pointC, const QPointF& pointL, const GioRenderOptions* pOptions) {
  // should we do it traditionally ?
  const GioMtcRenderOptions* pxopt = dynamic_cast<const GioMtcRenderOptions*>(pOptions);
  if(!pxopt)  {
    VioGeneratorStyle::AddStatePaths(delements,pointC,pointL,pOptions);
    return; 
  }
  if(pxopt->mExtraPens.size()==0) {
    VioGeneratorStyle::AddStatePaths(delements,pointC,pointL,pOptions);
    return; 
  }
  // body, solid e.g. white
  if(pxopt->mExtraPens.size()>0) {
    // add another empty path 
    GioDrawElement nelem2;
    delements.push_back(nelem2);
    GioDrawElement& delem2=delements.last();
    delem2.pPen=&(pOptions->mNoPen);
    delem2.pBrush=&(pOptions->mBodyBrush);
    // set up a single path
    StateSinglePath(delem2.mPath,pointC,pointL,pOptions);
  }
  // have a copy of diameter point l
  QPointF pointLj=QPointF(pointL);
  pointLj=QPointF(pointLj.x()-0.5*StateMarkGap(),pointLj.y()-0.5*StateMarkGap());
  // use up all extra pens
  for(int j=0; j< pxopt->mExtraPens.size(); j++) {
    // add another empty path
    GioDrawElement nelem;
    delements.push_back(nelem);
    GioDrawElement& delem=delements.last();
    // set pen and brush
    delem.pBrush=&(pOptions->mNoBrush);
    delem.pPen=&(pxopt->mExtraPens.at(j));
    // set up a path
    StateSinglePath(delem.mPath,pointC,pointLj,pOptions);
    pointLj=QPointF(pointLj.x()-StateMarkGap(),pointLj.y()-StateMarkGap());
  }
  // inner line, eg black
  if(pxopt->mExtraPens.size()>0) {
    // add another empty path (inner black line)
    GioDrawElement nelem1;
    delements.push_back(nelem1);
    GioDrawElement& delem1=delements.last();
    delem1.pPen=&(pOptions->mLinePen);
    delem1.pBrush=&(pOptions->mNoBrush);
    // set up a single path
    StateSinglePath(delem1.mPath,pointC,pointLj,pOptions);
  }
}

