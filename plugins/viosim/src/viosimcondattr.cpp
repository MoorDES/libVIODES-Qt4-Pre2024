/* viosimcondattr.cpp  - vio attribute for faudes sim condition  */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2010 Thomas Moor

*/


// #define FAUDES_DEBUG_VIO_TYPE

#include "viosimcondattr.h"


/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioAttributeSimCondModel

****************************************************************
****************************************************************
****************************************************************
*/

// construct
VioAttributeSimCondModel::VioAttributeSimCondModel(QObject* parent, VioStyle* config, bool alloc) : 
  VioAttributeModel(parent, config, false),
  mpFaudesSimCondition(0) 
{
  FD_DQT("VioAttributSimCondModel::VioAttributeSimCondModel(): " << VioStyle::StrFromQStr(mFaudesType));
  // allocate faudes object and have default rep data
  if(alloc) {
    DoVioAllocate();
    DoVioUpdate();
  }
  FD_DQT("VioAttributModel::VioAttributeSimCondModel(): done");
}

// destruct
VioAttributeSimCondModel::~VioAttributeSimCondModel(void) {
}

// construct on heap
VioAttributeSimCondModel* VioAttributeSimCondModel::NewModel(QObject* parent) const {
  FD_DQT("VioAttributeSimCondModel::NewModel(): type " << VioStyle::StrFromQStr(mFaudesType)); 
  return new VioAttributeSimCondModel(parent,pConfig);
}

// construct view on heap
VioView* VioAttributeSimCondModel::NewView(QWidget* parent) const {
  FD_DQT("VioAttributeSimCondModel::NewView(): " << VioStyle::StrFromQStr(FaudesType()));
  return new VioAttributeSimCondView(parent, pConfig);
}

// construct widget on heap
VioWidget* VioAttributeSimCondModel::NewWidget(QWidget* parent) const {
  FD_DQT("VioAttributeSimCondMode::NewWidget(): type " << VioStyle::StrFromQStr(FaudesType()));
  return new VioAttributeSimCondWidget(parent, pConfig);
}


// allocate faudes object
void VioAttributeSimCondModel::DoFaudesAllocate(void) {
  FD_DQT("VioAttributeSimCondModel::DoFaudesAllocate()");
  // let base handle this
  VioAttributeModel::DoFaudesAllocate();
  // impose my requirements
  if(DoTypeCheck(mData->FaudesObject())) {
    FD_DQT("VioAttributeSimCondModel::DoFaudesAllocate(): fallback ctype");
    mData->FaudesObject(new faudes::AttributeSimCondition());  
  }
  FD_DQT("VioAttributeSimCondModel::DoFaudesAllocate(): ctype " << typeid(*FaudesObject()).name());
}

// test whether we can host this faudes object
int VioAttributeSimCondModel::DoTypeCheck(const faudes::Type* fobject) const {  
  // we host anything that casts to flags Attribute
  if(dynamic_cast<const faudes::AttributeSimCondition*>(fobject)) return 0;
  return 1; 
}

// update visual data from (new) faudes object (incl layout change TODO: signals)
void VioAttributeSimCondModel::DoVioUpdate(void) {
  FD_DQT("VioAttributeSimCondModel::DoVioUpdate()");
  // have my typed reference
  mpFaudesSimCondition = dynamic_cast<faudes::AttributeSimCondition*>(mData->FaudesObject());
  // update my features
  mStartEvents.clear();
  mStopEvents.clear();
  if(mpFaudesSimCondition->IsEventCondition()) {
    VioStyle::EventsQStrList(mStartEvents,&mpFaudesSimCondition->EventCondition().mStart);
    VioStyle::EventsQStrList(mStopEvents,&mpFaudesSimCondition->EventCondition().mStop);
  }
  // call base for actual update regarding flags (incl notify view)
  VioAttributeModel::DoVioUpdate();
}


// (user) edit cond type
void VioAttributeSimCondModel::VioCondType(int type) {
  FD_DQT("VioAttributeSimCondModel::VioCondType(): set " << type);
  // event cond
  if(type==0 && !mpFaudesSimCondition->IsEventCondition()) {
    faudes::SimEventCondition evcond;
    mpFaudesSimCondition->EventCondition(evcond);
    DoVioUpdate();
  }
}


// (user) edit start evs
void VioAttributeSimCondModel::VioStartEvents(const QList<QString>& rStartEvs) {
  FD_DQT("VioAttributeSimCondModel::VioStartEvents(): set #" << rStartEvs.size());
  // detect changes
  if(mStartEvents==rStartEvs) return;
  // set/tell
  FD_DQT("VioAttributeSimCondModel::VioStartEvents(): change"); 
  // write to viodata
  mStartEvents=rStartEvs;
  // write to faudes attribute 
  faudes::SimEventCondition evcond=mpFaudesSimCondition->EventCondition();
  evcond.mStart.Clear();
  foreach(const QString& evname, mStartEvents) {
    if(!VioStyle::ValidSymbol(evname)) continue;    
    evcond.mStart.Insert(VioStyle::StrFromQStr(evname));
  }
  mpFaudesSimCondition->EventCondition(evcond);
  // update debugging widgets
#ifdef FAUDES_DEBUG_VIO_WIDGETS
  mVioText=VioStyle::QStrFromStr(mpFaudesSimCondition->ToString());
#endif
  // notify view
  emit NotifyAnyChange();
  // since we intenionally allow this even if type is not event cond ...
  VioCondType(0);
  FD_DQT("VioAttributeSimCondModel::VioStartEvents(): #" << rStartEvs.size()); 
}

