/* piotseview.cpp  - widgets for property editing  */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006, 2007  Thomas Moor, Klaus Schmidt, Sebastian Perk

*/

#include "piotseview.h"
#include "viogenerator.h"

/*
 *******************************************************
 *******************************************************

 PioVProp implementation

 *******************************************************
 *******************************************************
 */

// construct base
PioVProp::PioVProp(QWidget* parent, VioStyle* config) : QWidget(parent) {
  (void) config; 
  FD_DQG("PioVProp(parent " << parent <<" ,config " << config <<")");

  // not connected
  pVioGeneratorModel=0;
  pGeneratorConfig=0;
  mElement=VioElement::FromType(VioElement::EVoid);

  // general widget settings
  setAutoFillBackground(true);
  setMinimumWidth(150);

  // symbolic name line edit
  mEditName=new VioSymbolEdit(this);  

  // symbolic name label
  mLabelName = new QLabel(this);
  mLabelName->setText("Name");
  mLabelName->setBuddy(mEditName);

  // symbolic name hbox
  QHBoxLayout* hbox= new QHBoxLayout();
  hbox->addWidget(mLabelName);
  hbox->addSpacing(10);
  hbox->addWidget(mEditName);

  // overall layout
  mVbox = new QVBoxLayout(this);
  mVbox->setMargin(0);
  mVbox->setSpacing(0);
  mVbox->addLayout(hbox);
  mVbox->addSpacing(10);

  // connect name chyange
  QObject::connect(mEditName,SIGNAL(returnPressed(void)),this,SLOT(UpdateModel(void)));
  QObject::connect(mEditName,SIGNAL(returnPressed(void)),this,SIGNAL(DoneEditing(void)));

  // clear other pointers
  mAttribute=0;
  mConfigure=0;

  // initialise update block
  mBlockModelUpdate=false;

};

// destruct
PioVProp::~PioVProp(void) {
  FD_DQG("PioVProp::~PioVProp()");
}

// set viogenerator
void PioVProp::GeneratorModel(VioGeneratorModel* genmodel) {
  FD_DQG("PioVProp::GeneratorModel()");
  if(genmodel) FD_DQG("PioVProp::GeneratorModel(): ctype " << typeid(*genmodel).name() << " " << typeid(genmodel->Layout()).name()  );
  // disconnect
  if(pVioGeneratorModel)
    disconnect(pVioGeneratorModel, 0, this, 0);
  // remove my widgets
  if(mAttribute) {
    mVbox->removeWidget(mAttribute);
    delete mAttribute;
    mAttribute=0;
  }
  if(mConfigure) {
    mVbox->removeWidget(mConfigure);
    delete mConfigure;
    mConfigure=0;
  }
  // record
  pVioGeneratorModel=genmodel;
  pGeneratorConfig = dynamic_cast<VioGeneratorStyle*>(genmodel->Configuration());
  // bail out on null
  if(!pVioGeneratorModel) return;
  // connect model change
  connect(pVioGeneratorModel,SIGNAL(NotifyChange(void)),this,SLOT(UpdateView(void)));
  connect(pVioGeneratorModel,SIGNAL(NotifySelectionChange(void)),this,SLOT(ShowSelection(void)));
  // clear 
  DoClear();
  // show
  ShowSelection();
}

// get viogenerator
const VioGeneratorModel* PioVProp::GeneratorModel(void) const {
  return pVioGeneratorModel;
}

// read only access to faudes generator 
const fGenerator* PioVProp::Generator(void) const {
  if(!pVioGeneratorModel) return 0;
  return pVioGeneratorModel->Generator();
}


// set symbol mode of name specifier
void PioVProp::SymbolMode(VioSymbol::Mode mode) {
  mEditName->setSymbolMode(mode);
}  


// get faudes element that is currently displyed
const VioElement& PioVProp::Element(void) const { 
  return mElement; 
};


// get symbolic name
QString PioVProp::Name(void) {
  return mEditName->symbol();
}


// set symbolic name   
void PioVProp::Name(const QString& qname) {
  // set in line
  mEditName->setText(qname);
  mEditName->setEnabled(!qname.startsWith("< sel"));
}

// set attribute (silent)
void PioVProp::Attribute(faudes::AttributeVoid* attr) {
  // bail out 
  if(!pVioGeneratorModel) return;
  if(!mAttribute) return;
  if(!attr) return;
  // silent set
  FD_DQG("PioVProp::Attribute(): set");
  bool old=mBlockModelUpdate; 
  mBlockModelUpdate=true;
  mAttribute->FaudesObject(attr);
  mBlockModelUpdate=old;
}

