/* viogenerator.cpp  - vio generator model and widget */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2009  Thomas Moor, Ruediger Berndt

*/

// Todo: connect view to model in View::Model(model)

//#define FAUDES_DEBUG_VIO_GENERATOR

#include "viogenerator.h"
#include "viogenlist.h"
#include "viogengraph.h"
#include "piotseview.h"
#include "gioscene.h"


/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioGeneratorLayout

****************************************************************
****************************************************************
****************************************************************
*/

// defaults
VioGeneratorLayout::VioGeneratorLayout(QObject* parent) : QObject(parent) {
  mToggleStage=-1;    // -1 indicates default for delayed initialisation
  mPropBuiltIn=false;
  mListWidth=300;
  mGraphWidth=600;
  mPropWidth=150;
  mGraphScale=1.0;
  mGraphGridVisible=false;
}

// assignment
VioGeneratorLayout& VioGeneratorLayout::operator=(const VioGeneratorLayout& rSrc) {
  mToggleStage=rSrc.mToggleStage;
  mSplitterState= rSrc.mSplitterState;
  mPropBuiltIn=rSrc.mPropBuiltIn;
  mPropWidth=rSrc.mPropWidth;
  mListWidth=rSrc.mListWidth;
  mGraphWidth=rSrc.mGraphWidth;
  mGraphScale=rSrc.mGraphScale;
  mGraphGridVisible=rSrc.mGraphGridVisible;
  return *this;
}  

// write layout
void VioGeneratorLayout::WriteCore(faudes::TokenWriter& rTw) const {
  // binary token debugging
#if FAUDES_DEBUG_VIO_GENERATOR_TDB
  int sum=qChecksum(mSplitterState.constData(),mSplitterState.size());
  int size = mSplitterState.size();
  FD_WARN("VioGeneratorLayout::WriteCore(): binary token " << size << " sum " << sum);

  faudes::TokenWriter w1(faudes::TokenWriter::String);
  w1.WriteBinary(mSplitterState.constData(),mSplitterState.size());
  std::string s1=w1.Str();
  std::cout << "TOKENS " << s1 << std::endl;

  faudes::TokenReader r1(faudes::TokenReader::String,s1);
  std::string ssl;
  rTr.ReadBinary(ssl);

  faudes::TokenWriter w2(faudes::TokenWriter::String);
  w2.WriteBinary(ss1.data(),ss1.size());
  std::string s2=w2.Str();
  std::cout << "TOKENS " << s2 << std::endl;

  QByteArray rb = QByteArray::fromRawData(ss1.data(),ss1.size());
  sum=qChecksum(rb.constData(),rb.size());
  size = mSplitterState.size();
  FD_WARN("VioGeneratorLayout::WriteCore(): binary token " << size << " sum " << sum);


#endif

  // data
  rTw.WriteInteger(mToggleStage);
  rTw.WriteBinary(mSplitterState.constData(),mSplitterState.size());
  rTw.WriteInteger(mPropBuiltIn);
  rTw.WriteInteger(mPropWidth);
  rTw.WriteInteger(mGraphWidth);
  rTw.WriteFloat(mGraphScale);
  // post 0.62
  rTw.WriteInteger(mGraphGridVisible);
  rTw.WriteInteger(mListWidth);
}

// read layout 
void VioGeneratorLayout::ReadCore(faudes::TokenReader& rTr) {

  // data only
  mToggleStage= (int) rTr.ReadFloat(); // this was a bug in 0.43 writing "-1"
  std::string data;
  rTr.ReadBinary(data);
  mSplitterState = QByteArray::fromRawData(data.data(),data.size());
  mPropBuiltIn= rTr.ReadInteger();
  mPropWidth= rTr.ReadInteger();
  mGraphWidth= rTr.ReadInteger();
  mGraphScale=rTr.ReadFloat();
  if(rTr.Eos("VioLayout")) return;   // read pre 0.62 files
  mGraphGridVisible= rTr.ReadInteger();
  if(rTr.Eos("VioLayout")) return;   // read other 0.6x files
  mListWidth= rTr.ReadInteger();

#if FAUDES_DEBUG_VIO_GENERATOR_TDB
  int sum=qChecksum(mSplitterState.constData(),mSplitterState.size());
  int size = mSplitterState.size();
  FD_WARN("VioGeneratorLayout::ReadCore(): binary token " << size << " sum " << sum);
#endif


}

// write layout section
void VioGeneratorLayout::Write(faudes::TokenWriter& rTw) const {
  rTw.WriteBegin("VioLayout");
  WriteCore(rTw);
  rTw.WriteEnd("VioLayout");
}

// read layout 
void VioGeneratorLayout::Read(faudes::TokenReader& rTr) {
  // section
  try {
    rTr.ReadBegin("VioLayout");
  } catch(faudes::Exception expection) {
    // no such section: bail out
    return;
  } 
  // try core
  try {
    ReadCore(rTr);
  } catch(faudes::Exception expection) {
    // error: ignore
  } 
  // skip until end
  while(!rTr.Eos("VioLayout")){
    faudes::Token token;
    rTr.Get(token);
  }
  // end section
  rTr.ReadEnd("VioLayout");
  FD_DQG("VioGeneratorLayout::Read(): ok with scale " << mGraphScale);
}


/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioGeneratorData

****************************************************************
****************************************************************
****************************************************************
*/

// construct
VioGeneratorData::VioGeneratorData(QObject* parent) : VioData(parent) {
};
 
// destruct
VioGeneratorData::~VioGeneratorData(void) {
  foreach(VioGeneratorAbstractData* adat,mDataList) 
    if(adat) delete adat;
  mDataList.clear();
};

// clear
void VioGeneratorData::Clear(void) {
  // clear my members
  foreach(VioGeneratorAbstractData* adat,mDataList) 
    if(adat) delete adat;
  mDataList.clear();
  // call base
  VioData::Clear();
};

// conversion 
QMimeData* VioGeneratorData::ToMime(void) {
  // use tokenized text for conversion
  faudes::TokenWriter rTw(faudes::TokenWriter::String);
  rTw.Endl(false);
  rTw.WriteBegin("VioGeneratorData");
  if(mFaudesObject) mFaudesObject->Write(rTw);
  for(int i=0; i<mDataList.size(); i++) 
    if(mDataList.at(i)) mDataList.at(i)->ToTokenWriter(rTw); 
  rTw.WriteEnd("VioGeneratorData");
  // return as mime text
  QMimeData* mdat= new QMimeData();
  mdat->setText(rTw.Str().c_str());
  return mdat;
}

// conversion (0 on success)
int VioGeneratorData::FromMime(const QMimeData* pMime) {
  FD_DQG("VioGeneratorData::FromMimeData()");
  Clear();
  int res=0;
  // convert to std string (can we avoid the copy somehow??)
  std::string tstr=pMime->text().toAscii().constData();
  // convert to token stream
  faudes::TokenReader rTr(faudes::TokenReader::String, tstr);
  // make sure we have a faudes object
  if(!mFaudesObject) mFaudesObject= new faudes::vGenerator();
  faudes::vGenerator* gen = dynamic_cast<faudes::vGenerator*>(mFaudesObject);
  if(!gen) return 1; // error
  // test format strict
  faudes::Token token;
  rTr.Peek(token);
  if(token.Type()==faudes::Token::Begin) 
  if(token.StringValue()=="VioGeneratorData") {
    FD_DQG("VioGeneratorData::FromMimeData(): found generator data");
    // do read
    try {
      rTr.ReadBegin("VioGeneratorData");
      mFaudesObject->Read(rTr);
      while(VioGeneratorAbstractData* adat = VioGeneratorAbstractData::NewFromTokenReader(rTr))
        mDataList.append(adat);
      rTr.ReadEnd("VioGeneratorData");
    } catch(faudes::Exception& exception) {
      Clear();
      res=1;
    }
    // done
    FD_DQG("VioGeneratorData::FromMimeData(): done " << res); 
    return res;
  } 
  // find alphabet from nameset data
  if(token.Type()==faudes::Token::Begin) 
  if(token.StringValue()=="VioNameSetData") {
    FD_DQG("VioGeneratorData::FromMimeData(): found name data");
    // do read
    try {
      rTr.ReadBegin("VioNameSetData");
      faudes::NameSet* falphabet = gen->Alphabet().New();
      falphabet->Read(rTr);
      gen->InsEvents(*falphabet);
      gen->EventAttributes(*falphabet);
      delete falphabet;
    } catch(faudes::Exception& exception) {
      Clear();
      res=1;
    }
  }
  return res;
}

// conversion (0 on success)
int VioGeneratorData::TestMime(const QMimeData* pMime) {
  return 1;
}



/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioGeneratorAbstractData

****************************************************************
****************************************************************
****************************************************************
*/


// construct
VioGeneratorAbstractData::VioGeneratorAbstractData(QObject* parent) : 
  QObject(parent) 
{
}

// destruct
VioGeneratorAbstractData::~VioGeneratorAbstractData(void) {
}

// construct from token stream
VioGeneratorAbstractData* VioGeneratorAbstractData::NewFromTokenReader(faudes::TokenReader& rTr) {
  faudes::Token token;
  rTr.Peek(token);
  if(token.Type()!=faudes::Token::Begin) return 0;
  if(token.StringValue()=="GraphData") {
    VioGeneratorGraphData* gdat=new VioGeneratorGraphData();
    gdat->FromTokenReader(rTr);
    return gdat;
  }
  return 0;
}


/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioGeneratorModel

****************************************************************
****************************************************************
****************************************************************
*/

// construct
VioGeneratorModel::VioGeneratorModel(QObject* parent, VioStyle* config, bool alloc) : 
  VioModel(parent,config,false),
  mpFaudesGenerator(0),
  pGeneratorConfig(0),
  mTransList(0),
  mStateList(0),
  mEventList(0),
  mGraph(0), 
  mpUserLayout(0)
{
  FD_DQG("VioGeneratorModel::VioGeneratorModel(): " << VioStyle::StrFromQStr(mFaudesType));
  // typed version of configuration
  pGeneratorConfig = dynamic_cast<VioGeneratorStyle*>(pConfig);
  if(!pGeneratorConfig) {
    FD_WARN("VioGeneratorModel::VioGeneratorModel(): invalid style, using default.");
    pGeneratorConfig= new VioGeneratorStyle(mFaudesType);
  }
  // style layout options
  mLayoutFlags=pGeneratorConfig->mLayoutFlags;
  // allocate faudes object and have default rep data
  if(alloc) {
    DoVioAllocate();
    DoVioUpdate();
  }
  // have a uniform change signal
  connect(this,SIGNAL(NotifyElementIns(const VioElement&)),this,SIGNAL(NotifyChange(void)));
  connect(this,SIGNAL(NotifyElementDel(const VioElement&)),this,SIGNAL(NotifyChange(void)));
  connect(this,SIGNAL(NotifyElementEdit(const VioElement&, const VioElement&)),
    this,SIGNAL(NotifyChange(void)));
  connect(this,SIGNAL(NotifyElementProp(const VioElement&)),this,SIGNAL(NotifyChange(void)));
  connect(this,SIGNAL(NotifyTrimElements(void)),this,SIGNAL(NotifyChange(void)));
  connect(this,SIGNAL(NotifyAnyAttr(void)),this,SIGNAL(NotifyChange(void)));
  connect(this,SIGNAL(NotifyAnyChange(void)),this,SIGNAL(NotifyChange(void)));
  FD_DQG("VioGeneratorModel::VioGeneratorModel(): done");
}

// destruct
VioGeneratorModel::~VioGeneratorModel(void) {
}

// assign
VioGeneratorModel& VioGeneratorModel::DoAssign(const VioModel& rSrc) {
  FD_DQG("VioGeneratorModel::DoAssign()");
  // let base fix the faudes object
  VioModel::DoAssign(rSrc);
  // test whether the source is a generator
  VioGeneratorModel* viogen = const_cast<VioGeneratorModel*> (
     qobject_cast<const VioGeneratorModel*>(&rSrc) );
  if(viogen) {
    FD_DQG("VioGeneratorModel::Assign(): sensed generator source");
    VioData* pdata = viogen->Data(); 
    VioGeneratorData* gdat= qobject_cast<VioGeneratorData*>(pdata);
    for(int i=0; i<gdat->mDataList.size(); i++) {
      VioGeneratorAbstractData* vdat= gdat->mDataList.at(i);
      // try for each model
      for(int j=0; j< mModelList.size(); j++) {
        if(mModelList.at(j)->Data(vdat)==0) break;
      }
    }
    delete pdata;
  }
  // done
  FD_DQG("VioGeneratorModel::DoAssign(): done");
  return *this;        
}

