/* vioattribute.cpp  - vio attribute model and widget */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2009  Thomas Moor, Ruediger Berndt

*/


#include "viomtcstateattr.h"
#include "viomtcgenerator.h"

/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioAttributeMtcStateModel

****************************************************************
****************************************************************
****************************************************************
*/

// construct
VioAttributeMtcStateModel::VioAttributeMtcStateModel(QObject* parent, VioStyle* config, bool alloc) : 
  VioAttributeModel(parent, config, false),
  mpFaudesColoredState(0) 
{
  FD_DQT("VioAttributMtcStateModel::VioAttributeMtcStateModel(): " << VioStyle::StrFromQStr(mFaudesType));
  // allocate faudes object and have default rep data
  if(alloc) {
    DoVioAllocate();
    DoVioUpdate();
  }
  FD_DQT("VioAttributModel::VioAttributeMtcStateModel(): done");
}

// destruct
VioAttributeMtcStateModel::~VioAttributeMtcStateModel(void) {
}

// construct on heap
VioAttributeMtcStateModel* VioAttributeMtcStateModel::NewModel(QObject* parent) const {
  FD_DQT("VioAttributeMtcStateModel::NewModel(): type " << VioStyle::StrFromQStr(mFaudesType)); 
  return new VioAttributeMtcStateModel(parent,pConfig);
}

// construct view on heap
VioView* VioAttributeMtcStateModel::NewView(QWidget* parent) const {
  FD_DQT("VioAttributeMtcStateModel::NewView(): " << VioStyle::StrFromQStr(FaudesType()));
  return new VioAttributeMtcStateView(parent, pConfig);
}

// construct on heap
VioWidget* VioAttributeMtcStateModel::NewWidget(QWidget* parent) const {
  FD_DQT("VioAttributeMtcStateMode::NewWidget(): type " << VioStyle::StrFromQStr(FaudesType()));
  return new VioAttributeMtcStateWidget(parent, pConfig);
}


// allocate faudes object
void VioAttributeMtcStateModel::DoFaudesAllocate(void) {
  FD_DQT("VioAttributeMtcStateModel::DoFaudesAllocate()");
  // let base handle this
  VioAttributeModel::DoFaudesAllocate();
  // impose my requirements
  if(DoTypeCheck(mData->FaudesObject())) {
    FD_DQT("VioAttributeMtcStateModel::DoFaudesAllocate(): fallback ctype");
    mData->FaudesObject(new faudes::AttributeColoredState());  
  }
  FD_DQT("VioAttributeMtcStateModel::DoFaudesAllocate(): ctype " << typeid(*mData->FaudesObject()).name());
}

// test whether we can host this faudes object
int VioAttributeMtcStateModel::DoTypeCheck(const faudes::Type* fobject) const {  
  // we host anything that casts to flags Attribute
  if(dynamic_cast<const faudes::AttributeColoredState*>(fobject)) return 0;
  return 1; 
}

// update visual data from (new) faudes object (incl layout change TODO: signals)
void VioAttributeMtcStateModel::DoVioUpdate(void) {
  FD_DQT("VioAttributeMtcStateModel::DoVioUpdate()");
  // have my typed reference
  mpFaudesColoredState = dynamic_cast<faudes::AttributeColoredState*>(mData->FaudesObject());
  // call base for actual update (incl notify view)
  VioAttributeModel::DoVioUpdate();
}



// (user) edit marking
void VioAttributeMtcStateModel::VioMarking(const faudes::ColorSet& rMarking) {
  FD_DQT("VioAttributeMtcStateModel::VioMarking(): set #" << rMarking.Size() ); 
  // detect changes
  if(rMarking == mpFaudesColoredState->Colors()) return;
  // set/tell
  FD_DQT("VioAttributeMtcStateModel::VioMarking(): color change"); 
  // write to faudes attribute 
  mpFaudesColoredState->Colors()= rMarking;
  // update debugging widgets
#ifdef FAUDES_DEBUG_VIO_WIDGETS
  mVioText=VioStyle::QStrFromStr(mpFaudesColoredState->ToString());
#endif
  // notify view
  emit NotifyAnyChange();
  FD_DQT("VioAttributeMtcStateModel::VioMarking(): done"); 
}


// merge: clear
int VioAttributeMtcStateModel::MergeClear(void) {
  // base handles flags
  VioAttributeModel::MergeClear();
  return 0;
}

// merge: insert
void VioAttributeMtcStateModel::MergeInsert(const faudes::AttributeFlags* fattr) {
  // base handles flags
  VioAttributeModel::MergeInsert(fattr);
}
 

// partial assign
void VioAttributeMtcStateModel::MergeAssign(faudes::AttributeFlags* fattr) {
  // base handles flags
  VioAttributeModel::MergeAssign(fattr);
}


/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioAttributeMtcStateView

****************************************************************
****************************************************************
****************************************************************
*/

// construct
VioAttributeMtcStateView::VioAttributeMtcStateView(QWidget* parent, VioStyle* config, bool alloc) : 
  VioAttributeView(parent,config,false),
  mColorColumns(0)
{
  FD_DQT("VioAttributeMtcStateView::VioAttributeMtcStateView(): " << VioStyle::StrFromQStr(FaudesType()));
  // my alloc
  if(alloc) DoVioAllocate();
  FD_DQT("VioAttributeMtcStateView::VioAttributeMtcStateView(): done");
}

// destruct
VioAttributeMtcStateView::~VioAttributeMtcStateView(void) {
}