// set attribute from selection
void PioVProp::AttributeFromSelection(void) {
  // bail out 
  if(!pVioGeneratorModel) return;
  if(!mAttribute) return;
  FD_DQG("PioVProp::AttributeFromSelection(): #" << pVioGeneratorModel->Selection().size());
  // silent set
  bool old=mBlockModelUpdate; 
  mBlockModelUpdate=true;
  mAttribute->Model()->MergeClear();
  foreach(VioElement elem,pVioGeneratorModel->Selection()) {
    if(!pVioGeneratorModel->ElementExists(elem)) continue;
    faudes::AttributeFlags*  attr = pVioGeneratorModel->ElementAttr(elem);
    if(!attr) continue;
    mAttribute->Model()->MergeInsert(attr); 
    delete attr;
  }
  mAttribute->Model()->MergeDone();
  mBlockModelUpdate=old;
}

// set attribute to selection
void PioVProp::AttributeToSelection(void) {
  // bail out 
  if(!pVioGeneratorModel) return;
  if(!mAttribute) return;
  FD_DQG("PioVProp::AttributeToSelection(): #" << pVioGeneratorModel->Selection().size());
  // block update
  bool old=mBlockModelUpdate; 
  mBlockModelUpdate=true;
  // loop selection
  foreach(VioElement elem,pVioGeneratorModel->Selection()) {
    if(!pVioGeneratorModel->ElementExists(elem)) continue;
    faudes::AttributeFlags*  attr = pVioGeneratorModel->ElementAttr(elem);
    if(!attr) continue;
    mAttribute->Model()->MergeAssign(attr); 
    pVioGeneratorModel->ElementAttr(elem,*attr);
    delete attr;
  }
  FD_DQG("PioVProp::AttributeToSelection(): done");
  mBlockModelUpdate=old;
}


// set model from visual representation
void PioVProp::UpdateModel(void) {
  // bail out on block
  if(mBlockModelUpdate) return;
  // call virtual
  FD_DQG("PioVProp::UpdateModel()");
  DoModelUpdate();
}

// set view from model
void PioVProp::UpdateView(void) {
  if(mBlockModelUpdate) return;
  FD_DQG("PioVProp::UpdateView()");
  DoVioUpdate();
}

// set values from faudes element if connected to viogenerator, 
void PioVProp::Show(const VioElement& elem) {
  // bail out in no model
  if(!pVioGeneratorModel) return; 
  // set it
  FD_DQG("PioVProp::Show(): " << pVioGeneratorModel->ElementStr(elem));
  mElement=elem;
  UpdateView();
}

// set values from selection 
void PioVProp::ShowSelection(void) {
  // bail out in no model
  if(!pVioGeneratorModel) return; 
  // default implementation
  FD_DQG("PioVProp::ShowSelection(): #" << pVioGeneratorModel->Selection().size());
  mElement=VioElement::FromType();
  if(pVioGeneratorModel->Selection().size() ==1) 
    mElement=pVioGeneratorModel->Selection().at(0);
  FD_DQG("PioVProp::ShowSelection(): elem " << mElement.Str());
  UpdateView();  
}


/*
 *******************************************************
 *******************************************************

 PioTProp implementation

 *******************************************************
 *******************************************************
 */

// construct state
PioTProp::PioTProp(QWidget* parent, VioStyle* config) : PioVProp(parent, config) {
  (void) config;

  FD_DQG("PioTProp(parent)");

  // adjust title
  mLabelName->setText("Label");
 
  // clear view
  DoClear();

};

// destruct
PioTProp::~PioTProp(void) {
  FD_DQG("PioTProp::~PioTProp()");
}

// clear view
void PioTProp::DoClear(void) {

  // fix attribute
  if(!mAttribute) {
    if(pGeneratorConfig)
      mAttribute= qobject_cast<VioAttributeWidget*>(pGeneratorConfig->mTransAttribute->NewWidget());
    if(!mAttribute)
      mAttribute= qobject_cast<VioAttributeWidget*>(VioTypeRegistry::NewWidget("AttributeFlags"));
    if(!mAttribute)
      FD_WARN("PioTProp::DoClear(): type registry broken!");
    if(!mAttribute)
      mAttribute= new VioAttributeWidget();
    mAttribute->Model()->Context(pVioGeneratorModel);
    mVbox->addWidget(mAttribute);
    QObject::connect(mAttribute,SIGNAL(NotifyAnyChange(void)),this,SLOT(UpdateModel(void)));
  }

  // do clear
  Name("< none >");
  // silent clr
  bool old=mBlockModelUpdate; 
  mBlockModelUpdate=true;
  mAttribute->Clear();
  mBlockModelUpdate=old;
}


// update view for a given trans
void PioTProp::DoVioUpdate(void) {
  // bail out in no model
  if(!pVioGeneratorModel) return; 
  // set my data: if its one transition
  if(mElement.Type()==VioElement::ETrans) 
  if(pVioGeneratorModel->ElementExists(mElement)) {
    //mEditName->setCompleter(pVioGenerator->EventCompleter());
    Name(VioStyle::DispEventName(Generator(),mElement.Trans().Ev));
    faudes::AttributeFlags* attr=pVioGeneratorModel->ElementAttr(mElement);
    Attribute(attr);
  }
  if(!pVioGeneratorModel->ElementExists(mElement)) {
    DoClear();
  }  
  // set my data: if its the selection
  if(mElement.Type()==VioElement::EVoid) {
    Name("< selection >");    
    AttributeFromSelection();
  }
  FD_DQG("PioTProp::DoVioUpdate(): done");
}