// (user) edit stop evs
void VioAttributeSimCondModel::VioStopEvents(const QList<QString>& rStopEvs) {
  FD_DQT("VioAttributeSimCondModel::VioStopEvents(): set #" << rStopEvs.size());
  // detect changes
  if(mStopEvents==rStopEvs) return;
  // set/tell
  FD_DQT("VioAttributeSimCondModel::VioStopEvents(): change"); 
  // write to viodata
  mStopEvents=rStopEvs;
  // write to faudes attribute 
  faudes::SimEventCondition evcond=mpFaudesSimCondition->EventCondition();
  evcond.mStop.Clear();
  foreach(const QString& evname, mStopEvents) {
    if(!VioStyle::ValidSymbol(evname)) continue;    
    evcond.mStop.Insert(VioStyle::StrFromQStr(evname));
  }
  mpFaudesSimCondition->EventCondition(evcond);
  // update debugging widgets
#ifdef FAUDES_DEBUG_VIO_WIDGETS
  mVioText=VioStyle::QStrFromStr(mpFaudesSimCondition->ToString());
#endif
  // notify view
  emit NotifyAnyChange();
  // since we intenionally allow this even if type is not event cond ...
  VioCondType(0);
  FD_DQT("VioAttributeSimCondModel::VioStopEvents(): done #" << rStopEvs.size()); 
}

// merge: clear
int VioAttributeSimCondModel::MergeClear(void) {
  // set my stuff to empty
  mpFaudesSimCondition->EventCondition(faudes::SimEventCondition());
  mStopEvents.clear();
  mStartEvents.clear();
  // base handles flags
  VioAttributeModel::MergeClear();
  return 0;
}

// merge: insert
void VioAttributeSimCondModel::MergeInsert(const faudes::AttributeFlags* fattr) {
  // base handles flags
  VioAttributeModel::MergeInsert(fattr);
}
 

// partial assign
void VioAttributeSimCondModel::MergeAssign(faudes::AttributeFlags* fattr) {
  // base handles flags
  VioAttributeModel::MergeAssign(fattr);
}


/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioAttributeSimCondView

****************************************************************
****************************************************************
****************************************************************
*/

// construct
VioAttributeSimCondView::VioAttributeSimCondView(QWidget* parent, VioStyle* config, bool alloc) : 
  VioAttributeView(parent,config,false),
  mStartWidget(0),
  mStopWidget(0)
{
  FD_DQT("VioAttributeSimCondView::VioAttributeSimCondView(): " << VioStyle::StrFromQStr(FaudesType()));
  // my alloc
  if(alloc) DoVioAllocate();
  FD_DQT("VioAttributeSimCondView::VioAttributeSimCondView(): done");
}

// destruct
VioAttributeSimCondView::~VioAttributeSimCondView(void) {
}

// typed faudes object access
const faudes::AttributeSimCondition* VioAttributeSimCondView::AttributeSimCondition(void) const {
  if(!pAttributeSimCondModel) return 0;
  return pAttributeSimCondModel->AttributeSimCondition();
}

// allocate view data
void VioAttributeSimCondView::DoVioAllocate(void) {
  FD_DQT("VioAttributeSimCondView::DoVioAllocate()");
  // call base
  VioAttributeView::DoVioAllocate();
  // add my features
  mTypeCombo=new QComboBox();
  mTypeCombo->addItem("Event");
  mTypeCombo->addItem("State (conj)");
  mTypeCombo->addItem("State (disj)");
  connect(mTypeCombo,SIGNAL(activated(int)),this,SLOT(UpdateFromTypeCombo(int)));
  mTypeCombo->hide(); // not implemented
  mStartWidget= new VioSymbolTableWidget();
  mStartWidget->setHeader("Start");
  connect(mStartWidget,SIGNAL(editingFinished(int,int)),this,SLOT(UpdateFromStartWidget()));
  connect(mStartWidget,SIGNAL(resizeModel()),this,SLOT(UpdateFromStartWidget()));
  mStopWidget= new VioSymbolTableWidget();
  mStopWidget->setHeader("Stop");
  connect(mStopWidget,SIGNAL(editingFinished(int,int)),this,SLOT(UpdateFromStopWidget()));
  connect(mStopWidget,SIGNAL(resizeModel()),this,SLOT(UpdateFromStopWidget()));
  QHBoxLayout* hbox = new QHBoxLayout();
  hbox->setMargin(0);
  hbox->setSpacing(2);
  hbox->addWidget(mStartWidget);
  hbox->addWidget(mStopWidget);
  mVbox->addWidget(mTypeCombo);
  mVbox->addLayout(hbox);
  FD_DQT("VioAttributeSimCondView::DoVioAllocate(): done");
}

