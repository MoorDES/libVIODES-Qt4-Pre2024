/** viodiagstateattr.cpp  - vio attribute for faudes diagnoser label  */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2010 Thomas Moor

*/


#include "viodiagstateattr.h"

/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioAttributeDiagStateModel

****************************************************************
****************************************************************
****************************************************************
*/

// construct
VioAttributeDiagStateModel::VioAttributeDiagStateModel(QObject* parent, VioStyle* config, bool alloc) : 
  VioAttributeModel(parent, config, false),
  mpFaudesDiagnoserState(0) 
{
  FD_DQT("VioAttributDiagStateModel::VioAttributeDiagStateModel(): " << VioStyle::StrFromQStr(mFaudesType));
  // allocate faudes object and have default rep data
  if(alloc) {
    DoVioAllocate();
    DoVioUpdate();
  }
  FD_DQT("VioAttributModel::VioAttributeDiagStateModel(): done");
}

// destruct
VioAttributeDiagStateModel::~VioAttributeDiagStateModel(void) {
}

// construct on heap
VioAttributeDiagStateModel* VioAttributeDiagStateModel::NewModel(QObject* parent) const {
  FD_DQT("VioAttributeDiagStateModel::NewModel(): type " << VioStyle::StrFromQStr(mFaudesType)); 
  return new VioAttributeDiagStateModel(parent,pConfig);
}

// construct view on heap
VioView* VioAttributeDiagStateModel::NewView(QWidget* parent) const {
  FD_DQT("VioAttributeDiagStateModel::NewView(): " << VioStyle::StrFromQStr(FaudesType()));
  return new VioAttributeDiagStateView(parent, pConfig);
}

// construct widget on heap
VioWidget* VioAttributeDiagStateModel::NewWidget(QWidget* parent) const {
  FD_DQT("VioAttributeDiagStateMode::NewWidget(): type " << VioStyle::StrFromQStr(FaudesType()));
  return new VioAttributeDiagStateWidget(parent, pConfig);
}


// allocate faudes object
void VioAttributeDiagStateModel::DoFaudesAllocate(void) {
  FD_DQT("VioAttributeDiagStateModel::DoFaudesAllocate()");
  // let base handle this
  VioAttributeModel::DoFaudesAllocate();
  // impose my requirements
  if(DoTypeCheck(mData->FaudesObject())) {
    FD_DQT("VioAttributeDiagStateModel::DoFaudesAllocate(): fallback ctype");
    mData->FaudesObject(new faudes::AttributeDiagnoserState());  
  }
}

// test whether we can host this faudes object
int VioAttributeDiagStateModel::DoTypeCheck(const faudes::Type* fobject) const {  
  // we host anything that casts to flags Attribute
  if(dynamic_cast<const faudes::AttributeDiagnoserState*>(fobject)) return 0;
  return 1; 
}

// update visual data from (new) faudes object 
void VioAttributeDiagStateModel::DoVioUpdate(void) {
  FD_DQT("VioAttributeDiagStateModel::DoVioUpdate()");
  // have my typed reference
  mpFaudesDiagnoserState = dynamic_cast<faudes::AttributeDiagnoserState*>(mData->FaudesObject());
  // call base for actual update regarding flags (incl notify view)
  VioAttributeModel::DoVioUpdate();
}



/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioAttributeDiagStateView

****************************************************************
****************************************************************
****************************************************************
*/

// construct
VioAttributeDiagStateView::VioAttributeDiagStateView(QWidget* parent, VioStyle* config, bool alloc) : 
  VioAttributeView(parent,config,false),
  mStateLabel(0)
{
  FD_DQT("VioAttributeDiagStateView::VioAttributeDiagStateView(): " << VioStyle::StrFromQStr(FaudesType()));
  // my alloc
  if(alloc) DoVioAllocate();
  FD_DQT("VioAttributeDiagStateView::VioAttributeDiagStateView(): done");
}

// destruct
VioAttributeDiagStateView::~VioAttributeDiagStateView(void) {
}