// update model
void PioTProp::DoModelUpdate(void) {
  // if disconnected, emit uniform signal and return 
  if(!pVioGeneratorModel) {
    emit NotifyModified(true);
    return;
  }
  // else update model
  bool changed=false;
  FD_DQG("PioTProp::DoModelUpdate: " << pVioGeneratorModel->ElementStr(mElement));
  if(mElement.Type()==VioElement::ETrans) {
    // identfy possible move (aka change of event name)
    faudes::Transition oftrans=mElement.Trans();
    faudes::Transition nftrans=oftrans;
    std::string ofname=Generator()->EventName(oftrans.Ev);
    std::string nfname=VioStyle::StrFromQStr(Name());
    FD_DQG("PioTprop::DoModelUpdate: (new) event " << nfname);
    // insist in valid symbol
    if(!faudes::SymbolTable::ValidSymbol(nfname)) nfname=ofname;
    // set up possible new event
    VioElement nevent = VioElement::FromEvent(
      VioStyle::IdxFromSymbol(nfname,Generator()->EventSymbolTablep()));
    // ensure that new event exits in generator
    if(!pVioGeneratorModel->ElementExists(nevent)) {
      nevent=pVioGeneratorModel->ElementIns(VioElement::FromEvent(0));
      nevent=pVioGeneratorModel->ElementName(nevent,VioStyle::QStrFromStr(nfname));
    } 
    // setup possibly new transition
    nftrans.Ev=Generator()->EventIndex(nfname);
    FD_DQG("PioTprop::DoModelUpdate: (new) trans " << nftrans.Str());
    // if it is indeed new, avoid doublets
    if(nftrans != oftrans) 
    if(Generator()->ExistsTransition(nftrans)) {
      nfname=Generator()->UniqueEventName(nfname);
    }
    // again: set up possible new event
    nevent = VioElement::FromEvent(
      VioStyle::IdxFromSymbol(nfname,Generator()->EventSymbolTablep()));
    // again: ensure that new event exits in generator
    if(!pVioGeneratorModel->ElementExists(nevent)) {
      nevent=pVioGeneratorModel->ElementIns(VioElement::FromEvent(0));
      nevent=pVioGeneratorModel->ElementName(nevent,VioStyle::QStrFromStr(nfname));
    } 
    // again: setup possibly new transition
    nftrans.Ev=Generator()->EventIndex(nfname);
    FD_DQG("PioTprop::DoModelUpdate: (new) trans " << nftrans.Str() << " vs " << oftrans.Str());
    // actually move transition
    if(nftrans != oftrans) { // hairy detail: dont use mElement as ref-arg, since selection may change ---- hit me
      pVioGeneratorModel->UndoEditStart();
      mElement=pVioGeneratorModel->ElementEdit(VioElement::FromTrans(oftrans),VioElement::FromTrans(nftrans)); 
      pVioGeneratorModel->UndoEditStop();
      if(mElement.Type()!=VioElement::EVoid) changed|=true;
    }
  }
  // change one attribute 
  if(pVioGeneratorModel->ElementExists(mElement))   
  if(!pVioGeneratorModel->ElementAttrTest(mElement,*mAttribute->Attribute())) {
    pVioGeneratorModel->UndoEditStart();
    changed|= true;
    pVioGeneratorModel->ElementAttr(
		     mElement,*mAttribute->Attribute()); 
    pVioGeneratorModel->UndoEditStop();
  }
  // apply to selection
  if(mElement.Type()==VioElement::EVoid) {
    pVioGeneratorModel->UndoEditStart();
    AttributeToSelection();
    changed=true;
    pVioGeneratorModel->UndoEditStop();
  }
  FD_DQG("PioTProp::DoModelUpdate: done with changed " << changed);
  if(changed) emit PioTProp::NotifyModified(true);
};


/*
 *******************************************************
 *******************************************************

 PioSProp implementation

 *******************************************************
 *******************************************************
 */

// construct state
PioSProp::PioSProp(QWidget* parent, VioStyle* config) : PioVProp(parent, config) {
  (void) config;

  FD_DQG("PioSProp(parent)");
  // adjust title
  mLabelName->setText("State"); 
  // clear view
  DoClear();

};

// destruct
PioSProp::~PioSProp(void) {
  FD_DQG("PioSProp::~PioSProp()");
}