// construct on heap
VioGeneratorModel* VioGeneratorModel::NewModel(QObject* parent) const {
  FD_DQG("VioGeneratorModel::New;pdel(): type " << VioStyle::StrFromQStr(mFaudesType));
  return new VioGeneratorModel(parent,pConfig);
}

// construct on data heap
VioData* VioGeneratorModel::NewData(QObject* parent) const {
  FD_DQG("VioGeneratorModel::NewData(): type " << VioStyle::StrFromQStr(mFaudesType));
  VioGeneratorData* vdat=new VioGeneratorData(parent);
  vdat->FaudesObject(mData->FaudesObject()->New());
  return vdat;
}


// construct view on heap
VioView* VioGeneratorModel::NewView(QWidget* parent) const {
  FD_DQG("VioGeneratorModel::NewView(): " << VioStyle::StrFromQStr(FaudesType()));
  return new VioGeneratorView(parent, pConfig);
}

// construct property view on heap 
VioView* VioGeneratorModel::NewPropertyView(QWidget* parent) const {
  FD_DQG("VioGeneratorModel::NewPropertyView(): " << VioStyle::StrFromQStr(FaudesType()));
  return new VioGeneratorPropertyView(parent, pConfig);
}

// construct on heap
VioWidget* VioGeneratorModel::NewWidget(QWidget* parent) const {
  FD_DQG("VioGeneratorModel::NewWidget(): type " << VioStyle::StrFromQStr(FaudesType()));
  return new VioGeneratorWidget(parent, pConfig);
}

// allocate faudes object
void VioGeneratorModel::DoFaudesAllocate(void) {
  FD_DQG("VioGeneratorModel::DoFaudesAllocate(): fobject at " << FaudesObject());
  // call base
  VioModel::DoFaudesAllocate();
  // test my requirements
  if(DoTypeCheck(mData->FaudesObject())) {
    FD_WARN("VioGeneratorModel::DoFaudesAllocate: incompatible fobject, using fallback");
    mData->FaudesObject(new faudes::System());   
  }
  // done
  FD_DQG("VioGeneratorModel::DoFaudesAllocate(): ftype " << VioStyle::StrFromQStr(mFaudesType)
	 << " ctype " << typeid(*(mData->FaudesObject())).name() << " at " << FaudesObject());
}

// test whether we can host this faudes object
int VioGeneratorModel::DoTypeCheck(const faudes::Type* fobject) const {  
  // must have a vgenerator interface
  const faudes::vGenerator* vgen=dynamic_cast<const faudes::vGenerator*>(fobject);
  if(!vgen) return 1;

  FD_DQG("VioGeneratorModel::DoTypeCheck(): check " << typeid(*fobject).name());
  FD_DQG("VioGeneratorModel::DoTypeCheck(): require trans  " 
	 << typeid(*pGeneratorConfig->mTransAttribute->Attribute()).name());
  FD_DQG("VioGeneratorModel::DoTypeCheck(): require trans  " 
	 << typeid(*pGeneratorConfig->mStateAttribute->Attribute()).name());
  FD_DQG("VioGeneratorModel::DoTypeCheck(): require trans  " 
	 << typeid(*pGeneratorConfig->mEventAttribute->Attribute()).name());

  // must have flags at least as specified (TODO: test)
  //if(vgen->TransRel().AttributeTry(*pGeneratorConfig->mTransAttribute->Attribute())) return 1;
  //if(vgen->States().AttributeTry(*pGeneratorConfig->mStateAttribute->Attribute())) return 1;
  //if(vgen->Alphabet().AttributeTry(*pGeneratorConfig->mEventAttribute->Attribute())) return 1;

  // passed
  return 0; 
}

// allocate visual model data
void VioGeneratorModel::DoVioAllocate(void) {
  FD_DQG("VioGeneratorModel::DoVioAllocate()");
  // prepare data container
  mpGeneratorData= new VioGeneratorData(this);
  mData=mpGeneratorData;
  // call base, incl virtual faudes object allocation
  VioModel::DoVioAllocate();
  // have typed pointer
  mpFaudesGenerator = dynamic_cast<faudes::vGenerator*>(mData->FaudesObject());
  // fix behaviour (default anyway, but we never know)
  mpFaudesGenerator->ReindexOnWrite(false);
  // set up representation models (order matters for style layout)
  FD_DQG("VioGeneratorModel::DoVioAllocate(): rep models for style layout " << mLayoutFlags);
  switch(mLayoutFlags & VioGeneratorStyle::LayoutMask) { 
  case VioGeneratorStyle::Generator: {
    mTransList =  new VioGeneratorListModel(this,VioElement::ETrans);
    InsertRepresentationModel(mTransList);
    mStateList =  new VioGeneratorListModel(this,VioElement::EState);
    InsertRepresentationModel(mStateList);
    mEventList =  new VioGeneratorListModel(this,VioElement::EEvent);
    InsertRepresentationModel(mEventList);
    mGraph  = new VioGeneratorGraphModel(this);
    InsertRepresentationModel(mGraph); 
    break;  
  } 
  default: break;
  } 
  // set up user layout
  FD_DQG("VioGeneratorModel::DoVioAllocate(): default user layout");
  mpUserLayout = new VioGeneratorLayout(this);
}

// update visual data from (new) faudes object
// nonstd: we do not notifyanychange to view,
// this is dealt with  by the abstract models
void VioGeneratorModel::DoVioUpdate(void) {
  FD_DQG("VioGeneratorModel::DoVioUpdate()");
  // have typed pointer 
  mpFaudesGenerator = dynamic_cast<faudes::vGenerator*>(mData->FaudesObject());
  // notify representation models 
  emit NotifyAnyChange();
  // done
  FD_DQG("VioGeneratorModel::DoVioUpdate(): done");
}



// convenience access to faudes generator
const faudes::Generator* VioGeneratorModel::Generator(void) const {
  return mpFaudesGenerator;
}

// convenience access to generator configuration
VioGeneratorStyle* VioGeneratorModel::GeneratorConfiguration(void) const {
  return pGeneratorConfig;
}

// token io: implementation
void VioGeneratorModel::DoVioWrite(faudes::TokenWriter& rTw) const {
  FD_DQG("VioGeneratorModel::DoVioWrite()");
  // begin model data
  rTw.WriteBegin("VioModels");
  // write model data
  for(int i=0; i< mModelList.size(); i++)
    mModelList.at(i)->Write(rTw);
  // end model data
  rTw.WriteEnd("VioModels");
  // write layout
  mpUserLayout->Write(rTw);
}

// token io: implementation
void VioGeneratorModel::DoVioRead(faudes::TokenReader& rTr) {
  FD_DQG("VioGeneratorModel::DoVioRead()");
  // begin tag
  rTr.ReadBegin("VioModels");
  // read models
  for(int i=0; i< mModelList.size(); i++) {
    // read per model, try to recover from errors
    int secl = rTr.Level();
    try{
      mModelList.at(i)->Read(rTr);
    } catch(faudes::Exception& ex) {
      bool res=rTr.Recover(secl);
      if(!res) throw(faudes::Exception(ex));
      FD_WARN("VioGeneratorModel(" << VioStyle::StrFromQStr(FaudesName()) << "::DoVioRead(): ignore invalid model data");
    }
  }
  // end tag
  rTr.ReadEnd("VioModels");
  // read layout
  mpUserLayout->Read(rTr);
  // report
  FD_DQG("VioGeneratorModel::DoVioRead(): done");
}

// connect another representation model (we take ownership)
void VioGeneratorModel::InsertRepresentationModel(VioGeneratorAbstractModel* repmodel) {
  FD_DQG("VioGeneratorModel::InsertRepresentationModel(" << typeid(repmodel).name());
  // take ownership
  repmodel->setParent(this);
  // record
  mModelList.append(repmodel);
  // connect element notification
  connect(this, SIGNAL(NotifyElementIns(const VioElement&)), 
    repmodel, SLOT(UpdateElementIns(const VioElement&)));
  connect(this, SIGNAL(NotifyElementDel(const VioElement&)), 
    repmodel, SLOT(UpdateElementDel(const VioElement&)));
  connect(this, SIGNAL(NotifyElementEdit(const VioElement&, const VioElement&)), 
    repmodel, SLOT(UpdateElementEdit(const VioElement&, const VioElement&)));
  connect(this, SIGNAL(NotifyElementProp(const VioElement&)), 
    repmodel, SLOT(UpdateElementProp(const VioElement&)));
  // connect global notification
  connect(this, SIGNAL(NotifyTrimElements(void)), 
    repmodel, SLOT(UpdateTrimElements(void)));
  connect(this, SIGNAL(NotifyAnyAttr(void)), 
    repmodel, SLOT(UpdateAnyAttr(void)));
  connect(this, SIGNAL(NotifyAnyChange(void)), 
    repmodel, SLOT(UpdateAnyChange(void)));
  // connect selection
  connect(this, SIGNAL(NotifySelectionElement(const VioElement&, bool)), 
    repmodel, SLOT(UpdateSelectionElement(const VioElement&, bool)));
  connect(this, SIGNAL(NotifySelectionClear(void)), 
    repmodel, SLOT(UpdateSelectionClear(void)));
  connect(this, SIGNAL(NotifySelectionAny(void)), 
    repmodel, SLOT(UpdateSelectionAny(void)));
  // modified notification
  connect(repmodel, SIGNAL(NotifyModified(bool)), this, SLOT(ChildModified(bool)));
  // done
  FD_DQG("VioGeneratorModel::InsertRepresentationModel(" << typeid(repmodel).name() << " done");
}


// convenience: exits element
bool VioGeneratorModel::ElementExists(const VioElement& elem) const {
  //FD_DQG("VioGeneratorModel::ElementExists("<< elem.Str() << ")");
  // switch types (for consise interface)
  switch(elem.Type()) {
  case VioElement::ETrans: 
    return mpFaudesGenerator->ExistsTransition(elem.Trans());
  case VioElement::EState: 
    return mpFaudesGenerator->ExistsState(elem.State());
  case VioElement::EEvent: 
    return mpFaudesGenerator->ExistsEvent(elem.Event());
  default:
    break;
  }
  // default: false
  return false;
}

// edit: ins element
VioElement VioGeneratorModel::ElementIns(const VioElement& elem) {
  FD_DQG("VioGeneratorModel::ElementIns("<< elem.Str() << ")");
  // prepare res: invalid by default
  VioElement res;
  // switch types (proce for consise interface)
  switch(elem.Type()) {
  //** insert a transition (dont insert invalid, but signal)
  case VioElement::ETrans: {
    // get the transition
    faudes::Transition ftrans=elem.Trans();
    // bail out if it exists
    if(mpFaudesGenerator->ExistsTransition(ftrans)) break;
    // bail out if states dont exists
    if(!mpFaudesGenerator->ExistsState(ftrans.X1)) break;
    if(!mpFaudesGenerator->ExistsState(ftrans.X2)) break;
    // if all members exist, do insert
    if(mpFaudesGenerator->ExistsState(ftrans.X1))
    if(mpFaudesGenerator->ExistsEvent(ftrans.Ev))
    if(mpFaudesGenerator->ExistsState(ftrans.X2))
      mpFaudesGenerator->SetTransition(ftrans);
    // res is the trans
    res.Trans(ftrans);
    break;
  }
  //** insert a state (create new on invalid)
  case VioElement::EState: {
    // get the state
    faudes:: Idx fstate=elem.State();
    // if state exists there is nothing to do
    if(mpFaudesGenerator->ExistsState(fstate)) break;
    // if its zero create a state
    if(fstate==0) fstate=mpFaudesGenerator->InsState();
    // insert to faudes generator
    mpFaudesGenerator->InsState(fstate);
    // res is the inserted state
    res.State(fstate);
    break;
  }
  //** insert an event (use default name)
  case VioElement::EEvent: {
    // get the event
    faudes:: Idx fev=elem.Event();
    // if event exists theres nothing to do
    if(mpFaudesGenerator->ExistsEvent(fev)) break;
    // figure name
    std::string evname=mpFaudesGenerator->EventName(fev);
    // if name is unknown, create std name (eq insert index zero)
    if(evname=="") evname=mpFaudesGenerator->UniqueEventName(VioStyle::EventSymbol());
    // insert to faudes generator
    fev=mpFaudesGenerator->InsEvent(evname);
    // res is the inserted event
    res=VioElement::FromEvent(fev);
    break;
  }
  default:
    break;
  }
  // if we have a result, emit notification
  if(!res.IsVoid()) { Modified(true); emit NotifyElementIns(res); }
  FD_DQG("VioGeneratorModel::ElementIns("<< res.Str() << "): done");
  return res;
}

