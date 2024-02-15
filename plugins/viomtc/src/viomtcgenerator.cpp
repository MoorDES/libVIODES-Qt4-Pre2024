/* viomtcgenerator.cpp  - mtc generator model and widget */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2009  Thomas Moor, Ruediger Berndt

*/

#include "viomtcgenerator.h"
#include "viogenlist.h"

/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioMtcGeneratorLayout

****************************************************************
****************************************************************
****************************************************************
*/

// defaults
VioMtcGeneratorLayout::VioMtcGeneratorLayout(QObject* parent) : VioGeneratorLayout(parent) {
  // have a sensible default
  for(int i = VioFirstColor; i < VioStyle::Colors(); i++) {
    mColorList.append(VioStyle::ColorName(i));
    mColorMap[VioStyle::ColorName(i)]=i;
  }
}

// assignment
VioMtcGeneratorLayout& VioMtcGeneratorLayout::operator=(const VioMtcGeneratorLayout& rSrc) {
  VioGeneratorLayout::operator=(rSrc);
  mColorMap = rSrc.mColorMap;
  mColorList = rSrc.mColorList;
  return *this;
}  

// assignment
VioMtcGeneratorLayout& VioMtcGeneratorLayout::operator=(const VioGeneratorLayout& rSrc) {
  VioGeneratorLayout::operator=(rSrc);
  mColorMap.clear();
  mColorList.clear();
  return *this;
}  

// access
const QMap<QString, int>& VioMtcGeneratorLayout::ColorMap(void) const {
  return mColorMap;}

// access
const QList<QString>& VioMtcGeneratorLayout::ColorList(void) const{
  return mColorList;}

// set colors
void VioMtcGeneratorLayout::VioColors(const QList<QString>& rColorList, const QMap<QString,int>& rColorMap) {
  FD_DQT("VioMtcGenerator::VioColors(): set #" << rColorMap.size());
  if(rColorMap==mColorMap && rColorList==mColorList) return; 
  if(rColorList.size()!=rColorMap.size()) return; 
  FD_DQT("VioMtcGeneratorLayout::ColorMap(): change"); 
  mColorMap=rColorMap;
  mColorList=rColorList;
}


// write layout
void VioMtcGeneratorLayout::WriteCore(faudes::TokenWriter& rTw) const {
  // write base
  VioGeneratorLayout::WriteCore(rTw);
  // my data: color map
  rTw.WriteBegin("ColorMap");
  int oldcol=rTw.Columns();
  rTw.Columns(2);
  for(int i=0; i< mColorList.size(); i++) {
    rTw.WriteString(VioStyle::StrFromQStr(mColorList.at(i)));
    rTw.WriteString(VioStyle::StrFromQStr(VioStyle::ColorName(mColorMap[mColorList.at(i)])));
  }  
  rTw.WriteEnd("ColorMap");
}

// read layout 
void VioMtcGeneratorLayout::ReadCore(faudes::TokenReader& rTr) {
  // read base
  VioGeneratorLayout::ReadCore(rTr);
  // my data: color map
  faudes::Token token;
  rTr.Peek(token);
  if(token.Type()!=faudes::Token::Begin) return;
  if(token.StringValue()!="ColorMap") return;
  mColorMap.clear();
  mColorList.clear();
  rTr.ReadBegin("ColorMap");
  while(!rTr.Eos("ColorMap")) {
      QString markname = VioStyle::QStrFromStr(rTr.ReadString());
      QString colname = VioStyle::QStrFromStr(rTr.ReadString());
      mColorMap.insert(markname,VioStyle::ColorIndex(colname));
      mColorList.append(markname);
  }
  rTr.ReadEnd("ColorMap");
}

// write layout section
void VioMtcGeneratorLayout::Write(faudes::TokenWriter& rTw) const {
  rTw.WriteBegin("VioLayout");
  WriteCore(rTw);
  rTw.WriteEnd("VioLayout");
}

// read layout 
void VioMtcGeneratorLayout::Read(faudes::TokenReader& rTr) {
  try {
    rTr.ReadBegin("VioLayout");
    ReadCore(rTr);
    rTr.ReadEnd("VioLayout");
  } catch(faudes::Exception expection) {
  } 
  FD_DQG("VioMtcGeneratorLayout::Read(): scale " << mGraphScale);
}


/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioMtcGeneratorModel

****************************************************************
****************************************************************
****************************************************************
*/

