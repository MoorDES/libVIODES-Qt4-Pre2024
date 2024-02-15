/* viogengraph.cpp  - graph representation of generators */

/*
   Visual IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2009  Thomas Moor

*/


//#define FAUDES_DEBUG_VIO_GENERATOR

// my header
#include "viogengraph.h"

// resolve forwards
#include "gioscene.h"
#include "gioview.h"


/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioGeneratorGraphData

****************************************************************
****************************************************************
****************************************************************
*/

// construct
VioGeneratorGraphData::VioGeneratorGraphData(QObject* parent) : 
  VioGeneratorAbstractData(parent) 
{
}

// destruct
VioGeneratorGraphData::~VioGeneratorGraphData(void) {
}

// clear to default/empty faudes object
void VioGeneratorGraphData::Clear(void) {
  mTransItemsData.clear();
  mStateItemsData.clear();
}

// conversion 
void VioGeneratorGraphData::ToTokenWriter(faudes::TokenWriter& rTw) const {
  // write graph representation
  rTw.WriteBegin("GraphData");
  // 1. states
  rTw.WriteBegin("States");
  for(int i=0; i<mStateItemsData.size(); i++) {
    mStateItemsData[i].write(rTw); // wrt generator!!
  }
  rTw.WriteEnd("States");
  // 2. transitions
  rTw.WriteBegin("TransRel");
  for(int i=0; i<mTransItemsData.size(); i++) {
    mTransItemsData[i].write(rTw); // wrt generator!
  }
  rTw.WriteEnd("TransRel");
  // done
  rTw.WriteEnd("GraphData");
};


// conversion (ret 0 on sucess)
int  VioGeneratorGraphData::FromTokenReader(faudes::TokenReader& rTr) {
  // clear all
  Clear();
  // figure first token
  faudes::Token token;
  rTr.Peek(token);
  if(token.Type()!=faudes::Token::Begin) return 1;
  if(token.StringValue()!="GraphData") return 1;
  // accept and read my section
  try {
    rTr.ReadBegin("GraphData");
    // 1. read states
    rTr.ReadBegin("States");
    while(!rTr.Eos("States")){ 
      GioState::Data sdata;
      sdata.read(rTr);
      mStateItemsData.append(sdata);
    }
    rTr.ReadEnd("States");
    // 2. read transitions
    rTr.ReadBegin("TransRel");
    while(!rTr.Eos("TransRel")){ 
      GioTrans::Data tdata;
      tdata.read(rTr);
      mTransItemsData.append(tdata);
    }
    rTr.ReadEnd("TransRel");
    // done
    rTr.ReadEnd("GraphData");
  }
  // catch exception and return error code
  catch (faudes::Exception& exception) {  
    FD_DQG("GioSceneRo::Data::read: cannot read data");
    Clear();
    return 1;
  }
  // all fine
  return 0;
}


// state reindeing
void VioGeneratorGraphData::ApplyStateIndicees(const QMap<faudes::Idx,faudes::Idx> & rNewIdx) {
 for(int i=0; i<mStateItemsData.size(); i++) 
   if(rNewIdx.contains(mStateItemsData[i].mIdx))
     mStateItemsData[i].mIdx= rNewIdx.value(mStateItemsData[i].mIdx);
 for(int i=0; i<mTransItemsData.size(); i++) {
   if(rNewIdx.contains(mTransItemsData[i].mIdxA))
     mTransItemsData[i].mIdxA= rNewIdx.value(mTransItemsData[i].mIdxA);
   if(rNewIdx.contains(mTransItemsData[i].mIdxB))
     mTransItemsData[i].mIdxB= rNewIdx.value(mTransItemsData[i].mIdxB);
 }
}

/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioGeneratorGraphModel

****************************************************************
****************************************************************
****************************************************************
*/