// edit: del element
VioElement VioGeneratorModel::ElementDel(const VioElement& elem) {
  FD_DQG("VioGeneratorModel::ElementDel("<< elem.Str() << ")");
  // todo: selection
  // prepare res: void
  VioElement res;
  // delete in faudes generator ...
  switch(elem.Type()) {
  //** delete  a transition
  case VioElement::ETrans: {
    // pass on even if it does not exixt
    res=elem;
    // nothing to do if it does not exist
    if(!mpFaudesGenerator->ExistsTransition(elem.Trans())) break; 
    // delete
    mpFaudesGenerator->ClrTransition(elem.Trans()); 
    break;
  }
  //** delete a state
  case VioElement::EState: {
    // bail out if it does not exist
    if(!mpFaudesGenerator->ExistsState(elem.State())) break;
    // delete affected transitions
    QList<VioElement> rmtrans;
    faudes::TransSet::Iterator tit=mpFaudesGenerator->TransRelBegin();
    for(;tit!=mpFaudesGenerator->TransRelEnd();tit++) { 
      if(tit->X1==elem.State() || tit->X2==elem.State()) rmtrans.append(VioElement::FromTrans(*tit));
    }
    foreach(const VioElement& telem, rmtrans)
      ElementDel(telem);
    // delete state
    mpFaudesGenerator->DelState(elem.State()); 
    res=elem;
    break;
  }
  //** delete an event
  case VioElement::EEvent: {
    // bail out if it does not exist
    if(!mpFaudesGenerator->ExistsEvent(elem.Event())) break;
    // delete affected transitions
    QList<VioElement> rmtrans;
    faudes::TransSet::Iterator tit=mpFaudesGenerator->TransRelBegin();
    for(;tit!=mpFaudesGenerator->TransRelEnd();tit++) 
      if(tit->Ev==elem.Event()) rmtrans.append(VioElement::FromTrans(*tit));
    foreach(const VioElement& telem, rmtrans)
      ElementDel(telem);
    // delete event
    mpFaudesGenerator->DelEvent(elem.Event()); 
    res=elem;
    break;
  }
  default:
    break;
  }
  // if we have a result, emit notification
  if(!res.IsVoid()) { Modified(true); emit NotifyElementDel(res); }
  FD_DQG("VioGeneratorModel::ElementDel("<< res.Str() << "): done");
  return res;
}


// edit: move element (incl attributes)
VioElement VioGeneratorModel::ElementEdit(const VioElement& selem, const VioElement& delem) {
  FD_DQG("VioGeneratorModel::ElementEdit("<< selem.Str() << ", " << delem.Str() << ")");
  // prepare result
  VioElement res;
  // inconsitent types
  if(selem.Type()!=delem.Type()) return res;
  // if source and dest are the same
  if(selem==delem) {
    //emit NotifyElemetProp(selem); 
    return res;
  }
  // record selection
  bool sselected = IsSelected(selem); 
  if(sselected) Select(selem,false); 
  FD_DQG("VioGeneratorModel::ElementEdit("<< selem.Str() << ", " << delem.Str() << "): T2");
  // move in faudes generator ...
  switch(selem.Type()) {
  //** move a transition 
  case VioElement::ETrans: {
    // insist in destination to be valid
    if(mpFaudesGenerator->ExistsState(delem.Trans().X1))
    if(mpFaudesGenerator->ExistsEvent(delem.Trans().Ev))
    if(mpFaudesGenerator->ExistsState(delem.Trans().X2)) {
      // no other effects: just move, copy attribute if source available
      mpFaudesGenerator->SetTransition(delem.Trans());
      if(mpFaudesGenerator->ExistsTransition(selem.Trans())) {
        mpFaudesGenerator->TransAttribute(delem.Trans(),mpFaudesGenerator->TransAttribute(selem.Trans()));
        mpFaudesGenerator->ClrTransition(selem.Trans());
      }
    }
    emit NotifyElementEdit(selem,delem);
    res=delem;
    break;
  }
  //** move a state 
  case VioElement::EState: {
    // insist in source to exist
    if(!mpFaudesGenerator->ExistsState(selem.State())) break;
    // insert new state
    std::string name=mpFaudesGenerator->StateName(selem.State());
    mpFaudesGenerator->InsState(delem.State());
    mpFaudesGenerator->StateAttribute(delem.State(),mpFaudesGenerator->StateAttribute(selem.State()));
    mpFaudesGenerator->StateName(selem.State(),"");
    mpFaudesGenerator->StateName(selem.State(),name);
    emit NotifyElementEdit(selem,delem);
    // move affected transitions
    QList<VioElement> mvtrans;
    faudes::TransSet::Iterator tit=mpFaudesGenerator->TransRelBegin();
    for(;tit!=mpFaudesGenerator->TransRelEnd(); tit++) {
      if(tit->X1==selem.State() || tit->X2==selem.State()) mvtrans.append(VioElement::FromTrans(*tit));
    }
    foreach(const VioElement& stelem, mvtrans) {
      faudes::Transition dtrans=stelem.Trans();
      if(dtrans.X1==selem.State()) dtrans.X1=delem.State();
      if(dtrans.X2==selem.State()) dtrans.X2=delem.State();
      ElementEdit(stelem,VioElement::FromTrans(dtrans));
    }
    // delete old state
    mpFaudesGenerator->DelState(selem.State());
    emit NotifyElementDel(selem); 
    res=delem;
    break;
  }
  //** move an event
  case VioElement::EEvent: {
    // insist in source to exist
    if(!mpFaudesGenerator->ExistsEvent(selem.Event())) break;
    // bail out on unknown event index
    if(mpFaudesGenerator->EventName(delem.Event())=="") break;
    // insert new event
    mpFaudesGenerator->InsEvent(delem.Event());
    mpFaudesGenerator->EventAttribute(delem.Event(),mpFaudesGenerator->EventAttribute(selem.Event()));
    emit NotifyElementEdit(selem,delem);
    // move affected transitions
    QList<VioElement> mvtrans;
    faudes::TransSet::Iterator tit=mpFaudesGenerator->TransRelBegin();
    for(;tit!=mpFaudesGenerator->TransRelEnd(); tit++) {
      if(tit->Ev==selem.Event()) mvtrans.append(VioElement::FromTrans(*tit));
    }
    foreach(const VioElement& stelem, mvtrans) {
      faudes::Transition dtrans=stelem.Trans();
      dtrans.Ev=delem.Event();
      ElementEdit(stelem,VioElement::FromTrans(dtrans));
    }
    // delete old event
    mpFaudesGenerator->DelEvent(selem.Event());
    // emit NotifyElementDel(selem); (included in move notification)
    res=delem;
    break;
  }
  default: break;
  }
  // if we have a result, fix selection
  FD_DQG("VioGeneratorModel::ElementEdit("<< res.Str() << "): done");
  if(sselected) Select(res,true); 
  if(!res.IsVoid()) Modified(true);
  return res;
}

// edit: name element
VioElement VioGeneratorModel::ElementName(const VioElement& elem, const QString& name) {
  FD_DQG("VioGeneratorModel::ElementName("<< elem.Str() << ", " << VioStyle::StrFromQStr(name) << ")");
  // prepare result
  VioElement res;
  std::string newname=VioStyle::StrFromQStr(name);
  // if elem non existent, do nothing
  if(!elem.IsVoid())
  if(!ElementExists(elem)) return res;
  // todo: selection 1. delete 2. move
  // rename in faudes generator ...
  switch(elem.Type()) {
  //** transitions have no name
  case VioElement::ETrans: {
    break;
  }
  //** rename a state
  case VioElement::EState: {
    // bail out if nothing to do
    if(mpFaudesGenerator->StateName(elem.State())==newname) break;
    if(mpFaudesGenerator->ExistsState(newname)) break;
    // rename in faudes generator
    mpFaudesGenerator->StateName(elem.State(),newname);
    emit NotifyElementProp(elem);
    res=elem;
    break;
  }
  //** rename an event
  case VioElement::EEvent: {
    // bail out if new name exits or is unchanged
    if(mpFaudesGenerator->ExistsEvent(newname)) break;
    if(mpFaudesGenerator->EventName(elem.Event())==newname) break;
    // get new index/name global and use move
    faudes::Idx nidx = mpFaudesGenerator->InsEvent(newname);
    VioElement delem = VioElement::FromEvent(nidx);
    res=ElementEdit(elem,delem);
    break;
  }
  //** rename the generator
  case VioElement::EVoid: {
    // has it changed? return non-void
    if(mpFaudesGenerator->Name()!=newname) res=VioElement::FromEvent(0);
    // set the name
    mpFaudesGenerator->Name(newname);
    break;
  }
  default: break;
  }
  FD_DQG("VioGeneratorModel::ElementName("<< elem.Str() << "): done");
  if(!res.IsVoid()) { Modified(true);}
  return res;
}

// edit: attribute element
VioElement VioGeneratorModel::ElementAttr(const VioElement& elem, const faudes::AttributeVoid& attr) {
  FD_DQG("VioGeneratorModel::ElementAttr("<< elem.Str() << ", " << attr.ToString() << ")");
  // prepare res
  VioElement res=elem; 
  // if elem non existent, do nothing
  if(!ElementExists(elem)) return res;
  // if the attribute is equal to the existing, do nothing
  if(ElementAttrTest(elem,attr)) return res;
  // a reference
  const faudes::AttributeVoid* pattr=&attr;
  // tweak state
  faudes::AttributeFlags* cattr=0;
  if(elem.IsState())
  if(const faudes::AttributeFlags* fattr=dynamic_cast<const faudes::AttributeFlags*>(pattr)) {
    FD_DQG("VioGeneratorModel::ElementAttr("<< res.Str() << "): tweak state flags");
    bool init = fattr->mFlags &   0x80000000;
    bool marked = fattr->mFlags & 0x40000000;
    if(init) mpFaudesGenerator->SetInitState(elem.State());
    else mpFaudesGenerator->ClrInitState(elem.State());
    if(marked) mpFaudesGenerator->SetMarkedState(elem.State());
    else mpFaudesGenerator->ClrMarkedState(elem.State());
    if(init || marked) {
      // need a copy
      cattr = static_cast<faudes::AttributeFlags*>(pattr->Copy());
      cattr->mFlags &= 0x3fffffff;
      pattr=cattr;
    }
  }
  // figure and set attribute
  //todo: fix libfaudes error in attributry: void takes all and ignores
  //todo: add type attribute type test on generator level
  FD_DQG("VioGeneratorModel::ElementAttr("<< res.Str() << "): " << typeid(attr).name() << " set in " << mpFaudesGenerator->Name() << " to " << pattr->ToString());
  try {
  switch(elem.Type()) {
    case VioElement::ETrans: mpFaudesGenerator->TransAttribute(elem.Trans(),*pattr); break;
    case VioElement::EState: mpFaudesGenerator->StateAttribute(elem.State(),*pattr); break;
    case VioElement::EEvent: mpFaudesGenerator->EventAttribute(elem.Event(),*pattr); break;
    case VioElement::EVoid: mpFaudesGenerator->GlobalAttribute(attr); break;
    default: break;
  }
  } catch(faudes::Exception& exception) {
  } 
  // dispose copy
  if(cattr) delete cattr;
  // done
  Modified(true); 
  emit NotifyElementProp(res); 
  FD_DQG("VioGeneratorModel::ElementAttr("<< res.Str() << "): done");
  return res;
}

// edit: query attribute
faudes::AttributeFlags* VioGeneratorModel::ElementAttr(const VioElement& elem) const {
  //FD_DQG("VioGeneratorModel::ElementAttr("<< elem.Str() << "): get");
  // prepare for return on heap
  faudes::AttributeFlags* res=0;
  const faudes::AttributeVoid* attr=0; 
  // figure attribute as const ref
  if(ElementExists(elem)) {
    switch(elem.Type()) {
    case VioElement::ETrans: attr=&mpFaudesGenerator->TransAttribute(elem.Trans()); break;
    case VioElement::EState: attr=&mpFaudesGenerator->StateAttribute(elem.State()); break;
    case VioElement::EEvent: attr=&mpFaudesGenerator->EventAttribute(elem.Event()); break;
    default: break;
    }
  }
  if(elem.IsVoid()) {
    attr=&mpFaudesGenerator->GlobalAttribute();
  }
  //FD_DQG("VioGeneratorModel::ElementAttr("<< elem.Str() << "): got " << attr->ToString()
  //	  << " ctype " << typeid(*attr).name());
  // if it is derived from flags, allocate and copy
  const faudes::AttributeFlags* fattr=dynamic_cast<const faudes::AttributeFlags*>(attr);
  if(fattr) {
    res=fattr->Copy();
  }
  // else, pretend flags attribute
  if(!res) {
    res=new faudes::AttributeFlags();
  }
  // tweak state flags
  if(elem.IsState()) {
    res->mFlags &= 0x3fffffff;
    if(mpFaudesGenerator->ExistsInitState(elem.State())) res->mFlags |= 0x80000000;
    if(mpFaudesGenerator->ExistsMarkedState(elem.State())) res->mFlags |= 0x40000000;
  }
  // done
  //FD_DQG("VioGeneratorModel::ElementAttr("<< elem.Str() << "): ret " << res->ToString()
  //	 << " ctype " << typeid(*res).name());
  return res;
}