// clear view
void PioSProp::DoClear(void) {
  // fix attribute
  if(!mAttribute) {
    if(pGeneratorConfig)
      mAttribute= qobject_cast<VioAttributeWidget*>(pGeneratorConfig->mStateAttribute->NewWidget());
    if(!mAttribute)
      mAttribute= qobject_cast<VioAttributeWidget*>(VioTypeRegistry::NewWidget("AttributeFlags"));
    if(!mAttribute)
      FD_WARN("PioTProp::DoClear(): type registry broken!");
    if(!mAttribute)
      mAttribute= new VioAttributeWidget();
    mAttribute->Model()->Context(pVioGeneratorModel);
    mVbox->addWidget(mAttribute);
    QObject::connect(mAttribute,SIGNAL(NotifyAnyChange(void)),this,SLOT(UpdateModel(void)));
  }
  // clear
  Name("< none >");
  // silent clr
  bool old=mBlockModelUpdate; 
  mBlockModelUpdate=true;
  mAttribute->Clear();
  mBlockModelUpdate=old;
}


// update view for a given state
void PioSProp::DoVioUpdate(void) {
  // bail out in no model
  if(!pVioGeneratorModel) return; 
  FD_DQG("PioSProp::DoVioUpdate() from "<< pVioGeneratorModel << " with idx " << mElement.State());
  // set my data: if its one state 
  if(mElement.Type()==VioElement::EState) 
  if(pVioGeneratorModel->ElementExists(mElement)) {
    //mEditName->setCompleter(pVioGenerator->StateCompleter());
    mEditName->setSymbolMode(VioSymbol::FakeSymbols);
    Name(VioStyle::DispStateName(Generator(),mElement.State()));
    //const faudes::AttributeVoid* attr=&Generator()->StateAttribute(mElement.State());
    faudes::AttributeVoid* attr=pVioGeneratorModel->ElementAttr(mElement);
    FD_DQG("PioSProp::DoVioUpdate() from "<< pVioGeneratorModel << " with idx " 
	   << mElement.State() << " found " << attr->ToString() << " ctype " << typeid(*attr).name());
    // this is ok: the attr model takes ownership of the faudes attribute
    Attribute(attr);
  }
  if(!pVioGeneratorModel->ElementExists(mElement)) {
    DoClear();
  }  
  // set my data: if its the selection
  if(mElement.Type()==VioElement::EVoid) {
    Name("< selection >");    
    AttributeFromSelection();
  }
  FD_DQG("PioSProp::DoVioUpdate(): done");
}

// update model
void PioSProp::DoModelUpdate(void) {
  // if disconnected, emit uniform signal and return 
  if(!pVioGeneratorModel) {
    emit NotifyModified(true);
    return;
  }
  // else update model
  bool changed=false;
  FD_DQG("PioSProp::DoModelUpdate: " << pVioGeneratorModel->ElementStr(mElement));
  // change of name
  if(mElement.Type()==VioElement::EState) 
  if(pVioGeneratorModel->ElementExists(mElement)) {
    // get possibly new state name
    faudes::Idx oidx=mElement.State();
    faudes::Idx nidx=0;
    std::string ofname=Generator()->StateName(oidx);
    std::string nfname="";
    if(VioStyle::ValidFakeSymbol(Name())) nfname=VioStyle::StrFromQStr(Name());
    FD_DQG("PioSProp::DoModelUpdate:(): with (new) name " << nfname);
    // avoid doublets
    nidx=Generator()->StateIndex(nfname);
    if((nidx!=oidx) && (nidx!=0)) {
      nfname=Generator()->UniqueStateName(nfname);
      if(nfname==Generator()->UniqueStateName(ofname)) nfname="";
    }
    // assigne name to state  
    if(faudes::SymbolTable::ValidSymbol(nfname) && nfname!=ofname) {
      pVioGeneratorModel->UndoEditStart();
      pVioGeneratorModel->ElementName(VioElement::FromState(oidx),VioStyle::QStrFromStr(nfname));
      changed=true;
      pVioGeneratorModel->UndoEditStop(); 
    }
    // reindex
    nidx=VioStyle::IdxFromSymbol(nfname);
    if(nidx!=0 && oidx!=nidx) 
    if(!Generator()->ExistsState(nidx)) {
      pVioGeneratorModel->UndoEditStart();
      changed |= pVioGeneratorModel->ElementEdit(
	VioElement::FromState(oidx),VioElement::FromState(nidx)).Type()!=VioElement::EVoid;
      mElement.State(nidx);
      pVioGeneratorModel->UndoEditStop(); 
    }
  }
  // change one attribute 
  if(pVioGeneratorModel->ElementExists(mElement))   
  if(!pVioGeneratorModel->ElementAttrTest(mElement,*mAttribute->Attribute())) {
    pVioGeneratorModel->UndoEditStart();
    changed|= true;
    pVioGeneratorModel->ElementAttr(
		     mElement,*mAttribute->Attribute()); 
    pVioGeneratorModel->UndoEditStop();
  }
  // apply to selection
  if(mElement.Type()==VioElement::EVoid) {
    pVioGeneratorModel->UndoEditStart();
    AttributeToSelection();
    pVioGeneratorModel->UndoEditStop();
    changed=true;
  }
  FD_DQG("PioSProp::DoModelUpdate: done with changed " << changed);
  if(changed) emit NotifyModified(true);
};