// construct
VioGeneratorGraphModel::VioGeneratorGraphModel(VioGeneratorModel* parent) : 
  VioGeneratorAbstractModel(parent),
  mGraphScene(0)
{
  FD_DQG("VioGeneratorGraphModel::VioGeneratorGraphModel()");
  pVioGeneratorModel=parent;
  mGraphScene= new GioScene(this);
  connect(mGraphScene,SIGNAL(NotifyModified(bool)),this,SLOT(ChildModified(bool)));
  connect(mGraphScene,SIGNAL(MouseClick(const VioElement&)),pVioGeneratorModel,SIGNAL(MouseClick(const VioElement&)));
  connect(mGraphScene,SIGNAL(MouseDoubleClick(const VioElement&)),pVioGeneratorModel,SIGNAL(MouseDoubleClick(const VioElement&)));
  FD_DQG("VioGeneratorGraphModel::VioGeneratorGraphModel(): done");
}

// access as qt graphics scene
GioScene* VioGeneratorGraphModel::GraphScene(void) { return mGraphScene; };

// create new view for this representationmodel
VioGeneratorAbstractView* VioGeneratorGraphModel::NewView(VioGeneratorView* parent) {
  FD_DQG("VioGeneratorGraphModel::NewView()");
  // create
  VioGeneratorAbstractView* view = new VioGeneratorGraphView(parent);
  // set model (incl connect)
  view->Model(this);
  // done
  return view;
}

// token io: vio data
void VioGeneratorGraphModel::DoVioWrite(faudes::TokenWriter& rTw) const {
  FD_DQT("VioGeneratorGraphModel::DoVioWrite()");
  mGraphScene->GioWrite(rTw);
}

// token io: vio data
void VioGeneratorGraphModel::DoVioRead(faudes::TokenReader& rTr) {
  FD_DQT("VioGeneratorGraphModel::DoVioRead()");
  mGraphScene->GioRead(rTr);
}

// write graph
int VioGeneratorGraphModel::WriteEps(const QString& filename) const {
  return mGraphScene->EpsWrite(filename);}
int VioGeneratorGraphModel::WritePdf(const QString& filename) const {
  return mGraphScene->PdfWrite(filename);}
int VioGeneratorGraphModel::WriteSvg(const QString& filename) const {
  return mGraphScene->SvgWrite(filename);}
int VioGeneratorGraphModel::WriteJpg(const QString& filename) const {
  return mGraphScene->JpgWrite(filename);}
int VioGeneratorGraphModel::WritePng(const QString& filename) const {
  return mGraphScene->PngWrite(filename);}


// update selction notification: pass on to scene
void VioGeneratorGraphModel::UpdateSelectionElement(const VioElement& elem, bool on) 
  { mGraphScene->UpdateSelectionElement(elem, on); };
void VioGeneratorGraphModel::UpdateSelectionClear(void) 
  { mGraphScene->UpdateSelectionClear(); };
void VioGeneratorGraphModel::UpdateSelectionAny(void) 
  { mGraphScene->UpdateSelectionAny(); };


// update notification: pass on to graph scene
void VioGeneratorGraphModel::UpdateElementIns(const VioElement& elem) 
  { mGraphScene->UpdateElementIns(elem);};
// update notification: pass on to graph scene
void VioGeneratorGraphModel::UpdateElementDel(const VioElement& elem) 
  { mGraphScene->UpdateElementDel(elem);};
// update notification: pass on to graph scene
void VioGeneratorGraphModel::UpdateElementEdit(const VioElement& selem, const VioElement& delem) 
  { mGraphScene->UpdateElementEdit(selem,delem);};
// update notification: pass on to graph scene
void VioGeneratorGraphModel::UpdateElementProp(const VioElement& elem)
  { mGraphScene->UpdateElementProp(elem);};
// update notification: pass on to graph scene
void VioGeneratorGraphModel::UpdateTrimElements(void) 
  { mGraphScene->UpdateTrimElements();};
// update notification: pass on to graph scene
void VioGeneratorGraphModel::UpdateAnyAttr(void) 
  { mGraphScene->UpdateAnyAttr();};

// update notification: reimplement
void VioGeneratorGraphModel::UpdateAnyChange(void) { 
  FD_DQG("VioGeneratorGraphModel::UpdateAnyChange()");
  // view to get prepared, incl debugging
  emit NotifyAnyChange();
  // tell my scene
  mGraphScene->UpdateAnyChange();
  // done 
  FD_DQG("VioGeneratorGraphModel::UpdateAnyChange(): done #" << Size());
}