// edit: test attribute (true for equal)
bool VioGeneratorModel::ElementAttrTest(const VioElement& elem, const faudes::AttributeVoid& attr) const {
  // get the attribute 
  faudes::AttributeVoid* elattr=ElementAttr(elem);
  // compare
  bool res = elattr->Equal(attr);
  // dispose
  delete elattr;
  // report
  return res;
}

// query attribute configuration
const VioAttributeStyle* VioGeneratorModel::AttributeConfiguration(VioElement::EType etype) const {
  switch(etype) {
    case VioElement::ETrans: 
      return pGeneratorConfig->mTransAttribute->AttributeConfiguration();
    case VioElement::EState: 
      return pGeneratorConfig->mStateAttribute->AttributeConfiguration();
    case VioElement::EEvent: 
      return pGeneratorConfig->mEventAttribute->AttributeConfiguration();
  default: 
     return pGeneratorConfig->mGlobalAttribute->AttributeConfiguration();
  }
  return 0;
}

// query boolean properts definitions
const QList<VioBooleanProperty>& VioGeneratorModel::ElementBooleanProperties(VioElement::EType etype) const {
  return AttributeConfiguration(etype)->BooleanProperties();
}

// query boolean property
bool VioGeneratorModel::ElementBooleanProperty(const VioElement& elem, int prop) const {
  //FD_DQG("VioGeneratorModel::ElementBooleanProperty("<< elem.Str() << "): query " << prop);
  faudes::AttributeFlags* attr=ElementAttr(elem);
  //FD_DQG("VioGeneratorModel::ElementBooleanProperty("<< elem.Str() << "): flags " << faudes::ToStringInteger16(attr->mFlags));
  const QList<VioBooleanProperty>& props= ElementBooleanProperties(elem.Type());
  if(prop<0 || prop>=props.size()) return false;
  bool bp =  props.at(prop).Test(attr->mFlags);
  delete attr;
  //FD_DQG("VioGeneratorModel::ElementBooleanProperty("<< elem.Str() << "): got " << bp);
  return bp;
}

// edit boolean property (return true on change)
bool VioGeneratorModel::ElementBooleanProperty(const VioElement& elem, int prop, bool val) {
  FD_DQG("VioGeneratorModel::ElementBooleanProperty("<< elem.Str() << "): set prop " << prop << " to "<< val);
  faudes::AttributeFlags* attr=ElementAttr(elem);
  const QList<VioBooleanProperty>& props= ElementBooleanProperties(elem.Type());
  if(prop<0 || prop>=props.size()) return false;
  bool oldval =props.at(prop).Test(attr->mFlags);
  if(val) props.at(prop).Set(attr->mFlags);
  else props.at(prop).Clr(attr->mFlags);
  ElementAttr(elem,*attr);
  delete attr;
  FD_DQG("VioGeneratorModel::ElementBooleanProperty("<< elem.Str() << "): done ");
  return oldval!=val;
}

// debug string
std::string VioGeneratorModel::ElementStr(const VioElement& elem) const {
  std::string res;
  switch(elem.Type()) {
    case VioElement::ETrans: res=mpFaudesGenerator->TStr(elem.Trans()); break;
    case VioElement::EState: res=mpFaudesGenerator->SStr(elem.State()); break;
    case VioElement::EEvent: res=mpFaudesGenerator->EStr(elem.Event()); break;
    default: res=elem.Str();
  }
  return res;
}


// seletion: uniform type
void VioGeneratorModel::Select(const VioElement& elem, bool on) {
  // ignore void
  if(elem.IsVoid()) return;  
  // ignore invalid except for transitions
  if(!elem.IsValid() && !elem.IsTrans()) return;  
  // report
  FD_DQG("VioGeneratorModel::Select(): #" << mSelection.size() << " " << elem.Str() << " to " << on);
  // uniform type
  if(on && mSelection.size()>0)
    if(mSelection.at(0).Type()!=elem.Type()) 
      SelectionClear();
  // let base do it
  VioModel::Select(elem,on);
}

// seletion: all
void VioGeneratorModel::SelectAllStates(void) {
  // reimplement to avoid per element signals
  mSelection.clear();
  faudes::StateSet::Iterator sit=mpFaudesGenerator->StatesBegin();
  for(;sit!=mpFaudesGenerator->StatesEnd();sit++)  
    mSelection.append(VioElement::FromState(*sit));
  // emit
  emit NotifySelectionAny();
}

// seletion: all
void VioGeneratorModel::SelectAllTransitions(void) {
  // reimplement to avoid per element signals
  mSelection.clear();
  faudes::TransSet::Iterator tit=mpFaudesGenerator->TransRelBegin();
  for(;tit!=mpFaudesGenerator->TransRelEnd();tit++)  
    mSelection.append(VioElement::FromTrans(*tit));
  // emit
  emit NotifySelectionAny();
}


// record changes and emit signal 
void VioGeneratorModel::Modified(bool ch) { 
  // call base
  VioModel::Modified(ch);
  // pass on clr to my childs
  if(!ch) {
    for(int i=0; i< mModelList.size(); i++)
      mModelList.at(i)->Modified(false);
  }
}


// get all data
VioData* VioGeneratorModel::Data(void) {
  FD_DQG("VioGeneratorModel::Data(): get");
  VioGeneratorData* gdat= new VioGeneratorData();
  // copy faudes generator
  faudes::vGenerator* gen = mpFaudesGenerator->Copy();
  gdat->FaudesObject(gen);
  // get all abstract model data
  for(int i=0; i< mModelList.size(); i++) {
    VioGeneratorAbstractData* vdat= mModelList.at(i)->Data();
    if(vdat) gdat->mDataList.append(vdat);
  }
  FD_DQG("VioGeneratorModel::Data(): done with states# " << gen->Size());
  // done
  return gdat;
}


// get selection data
VioData* VioGeneratorModel::SelectionData(void) {
  FD_DQG("VioGeneratorModel::SelectionData()");
  VioGeneratorData* gdat= new VioGeneratorData();
  // copy selected portion of faudes generator
  faudes::vGenerator* gen = mpFaudesGenerator->New();
  foreach(VioElement elem, mSelection) {
    if(!elem.IsValid()) continue;
    switch(elem.Type()) {
    case VioElement::ETrans: 
      // state x1
      gen->InsState(elem.Trans().X1);
      if(mpFaudesGenerator->ExistsMarkedState(elem.Trans().X1)) gen->SetMarkedState(elem.Trans().X1);
      if(mpFaudesGenerator->ExistsInitState(elem.Trans().X1)) gen->SetInitState(elem.Trans().X1);
      gen->StateName(elem.Trans().X1, mpFaudesGenerator->StateName(elem.Trans().X1));
      gen->StateAttribute(elem.Trans().X1, mpFaudesGenerator->StateAttribute(elem.Trans().X1)); 
      // state x2
      gen->InsState(elem.Trans().X2); 
      if(mpFaudesGenerator->ExistsMarkedState(elem.Trans().X2)) gen->SetMarkedState(elem.Trans().X2);
      if(mpFaudesGenerator->ExistsInitState(elem.Trans().X2)) gen->SetInitState(elem.Trans().X2);
      gen->StateName(elem.Trans().X2, mpFaudesGenerator->StateName(elem.Trans().X2));
      gen->StateAttribute(elem.Trans().X2, mpFaudesGenerator->StateAttribute(elem.Trans().X2)); 
      // event
      gen->InsEvent(elem.Trans().Ev); 
      gen->EventAttribute(elem.Trans().Ev, mpFaudesGenerator->EventAttribute(elem.Trans().Ev));
      // trans 
      gen->SetTransition(elem.Trans()); 
      gen->TransAttribute(elem.Trans(), mpFaudesGenerator->TransAttribute(elem.Trans())); 
      break;
    case VioElement::EState: 
      gen->InsState(elem.State()); 
      if(mpFaudesGenerator->ExistsMarkedState(elem.State())) gen->SetMarkedState(elem.State());
      if(mpFaudesGenerator->ExistsInitState(elem.State())) gen->SetInitState(elem.State());
      gen->StateAttribute(elem.State(), mpFaudesGenerator->StateAttribute(elem.State())); 
      gen->StateName(elem.State(), mpFaudesGenerator->StateName(elem.State()));      
      break;
    case VioElement::EEvent: 
      gen->InsEvent(elem.Event());
      gen->EventAttribute(elem.Event(), mpFaudesGenerator->EventAttribute(elem.Event())); 
      break;
    default: break;
    }
  }  
  gen->GlobalAttribute(mpFaudesGenerator->GlobalAttribute()); 
  gdat->FaudesObject(gen);
  // get selected abstract model data
  for(int i=0; i< mModelList.size(); i++) {
    VioGeneratorAbstractData* vdat= mModelList.at(i)->SelectionData();
    if(vdat) gdat->mDataList.append(vdat);
  }
  FD_DQG("VioGeneratorModel::SelectionData(): #" << gdat->mDataList.size() << " records out of " << mModelList.size());
  return gdat;
}

// test data (return 0 on accept)
int VioGeneratorModel::TypeCheckData(const VioData* pData) {
  FD_DQT("VioGeneratorModel::TypeCheckData(): test types: A");
  const VioGeneratorData* gdat= qobject_cast<const VioGeneratorData*>(pData);
  if(!gdat) return 1;
  FD_DQT("VioGeneratorModel::TypeCheckData(): test types: B");
  const faudes::vGenerator* gen = dynamic_cast<const faudes::vGenerator*>(pData->FaudesObject());
  if(!gen) return 1;
  // ok
  FD_DQT("VioGeneratorModel::TypeCheckData(): ok: generator #states="  << gen->Size() << 
	 " #events=" << gen->AlphabetSize());
  return 0;
}


// merge data (return 1 on changes, assume type ok, select new items)
int VioGeneratorModel::DoMergeData(const VioData* pData) {
  FD_DQT("VioGeneratorModel::DoMergeData()");
  const VioGeneratorData* gdat= qobject_cast<const VioGeneratorData*>(pData);
  const faudes::vGenerator* gen = dynamic_cast<const faudes::vGenerator*>(pData->FaudesObject());
  //gen->Write();
  // track changes
  bool changed=false;
  VioElement selelem;
  // insert states: record map
  QMap<faudes::Idx,faudes::Idx> dstidx;
  faudes::StateSet::Iterator sit=gen->StatesBegin();
  for(;sit!=gen->StatesEnd(); sit++) {
    faudes::Idx x=*sit;
    // inser the state as new index
    if(mpFaudesGenerator->ExistsState(x)) 
      x=mpFaudesGenerator->InsState();
    else
      mpFaudesGenerator->InsState(x);
    // record new index
    dstidx[*sit]=x;
    // figure name
    std::string xstr=gen->StateName(*sit); 
    if(xstr!="") {
      xstr=mpFaudesGenerator->UniqueStateName(xstr); 
      mpFaudesGenerator->StateName(x,xstr); 
    }
    // figure attribute
    if(gen->ExistsInitState(*sit)) mpFaudesGenerator->SetInitState(x);
    if(gen->ExistsMarkedState(*sit)) mpFaudesGenerator->SetMarkedState(x);
    mpFaudesGenerator->StateAttribute(x, gen->StateAttribute(*sit));
    // track selection
    if(selelem.Type()==VioElement::EState || selelem.Type()==VioElement::EVoid) { 
      selelem=VioElement::FromState(x);
      mSelection.append(selelem);
    }
    changed=true;
  }
  // insert events
  faudes::EventSet::Iterator eit=gen->AlphabetBegin();
  for(;eit!=gen->AlphabetEnd(); eit++) {
    mpFaudesGenerator->InsEvent(*eit);
    mpFaudesGenerator->EventAttribute(*eit, gen->EventAttribute(*eit));
    if(selelem.Type()==VioElement::EEvent || selelem.Type()==VioElement::EVoid) {
      selelem=VioElement::FromEvent(*eit);
      mSelection.append(selelem);
    }
    changed=true;
  }
  // insert transitions
  faudes::TransSet::Iterator tit=gen->TransRelBegin();
  for(;tit!=gen->TransRelEnd(); tit++) {
    faudes::Transition ftrans(dstidx[tit->X1],tit->Ev,dstidx[tit->X2]);
    mpFaudesGenerator->SetTransition(ftrans);
    mpFaudesGenerator->TransAttribute(ftrans, gen->TransAttribute(*tit));
  }
  // pass on abstract model data
  FD_DQT("VioGeneratorModel::DoMargeData(): insert abstract model data #"  << gdat->mDataList.size());
  for(int i=0; i<gdat->mDataList.size(); i++) {
    VioGeneratorAbstractData* vdat= gdat->mDataList.at(i);
    vdat->ApplyStateIndicees(dstidx);
    // try for each model
    for(int j=0; j< mModelList.size(); j++) {
      if(mModelList.at(j)->Data(vdat)==0) break;
    }
  }
  return changed;
}


