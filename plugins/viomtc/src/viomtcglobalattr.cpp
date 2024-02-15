/* vioattribute.cpp  - vio attribute model and widget */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2010  Thomas Moor

*/


#include "viomtcglobalattr.h"
#include "viomtcgenerator.h"

/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioAttributeMtcGlobalModel

****************************************************************
****************************************************************
****************************************************************
*/

// construct
VioAttributeMtcGlobalModel::VioAttributeMtcGlobalModel(QObject* parent, VioStyle* config, bool alloc) : 
  VioAttributeModel(parent, config, false)
{
  FD_DQT("VioAttributMtcGlobalModel::VioAttributeMtcGlobalModel(): " << VioStyle::StrFromQStr(mFaudesType));
  // allocate faudes object and have default rep data
  if(alloc) {
    DoVioAllocate();
    DoVioUpdate();
  }
  FD_DQT("VioAttributModel::VioAttributeMtcGlobalModel(): done");
}

// destruct
VioAttributeMtcGlobalModel::~VioAttributeMtcGlobalModel(void) {
}

// construct on heap
VioAttributeMtcGlobalModel* VioAttributeMtcGlobalModel::NewModel(QObject* parent) const {
  FD_DQT("VioAttributeMtcGlobalModel::NewModel(): type " << VioStyle::StrFromQStr(mFaudesType)); 
  return new VioAttributeMtcGlobalModel(parent,pConfig);
}

// construct view on heap
VioView* VioAttributeMtcGlobalModel::NewView(QWidget* parent) const {
  FD_DQT("VioAttributeMtcGlobalModel::NewView(): " << VioStyle::StrFromQStr(FaudesType()));
  return new VioAttributeMtcGlobalView(parent, pConfig);
}

// construct on heap
VioWidget* VioAttributeMtcGlobalModel::NewWidget(QWidget* parent) const {
  FD_DQT("VioAttributeMtcGlobalMode::NewWidget(): type " << VioStyle::StrFromQStr(FaudesType()));
  return new VioAttributeMtcGlobalWidget(parent, pConfig);
}


// allocat visual 
void VioAttributeMtcGlobalModel::DoVioAllocate(void) {
  FD_DQT("VioAttributeMtcGlobalModel::DoVioAllocate()");
  // give base a chance
  VioAttributeModel::DoVioAllocate();
}


// update visual data from (new) faudes object 
void VioAttributeMtcGlobalModel::DoVioUpdate(void) {
  FD_DQT("VioAttributeMtcGlobalModel::DoVioUpdate()");
  // call base for actual update (incl notify view)
  VioAttributeModel::DoVioUpdate();
}

// token io
void VioAttributeMtcGlobalModel::DoVioWrite(faudes::TokenWriter& rTw) const {
  FD_DQT("VioAttributeMtcGlobalModel::DoVioWrite()");
  // my data: color map
  rTw.WriteBegin("VioData");
  rTw.WriteEnd("VioData");
  rTw.Columns(oldcol);
}

// token io
void VioAttributeMtcGlobalModel::DoVioRead(faudes::TokenReader& rTr) {
  FD_DQT("VioAttributeMtcGlobalModel::DoVioRead()");
  // my data: color map
  mColorMap.clear();
  mColorList.clear();
  rTr.ReadEnd("VioData");
}




/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioAttributeMtcGlobalWidget

****************************************************************
****************************************************************
****************************************************************
*/

// construct
VioAttributeMtcGlobalWidget::VioAttributeMtcGlobalWidget(QWidget* parent, VioStyle* config, bool alloc) : 
  VioAttributeWidget(parent,config,false) 
{
  FD_DQT("VioAttributeMtcGlobalWidget::VioAttributeMtcGlobalWidget(): " << VioStyle::StrFromQStr(pConfig->ConfigName()));
  // allocate model and view
  if(alloc) {
    // have view
    mView= new VioAttributeMtcGlobalView(this,pConfig);
    mVbox->addWidget(mView);
    // fix typed refs and connect to widget
    DoVioAllocate();
    // have a model and set
    Model(new VioAttributeMtcGlobalModel(this,pConfig));
  }
  // fix modified flag
  mModified=false;
  FD_DQT("VioAttributeMtcGlobalWidget::VioAttributeMtcGlobalWidget(): done");
}

// destruct
VioAttributeMtcGlobalWidget::~VioAttributeMtcGlobalWidget(void) {
}

// fix view
void VioAttributeMtcGlobalWidget::DoVioAllocate(void) {
  // call base to connect
  VioAttributeWidget::DoVioAllocate();
}

// set by vio model
int VioAttributeMtcGlobalWidget::Model(VioModel* model) {
  FD_DQT("VioAttributeMtcGlobal::Model(" << model << "): type " << VioStyle::StrFromQStr(FaudesType()));
  // bail out on identity
  if(model==mModel) return 0;
  // call base to set view and connects (incl callback diovioupdate)
  int res=VioAttributeWidget::Model(model);
  // update typed ref
  pAttributeMtcGlobalModel=qobject_cast<VioAttributeMtcGlobalModel*>(mModel);
  // done
  FD_DQT("VioAttributeMtcGlobal::Model(" << model << "): done");  
  return res;
}