// update notification: reimplement
void VioGeneratorGraphModel::UpdateNewModel(void) { 
  FD_DQG("VioGeneratorGraphModel::UpdateNewModel()");
  DoVioUpdate();
}

// update visual data from (new) faudes object
void VioGeneratorGraphModel::DoVioUpdate(void) { 
  FD_DQG("VioGeneratorGraphModel::DoVioUpdate()");
  // tell my scene
  mGraphScene->UpdateNewModel();  
  // call base (emits signal)
  VioGeneratorAbstractModel::DoVioUpdate();
  // report
  FD_DQG("VioGeneratorGraphModel::DoVioUpdate(): done");
}

// clear all
void VioGeneratorGraphModel::Clear(void) {
}

// user edit: get size 
int VioGeneratorGraphModel::Size(void) const {
  return pVioGeneratorModel->Size();
}


// re-implement data access
VioGeneratorAbstractData* VioGeneratorGraphModel::Data(void) {
  FD_DQT("VioGeneratorGraphModel::Data(): get");
  FD_DQT("VioGeneratorGraphModel::Data(): faudes name: " <<Generator()->Name());
  VioGeneratorGraphData* gdat=new VioGeneratorGraphData();
  //copy from scene: states
  FD_DQT("VioGeneratorGraphModel::Data(): states #" <<Generator()->Size());
  faudes::StateSet::Iterator sit=Generator()->StatesBegin();
  for(;sit!=Generator()->StatesEnd(); sit++) 
    if(GioState* state= mGraphScene->StateItem(*sit))
      gdat->mStateItemsData.append(state->data());
  //copy from scene: transitions
  FD_DQT("VioGeneratorGraphModel::Data(): trans #" <<Generator()->TransRelSize());
  faudes::TransSet::Iterator tit=Generator()->TransRelBegin();
  foreach(GioTrans* trans, mGraphScene->Trans())
      gdat->mTransItemsData.append(trans->data());
  // done
  FD_DQT("VioGeneratorGraphModel::Data(): items #" << 
    gdat->mTransItemsData.size()+gdat->mStateItemsData.size());
  return gdat;
}

// re-implement data access
VioGeneratorAbstractData* VioGeneratorGraphModel::SelectionData(void) {
  FD_DQG("VioGeneratorGraphModel::SelectionData(): get");
  FD_DQG("VioGeneratorGraphModel::SelectionData(): faudes name: " <<Generator()->Name());
  FD_DQG("VioGeneratorGraphModel::SelectionData(): generator states #" <<Generator()->Size());
  VioGeneratorGraphData* gdat=new VioGeneratorGraphData();
  //copy from scene: states
  foreach(GioState* gstate, mGraphScene->SelectedStates()) 
    gdat->mStateItemsData.append(gstate->data());
  //copy from scene: transitions
  foreach(GioTrans* gtrans, mGraphScene->SelectedTransitions()) 
    gdat->mTransItemsData.append(gtrans->data());
  // done
  FD_DQG("VioGeneratorGraphModel::SelectionData(): items #" << 
    gdat->mTransItemsData.size()+gdat->mStateItemsData.size());
  return gdat;
}