// vio data access: set (0 on accept)
int VioGeneratorModel::Data(const VioData* pData) {
  FD_DQT("VioGeneratorModel::Data(): set all data");
  // reject wrong data type
  if(TypeCheckData(pData)!=0) return 1;
  // clear and merge
  Clear(); // incl clear selection??
  bool changed=DoMergeData(pData);
  if(changed) {
    //Modified(true);
    emit NotifyAnyChange();       // universal update
  }
  // selction changed... perhaps
  SelectionClear();
  emit VioModel::NotifySelectionAny();    // universal selection change
  return 0;
};


// insert data (return 0 on accept)
int VioGeneratorModel::InsertData(const VioData* pData) {
  FD_DQT("VioGeneratorModel::InsertData()");
  if(TypeCheckData(pData)!=0) return 1;
  const VioGeneratorData* gdat= qobject_cast<const VioGeneratorData*>(pData);
  const faudes::vGenerator* gen = dynamic_cast<const faudes::vGenerator*>(pData->FaudesObject());
  // do the insert (incl selection)
  bool changed=DoMergeData(pData);
  FD_DQG("VioGeneratorModel::InsertData(): changed " << changed);
  // modified and signals
  if(changed) {
    Modified(true);
    emit NotifyAnyChange();       // universal update
  }
  // selction changed... perhaps
  emit NotifySelectionAny();    // universal selection change
  // done
  return 0;
}


// delete selection
void VioGeneratorModel::DeleteSelection(void) {
  FD_DQG("VioGeneratorModel::DeleteSelection()");
  foreach(VioElement elem, mSelection) ElementDel(elem);
  FD_DQG("VioGeneratorModel::DeleteSelection(): done ");
}



/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioGeneratorAbstractModel

****************************************************************
****************************************************************
****************************************************************
*/

// construct
VioGeneratorAbstractModel::VioGeneratorAbstractModel(VioGeneratorModel* parent) : 
  QObject(parent),
  mModified(false) 
{
  FD_DQG("VioGeneratorAbstractModel::VioGeneratorAbstractModel()");
  pVioGeneratorModel=parent;
}

// create new view for this representationmodel
VioGeneratorAbstractView* VioGeneratorAbstractModel::NewView(VioGeneratorView* parent) {
  FD_DQG("VioGeneratorAbstractModel::NewView()");
  // create
  VioGeneratorAbstractView* view = new VioGeneratorAbstractView(parent);
  // set model (incl connect)
  view->Model(this);
  // done
  return view;
}

// convenience access to generator model
VioGeneratorModel* VioGeneratorAbstractModel::GeneratorModel(void) {
  return pVioGeneratorModel;
}

// convenience access to generator style
VioGeneratorStyle* VioGeneratorAbstractModel::GeneratorConfiguration(void) const {
  return pVioGeneratorModel->GeneratorConfiguration();
}

// convenience access to faudes generator
const faudes::Generator* VioGeneratorAbstractModel::Generator(void) const {
  return pVioGeneratorModel->Generator();
}

 
// token io: pass through to virtual
void VioGeneratorAbstractModel::Write(faudes::TokenWriter& rTw) const {
  DoVioWrite(rTw);
}

// token io: pass through to virtual
void VioGeneratorAbstractModel::Write(const QString& rFilename) const {
  try {
    faudes::TokenWriter tw(VioStyle::LfnFromQStr(rFilename));
    Write(tw);
  }
  catch (std::ios::failure&) {
    std::stringstream errstr;
    errstr << "Exception opening/writing file \"" << VioStyle::StrFromQStr(rFilename) << "\"";
    throw faudes::Exception("VioGeneratorAbstractModel::Write", errstr.str(), 2);
  }
}

// token io: pass through to virtual
void VioGeneratorAbstractModel::Read(faudes::TokenReader& rTr) {
  DoVioRead(rTr);
}

// token io: pass through to virtual
void VioGeneratorAbstractModel::Read(const QString& rFilename) {
  faudes::TokenReader tr(VioStyle::LfnFromQStr(rFilename));
  Read(tr);
}

// update selction notification: default to any, which is not implemented
void VioGeneratorAbstractModel::UpdateSelectionElement(const VioElement& elem, bool on) 
{ (void) elem; (void) on; UpdateSelectionAny();};
void VioGeneratorAbstractModel::UpdateSelectionClear(void) 
  { UpdateSelectionAny();};
void VioGeneratorAbstractModel::UpdateSelectionAny(void) 
  { };

// update edit notification: default to update all
void VioGeneratorAbstractModel::UpdateElementIns(const VioElement& elem) 
  { (void) elem; UpdateAnyChange();};
void VioGeneratorAbstractModel::UpdateElementDel(const VioElement& elem) 
  { (void) elem; UpdateAnyChange();};
void VioGeneratorAbstractModel::UpdateElementEdit(const VioElement& selem, const VioElement& delem) 
  { (void) selem; (void) delem; UpdateAnyChange();};
void VioGeneratorAbstractModel::UpdateElementProp(const VioElement& elem) 
  { (void) elem; UpdateAnyChange();};
void VioGeneratorAbstractModel::UpdateTrimElements(void) 
  { UpdateAnyChange();};
void VioGeneratorAbstractModel::UpdateAnyAttr(void) 
  { UpdateAnyChange();};
void VioGeneratorAbstractModel::UpdateAnyChange(void) 
  { UpdateNewModel();};


// update notification: reimplement in derivate
void VioGeneratorAbstractModel::UpdateNewModel(void) { 
  FD_DQG("VioGeneratorAbstractModel::UpdateNewModel()");
  // update visual data from (new) faudes object
  DoVioUpdate();
}


// update visual data from (new) faudes object
void VioGeneratorAbstractModel::DoVioUpdate(void) { 
  FD_DQG("VioGeneratorAbstractModel::DoVioUpdate()");
  // derived classes to reimplement
  // pass on to view
  emit NotifyAnyChange();
}

// query changes (dont emit signal)
bool VioGeneratorAbstractModel::Modified(void) const { 
  return mModified;
};

// collect and pass on modifications of childs
void VioGeneratorAbstractModel::ChildModified(bool changed) { 
  // ignre netagtives
  if(!changed) return;
  // report
  FD_DQG("VioGeneratorAbstractModel::ChildModified(1): model modified " << mModified);
  Modified(true);
};

// record changes and emit signal)
void VioGeneratorAbstractModel::Modified(bool ch) {
  // set
  if(!mModified && ch) {
    mModified=true;
    emit NotifyModified(mModified);
  }
  // clr
  if(mModified && !ch) {
    mModified=false;
    FD_DQG("VioGeneratorAbstractModel::Modified(" << this << "): emit modified notification");
    emit NotifyModified(mModified);
  }
}


/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioGeneratorView

****************************************************************
****************************************************************
****************************************************************
*/

// construct 
VioGeneratorView::VioGeneratorView(QWidget* parent, VioStyle* config, bool alloc) : 
  VioView(parent,config,false), 
  pGeneratorModel(0),
  mUserLayout(0),
  mSplitter(0),
  mTabbed(0),
  pListView(0), 
  pGraphView(0), 
  mPropView(0), 
  mToggleAction(0),
  mPropAction(0),
  mZoomInAction(0),
  mZoomOutAction(0),
  mZoomFitAction(0),
  mExportAction(0),
  mSelectAllStatesAction(0),
  mSelectAllTransitionsAction(0)
{
  FD_DQG("VioGeneratorView::VioGeneratorView(): " << VioStyle::StrFromQStr(FaudesType()));
  // typed version of configuration
  pGeneratorConfig = dynamic_cast<VioGeneratorStyle*>(pConfig);
  if(!pGeneratorConfig) {
    FD_WARN("VioGeneratorModel::VioGeneratorModel(): invalid style, using default.");
    pGeneratorConfig= new VioGeneratorStyle(mFaudesType);
  }
  mLayoutFlags=pGeneratorConfig->mLayoutFlags;
  // my alloc
  if(alloc) DoVioAllocate();
  FD_DQG("VioGeneratorView::VioGeneratorView(): done");
}

// destruct
VioGeneratorView::~VioGeneratorView(void) {
}