/*
 *******************************************************
 *******************************************************

 PioEProp implementation

 *******************************************************
 *******************************************************
 */

// construct state
PioEProp::PioEProp(QWidget* parent, VioStyle* config) : PioVProp(parent, config) {
  (void) config;
  FD_DQG("PioEProp(parent)");
  // adjust title
  mLabelName->setText("Event");
   // clear view
  DoClear();
};

// destruct
PioEProp::~PioEProp(void) {
  FD_DQG("PioEProp::~PioEProp()");
}


// clear view
void PioEProp::DoClear(void) {
  // fix attribute
  if(!mAttribute) {
    if(pGeneratorConfig)
      mAttribute= qobject_cast<VioAttributeWidget*>(pGeneratorConfig->mEventAttribute->NewWidget());
    if(!mAttribute)
      mAttribute= qobject_cast<VioAttributeWidget*>(VioTypeRegistry::NewWidget("AttributeFlags"));
    if(!mAttribute)
      FD_WARN("PioTProp::DoClear(): type registry broken!");
    if(!mAttribute)
      mAttribute= new VioAttributeWidget();
    mAttribute->Model()->Context(pVioGeneratorModel);
    mVbox->addWidget(mAttribute);
    QObject::connect(mAttribute,SIGNAL(NotifyAnyChange(void)),this,SLOT(UpdateModel(void)));
  }
  // clear
  Name("< none >");
  // silent clr
  bool old=mBlockModelUpdate; 
  mBlockModelUpdate=true;
  mAttribute->Clear();
  mBlockModelUpdate=old;
}


// update view for a given event
void PioEProp::DoVioUpdate(void) {
  // bail out in no model
  if(!pVioGeneratorModel) return; 
  FD_DQG("PioEProp::DoVioUpdate() from "<< pVioGeneratorModel << " with idx " << mElement.Event());
  // set my data: if its one state 
  if(mElement.Type()==VioElement::EEvent) 
  if(pVioGeneratorModel->ElementExists(mElement)) {
    //mEditName->setCompleter(pVioGenerator->EventCompleter());
    Name(VioStyle::DispEventName(Generator(),mElement.Event()));
    faudes::AttributeFlags* attr=pVioGeneratorModel->ElementAttr(mElement);
    Attribute(attr);
  }
  if(!pVioGeneratorModel->ElementExists(mElement)) {
    DoClear();
  }  
  // set my data: if its the selection
  if(mElement.Type()==VioElement::EVoid) {
    Name("< selection >");    
    AttributeFromSelection();
  }
  FD_DQG("PioEProp::DoVioUpdate(): done");
}

// update model
void PioEProp::DoModelUpdate(void) {
  // if disconnected, emit uniform signal and return 
  if(!pVioGeneratorModel) {
    emit NotifyModified(true);
    return;
  }
  // else update model
  bool changed=false;
  FD_DQG("PioEProp::DoModelUpdate: " << pVioGeneratorModel->ElementStr(mElement));
  // change of name aka move event
  if(mElement.Type()==VioElement::EEvent) 
  if(pVioGeneratorModel->ElementExists(mElement)) {
    // get possibly new event
    faudes::Idx oidx=mElement.Event();
    std::string ofname=Generator()->EventName(oidx);
    std::string nfname=VioStyle::StrFromQStr(Name());
    // insist in valid symbol
    if(!faudes::SymbolTable::ValidSymbol(nfname)) nfname=ofname;
    FD_DQG("PioEprop::PropertyChanged: (new) event " << nfname);
    // avoid doublets
    if(nfname!=ofname && Generator()->ExistsEvent(nfname)) {
      nfname=Generator()->UniqueEventName(nfname);
      if(nfname==Generator()->UniqueEventName(ofname)) nfname=ofname;
    }
    // actually rename event
    if(nfname != ofname) {
      pVioGeneratorModel->UndoEditStart();
      // ElementName() will deselect/select and hence call Do UpdateView
      // ... this is why we need to call ElementName on a copy of the Element
      mElement = pVioGeneratorModel->ElementName(VioElement(mElement),VioStyle::QStrFromStr(nfname)); 
      changed|= mElement.Type()!=VioElement::EVoid;
      pVioGeneratorModel->UndoEditStop();
    }
  }
  // change one attribute 
  if(pVioGeneratorModel->ElementExists(mElement))   
  if(!pVioGeneratorModel->ElementAttrTest(mElement,*mAttribute->Attribute())) {
    pVioGeneratorModel->UndoEditStart();
    changed=true;
    pVioGeneratorModel->ElementAttr(
		     mElement,*mAttribute->Attribute());
    pVioGeneratorModel->UndoEditStop(); 
  }
  // apply to selection
  if(mElement.Type()==VioElement::EVoid) {
    pVioGeneratorModel->UndoEditStart();
    AttributeToSelection();
    changed=true;
    pVioGeneratorModel->UndoEditStop();
   }
  FD_DQG("PioEProp::DoModelUpdate: done with changed " << changed);
  if(changed) emit NotifyModified(true);
};