// construct
VioMtcGeneratorModel::VioMtcGeneratorModel(QObject* parent, VioStyle* config, bool alloc) : 
  VioGeneratorModel(parent,config,false),
  pMtcGeneratorConfig(0)
{
  FD_DQT("VioMtcGeneratorModel::VioMtcGeneratorModel(): " << VioStyle::StrFromQStr(mFaudesType));
  // typed version of configuration
  pMtcGeneratorConfig = dynamic_cast<VioMtcGeneratorStyle*>(pConfig);
  if(!pMtcGeneratorConfig) {
    FD_WARN("VioMtcGeneratorModel::VioMtcGeneratorModel(): invalid style, using default.");
    pGeneratorConfig= new VioMtcGeneratorStyle(mFaudesType);
  }
  // allocate faudes object and have default rep data
  if(alloc) {
    DoVioAllocate();
    DoVioUpdate();
  }
  FD_DQT("VioMtcGeneratorModel::VioMtcGeneratorModel(): done");
}

// destruct
VioMtcGeneratorModel::~VioMtcGeneratorModel(void) {
}

// construct on heap
VioMtcGeneratorModel* VioMtcGeneratorModel::NewModel(QObject* parent) const {
  FD_DQT("VioMtcGeneratorModel::New(): type " << VioStyle::StrFromQStr(mFaudesType));
  return new VioMtcGeneratorModel(parent,pConfig);
}

// construct view on heap
VioView* VioMtcGeneratorModel::NewView(QWidget* parent) const {
  FD_DQT("VioMtcGeneratorModel::NewView(): " << VioStyle::StrFromQStr(FaudesType()));
  return new VioMtcGeneratorView(parent, pConfig);
}

// construct property view on heap 
VioView* VioMtcGeneratorModel::NewPropertyView(QWidget* parent) const {
  FD_DQT("VioMtcGeneratorModel::NewPropertyView(): " << VioStyle::StrFromQStr(FaudesType()));
  return new VioGeneratorPropertyView(parent, pConfig);
}

// construct property user config on heap (cannot do so)
VioView* VioMtcGeneratorModel::NewConfigView(QWidget* parent) const {
  FD_DQT("VioMtcGeneratorModel::NewConfigView(): " << VioStyle::StrFromQStr(FaudesType()));
  return new VioMtcGeneratorConfigView(parent, pConfig);
}

// construct on heap
VioWidget* VioMtcGeneratorModel::NewWidget(QWidget* parent) const {
  FD_DQT("VioMtcGeneratorModel::NewWidget(): type " << VioStyle::StrFromQStr(FaudesType()));
  return new VioMtcGeneratorWidget(parent, pConfig);
}

// allocate faudes object
void VioMtcGeneratorModel::DoFaudesAllocate(void) {
  FD_DQT("VioMtcGeneratorModel::DoFaudesAllocate(): fobject at " << mData->FaudesObject());
  // call vio base
  VioModel::DoFaudesAllocate();
  // test my requirements
  if(DoTypeCheck(mData->FaudesObject())) {
    FD_WARN("VioMtcGeneratorModel::DoFaudesAllocate: incompatible fobject, using fallback");
    mData->FaudesObject(new faudes::MtcSystem());   
  }
  // done
  FD_DQT("VioMtcGeneratorModel::DoFaudesAllocate(): ftype " << VioStyle::StrFromQStr(mFaudesType)
	 << " ctype " << typeid(*mData->FaudesObject()).name() << " at " << mData->FaudesObject());
}

// allocate visual model data
void VioMtcGeneratorModel::DoVioAllocate(void) {
  FD_DQT("VioMtcGeneratorModel::DoVioAllocate()");
  // call base
  VioGeneratorModel::DoVioAllocate();
  // set up mtc extended user layout
  FD_DQT("VioMtcGeneratorModel::DoVioAllocate(): default user layout");
  pMtcUserLayout = new VioMtcGeneratorLayout(this);
  mpUserLayout=pMtcUserLayout;
  FD_DQT("VioMtcGeneratorModel::DoVioAllocate(): default user layout: done");
}


// set layout features
void VioMtcGeneratorModel::VioColors(const QList<QString>& rColorList, const QMap<QString,int>& rColorMap) {
  pMtcUserLayout->VioColors(rColorList,rColorMap);
  emit NotifyAnyAttr();
}

/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioMtcGeneratorView

****************************************************************
****************************************************************
****************************************************************
*/