// allocate my data
void VioGeneratorView::DoVioAllocate(void) {
  FD_DQG("VioGeneratorView::DoVioAllocate(): create layout");
  // allocate base
  VioView::DoVioAllocate();
  // debugging widget
#ifdef FAUDES_DEBUG_VIO_WIDGETS
  mTextInfo->setText(tr("Generator editing widget \"%1\" opt %2").arg(FaudesType()).arg(mLayoutFlags));
  mTextEdit->setReadOnly(true);
  mApplyButton->hide();
#endif
  FD_DQG("VioGeneratorView::DoVioAllocalte(): layout for type " << VioStyle::StrFromQStr(FaudesType()));
  // prepare action: toggle view
  mToggleAction=new QAction("Select Views",this);
  mToggleAction->setStatusTip("show/hide views");
  mToggleAction->setEnabled(false);
  mToggleAction->setShortcut(tr("Ctrl+#"));
  mViewActions.append(mToggleAction);
  connect(mToggleAction, SIGNAL(triggered(bool)), this, SLOT(ToggleViews())); 
  // my splitter
  mSplitter = new QSplitter();
  mSplitter->setMinimumSize(QSize(100,100));
  mSplitter->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
  mVbox->addWidget(mSplitter);
  connect(mSplitter,SIGNAL(splitterMoved(int,int)),this,SLOT(TrackSplitter(void)),Qt::QueuedConnection);
  // create views by base layout
  switch(mLayoutFlags & VioGeneratorStyle::LayoutMask) {
  // std generator layout
  case VioGeneratorStyle::Generator: {
    // layout supports toggle
    mToggleAction->setEnabled(true);
    // my list tab
    mTabbed = new QTabWidget();
    mTabbed->setContentsMargins(0,0,0,0);
    mTabbed->setMinimumWidth(150);
    mTabbed->setSizePolicy(QSizePolicy(QSizePolicy::Minimum,QSizePolicy::Expanding));
    pListView=mTabbed;
    // my three tse list views
    VioGeneratorAbstractView* lview; 
    lview= new VioGeneratorListView();
    connect(this, SIGNAL(NotifyShow(const VioElement&)), lview, SLOT(Show(const VioElement&)));
    connect(this, SIGNAL(NotifyHighlite(const VioElement&, bool)), lview, SLOT(Highlite(const VioElement&,bool)));
    connect(this, SIGNAL(NotifyHighliteClear(void)), lview, SLOT(HighliteClear(void)));
    mTabbed->addTab(lview, "Transitions");
    mViewList.append(lview);
    lview= new VioGeneratorListView(this);
    connect(this, SIGNAL(NotifyShow(const VioElement&)), lview, SLOT(Show(const VioElement&)));
    connect(this, SIGNAL(NotifyHighlite(const VioElement&, bool)), lview, SLOT(Highlite(const VioElement&,bool)));
    connect(this, SIGNAL(NotifyHighliteClear(void)), lview, SLOT(HighliteClear(void)));
    mTabbed->addTab(lview, "States");
    mViewList.append(lview);
    lview= new VioGeneratorListView();
    connect(this, SIGNAL(NotifyShow(const VioElement&)), lview, SLOT(Show(const VioElement&)));
    connect(this, SIGNAL(NotifyHighlite(const VioElement&, bool)), lview, SLOT(Highlite(const VioElement&,bool)));
    connect(this, SIGNAL(NotifyHighliteClear(void)), lview, SLOT(HighliteClear(void)));
    mTabbed->addTab(lview, "Alphabet");
    mViewList.append(lview);
    // have tabbed in splitter
    mSplitter->addWidget(mTabbed);
    // 4th model is the graph
    VioGeneratorAbstractView* gview;
    gview = new VioGeneratorGraphView(this);
    gview->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Expanding));
    connect(this, SIGNAL(NotifyShow(const VioElement&)), gview, SLOT(Show(const VioElement&)));
    connect(this, SIGNAL(NotifyHighlite(const VioElement&, bool)), gview, SLOT(Highlite(const VioElement&,bool)));
    connect(this, SIGNAL(NotifyHighliteClear(void)), gview, SLOT(HighliteClear(void)));
    mSplitter->addWidget(gview);
    mViewList.append(gview);
    pGraphView=qobject_cast<VioGeneratorGraphView*>(gview);
    // adjust actions
    mToggleAction->setText("&Graph/List");
    break;
  }
  // invalid layout, list only fallback 
  default: {
    // layout supports toggle
    mToggleAction->setEnabled(true);
    // my list tab
    mTabbed = new QTabWidget();
    mTabbed->setContentsMargins(0,0,0,0);
    pListView=mTabbed;
    // my three tse list views
    VioGeneratorAbstractView* lview; 
    lview= new VioGeneratorListView(this);
    mTabbed->addTab(lview, "Transitions");
    mViewList.append(lview);
    lview= new VioGeneratorListView(this);
    mTabbed->addTab(lview, "States");
    mViewList.append(lview);
    lview= new VioGeneratorListView(this);
    mTabbed->addTab(lview, "Alphabet");
    mViewList.append(lview);
    // have tabbed in splitter
    mSplitter->addWidget(mTabbed);
    mToggleAction->setEnabled(false);
    break;
  }
  } // end: switch
  // add layout options: propertyview
  if(mLayoutFlags & VioGeneratorStyle::Properties) {
    mPropView = new VioGeneratorPropertyView(this, pGeneratorConfig);
    mSplitter->addWidget(mPropView);
  }
  // more actions: prop on/off
  if(mPropView) {
    mPropAction=new QAction("&Properties",this);
    mPropAction->setStatusTip("show/hide property view");
    mPropAction->setCheckable(true);
    mPropAction->setEnabled(true);
    mPropAction->setChecked(!mPropView->isHidden());
    mViewActions.append(mPropAction); 
    connect(mPropAction, SIGNAL(triggered(bool)), this, SLOT(ShowPropertyView(bool))); 
  }
  // more actions: zoom
  if(pGraphView) {
    QAction* sep= new QAction(this);
    sep->setSeparator(true);
    mViewActions.append(sep); // destool hack, good for viodiag, too
    mZoomInAction=new QAction("Zoom In",this);
    mZoomInAction->setStatusTip("zoom graph view");
    mZoomInAction->setEnabled(true);
    mZoomInAction->setShortcut(tr("Ctrl++"));
    mViewActions.append(mZoomInAction);
    connect(mZoomInAction, SIGNAL(triggered()), this, SLOT(ZoomIn())); 
    mZoomOutAction=new QAction("Zoom Out",this);
    mZoomOutAction->setStatusTip("zoom graph view");
    mZoomOutAction->setEnabled(true);
    mZoomOutAction->setShortcut(tr("Ctrl+-"));
    mViewActions.append(mZoomOutAction);
    connect(mZoomOutAction, SIGNAL(triggered()), this, SLOT(ZoomOut())); 
    mZoomFitAction=new QAction("Fit in View",this);
    mZoomFitAction->setStatusTip("zoom graph view");
    mZoomFitAction->setEnabled(true);
    mZoomFitAction->setShortcut(tr("Ctrl+*"));
    mViewActions.append(mZoomFitAction);
    connect(mZoomFitAction, SIGNAL(triggered(bool)), this, SLOT(ZoomFit())); 
  }
  // more actions: grid
  if(pGraphView) {
    mGridAction=new QAction("Show Grid",this);
    mGridAction->setStatusTip("show grid in graph view");
    mGridAction->setEnabled(true);
    mGridAction->setCheckable(true);
    mGridAction->setShortcut(tr("Ctrl+."));
    mViewActions.append(mGridAction);
    connect(mGridAction, SIGNAL(triggered(bool)), this, SLOT(GridVisible(bool))); 
  }
  // more actions: graph export
  if(pGraphView) {
  FD_DQG("VioGeneratorView::DoVioUpdate(): have xport action");
    mExportAction = new QAction(tr("Export Graph as Image ..."),this);
    mExportAction->setEnabled(true);
    mFileActions.append(mExportAction);
    connect(mExportAction, SIGNAL(triggered(bool)), this, SLOT(UserExport())); 
  }

  // more actions: faudes select
  mSelectAllStatesAction=new QAction("Select All States",this);
  mSelectAllStatesAction->setStatusTip("select all states in faudes generator");
  mSelectAllStatesAction->setEnabled(true);
  mEditActions.append(mSelectAllStatesAction); 
  mSelectAllTransitionsAction=new QAction("Select All Transitions",this);
  mSelectAllTransitionsAction->setStatusTip("select all transitions in faudes generator");
  mSelectAllTransitionsAction->setEnabled(true);
  mEditActions.append(mSelectAllTransitionsAction); 
  // have a user layout
  mUserLayout= new VioGeneratorLayout(this);
  FD_DQG("VioGeneratorView::DoVioAllocate(): create layout: done");
}

// update widget from (new) model
void VioGeneratorView::DoVioUpdate(void) {
  // have typed ref
  pGeneratorModel = qobject_cast<VioGeneratorModel*>(pModel);
  // bail out in void
  if(!pGeneratorModel) return; 
  // a new model has been set, update references
  // note: the model has been type faudes checked and we consider all 
  // models of one faudes type to have the same rep models (qt type, order, etc)
  FD_DQG("VioGeneratorView::DoVioUpdate(): update views for type " << VioStyle::StrFromQStr(FaudesType()));
  for(int i=0; i< mViewList.size(); i++) {
    if(!(i<pGeneratorModel->ModelList().size())) break; // cannot happen
    mViewList.at(i)->Model(pGeneratorModel->ModelList().at(i)); 
  };
  if(mPropView) mPropView->Model(pGeneratorModel);
  // get and set default user layout
  *mUserLayout =pGeneratorModel->Layout();
  UpdateUserLayout(); 
  // update my text version
  DoPlainText();
}

// update view from (new) model
void VioGeneratorView::DoPlainText(void) {
  FD_DQG("VioGeneratorView::DoPlainText(): update debug text version");
#ifdef FAUDES_DEBUG_VIO_WIDGETS
  if(pGeneratorModel) 
    mTextEdit->setPlainText(VioStyle::QStrFromStr(pGeneratorModel->Generator()->ToText()));
  else 
    mTextEdit->setPlainText("No Generator Model");
#endif
}



// get model
VioGeneratorModel* VioGeneratorView::Model(void) {
  return pGeneratorModel;
}

// set by vio model
int VioGeneratorView::Model(VioModel* model) {
  FD_DQG("VioGeneratorView::Model(" << model << ")");
  // bail out on identity
  if(model==pModel) return 0;
  // my disconnect
  if(pGeneratorModel && mSelectAllStatesAction)
    disconnect(mSelectAllStatesAction, 0, pGeneratorModel, 0);
  if(pGeneratorModel && mSelectAllTransitionsAction)
    disconnect(mSelectAllTransitionsAction, 0, pGeneratorModel, 0);
  if(pGeneratorModel)
    disconnect(pGeneratorModel, 0, this, 0);
  // call base (incl virtual DoVioUpdate)
  if(VioView::Model(model)) return 1;
  // my connect 
  if(pGeneratorModel && mSelectAllStatesAction)
    connect(mSelectAllStatesAction, SIGNAL(triggered(bool)), pGeneratorModel, SLOT(SelectAllStates()));    
  if(pGeneratorModel && mSelectAllTransitionsAction)
    connect(mSelectAllTransitionsAction, SIGNAL(triggered(bool)), pGeneratorModel, SLOT(SelectAllTransitions()));  
#ifdef FAUDES_DEBUG_VIO_WIDGETS
  if(pGeneratorModel) 
    connect(pGeneratorModel,SIGNAL(NotifyChange(void)),this,SLOT(DoPlainText(void)));
#endif
  // done
  return 0;
}

// update model from widget
void VioGeneratorView::DoModelUpdate(void) {
  FD_DQG("VioGeneratorView::DoModelUpdate(): type " << VioStyle::StrFromQStr(FaudesType()));
  // bail out if widget is not ready
  if(!mSplitter) return;
  // have my views report pending changes
  for(int i=0; i< mViewList.size(); i++) 
    mViewList.at(i)->DoModelUpdate();
  // record layout
  SaveUserLayout();
}

// walk through views
void::VioGeneratorView::ToggleViews(void) {
  FD_DQG("VioGeneratorView::ToggleViews()");
  // fix default indicator
  if(mUserLayout->mToggleStage<0) 
    mUserLayout->mToggleStage=0;
  // progress
  mUserLayout->mToggleStage+=1;
  // save old sizes
  QList<int> sizes=mSplitter->sizes();
  // toggle by base layout
  switch(mLayoutFlags & VioGeneratorStyle::LayoutMask) {
  // std generator layout: 3 stages
  case VioGeneratorStyle::Generator: {
    if(mUserLayout->mToggleStage>=3) mUserLayout->mToggleStage=0;
    int lwidth=sizes.at(0);
    int gwidth=sizes.at(1);
    int swidth=lwidth+gwidth;
    // show both
    if(mUserLayout->mToggleStage==0) { 
      lwidth=mUserLayout->mListWidth;
      gwidth=swidth-lwidth; 
      if(lwidth<100) lwidth=100;
      gwidth=swidth-lwidth; 
      if(gwidth<100) gwidth=swidth/2;
      lwidth=swidth-gwidth; 
    }; 
    // list only
    if(mUserLayout->mToggleStage==1) { lwidth = swidth; gwidth=0; }; 
    // graph only
    if(mUserLayout->mToggleStage==2) { gwidth = swidth; lwidth=0; }; 
    FD_DQG("VioGeneratorView::ShowViews(): lwidth " << lwidth << " gwidth " << gwidth);
    sizes[0]=lwidth;
    sizes[1]=gwidth;
  }
  // all other layouts TODO
  default: {
  }
  } // end switch 
  // adjust new sizes
  mSplitter->setSizes(sizes);
  mSplitter->refresh();
  // record layout
  SaveUserLayout();
}

// show/hide  abstract view
void::VioGeneratorView::ShowPropertyView(bool on) {
  if(!mPropView) return;
  FD_DQG("VioGeneratorView::ShowPropertyView(): " << on);
  QList<int> sizes = mSplitter->sizes();
  int lwidth=sizes[mSplitter->indexOf(pListView)];
  int gwidth=sizes[mSplitter->indexOf(pGraphView)];
  int pwidth=sizes[mSplitter->indexOf(mPropView)];
  if(!on && pwidth>10) {
    mUserLayout->mPropWidth=pwidth;
    sizes[mSplitter->indexOf(pGraphView)] += pwidth + mSplitter->handleWidth();
    sizes[mSplitter->indexOf(mPropView)] =0;
  } 
  if(on && pwidth < 10 ) {
    sizes[mSplitter->indexOf(mPropView)] = mUserLayout->mPropWidth;
    if(gwidth >= mUserLayout->mPropWidth + mSplitter->handleWidth())
      sizes[mSplitter->indexOf(pGraphView)] -= mUserLayout->mPropWidth + mSplitter->handleWidth();
  }
  mPropView->setVisible(on);
  mPropAction->setChecked(on);
  mSplitter->setSizes(sizes);
  // record layout
  SaveUserLayout();
}
 
// ZoomIn()
void VioGeneratorView::ZoomIn(qreal sf) {
  if(!pGraphView) return;
  FD_DQ("VioGeneratorView::ZoomIn()");
  mUserLayout->mGraphScale=pGraphView->Scale();
  mUserLayout->mGraphScale *=sf;
  if(mUserLayout->mGraphScale>10) mUserLayout->mGraphScale=10;
  if(mUserLayout->mGraphScale<0.1) mUserLayout->mGraphScale=0.1;
  pGraphView->Scale(mUserLayout->mGraphScale);
  // record layout
  SaveUserLayout();
}