/*
 *******************************************************
 *******************************************************

 PioGProp implementation

 *******************************************************
 *******************************************************
 */

// construct state
PioGProp::PioGProp(QWidget* parent, VioStyle* config) : PioVProp(parent, config) {
  (void) config;

  FD_DQG("PioGProp(parent)");

  // adjust title
  mLabelName->setText("Generator");
 
  // add initial state property
  mNumEvents = new QLabel();
  mNumStates = new QLabel();
  mNumTrans = new QLabel();

  // layout
  mVbox->addWidget(mNumEvents);
  mVbox->addWidget(mNumStates);
  mVbox->addWidget(mNumTrans);
  mVbox->addSpacing(10);

  // clear view
  DoClear();

};

// destruct
PioGProp::~PioGProp(void) {
  FD_DQG("PioGProp::~PioGProp()");
}

// clear view
void PioGProp::DoClear(void) {
  // fix attribute
  if(!mAttribute) {
    if(pGeneratorConfig)
      mAttribute= qobject_cast<VioAttributeWidget*>(pGeneratorConfig->mGlobalAttribute->NewWidget());
    if(!mAttribute)
      mAttribute= qobject_cast<VioAttributeWidget*>(VioTypeRegistry::NewWidget("AttributeFlags"));
    if(!mAttribute)
      FD_WARN("PioTProp::DoClear(): type registry broken!");
    if(!mAttribute)
      mAttribute= new VioAttributeWidget();
    mAttribute->Model()->Context(pVioGeneratorModel);
    mVbox->addWidget(mAttribute);
    QObject::connect(mAttribute,SIGNAL(NotifyAnyChange(void)),this,SLOT(UpdateModel(void)));
  }
  // fix configuration
  if(!mConfigure) {
    if(pVioGeneratorModel) 
      mConfigure=pVioGeneratorModel->NewConfigView();
    if(mConfigure) {
      mConfigure->Model(pVioGeneratorModel);
      mVbox->addSpacing(10); //??
      mVbox->addWidget(mConfigure);
      // QObject::connect(mConfigure,SIGNAL(NotifyAnyChange(void)),this,SLOT(UpdateModel(void)));
     }
  }
  // clear
  mElement=VioElement::FromEvent(0); // for g prop, void is valid 
  mNumEvents->setText("States: #_");
  mNumStates->setText("Events: #_");
  mNumTrans->setText("Transitions: #_");
  Name("< none >");
  // silent clr
  bool old=mBlockModelUpdate; 
  mBlockModelUpdate=true;
  mAttribute->Clear();
  mBlockModelUpdate=old;
}


// update view for a given trans
void PioGProp::DoVioUpdate(void) {
  // bail out in no model
  if(!pVioGeneratorModel || !mElement.IsVoid()) {
    DoClear();
    return; 
  }
  // report
  FD_DQG("PioGProp::DoVioUpdate() from "<< pVioGeneratorModel << " aka " << Generator()->Name());
  // set my data: generator name and global attribute
  Name(VioStyle::QStrFromStr(Generator()->Name()));
  faudes::AttributeFlags* attr=pVioGeneratorModel->ElementAttr(mElement);
  Attribute(attr);
  // set my statistics
  mNumEvents->setText(QString("Events: #%1").arg(Generator()->AlphabetSize()));
  mNumStates->setText(QString("States: #%1").arg(Generator()->Size()));
  mNumTrans->setText(QString("Transitions: #%1").arg(Generator()->TransRelSize()));
  // done
  FD_DQG("PioGProp::DoVioUpdate(): done");
}

// update model
void PioGProp::DoModelUpdate(void) {
  // if disconnected, emit uniform signal and return 
  if(!pVioGeneratorModel) {
    emit NotifyModified(true);
    return;
  }
  // ignore invalid element
  if(!mElement.IsVoid()) return;
  // else update model
  bool changed=false;
  FD_DQG("PioGProp::DoModelUpdate()");
  // change of event name
  std::string ofname=Generator()->Name();
  std::string nfname=VioStyle::StrFromQStr(Name());
  FD_DQG("PioGProp::DoModelUpdate: (new) name " << nfname);
  // insist in valid symbol
  //if(!faudes::SymbolTable::ValidSymbol(nfname)) nfname=ofname;
  if(nfname != ofname) {
    pVioGeneratorModel->ElementName(mElement,VioStyle::QStrFromStr(nfname));    
    changed|=true;
  }
  // set attribute
  if(!pVioGeneratorModel->ElementAttrTest(mElement,*mAttribute->Attribute())) {
    pVioGeneratorModel->UndoEditStart();
    changed=true;
    pVioGeneratorModel->ElementAttr(
		     mElement,*mAttribute->Attribute()); 
    pVioGeneratorModel->UndoEditStop();
  }
  /*
  // todo change selection event attributes
  */
  FD_DQG("PioGProp::DoModelUpdate: done with changed " << changed);
  if(changed) emit PioGProp::NotifyModified(true);
};