// re-implement data access
int  VioGeneratorGraphModel::Data(const VioGeneratorAbstractData* pData) {
  FD_DQT("VioGeneratorGraphModel::Data(): set: test A");
  const VioGeneratorGraphData* gdat=qobject_cast<const VioGeneratorGraphData*>(pData);
  if(!gdat) return 1;
  // hack: if we are merging, we want an offset; else we don'T (e.g. undo/redo)
  QPointF offset(0,0);
  if(!mGraphScene->Empty()) offset=QPointF(30,30);
  // states
  FD_DQT("VioGeneratorGraphModel::Data(): set states #" << gdat->mStateItemsData.size());
  foreach(GioState::Data sdat,gdat->mStateItemsData) {
    sdat.mPosition=sdat.mPosition+offset;
    if(!Generator()->ExistsState(sdat.mIdx)) continue; 
    FD_DQT("VioGeneratorGraphModel::Data(): set state idx " << sdat.mIdx);
    GioState* state = new GioState(pVioGeneratorModel);
    state->setData(sdat);
    FD_DQT("VioGeneratorGraphModel::Data(): insert state idx " << sdat.mIdx);
    mGraphScene->InsGioState(state);
  };
  // transitions
  FD_DQT("VioGeneratorGraphModel::Data(): set trans #" << gdat->mTransItemsData.size());
  foreach(GioTrans::Data tdat,gdat->mTransItemsData) {
    tdat.mPosition=tdat.mPosition+offset;
    faudes::Transition ftrans;
    ftrans.X1=tdat.mIdxA;
    ftrans.Ev=Generator()->EventIndex(tdat.mNameEv);
    ftrans.X2=tdat.mIdxB;
    //if(!Generator()->ExistsTransition(ftrans)) continue; 
    if(!Generator()->ExistsState(ftrans.X1)) continue; 
    if(!Generator()->ExistsState(ftrans.X2)) continue; 
    GioTrans* trans = new GioTrans(pVioGeneratorModel);
    trans->setData(tdat);
    mGraphScene->InsGioTrans(trans);
  };
  return 0;
}



// record changes and emit signal
void VioGeneratorGraphModel::Modified(bool ch) { 
  // call base (incl signal)
  VioGeneratorAbstractModel::Modified(ch);
  // pass on clr to childs
  if(!ch) {
    if(mGraphScene) mGraphScene->Modified(false);
  }
}


/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioGeneratorGraphView

****************************************************************
****************************************************************
****************************************************************
*/


// construct/destruct
VioGeneratorGraphView::VioGeneratorGraphView(VioGeneratorView* parent) :
  VioGeneratorAbstractView(parent),
  pGeneratorGraphModel(0),
  mGraphView(0)
{
  FD_DQG("VioGeneratorGraphView::VioGeneratorGraphView("<< parent <<")");
}

// set model (incl typecheck)
int VioGeneratorGraphView::Model(VioGeneratorAbstractModel* model) {
  FD_DQG("VioGeneratorGraphView::Model("<< model <<")");
  // bail on double set
  if(model==pGeneratorAbstractModel) return 0;
  // typecheck
  if(!qobject_cast<VioGeneratorGraphModel*>(model)) return 1;
  // my dis connections
  if(pGeneratorView && mGraphView) {
    disconnect(mGraphView,0,pGeneratorView,0);
  }
  // call base: set, disconnect, connect, dovioupdate (virtual)
  if(VioGeneratorAbstractView::Model(model)!=0) return 1;
  // my connection
  connect(mGraphView,SIGNAL(NotifyZoom(qreal)),pGeneratorView,SLOT(SaveUserLayout(void)));
  connect(pGraphScene,SIGNAL(NotifyConsistent(bool)),this,SLOT(UpdateConsistent(bool)));
  // done
  return 0;
}

// get model and friends: representation model
const VioGeneratorGraphModel* VioGeneratorGraphView::Model(void) const {
  return pGeneratorGraphModel;
}

// update notification: any editing in graphics scene
void VioGeneratorGraphView::UpdateAnyChange(void) { 
  // bail out on no model
  if(!pGraphScene) return;
  if(!pGeneratorGraphModel) return;
  FD_DQG("VioGeneratorGraphView::UpdateAnyChange(): Size #" << Model()->Generator()->Size());
  // debugging widgets
#ifdef FAUDES_DEBUG_VIO_WIDGETS
  mTextInfo->setText(tr("Graph"));
  mTextEdit->setPlainText(
     VioStyle::QStrFromStr(Generator()->TransRelToText()) + "\n" +
     VioStyle::QStrFromStr(Generator()->StatesToText()) 
  );
#endif
}

// update notification: new model
void VioGeneratorGraphView::UpdateNewModel(void) { 
  FD_DQG("VioGeneratorGraphView::UpdateNewModel()");
  DoVioUpdate();
}

// update notification: consistency changed
void VioGeneratorGraphView::UpdateConsistent(bool cons) {
  FD_DQG("VioGeneratorGraphView::UpdateConsistent(cons=" << cons <<")");
  if(cons) {
    mStack->setCurrentWidget(mGraphView);
  } else {
    mStack->setCurrentWidget(mFakeView);
  }
}