// ZoomOut()
void VioGeneratorView::ZoomOut(qreal sf) {
  if(!pGraphView) return;
  FD_DQ("VioGenerator::ZoomOut()");
  mUserLayout->mGraphScale=pGraphView->Scale();
  mUserLayout->mGraphScale *= sf;
  if(mUserLayout->mGraphScale>10) mUserLayout->mGraphScale=10;
  if(mUserLayout->mGraphScale<0.1) mUserLayout->mGraphScale=0.1;
  pGraphView->Scale(mUserLayout->mGraphScale);
  // record layout
  SaveUserLayout();
}

// ZoomFit()
void VioGeneratorView::ZoomFit(void) {
  if(!pGraphView) return;
  FD_DQ("VioGeneratorView::ZoomFit()");
  pGraphView->Fit();
  // record layout
  SaveUserLayout();
}



// Grid()
void VioGeneratorView::GridVisible(bool on) {
  if(!pGraphView) return;
  FD_DQ("VioGeneratorView::GridVisible()");
  mUserLayout->mGraphGridVisible=on;
  pGraphView->GridVisible(on);
  // record layout
  SaveUserLayout();
}

// Export EPS() (ret 0<>OK)
int VioGeneratorView::ExportEps(const QString& filename) {
  if(!pGraphView) return 1;
  if(!pGraphView->Model()) return 1;
  FD_DQ("VioGeneratorView::ExportEps()");
  return pGraphView->Model()->WriteEps(filename);
}

// Export PDF() (ret 0<>OK)
int VioGeneratorView::ExportPdf(const QString& filename) {
  if(!pGraphView) return 1;
  if(!pGraphView->Model()) return 1;
  FD_DQ("VioGeneratorView::ExportPdf()");
  return pGraphView->Model()->WritePdf(filename);
}

// Export SVG() (ret 0<>OK)
int VioGeneratorView::ExportSvg(const QString& filename) {
  if(!pGraphView) return 1;
  if(!pGraphView->Model()) return 1;
  FD_DQ("VioGeneratorView::ExportSvg()");
  return pGraphView->Model()->WriteSvg(filename);
}

// Export PNG() (ret 0<>OK)
int VioGeneratorView::ExportPng(const QString& filename) {
  if(!pGraphView) return 1;
  if(!pGraphView->Model()) return 1;
  FD_DQ("VioGeneratorView::ExportPng()");
  return pGraphView->Model()->WritePng(filename);
}

// Export JPG() (ret 0<>OK)
int VioGeneratorView::ExportJpg(const QString& filename) {
  if(!pGraphView) return 1;
  if(!pGraphView->Model()) return 1;
  FD_DQ("VioGeneratorView::ExportJpg()");
  return pGraphView->Model()->WriteJpg(filename);
}

// User Export
void VioGeneratorView::UserExport(void) {
  if(!Model()) return;
  FD_DQ("VioGeneratorView::UserExportSvg()");
  // set up dialog
  QFileDialog* fdiag = new QFileDialog();
  QSettings settings("Faudes", "VioDES");
  fdiag->restoreState(settings.value("exportGraphicsDialog").toByteArray());
  fdiag->setFilters(QStringList()  
    << "SVG Files (*.svg)" << "PDF Files (*.pdf)" << "EPS Files (*.eps)" 
    << "PNG Files (*.png)" << "JPG Files (*.jpg)" << "Any File (*.*)");
  fdiag->setFileMode(QFileDialog::AnyFile);
  fdiag->setWindowTitle(QString("Export %1 graph to image file").arg(Model()->FaudesName()));
  fdiag->setAcceptMode(QFileDialog::AcceptSave);
  fdiag->setDefaultSuffix("");
  // run dialog
  QString filename="";
  if(fdiag->exec()) {
    if(fdiag->selectedFiles().size()==1) 
      filename=fdiag->selectedFiles().at(0);
  }
  settings.setValue("exportGraphicsDialog", fdiag->saveState());
  if(filename=="") return;
  // guess default suffix
  if(QFileInfo(filename).suffix()=="") {
    if(fdiag->selectedNameFilter().contains("SVG"))
      filename = filename + ".svg";
    else if(fdiag->selectedNameFilter().contains("PDF"))
      filename = filename + ".pdf";
    else if(fdiag->selectedNameFilter().contains("EPS"))
      filename = filename + ".eps";
    else if(fdiag->selectedNameFilter().contains("PNG"))
      filename = filename + ".png";
    else if(fdiag->selectedNameFilter().contains("JPG"))
      filename = filename + ".jpg";
  }
  // figure suffix
  QFileInfo finfo(filename);
  if(finfo.suffix()=="svg")
    ExportSvg(filename);
  else if(finfo.suffix()=="pdf")
    ExportPdf(filename);
  else if(finfo.suffix()=="eps")
    ExportEps(filename);
  else if(finfo.suffix()=="png")
    ExportPng(filename);
  else if(finfo.suffix()=="jpg")
    ExportJpg(filename);
  else if(finfo.suffix()=="jpeg")
    ExportJpg(filename);
  else
    FD_WARN("VioGeneratorView::UserExport(): unknown format " << filename);
  //done
  delete fdiag;
}



// show/hide views from layout
void VioGeneratorView::UpdateUserLayout(void) {
  FD_DQG("VioGeneratorView::UpdateUserLayout(): stage " << mUserLayout->mToggleStage << " gwidth " << mUserLayout->mGraphWidth );
  FD_DQG("VioGeneratorView::UpdateUserLayout(): gwidth " << mUserLayout->mGraphWidth);
  FD_DQG("VioGeneratorView::UpdateUserLayout(): lwidth " << mUserLayout->mListWidth);  
  // bail out on invalid
  if(!mSplitter) return;
  // show/hide 
  if(mPropView) {
    mPropView->setVisible(mUserLayout->mPropBuiltIn);
    mPropAction->setChecked(mUserLayout->mPropBuiltIn);
  }
  // adjust splitter from non-default values
  if(mUserLayout->mToggleStage >=0 ) {
    bool ok= mSplitter->restoreState(mUserLayout->mSplitterState); 
    // we don't know why, but splitter restore is not functional with qt 4.6 
    // on macx ... perhaps we did something funny with our widget's size hint?
    if(!ok || true) {
      QList<int> sizes=mSplitter->sizes();
      switch(mLayoutFlags & VioGeneratorStyle::LayoutMask) {
      // std generator layout: 
      case VioGeneratorStyle::Generator: {

        FD_DQG("VioGeneratorView::UpdateUserLayout(): result from restore splitter with size " << mUserLayout->mSplitterState.size()  << ": " << ok);
        while(sizes.size()<3) sizes.append(0);
        int sum=0;
        for(int i=0; i<sizes.size(); i++) sum+=sizes[i]; 
        FD_DQG("VioGeneratorView::UpdateUserLayout(): fixing splitter, total width " << sum);
        sizes[0] = mUserLayout->mListWidth;
        sizes[1] = mUserLayout->mGraphWidth;
        sizes[2] = mUserLayout->mPropWidth;
        if(mUserLayout->mToggleStage==1) sizes[1]=0;
        if(mUserLayout->mToggleStage==2) sizes[0]=0;
        if(!mUserLayout->mPropBuiltIn ) sizes[2]=0;
      }
      // other layouts
      default: break;
      }      
      mSplitter->setSizes(sizes);
    }
  }
  // initialize splitter to sensible start
  if(mUserLayout->mToggleStage < 0 ) {
    mUserLayout->mToggleStage=0;
    QList<int> sizes=mSplitter->sizes();
    switch(mLayoutFlags & VioGeneratorStyle::LayoutMask) {
      // std generator layout: 
      case VioGeneratorStyle::Generator: {
        FD_DQG("VioGeneratorView::UpdateUserLayout(): init splitter ");
        sizes[0]=400;
        sizes[1]=400;
        for(int i = 2; i<sizes.size(); i++) 
          sizes[i]=0;
      }
      // all other layouts TODO
      default: {
      }
    } // end switch 
    // adjust new sizes
    mSplitter->setSizes(sizes);
  }
  // scale graphics
  if(pGraphView) {
    if(mUserLayout->mGraphScale>10) mUserLayout->mGraphScale=10;
    if(mUserLayout->mGraphScale<0.1) mUserLayout->mGraphScale=0.1;
    pGraphView->Scale(mUserLayout->mGraphScale);
    mUserLayout->mGraphScale=pGraphView->Scale();
    pGraphView->GridVisible(mUserLayout->mGraphGridVisible);
  }
  // done
  FD_DQG("VioGeneratorView::UpdateUserLayout(): done with stage " << mUserLayout->mToggleStage << " scale " << mUserLayout->mGraphScale );
}

// save layout to model 
void VioGeneratorView::SaveUserLayout(void) {
  if(!pGeneratorModel) return;
  FD_DQG("VioGeneratorView::SaveUserLayout()");
  // retrieve parameters
  mUserLayout->mSplitterState=mSplitter->saveState();
  mUserLayout->mPropBuiltIn=false;
  QList<int> sizes = mSplitter->sizes();
  if(mPropView) {
    mUserLayout->mPropBuiltIn=mPropView->isVisible(); 
    int pwidth = sizes[mSplitter->indexOf(mPropView)];
    if(pwidth >10) mUserLayout->mPropWidth = pwidth;
  }
  if(pGraphView) {
    mUserLayout->mGraphScale=pGraphView->Scale();
  }  
  if(pListView && pListView) {
    int lwidth = sizes[mSplitter->indexOf(pListView)];
    int gwidth = sizes[mSplitter->indexOf(pGraphView)];
    if(lwidth > 10 && gwidth > 10) {
      mUserLayout->mListWidth = lwidth;
      mUserLayout->mGraphWidth = gwidth;
    }
  }  

  FD_DQG("VioGeneratorView::SaveUserLayout(): splitter #" << mUserLayout->mSplitterState.size());
  FD_DQG("VioGeneratorView::SaveUserLayout(): gscale " << mUserLayout->mGraphScale);
  FD_DQG("VioGeneratorView::SaveUserLayout(): gwidth " << mUserLayout->mGraphWidth);
  FD_DQG("VioGeneratorView::SaveUserLayout(): lwidth " << mUserLayout->mListWidth);
  FD_DQG("VioGeneratorView::SaveUserLayout(): pwidth " << mUserLayout->mPropWidth);
  // save to model
  pGeneratorModel->Layout(*mUserLayout);
}


// we assume that the QSplitter signals only user changes (?)
void VioGeneratorView::TrackSplitter(void) {
  if(!pGeneratorModel) return;
  if(!isVisible()) return;
  FD_DQG("VioGeneratorView::TrackSplitter()");
  SaveUserLayout();
}


// directed data access
int VioGeneratorView::InsertData(const VioData* pData) {
  if(!pGeneratorModel) return 1; 
  return pGeneratorModel->InsertData(pData);
}

// directed data access
VioData* VioGeneratorView::SelectionData(void) {
  FD_DQG("VioGeneratorView::SelectionData()");
  if(!pGeneratorModel) return 0; 
  return pGeneratorModel->SelectionData();
}

// directed data access
void VioGeneratorView::DeleteSelection(void) {
  if(pGeneratorModel) pGeneratorModel->DeleteSelection();
}


// forward highlite requests (-> model -> all views ??? should we better just process?) 
void VioGeneratorView::Highlite(const VioElement& elem, bool on) {
  FD_DQG("VioGeneratorView::Highlite(): emit notification");
  emit NotifyHighlite(elem,on);
}

// forward highlite requests
void VioGeneratorView::HighliteClear(void) {
  FD_DQG("VioGeneratorView::HighliteClear(): emit notification");
  emit NotifyHighliteClear();
}

// forward show requests
void VioGeneratorView::Show(const VioElement& elem) {
  FD_DQG("VioGeneratorView::Show(): emit notification");
  emit NotifyShow(elem);
}


/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioGeneratorAbstractView

****************************************************************
****************************************************************
****************************************************************
*/

// construct/destruct
VioGeneratorAbstractView::VioGeneratorAbstractView(VioGeneratorView* parent) :
  QWidget(parent),
  pGeneratorAbstractModel(0),
  pGeneratorModel(0),
  pGeneratorView(parent)
{
  FD_DQG("VioGeneratorAbstractView::VioGeneratorAbstractView("<< parent <<")");
  // set layout
  mVbox = new QVBoxLayout(this);
  mVbox->setMargin(0);
  mVbox->setSpacing(0);
  // my sizepolicy
  setMinimumSize(QSize(100,100));
  setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
  // debugging widget
#ifdef FAUDES_DEBUG_VIO_WIDGETS
  mTextInfo = new QLabel();
  mTextInfo->setWordWrap(true);
  mTextInfo->setText(tr("Generator representation type \"%1\"").arg(typeid(this).name()));
  mVbox->addWidget(mTextInfo);
  mTextEdit = new QPlainTextEdit();
  mTextEdit->setReadOnly(true);
  mVbox->addWidget(mTextEdit);
#endif
}