// typed faudes object access
const faudes::AttributeDiagnoserState* VioAttributeDiagStateView::AttributeDiagnoserState(void) const {
  if(!pAttributeDiagStateModel) return 0;
  return pAttributeDiagStateModel->AttributeDiagnoserState();
}

// allocate view data
void VioAttributeDiagStateView::DoVioAllocate(void) {
  FD_DQT("VioAttributeDiagStateView::DoVioAllocate()");
  // call base
  VioAttributeView::DoVioAllocate();
  // add my features
  mStateLabel= new QLineEdit("--");
  mStateLabel->setReadOnly(true);
  QLabel* head = new QLabel("Diagnoser Lable");
  QVBoxLayout* vbox = new QVBoxLayout();
  vbox->setMargin(0);
  vbox->setSpacing(2);
  vbox->addWidget(head);
  vbox->addWidget(mStateLabel);
  mVbox->addSpacing(5);
  mVbox->addLayout(vbox);
  mVbox->addStretch(1); 
  FD_DQT("VioAttributeDiagStateView::DoVioAllocate(): done");
}

// update view from (new) model
void VioAttributeDiagStateView::DoVioUpdate(void) {
  FD_DQT("VioAttributeDiagStateView::DoVioUpdate(): type " << VioStyle::StrFromQStr(FaudesType()));
  // call base
  VioAttributeView::DoVioUpdate();
  // have typed model reference (incl set to 0)
  pAttributeDiagStateModel=qobject_cast<VioAttributeDiagStateModel*>(pModel);
  // bail out on no model
  if(!pAttributeDiagStateModel) return;
  // update the label
  QString dlabel = 
    VioStyle::QStrFromStr(pAttributeDiagStateModel->AttributeDiagnoserState()->Str());
  if(dlabel=="") dlabel="--";
  mStateLabel->setText(dlabel);
  FD_DQT("VioAttributeDiagStateView::DoVioUpdate(): done");
}


// update model from view
void VioAttributeDiagStateView::DoModelUpdate(void) {
  FD_DQT("VioAttributeDiagStateView::DoModelUpdate(): type " << VioStyle::StrFromQStr(FaudesType()));
  // call base
  VioAttributeView::DoModelUpdate();
  // bail out if widget is not ready or model not there
  if(!mStateLabel || !pModel) return;
  // refuse, we are read only
}


/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioAttributeDiagStateWidget

****************************************************************
****************************************************************
****************************************************************
*/

// construct
VioAttributeDiagStateWidget::VioAttributeDiagStateWidget(QWidget* parent, VioStyle* config, bool alloc) : 
  VioAttributeWidget(parent,config,false) 
{
  FD_DQT("VioAttributeDiagStateWidget::VioAttributeDiagStateWidget(): " << VioStyle::StrFromQStr(pConfig->ConfigName()));
  // allocate model and view
  if(alloc) {
    // have view
    mView= new VioAttributeDiagStateView(this,pConfig);
    mVbox->addWidget(mView);
    // fix typed refs and connect to widget
    DoVioAllocate();
    // have a model and set
    Model(new VioAttributeDiagStateModel(this,pConfig));
  }
  // fix modified flag
  mModified=false;
  FD_DQT("VioAttributeDiagStateWidget::VioAttributeDiagStateWidget(): done");
}

// destruct
VioAttributeDiagStateWidget::~VioAttributeDiagStateWidget(void) {
}

// fix view
void VioAttributeDiagStateWidget::DoVioAllocate(void) {
  // call base to connect
  VioAttributeWidget::DoVioAllocate();
}

// set by vio model
int VioAttributeDiagStateWidget::Model(VioModel* model) {
  FD_DQT("VioAttributeDiagState::Model(" << model << "): type " << VioStyle::StrFromQStr(FaudesType()));
  // bail out on identity
  if(model==mModel) return 0;
  // call base to set view and connects (incl callback diovioupdate)
  int res=VioAttributeWidget::Model(model);
  // update typed ref
  pAttributeDiagStateModel=qobject_cast<VioAttributeDiagStateModel*>(mModel);
  // done
  FD_DQT("VioAttributeDiagState::Model(" << model << "): done");  
  return res;
}