// update from model
void VioGeneratorGraphView::DoVioUpdate(void) {
  FD_DQG("VioGeneratorGraphView::DoVioUpdate("<<this<<")");
  // have typed ref (model could have changed)
  pGeneratorGraphModel=qobject_cast<VioGeneratorGraphModel*>(pGeneratorAbstractModel);
  pGraphScene=pGeneratorGraphModel->GraphScene(); 
  // first time 
  if(!mGraphView) {
    // have gio view
    mGraphView= new GioView();
    // have a fake view
    mFakeView = new QWidget();
    QLabel* ml= new QLabel("Incomplete Graph. Sorry.");
    mGridButton= new QPushButton("Arrange Graph via Grid");
    mGridButton->setDefault(false);
    mGridButton->setAutoDefault(false);
    mDotButton= new QPushButton("Arrange Graph via Dot");
    mDotButton->setDefault(false);
    mDotButton->setAutoDefault(false);
    QHBoxLayout* mh = new QHBoxLayout(mFakeView);
    QWidget* mhw = new QWidget();
    mh->addStretch(1);
    mh->addWidget(mhw);
    mh->addStretch(1);
    QVBoxLayout* mv = new QVBoxLayout(mhw);
    mv->addStretch(1);
    mv->addWidget(ml,0,Qt::AlignHCenter);
    mv->addSpacing(10);
    mv->addWidget(mGridButton);
    mv->addWidget(mDotButton);
    mv->addStretch(2);
    // stack the two
    mStack = new QStackedWidget();
    mStack->addWidget(mGraphView);
    mStack->addWidget(mFakeView);
    // add to layout
    mVbox->addWidget(mStack);
    // connect my buttons
    connect(mDotButton,SIGNAL(clicked(void)),this,SLOT(DotConstruct(void)));
    connect(mGridButton,SIGNAL(clicked(void)),this,SLOT(GridConstruct(void)));
  }
  // set scene
  mGraphView->setScene(pGraphScene);
  UpdateConsistent(pGraphScene->Consistent());
  // debugging widgets
  UpdateAnyChange();
}


// scale
void VioGeneratorGraphView::Scale(qreal sc) {
  if(!mGraphView) return;
  mGraphView->Scale(sc);
}

// scale
qreal VioGeneratorGraphView::Scale(void) {
  if(!mGraphView) return 1.0;
  return mGraphView->Scale();
}

// fit
void VioGeneratorGraphView::Fit(void) {
  if(!mGraphView) return;
  mGraphView->Fit();
}


// grid
void VioGeneratorGraphView::GridVisible(bool on) {
 if(!pGraphScene) return;
  pGraphScene->GridVisible(on);
}

// re-arrange
void VioGeneratorGraphView::DotConstruct(void) {
  // bail out on invalid
  if(!pGeneratorGraphModel) return;
  // doit
  pGeneratorGraphModel->Modified(true);
  pGeneratorModel->UndoEditStart();
  pGraphScene->DotConstruct();
  pGeneratorModel->UndoEditStop();
}

// re-arrange
void VioGeneratorGraphView::GridConstruct(void) {
  // bail out on invalid
  if(!pGeneratorGraphModel) return;
  // doit
  pGeneratorGraphModel->Modified(true);
  pGeneratorModel->UndoEditStart();
  pGraphScene->GridConstruct();
  pGeneratorModel->UndoEditStop();
}



// highlite/show request 
void VioGeneratorGraphView::Highlite(const VioElement& elem, bool on) {
  // bail out on no model
  if(!pGraphScene) return;
  if(!pGeneratorGraphModel) return;
  FD_DQG("VioGeneratorGraphView::Highlite()");
  pGraphScene->Highlite(elem,on);
}

// highlite/show request 
void VioGeneratorGraphView::HighliteClear(void) {
  // bail out on no model
  if(!pGraphScene) return;
  if(!pGeneratorGraphModel) return;
  FD_DQG("VioGeneratorGraphView::HighliteClear()");
  pGraphScene->HighliteClear();
}

// Show (todo)
void VioGeneratorGraphView::Show(const VioElement& elem) {
  (void) elem;
}