// set model (incl typecheck)
int VioGeneratorAbstractView::Model(VioGeneratorAbstractModel* model) {
  FD_DQG("VioGeneratorAbstractView::Model("<< model <<")");
  if(model==pGeneratorAbstractModel) return 0;
  // disconnect
  if(pGeneratorAbstractModel) {
    disconnect(pGeneratorAbstractModel,0,this,0);
    disconnect(pGeneratorModel,0,this,0);
  }
  // have typed refs
  pGeneratorAbstractModel=model;
  pGeneratorModel=model->GeneratorModel();
  pFaudesGenerator=model->Generator();
  // connect to receive element notification
  connect(model, SIGNAL(NotifyElementIns(const VioElement&)), 
    this, SLOT(UpdateElementIns(const VioElement&)));
  connect(model, SIGNAL(NotifyElementDel(const VioElement&)), 
    this, SLOT(UpdateElementDel(const VioElement&)));
  connect(model, SIGNAL(NotifyElementEdit(const VioElement&, const VioElement&)), 
    this, SLOT(UpdateElementEdit(const VioElement&, const VioElement&)));
  connect(model, SIGNAL(NotifyElementProp(const VioElement&)), 
    this, SLOT(UpdateElementProp(const VioElement&)));
  // connect to receive global notification
  connect(model, SIGNAL(NotifyTrimElements(void)), 
    this, SLOT(UpdateTrimElements(void)));
  connect(model, SIGNAL(NotifyAnyAttr(void)), 
    this, SLOT(UpdateAnyAttr(void)));
  connect(model, SIGNAL(NotifyAnyChange(void)), 
    this, SLOT(UpdateAnyChange(void)));
  // update from model
  DoVioUpdate();
  // done
  FD_DQG("VioGeneratorAbstractView::Model("<< model <<"): done");
  return 0;
}

// get model and friends: representation model
const VioGeneratorAbstractModel* VioGeneratorAbstractView::Model(void) const {
  return pGeneratorAbstractModel;
}

// get model and friends: faudes generator
const faudes::vGenerator* VioGeneratorAbstractView::Generator(void) const {
  return pFaudesGenerator;
}

// get model and friends: generator model
const VioGeneratorModel* VioGeneratorAbstractView::GeneratorModel(void) const {
  return pGeneratorModel;
}

// get model and friends: generator model
const VioGeneratorView* VioGeneratorAbstractView::GeneratorView(void) const {
  return pGeneratorView;
}


// update notification: default to update all
void VioGeneratorAbstractView::UpdateElementIns(const VioElement& elem) 
  { (void) elem; UpdateAnyChange();};
void VioGeneratorAbstractView::UpdateElementDel(const VioElement& elem) 
  { (void) elem; UpdateAnyChange();};
void VioGeneratorAbstractView::UpdateElementEdit(const VioElement& selem, const VioElement& delem) 
  { (void) selem; (void) delem; UpdateAnyChange();};
void VioGeneratorAbstractView::UpdateElementProp(const VioElement& elem) 
  { (void) elem; UpdateAnyChange();};
void VioGeneratorAbstractView::UpdateTrimElements(void) 
  { UpdateAnyChange();};
void VioGeneratorAbstractView::UpdateAnyAttr(void) 
  { UpdateAnyChange();};
void VioGeneratorAbstractView::UpdateAnyChange(void) 
  { UpdateNewModel();};


// update notification: reimplement in derivate
void VioGeneratorAbstractView::UpdateNewModel(void) { 
  FD_DQG("VioGeneratorAbstractView::UpdateNewModel()");
  DoVioUpdate();
}

// update from model: reimplement in derivate
void VioGeneratorAbstractView::DoVioUpdate(void) {
  FD_DQG("VioGeneratorAbstractView::DoVioUpdate()");
  // bail out if not ready
  if(!mTextEdit) return;
  // set text by faudes tokens
  mTextEdit->setPlainText(VioStyle::QStrFromStr(Generator()->ToText()));
}

// highlite/show request (defaults to nothing)
void VioGeneratorAbstractView::Highlite(const VioElement& elem, bool on) {
  (void) elem; (void) on;}
void VioGeneratorAbstractView::HighliteClear(void) {}
void VioGeneratorAbstractView::Show(const VioElement& elem) {
  (void) elem;}



/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioGeneratorPropertyView

****************************************************************
****************************************************************
****************************************************************
*/

// construct 
VioGeneratorPropertyView::VioGeneratorPropertyView(QWidget* parent, VioStyle* config, bool alloc) : 
  VioView(parent,config,false), 
  pGeneratorModel(0),
  mPropBox(0) 
{
  FD_DQG("VioGeneratorPropertyView::VioGeneratorPropertyView(): " << VioStyle::StrFromQStr(FaudesType()));
  if(alloc) DoVioAllocate();
  FD_DQG("VioGeneratorPropertyView::VioGeneratorPropertyView(): done");
}

// destruct
VioGeneratorPropertyView::~VioGeneratorPropertyView(void) {
}

// allocate visual items
void VioGeneratorPropertyView::DoVioAllocate(void) {
  FD_DQG("VioGeneratorPropertyView::DoVioAllocate(): layout for type " << VioStyle::StrFromQStr(FaudesType()));
  // allocate base
  VioView::DoVioAllocate();
  // debugging widget (turn of
#ifdef FAUDES_DEBUG_VIO_WIDGETS
  mTextInfo->setText(tr("Properties"));
  mTextEdit->hide();
  mApplyButton->hide();
#endif
  // my box
  mPropBox = new QVBoxLayout();
  mPropBox->setMargin(0);
  mPropBox->setSpacing(0);
  mPropTrans = new PioTProp(0,pConfig);
  mPropBox->addWidget(mPropTrans);
  mPropState = new PioSProp(0,pConfig);
  mPropBox->addWidget(mPropState);
  mPropEvent = new PioEProp(0,pConfig);
  mPropBox->addWidget(mPropEvent);
  mPropGlobal = new PioGProp(0,pConfig);
  mPropBox->addWidget(mPropGlobal);
  //mPropBox->setMinimumSize(QSize(100,100));
  //mPropBox->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Expanding);
  mVbox->addLayout(mPropBox);
  mVbox->addStretch(1);
}


// update widget from (new) model
void VioGeneratorPropertyView::DoVioUpdate(void) {
  // disconnect 
  if(pGeneratorModel) {
    disconnect(pGeneratorModel, 0 , this, SLOT(UpdateSelectionChange(void)));
  }
  // bail out on void
  if(!pModel) return; 
  // call base
  VioView::DoVioUpdate();
  // my settings
  FD_DQG("VioGeneratorPropertyView::DoVioUpdate(): sensed model " << VioStyle::StrFromQStr(FaudesType()));
  pGeneratorModel = qobject_cast<VioGeneratorModel*>(pModel);
  mPropTrans->GeneratorModel(pGeneratorModel);
  mPropState->GeneratorModel(pGeneratorModel);
  mPropEvent->GeneratorModel(pGeneratorModel);
  mPropGlobal->GeneratorModel(pGeneratorModel);
  // follow selection
  connect(pModel, SIGNAL(NotifySelectionChange(void)), 
    this, SLOT(UpdateSelectionChange(void))); 
  disconnect(pModel, 0, this, SLOT(Show(const VioElement&)));
  // now update
  FD_DQG("VioGeneratorPropertyView::DoVioUpdate(): update view for type " << VioStyle::StrFromQStr(FaudesType()));
  Show(VioElement());
}


// update model from widget
void VioGeneratorPropertyView::DoModelUpdate(void) {
  FD_DQG("VioGeneratorPropertyView::DoModelUpdate(): type " << VioStyle::StrFromQStr(FaudesType()));
  // bail out if widget is not ready
  if(!mPropBox) return;
  // todo
}


// reimplement  show element
void VioGeneratorPropertyView::Show(const VioElement& elem) {
  FD_DQG("VioGeneratorPropertyView::Show(): " << elem.Str());
  // bail out on not ready
  if(!mPropBox) return;
  // tell attribute widgets
  mPropTrans->Show(elem);
  mPropState->Show(elem);
  mPropEvent->Show(elem);
  mPropGlobal->Show(elem);
  // bring to front
  switch(elem.Type()) {
    case VioElement::ETrans:  mPropTrans->show(); mPropState->hide(); mPropEvent->hide(); mPropGlobal->hide(); break;
    case VioElement::EState:  mPropTrans->hide(); mPropState->show(); mPropEvent->hide(); mPropGlobal->hide(); break;
    case VioElement::EEvent:  mPropTrans->hide(); mPropState->hide(); mPropEvent->show(); mPropGlobal->hide(); break;
    case VioElement::EVoid: mPropTrans->hide(); mPropState->hide(); mPropEvent->hide(); mPropGlobal->show(); break;
  default: break;
  }
  FD_DQG("VioGeneratorPropertyView::Show(): done " << elem.Str());
}

// reimplement to follow selection
void VioGeneratorPropertyView::UpdateSelectionChange(void) {
  // bail out on not ready
  if(!pGeneratorModel) return;
  if(!mPropBox) return;
  // report
  FD_DQG("VioGeneratorPropertyView::UpdateSelectionChange(): #" << pGeneratorModel->Selection().size());
  // figure type
  VioElement elem;
  if(pGeneratorModel->Selection().size()>=1) 
    elem=pGeneratorModel->Selection().at(0);
  // bring to front
  switch(elem.Type()) {
    case VioElement::ETrans:  mPropTrans->show(); mPropState->hide(); mPropEvent->hide(); mPropGlobal->hide(); break;
    case VioElement::EState:  mPropTrans->hide(); mPropState->show(); mPropEvent->hide(); mPropGlobal->hide(); break;
    case VioElement::EEvent:  mPropTrans->hide(); mPropState->hide(); mPropEvent->show(); mPropGlobal->hide(); break;
    case VioElement::EVoid: mPropTrans->hide(); mPropState->hide(); mPropEvent->hide(); mPropGlobal->show(); break;
  default: break;
  }
}


/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioGeneratorWidget

****************************************************************
****************************************************************
****************************************************************
*/

// construct (we dont call the base class, sinc we have a different layout)
VioGeneratorWidget::VioGeneratorWidget(QWidget* parent, VioStyle* config, bool alloc) : 
  VioWidget(parent,config,false) // dont allocate
{
  FD_DQG("VioGeneratorWidget::VioGeneratorWidget(): " << VioStyle::StrFromQStr(pConfig->ConfigName()));
  // allocate model and view
  if(alloc) {
    // have view
    mView= new VioGeneratorView(0,pConfig);
    mVbox->addWidget(mView);
    // fix typed refs and connect to widget
    DoVioAllocate();
    // have a model and set
    Model(new VioGeneratorModel(this,pConfig));
  }
  // fix modified flag
  mModified=false;
  // done
  FD_DQG("VioGeneratorWidget::VioGeneratorWidget(): done");
}

// destruct
VioGeneratorWidget::~VioGeneratorWidget(void) {
}

// fix view
void VioGeneratorWidget::DoVioAllocate(void) {
  // fix typed refs
  pGeneratorView=qobject_cast<VioGeneratorView*>(mView);
  // connect view to widget
  FD_DQG("VioGeneratorWidget::DoModelAllocate(): connect");
  QObject::connect(mView,SIGNAL(NotifyModified(bool)),this,SLOT(ChildModified(bool)));
  QObject::connect(mView,SIGNAL(MouseClick(const VioElement&)),this,SIGNAL(MouseClick(const VioElement&)));
  QObject::connect(mView,SIGNAL(MouseDoubleClick(const VioElement&)),this,SIGNAL(MouseDoubleClick(const VioElement&)));
  FD_DQG("VioGeneratorWidget::DoVioAllocate(): done");
}


// set by vio model
int VioGeneratorWidget::Model(VioModel* model) {
  FD_DQG("VioGeneratorWidget::Model(" << model << "): type " << VioStyle::StrFromQStr(FaudesType()));
  // bail out on identity
  if(model==mModel) return 0;
  // call base to set view and connects
  int res=VioWidget::Model(model);
  // update my typed refs
  pGeneratorModel=qobject_cast<VioGeneratorModel*>(mModel);
  FD_DQG("VioGeneratorWidget::Model(" << model << "): done");  
  return res;
}