/*

// construct trans widget
PioTprop::PioTprop(QWidget* parent) : PioVProp(parent) {
  FD_DQG("PioTprop(parent)");

  // adjust titles
  mLabelName->setText("Event");

  // add flag properties (event attributes)
  mEventMask=0;
  for(int i=0; i<VioStyle::eventEffects().size(); i++) {
    const FlagEffect& effect = VioStyle::eventEffects()[i];
    if(effect.mPropType==VioEditProp)
      addFlag(effect.mName,effect.mMask, true);
    if(effect.mPropType==VioDispProp)
      addFlag(effect.mName,effect.mMask, false);
    if(effect.mPropType==VioEditProp)
      mEventMask|=effect.mMask;
  }
  mVbox->addSpacing(10);

  // add flag properties (trans attributes)
  mTransMask=0;
  for(int i=0; i<VioStyle::transEffects().size(); i++) {
    const FlagEffect& effect = VioStyle::transEffects()[i];
    if(effect.mPropType==VioEditProp)
      addFlag(effect.mName,effect.mMask, true);
    if(effect.mPropType==VioDispProp)
      addFlag(effect.mName,effect.mMask, false);
    if(effect.mPropType==VioEditProp)
      mTransMask|=effect.mMask;
  }

};

// set values from selection
void PioTprop::ShowSelection(void) {
  if(!pVioGenerator) return;
  FD_DQG("PioTprop::ShowSelection: #" << pVioGenerator->SelectedTrans().size());
  if(pVioGenerator->SelectedTrans().size()>1) 
    mFtrans=faudes::Transition(0,0,0);
  if(pVioGenerator->SelectedTrans().size()==1) 
    mFtrans=*pVioGenerator->SelectedTrans().begin();
  Update();
}


// show trans and set values from faudes trans
void PioTprop::ShowTrans(const faudes::Transition& ftrans) {
  if(!pVioGenerator) return;
  FD_DQG("PioTprop::ShowTrans: trans " << generator()->TStr(ftrans));
  mFtrans=ftrans;
  Update();
}

// set values from faudes trans or selection
void PioTprop::Update(void) {
  if(!pVioGenerator) return;
  FD_DQG("PioTprop::Update() from " << pVioGenerator);
  // set my data: if its one event
  if(generator()->ExistsTransition(mFtrans)){
    mEditName->setCompleter(pVioGenerator->EventCompleter());
    mEditName->setSymbolMode(VioSymbol::ComboBox);
    setName(generator()->EventName(mFtrans.Ev));
    setFlags(generator()->EventAttribute(mFtrans.Ev).mFlags | generator()->TransAttribute(mFtrans).mFlags);
  }
  // set my data: if its the selection
  if(mFtrans==faudes::Transition(0,0,0)) {
    mEditName->setCompleter(0);
    mEditName->setSymbolMode(VioSymbol::ReadOnly);
    setName("< selection >");
    clrFlags();
    foreach(const faudes::Transition& ftrans, pVioGenerator->SelectedTrans()) {
      joinFlags(generator()->EventAttribute(ftrans.Ev).mFlags |
        generator()->TransAttribute(ftrans).mFlags);
    }
  } 
}

//  set faudes trans from values
void PioTprop::PropertyChanged(void) {
  // if disconnected, emit uniform signal and return 
  if(!pVioGenerator) {
    emit Changed(this);
    return;
  }
  bool changed=false;
  FD_DQG("PioTprop::PropertyChanged: " << generator()->TStr(mFtrans));
  // change of event name
  if(mFtrans!=faudes::Transition(0,0,0)) {
    faudes::Transition oftrans=mFtrans;
    // insist in trans to exist
    if(!generator()->ExistsTransition(oftrans)) return;
    // get possibly new event name
    std::string ofname=generator()->EventName(oftrans.Ev);
    std::string nfname=ofname;
    if(faudes::SymbolTable::ValidSymbol(name())) nfname=name();
    FD_DQG("PioTprop::PropertyChanged: (new) event " << nfname);
    // ensure that event exits in generator
    pVioGenerator->InsEvent(nfname);
    // setup possibly new transition
    faudes::Transition nftrans(oftrans.X1,generator()->EventIndex(nfname),oftrans.X2);
    // if it is indeed new, avoid doublets
    if(nftrans != oftrans) 
    if(generator()->ExistsTransition(nftrans)) {
      nfname=generator()->UniqueEventName(nfname);
      nftrans.Ev=pVioGenerator->InsEvent(nfname);
    }
    // actually move transition
    if(nftrans != oftrans) {
      pVioGenerator->MoveTrans(oftrans,nftrans); // incl callback Update incl flags
      changed=true;
    }
    // edit new transition
    mFtrans=nftrans;
  }
  // change of specific attributes
  if(mFtrans!=faudes::Transition(0,0,0)) {
    // set event attributes
    fEventAttribute eattr=generator()->EventAttribute(mFtrans.Ev);
    faudes::fType eflags=flags(eattr.mFlags); 
    if((eattr.mFlags & mEventMask) != (eflags & mEventMask)) {
      FD_DQG("PioTprop::PropertyChanged: event attrib " << eattr.ToString());
      eattr.mFlags&= ~mEventMask; 
      eattr.mFlags|=  eflags & mEventMask;
      pVioGenerator->EventAttribute(mFtrans.Ev,eattr);
      changed=true;
    }
    // set transition attributes
    fTransAttribute tattr=generator()->TransAttribute(mFtrans);
    faudes::fType tflags=flags(tattr.mFlags);
    if((tattr.mFlags  & mTransMask) != (tflags & mTransMask)) {
      FD_DQG("PioTprop::PropertyChanged: trans attrib " << tattr.ToString());
      tattr.mFlags&= ~mTransMask;
      tattr.mFlags|=  tflags & mTransMask;
      pVioGenerator->TransAttribute(mFtrans,tattr);
      changed=true;
    }
  }
  // change attributes of selection
  if(mFtrans==faudes::Transition(0,0,0)) {
    foreach(const faudes::Transition& ftrans,pVioGenerator->SelectedTrans()) {
       // set event attributes
      fEventAttribute eattr=generator()->EventAttribute(ftrans.Ev);
      faudes::fType eflags=flags(eattr.mFlags); 
      if((eattr.mFlags & mEventMask) != (eflags & mEventMask)) {
        eattr.mFlags&= ~mEventMask; 
        eattr.mFlags|=  eflags & mEventMask;
        pVioGenerator->EventAttribute(ftrans.Ev,eattr);
        changed=true;
      }
      // set transition attributes
      fTransAttribute tattr=generator()->TransAttribute(ftrans);
      faudes::fType tflags=flags(tattr.mFlags);
      if((tattr.mFlags  & mTransMask) != (tflags & mTransMask)) {
        tattr.mFlags&= ~mTransMask;
        tattr.mFlags|=  tflags & mTransMask;
        pVioGenerator->TransAttribute(ftrans,tattr);
        changed=true;
      }
    }
  }
  // done
  FD_DQG("PioTprop:PropertyChanged: done with changed " << changed);
  if(changed) emit PioVProp::Changed(this);
}


// construct generator
PioGprop::PioGprop(QWidget* parent) : PioVProp(parent) {
  FD_DQG("PioGprop(parent)");

  // adjust title
  mLabelName->setText("Generator");

  // add initial state property
  mNumEvents = new QLabel(this);
  mNumEvents->setText("States: #_");
  mNumStates = new QLabel(this);
  mNumStates->setText("Events: #_");
  mNumTrans = new QLabel(this);
  mNumTrans->setText("Transitions: #_");

  // layout
  mVbox->addWidget(mNumEvents);
  mVbox->addWidget(mNumStates);
  mVbox->addWidget(mNumTrans);

};


// set values from faudes state  
void PioGprop::Update(void) {
  if(!pVioGenerator) return;
  FD_DQG("PioGprop::Update() from"<< pVioGenerator);
  // get name for combobox, use as display only
  mEditName->setCompleter(0);
  mEditName->setSymbolMode(VioSymbol::AnyString);
  setName(generator()->Name());
  // set my data
  mNumEvents->setText(QString("Events: #%1").arg(generator()->AlphabetSize()));
  mNumStates->setText(QString("States: #%1").arg(generator()->Size()));
  mNumTrans->setText(QString("Transitions: #%1").arg(generator()->TransRelSize()));
}

//  set faudes generator name from values
void PioGprop::PropertyChanged(void) {
  // if disconnected, emit uniform signal and return 
  if(!pVioGenerator) {
    emit Changed(this);
    return;
  }
  // prepare result
  bool changed=false;
  // get possibly new state name
  std::string ofname=generator()->Name();
  std::string nfname="";
  if(faudes::SymbolTable::ValidSymbol(name())) nfname=name();
  FD_DQG("PioGprop::PropertyChanged: " << generator()->Name() << " with (new) name " << nfname);
  // assigne name to generator  
  if(faudes::SymbolTable::ValidSymbol(nfname) && nfname!=ofname) {
    pVioGenerator->GeneratorName(nfname);
    changed=true;
  }
  FD_DQG("PioGprop::PropertyChanged: done with changed " << changed);
  if(changed) emit PioVProp::Changed(this);
}


*/