// typed faudes object access
const faudes::AttributeColoredState* VioAttributeMtcStateView::AttributeColoredState(void) const {
  if(!pAttributeMtcStateModel) return 0;
  return pAttributeMtcStateModel->AttributeColoredState();
}

// allocate view data
void VioAttributeMtcStateView::DoVioAllocate(void) {
  FD_DQT("VioAttributeMtcStateView::DoVioAllocate()");
  // call base
  VioAttributeView::DoVioAllocate();
  // add my features
  mColorColumns= new VioSymbolTableWidget();
  mColorColumns->setDimensions(1,1);
  mColorColumns->setHeader(QStringList() << "Markings");
  mVbox->addWidget(mColorColumns);
  connect(mColorColumns,SIGNAL(editingFinished(int,int)),this,SLOT(UpdateFromColorColumns(int,int)));
  connect(mColorColumns,SIGNAL(resizeModel()),this,SLOT(UpdateFromColorColumns()));
  FD_DQT("VioAttributeMtcStateView::DoVioAllocate(): done");
}

// update view from (new) model
void VioAttributeMtcStateView::DoVioUpdate(void) {
  FD_DQT("VioAttributeMtcStateView::DoVioUpdate(): type " << VioStyle::StrFromQStr(FaudesType()));
  // call base
  VioAttributeView::DoVioUpdate();
  // have typed model reference (incl set to 0)
  pAttributeMtcStateModel=qobject_cast<VioAttributeMtcStateModel*>(pModel);
  // bail out on no model
  if(!pAttributeMtcStateModel) return;
  // report
  FD_DQT("VioAttributeMtcStateView::DoVioUupdate: markings #" << pAttributeMtcStateModel->VioMarking().Size());
  // update colorcolumn: add colormap entries;
  mColorColumns->setDimensions(pAttributeMtcStateModel->VioMarking().Size(),1);
  faudes::NameSet::Iterator cit= pAttributeMtcStateModel->VioMarking().Begin();
  int j=0;
  for(;cit!= pAttributeMtcStateModel->VioMarking().End();cit++, j++){
    QString markname =  VioStyle::QStrFromStr(pAttributeMtcStateModel->VioMarking().SymbolicName(*cit));
    mColorColumns->setSymbol(j,0,markname);
  }
  // disable merge unsupported
  mColorColumns->setEnabled(!pAttributeMtcStateModel->Merged());
  FD_DQT("VioAttributeMtcStateView::DoVioUpdate(): done");
}

// color set has been changed
void VioAttributeMtcStateView::UpdateFromColorColumns(int row, int col) {
  FD_DQT("VioAtributeMtcStateView::UpdateFromColorColumns()");
  // bail out if widget is not ready or model not there
  if(!mColorColumns || !pAttributeMtcStateModel) return;
  // update marking: extract marking
  faudes::ColorSet marking;
  for(int j=0; j< mColorColumns->rowCount(); j++) {
    QString markname = mColorColumns->Symbol(j,0);
    if(markname=="") continue;
    marking.Insert(VioStyle::StrFromQStr(markname));
  }
  FD_DQT("VioAtributeMtcStateView::UpdateFromColorColumns(): found marking #" << marking.Size());
  // set model
  pAttributeMtcStateModel->VioMarking(marking); 
  // set changed
  Modified(true);
  FD_DQT("VioAtributeMtcStateView::UpdateFromColorColumns(): done");
}



// update model from view
void VioAttributeMtcStateView::DoModelUpdate(void) {
  FD_DQT("VioAttributeMtcStateView::DoModelUpdate(): type " << VioStyle::StrFromQStr(FaudesType()));
  // call base
  VioAttributeView::DoModelUpdate();
  // bail out if widget is not ready or model not there
  if(!mColorColumns || !pModel) return;
  // update my features
  UpdateFromColorColumns();
}


/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioAttributeMtcStateWidget

****************************************************************
****************************************************************
****************************************************************
*/

// construct
VioAttributeMtcStateWidget::VioAttributeMtcStateWidget(QWidget* parent, VioStyle* config, bool alloc) : 
  VioAttributeWidget(parent,config,false) 
{
  FD_DQT("VioAttributeMtcStateWidget::VioAttributeMtcStateWidget(): " << VioStyle::StrFromQStr(pConfig->ConfigName()));
  // allocate model and view
  if(alloc) {
    // have view
    mView= new VioAttributeMtcStateView(this,pConfig);
    mVbox->addWidget(mView);
    // fix typed refs and connect to widget
    DoVioAllocate();
    // have a model and set
    Model(new VioAttributeMtcStateModel(this,pConfig));
  }
  // fix modified flag
  mModified=false;
  FD_DQT("VioAttributeMtcStateWidget::VioAttributeMtcStateWidget(): done");
}

// destruct
VioAttributeMtcStateWidget::~VioAttributeMtcStateWidget(void) {
}

// fix view
void VioAttributeMtcStateWidget::DoVioAllocate(void) {
  // call base to connect
  VioAttributeWidget::DoVioAllocate();
}

// set by vio model
int VioAttributeMtcStateWidget::Model(VioModel* model) {
  FD_DQT("VioAttributeMtcState::Model(" << model << "): type " << VioStyle::StrFromQStr(FaudesType()));
  // bail out on identity
  if(model==mModel) return 0;
  // call base to set view and connects (incl callback diovioupdate)
  int res=VioAttributeWidget::Model(model);
  // update typed ref
  pAttributeMtcStateModel=qobject_cast<VioAttributeMtcStateModel*>(mModel);
  // done
  FD_DQT("VioAttributeMtcState::Model(" << model << "): done");  
  return res;
}