// construct 
VioMtcGeneratorView::VioMtcGeneratorView(QWidget* parent, VioStyle* config, bool alloc) : 
  VioGeneratorView(parent,config,false),
  pMtcGeneratorConfig(0)
{
  FD_DQT("VioMtcGeneratorView::VioMtcGeneratorView(): " << VioStyle::StrFromQStr(FaudesType()));
  // typed version of configuration
  pMtcGeneratorConfig = dynamic_cast<VioMtcGeneratorStyle*>(pConfig);
  if(!pMtcGeneratorConfig) {
    FD_WARN("VioMtcGeneratorModel::VioMtcGeneratorModel(): invalid style, using default.");
    pMtcGeneratorConfig= new VioMtcGeneratorStyle(mFaudesType);
  }
  // my alloc
  if(alloc) DoVioAllocate();
  FD_DQT("VioMtcGeneratorView::VioMtcGeneratorView(): done");
}

// destruct
VioMtcGeneratorView::~VioMtcGeneratorView(void) {
}

// update widget from (new) model
void VioMtcGeneratorView::DoVioUpdate(void) {
  FD_DQT("VioMtcGeneratorView::DoVioUpdate()");
  // call base
  VioGeneratorView::DoVioUpdate();
  // have typed ref
  pMtcGeneratorModel = qobject_cast<VioMtcGeneratorModel*>(pModel);
  FD_DQT("VioMtcGeneratorView::DoVioUpdate(): done");
  // bail out in void
  if(!pMtcGeneratorModel) return; 
}

// set by vio model
int VioMtcGeneratorView::Model(VioModel* model) {
  FD_DQT("VioMtcGeneratorView::Model(" << model << ")");
  // bail out on identity
  if(model==pModel) return 0;
  // my disconnect

  // call base (incl virtual DoVioUpdate)
  if(VioGeneratorView::Model(model)) return 1;
  // my connect 

  // done
  return 0;
}

// show/hide views from layout
void VioMtcGeneratorView::UpdateUserLayout(void) {
  FD_DQT("VioMtcGeneratorView::UpdateUserLayout(): stage " << mUserLayout->mToggleStage << " scale " << mUserLayout->mGraphScale );
  // call base
  VioGeneratorView::UpdateUserLayout();
}

// save layout to model 
void VioMtcGeneratorView::SaveUserLayout(void) {
  FD_DQT("VioMtcGeneratorView::SaveUserLayout()");
  // call base
  VioGeneratorView::SaveUserLayout();
}



/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioMtcGeneratorConfigView

****************************************************************
****************************************************************
****************************************************************
*/

// construct
VioMtcGeneratorConfigView::VioMtcGeneratorConfigView(QWidget* parent, VioStyle* config, bool alloc) : 
  VioView(parent,config,false),
  mColorColumns(0)
{
  FD_DQT("VioMtcGeneratorConfigView::VioMtcGeneratorConfigView(): " << VioStyle::StrFromQStr(FaudesType()));
  // my alloc
  if(alloc) DoVioAllocate();
  FD_DQT("VioMtcGeneratorConfigView::VioMtcGeneratorConfigView(): done");
}

// destruct
VioMtcGeneratorConfigView::~VioMtcGeneratorConfigView(void) {
}

// allocate view data
void VioMtcGeneratorConfigView::DoVioAllocate(void) {
  FD_DQT("VioMtcGeneratorConfigView::DoVioAllocate()");
  // call base
  VioView::DoVioAllocate();
  // add my features
  mColorColumns= new VioSymbolTableWidget();
  mColorColumns->setDimensions(1,2);
  mColorColumns->setHeader(QStringList() << "Markings" << "Colors");
  mVbox->addWidget(mColorColumns);
  connect(mColorColumns,SIGNAL(editingFinished(int,int)),this,SLOT(UpdateFromColorColumns(int,int)));
  connect(mColorColumns,SIGNAL(resizeModel()),this,SLOT(UpdateFromColorColumns()));
  FD_DQT("VioMtcGeneratorConfigView::DoVioAllocate(): done");
}