// update view from (new) model
void VioAttributeSimCondView::DoVioUpdate(void) {
  FD_DQT("VioAttributeSimCondView::DoVioUpdate(): type " << VioStyle::StrFromQStr(FaudesType()));
  // call base
  VioAttributeView::DoVioUpdate();
  // have typed model reference (incl set to 0)
  pAttributeSimCondModel=qobject_cast<VioAttributeSimCondModel*>(pModel);
  // bail out on no model
  if(!pAttributeSimCondModel) return;
  // report
  FD_DQT("VioAttributeSimCondView::DoVioUupdate: start/stop #" << 
    pAttributeSimCondModel->VioStartEvents().size() << "/" << pAttributeSimCondModel->VioStopEvents().size());
  // update completers
  //   mStartWidget->setSymbolWorld(1,cgen->GeneratorConfiguration()->ColorSet().toList());
  // update start and stop widgets
  mStartWidget->setSymbolList(pAttributeSimCondModel->VioStartEvents());
  mStopWidget->setSymbolList(pAttributeSimCondModel->VioStopEvents());
  // disable merge unsupported
  mStartWidget->setEnabled(!pAttributeSimCondModel->Merged());
  mStopWidget->setEnabled(!pAttributeSimCondModel->Merged());
  FD_DQT("VioAttributeSimCondView::DoVioUpdate(): done");
}

void VioAttributeSimCondView::UpdateFromTypeCombo(int type) {
  // bail out if widget is not ready or model not there
  if((!mStopWidget) || (!pAttributeSimCondModel)) return;
  if(type==-1) type = mTypeCombo->currentIndex();
  FD_DQT("VioAtributeSimCondView::UpdateFromTypeCombo(" << type << ")");
  pAttributeSimCondModel->VioCondType(type);
  // set changed
  Modified(true);
}


// stop events have been changed
void VioAttributeSimCondView::UpdateFromStopWidget(void) {
  // bail out if widget is not ready or model not there
  if(!mStopWidget || !pAttributeSimCondModel) return;
  FD_DQT("VioAtributeSimCondView::UpdateFromStopWidget()");
  // todo: fix: if user adds/edits an event, make it unique
  pAttributeSimCondModel->VioStopEvents(mStopWidget->symbolList());
  // set changed
  Modified(true);
}

// start events have been changed
void VioAttributeSimCondView::UpdateFromStartWidget(void) {
  // bail out if widget is not ready or model not there
  if(!mStartWidget || !pAttributeSimCondModel) return;
  FD_DQT("VioAtributeSimCondView::UpdateFromStartWidget()");
  pAttributeSimCondModel->VioStartEvents(mStartWidget->symbolList());
  // set changed
  Modified(true);
}



// update model from view
void VioAttributeSimCondView::DoModelUpdate(void) {
  FD_DQT("VioAttributeSimCondView::DoModelUpdate(): type " << VioStyle::StrFromQStr(FaudesType()));
  // call base
  VioAttributeView::DoModelUpdate();
  // bail out if widget is not ready or model not there
  if(!mStartWidget || !pModel) return;
  // update my features
  UpdateFromTypeCombo();
  UpdateFromStartWidget();
  UpdateFromStopWidget();
}


/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioAttributeSimCondWidget

****************************************************************
****************************************************************
****************************************************************
*/

// construct
VioAttributeSimCondWidget::VioAttributeSimCondWidget(QWidget* parent, VioStyle* config, bool alloc) : 
  VioAttributeWidget(parent,config,false) 
{
  FD_DQT("VioAttributeSimCondWidget::VioAttributeSimCondWidget(): " << VioStyle::StrFromQStr(pConfig->ConfigName()));
  // allocate model and view
  if(alloc) {
    // have view
    mView= new VioAttributeSimCondView(this,pConfig);
    mVbox->addWidget(mView);
    // fix typed refs and connect to widget
    DoVioAllocate();
    // have a model and set
    Model(new VioAttributeSimCondModel(this,pConfig));
  }
  // fix modified flag
  mModified=false;
  FD_DQT("VioAttributeSimCondWidget::VioAttributeSimCondWidget(): done");
}

// destruct
VioAttributeSimCondWidget::~VioAttributeSimCondWidget(void) {
}

// fix view
void VioAttributeSimCondWidget::DoVioAllocate(void) {
  // call base to connect
  VioAttributeWidget::DoVioAllocate();
}

// set by vio model
int VioAttributeSimCondWidget::Model(VioModel* model) {
  FD_DQT("VioAttributeSimCond::Model(" << model << "): type " << VioStyle::StrFromQStr(FaudesType()));
  // bail out on identity
  if(model==mModel) return 0;
  // call base to set view and connects (incl callback diovioupdate)
  int res=VioAttributeWidget::Model(model);
  // update typed ref
  pAttributeSimCondModel=qobject_cast<VioAttributeSimCondModel*>(mModel);
  // done
  FD_DQT("VioAttributeSimCond::Model(" << model << "): done");  
  return res;
}