// update view from (new) model
void VioMtcGeneratorConfigView::DoVioUpdate(void) {
  FD_DQT("VioMtcGeneratorConfigView::DoVioUpdate(): type " << VioStyle::StrFromQStr(FaudesType()));
  // call base
  VioView::DoVioUpdate();
  // have typed model reference (incl set to 0)
  pMtcGeneratorModel=qobject_cast<VioMtcGeneratorModel*>(pModel);
  // bail out on no model
  if(!pMtcGeneratorModel) return;
  // report
  FD_DQT("VioMtcGeneratorConfigView::DoVioUupdate: color map #" << pMtcGeneratorModel->Layout().ColorMap().size());
  // update completers
  mColorColumns->setSymbolWorld(1,Configuration()->ColorNames());
  mColorColumns->setSymbolMode(1,VioSymbol::KnownSymbolsOnly /* | VioSymbol::ComboBox  */ );
  // update colorcolumn: add colormap entries;
  mColorColumns->setDimensions(pMtcGeneratorModel->Layout().ColorList().size(),2);
  for(int j=0; j< pMtcGeneratorModel->Layout().ColorList().size(); j++) {
    QString markname =  pMtcGeneratorModel->Layout().ColorList().at(j);
    mColorColumns->setSymbol(j,0,markname);
    QString colname = VioStyle::ColorName(
     pMtcGeneratorModel->Layout().ColorMap()[markname] );
    mColorColumns->setSymbol(j,1,colname); 
  }
  FD_DQT("VioMtcGeneratorConfigView::DoVioUpdate(): done");
}

// color set has been changed
void VioMtcGeneratorConfigView::UpdateFromColorColumns(int row, int col) {
  FD_DQT("VioMtcGeneratorConfigView::UpdateFromColorColumns()");
  // bail out if widget is not ready or model not there
  if(!mColorColumns || !pMtcGeneratorModel) return;
  // retrieve entire table
  QMap<QString,int> newmap;
  QList<QString> newlist;
  for(int j=0; j< mColorColumns->rowCount(); j++) {
    QString markname = mColorColumns->Symbol(j,0);
    if(markname=="") continue;
    QString colname = mColorColumns->Symbol(j,1);
    if(colname=="") continue;
    newmap.insert(markname,VioStyle::ColorIndex(colname));
    newlist.append(markname);
  }
  // set model
  pMtcGeneratorModel->VioColors(newlist,newmap); 
  // set changed
  Modified(true);
  FD_DQT("VioMtcGeneratorConfigView::UpdateFromColorColumns(): done");
}



// update model from view
void VioMtcGeneratorConfigView::DoModelUpdate(void) {
  FD_DQT("VioMtcGeneratorConfigView::DoModelUpdate(): type " << VioStyle::StrFromQStr(FaudesType()));
  // call base
  // VioView::DoModelUpdate();
  // bail out if widget is not ready or model not there
  if(!mColorColumns || !pModel) return;
  // update my features
  UpdateFromColorColumns();
}


/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioMtcGeneratorWidget

****************************************************************
****************************************************************
****************************************************************
*/

// construct (we dont call the base class, sinc we have a different layout)
VioMtcGeneratorWidget::VioMtcGeneratorWidget(QWidget* parent, VioStyle* config, bool alloc) : 
  VioGeneratorWidget(parent,config,false) // dont allocate
{
  FD_DQT("VioMtcGeneratorWidget::VioMtcGeneratorWidget(): " << VioStyle::StrFromQStr(pConfig->ConfigName()));
  // allocate model and view
 if(alloc) {
    // have view
    mView= new VioMtcGeneratorView(this,pConfig);
    mVbox->addWidget(mView);
    // fix typed refs and connect to widget
    DoVioAllocate();
    // have a model and set
    Model(new VioMtcGeneratorModel(this,pConfig));
  }
  // fix modified flag
  mModified=false;
  // done
  FD_DQT("VioMtcGeneratorWidget::VioMtcGeneratorWidget(): done");
}

// destruct
VioMtcGeneratorWidget::~VioMtcGeneratorWidget(void) {
}

// fix view
void VioMtcGeneratorWidget::DoVioAllocate(void) {
  // let base fix tyoed refs and connect
  VioGeneratorWidget::DoVioAllocate();
  // fix typed refs
  pMtcGeneratorView=qobject_cast<VioMtcGeneratorView*>(mView);
  // connect view
  FD_DQT("VioMtcGeneratorWidget::DoModelAllocate(): connect");
  FD_DQT("VioMtcGeneratorWidget::DoVioAllocate(): done");
}

// set by vio model
int VioMtcGeneratorWidget::Model(VioModel* model) {
  FD_DQT("VioMtcGeneratorWidget::Model(" << model << "): type " << VioStyle::StrFromQStr(FaudesType()));
  // bail out on identity
  if(model==mModel) return 0;
  // call base to set view and connects
  int res=VioGeneratorWidget::Model(model);
  // update typed ref
  pMtcGeneratorModel=qobject_cast<VioMtcGeneratorModel*>(mModel);
  FD_DQT("VioMtcGeneratorWidget::Model(" << model << "): done");  
  return res;
}



