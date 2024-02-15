/* viotypes.cpp  - vio type registry */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2009 Ruediger Berndt, Thomas Moor;

*/

//#define FAUDES_DEBUG_VIO_TYPE

#include "viotypes.h"
#include "vionameset.h"
#include "vioattribute.h"


/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioData

****************************************************************
****************************************************************
****************************************************************
*/

// construct
VioData::VioData(QObject* parent) : 
 QObject(parent), 
 mFaudesObject(0)
{}

// destruct
VioData::~VioData(void) {
  if(mFaudesObject) delete mFaudesObject;
}

// clear to default/empty faudes object
void VioData::Clear(void) {
  if(mFaudesObject) mFaudesObject->Clear();
  mText="";
}

// conversion
QMimeData* VioData::ToMime(void) {
  QMimeData* mdat= new QMimeData();
  mdat->setText(mText);
  return mdat;
}

// conversion
int VioData::FromMime(const QMimeData* pMime) {
  if(!pMime->hasText()) return 1;
  mText=pMime->text();
  return 0;
}

// conversion
int VioData::TestMime(const QMimeData* pMime) {
  (void) pMime;
  return 1;
}

// faudes access
const faudes::Type* VioData::FaudesObject(void) const {
  return mFaudesObject;
}

// faudes access
faudes::Type* VioData::FaudesObject(void) {
  return mFaudesObject;
}

// faudes access
void VioData::FaudesObject(faudes::Type* fobject) {
  if(mFaudesObject) delete mFaudesObject;
  mFaudesObject=fobject;
}


// token io interface
void VioData::Write(faudes::TokenWriter& rTw, const QString& ftype) const {
  DoWrite(rTw,ftype);
}

// token io interface
void VioData::Read(faudes::TokenReader& rTr, const QString& ftype) {
  Clear();
  DoRead(rTr,ftype);
}

// token io: implementytion
void VioData::DoWrite(faudes::TokenWriter& rTw, const QString& ftype) const {
  FD_DQT("VioData::DoWrite()");
  // bail out on missing faudes object
  if(!mFaudesObject) return;
  // begin tag with faudes type
  faudes::Token btag;
  std::string section="Type";
  if(ftype!="") section=VioStyle::StrFromQStr(ftype);
  btag.SetBegin(section);
  if(mFaudesObject->Name()!="") 
  if(mFaudesObject->Name()!=section) 
    btag.InsAttributeString("name",mFaudesObject->Name());
  rTw << btag; 
  // my core
  DoWriteCore(rTw,ftype);
  // end of section
  btag.SetEnd(section);
  rTw << btag;
  // done
  FD_DQT("VioData::DoWrite()");
}

// token io: write
void VioData::DoWriteCore(faudes::TokenWriter& rTw, const QString& ftype) const {
  FD_DQT("VioData::DoWriteCore()");
  // bail out on missing faudes object
  if(!mFaudesObject) return;
  // write faudes object

  // long term is XML ... anyway, for now its just generators 
  if( dynamic_cast<faudes::Generator*>(mFaudesObject) ) 
     mFaudesObject->XWrite(rTw);
  else
     mFaudesObject->Write(rTw);

  // write binary
  rTw.WriteBegin("VioTokenText");
  QByteArray buff = mText.toAscii();
  rTw.WriteBinary(buff.constData(),buff.size()); 
  rTw.WriteEnd("VioTokenText");
  // done
  FD_DQT("VioNameSetData::DoWrite()");
}


// token io: read
void VioData::DoRead(faudes::TokenReader& rTr, const QString& ftype) {
  FD_DQT("VioData::DoRead()");
  // bail out on missing faudes object
  if(!mFaudesObject) return;
  // figure my section
  QString section="Type";
  if(ftype!="") section=ftype;
  // read begin
  faudes::Token btag;
  rTr.ReadBegin(VioStyle::StrFromQStr(section),btag);
  // set name
  mFaudesObject->Name(VioStyle::StrFromQStr(section));
  if(btag.ExistsAttributeString("name"))
    mFaudesObject->Name(btag.AttributeStringValue("name"));
  // read section
  DoReadCore(rTr,section);
  // read end
  rTr.ReadEnd(VioStyle::StrFromQStr(section));
  FD_DQN("VioData::DoRead(): done");
}  


// token io: read
void VioData::DoReadCore(faudes::TokenReader& rTr, const QString& section) {
  FD_DQT("VioData::DoReadCore()");
  // bail out on missing faudes object
  if(!mFaudesObject) return;
  // read faudes object
  mFaudesObject->Read(rTr);
  // test for binary data
  if(rTr.ExistsBegin("VioTokenText")) {
    rTr.ReadBegin("VioTokenText");
    std::string rstr;
    rTr.ReadBinary(rstr);
    mText= QString::fromAscii(rstr.c_str(),rstr.size());
    rTr.ReadEnd("VioTokenText");
  } 
}

/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioModel

****************************************************************
****************************************************************
****************************************************************
*/

// my statics
QList<VioModel*> VioModel::msUndoDropOrder;

// construct
VioModel::VioModel(QObject* parent, VioStyle* config, bool alloc) : 
  QObject(parent), 
  pConfig(config),
  mData(0),
  mFaudesLocked(false),   
  mFaudesType(""),
  mModified(false)
{
  // make sure we are configured
  if(!pConfig) pConfig=VioStyle::G();
  mFaudesType=pConfig->FaudesType();
  // report
  FD_DQT("VioModel::VioModel(): " << config << " " << mFaudesType);
  // allocate faudes object and have default rep data
  if(alloc) {
    DoVioAllocate();
    DoVioUpdate();
  }
  // fix modified
  mModified=false;
  // set up my undo stack
  UndoStackClear();
  // pass on clicks to show requests
  //connect(this,SIGNAL(MouseClick(const VioElement&)),
  //  this, SLOT(Show(const VioElement&)));
  // have uniform selection changed signal
  connect(this,SIGNAL(NotifySelectionElement(const VioElement&, bool)),this,SIGNAL(NotifySelectionChange()));
  connect(this,SIGNAL(NotifySelectionClear()),this,SIGNAL(NotifySelectionChange()));
  connect(this,SIGNAL(NotifySelectionAny()),this,SIGNAL(NotifySelectionChange()));
  FD_DQT("VioModel::VioModel(): done");
}

// destruct
VioModel::~VioModel(void) {
  // care about non qobjects
  if(mData->FaudesObject()) 
  if(!mFaudesLocked) mData->FaudesObject(0);
  // ensure that mData destruction does not delete locked faudes object
  mData->mFaudesObject=0;
  // free undo data
  UndoStackClear();
}

// assign
VioModel& VioModel::Assign(const VioModel& rSrc) {
  FD_DQT("VioModel::Assign()");
  Clear();
  DoAssign(rSrc);
  DoVioUpdate();
  FD_DQT("VioModel::Assign(): done");
  return *this;        
}

// assign
VioModel& VioModel::DoAssign(const VioModel& rSrc) {
  FD_DQT("VioModel::DoAssign()");
  // use faudes RTI to extract from faudes object
  if(!mFaudesLocked) 
    mData->FaudesObject()->Assign(*rSrc.FaudesObject());
  return *this;        
}

// construct model on heap
VioModel* VioModel::NewModel(QObject* parent) const {
  FD_DQT("VioModel::NewModel(): type " << mFaudesType);
  return new VioModel(parent,pConfig);
}

// construct data on heap
VioData* VioModel::NewData(QObject* parent) const {
  FD_DQT("VioModel::NewData(): type " << mFaudesType);
  VioData* vdat=new VioData(parent);
  vdat->FaudesObject(FaudesObject()->New());
  return vdat;
}

// construct view on heap
VioView* VioModel::NewView(QWidget* parent) const {
  FD_DQT("VioModel::NewView(): " << FaudesType());
  return new VioView(parent, pConfig);
}

// construct property view on heap (cannot do so)
VioView* VioModel::NewPropertyView(QWidget* parent) const {
  FD_DQT("VioModel::NewPropertyView(): " << FaudesType());
  (void) parent;
  return 0;
}

// construct property user config on heap (cannot do so)
VioView* VioModel::NewConfigView(QWidget* parent) const {
  FD_DQT("VioModel::NewConfigView(): " << FaudesType());
  (void) parent;
  return 0;
}

// construct on heap
VioWidget* VioModel::NewWidget(QWidget* parent) const {
  FD_DQT("VioModep::NewWidget(): type " << FaudesType());
  return new VioWidget(parent, pConfig);
}


// tell configuration
VioStyle* VioModel::Configuration(void) const { 
  return pConfig; 
};

// clear to default/empty faudes object
void VioModel::Clear(void) {
  FD_DQT("VioModel::Clear(): type " << mFaudesType);
  mData->Clear();
  DoVioUpdate();
}

// type info
const QString& VioModel::FaudesType(void) const {
  return mFaudesType;
}

// set faudes object, we take ownership
int VioModel::InsertFaudesObject(faudes::Type* fobject) {
  FD_DQT("VioModel::InsertFaudesObject(" << fobject <<"): type " << mFaudesType);
  // update/unlock on faudes object identity
  if(fobject==mData->FaudesObject()) {
    FD_DQT("VioModel::InsertFaudesObject(" << fobject <<"): unlock");
    mFaudesLocked=false;
    DoVioUpdate();
    return 0;
  }
  // type check
  if(DoTypeCheck(fobject)) return 1;
  FD_DQT("VioModel::InsertFaudesObject(" << fobject <<"): delete/set");
  // set to new faudes object
  mData->FaudesObject(fobject);
  mFaudesLocked=false;
  // update grapical representation data
  FD_DQT("VioModel::InsertFaudesObject(" << fobject <<"): update models and views");
  DoVioUpdate();
  // done
  FD_DQT("VioModel::InsertFaudesObject(" << fobject <<"): done " << mFaudesType);
  return 0;
}

// set faudes object by copy
int VioModel::FaudesObject(const faudes::Type* fobject) {
  FD_DQT("VioModel::FaudesObject(" << fobject <<"): by copy");
  // bail out on faudes object identity
  if(fobject==mData->FaudesObject()) return 0;
  // have a copy
  faudes::Type* copy = fobject->Copy();
  // base set 
  int res=InsertFaudesObject(copy);
  FD_DQT("VioModel::FaudesObject(" << fobject <<"): by copy: done");
  return res;
}


// should be automatic ..
void VioModel::FlushViews(void) {
  // tell lazy views to update model data
  if(!mFaudesLocked) emit NotifyFlush();
}

// get faudes object for inspection
const faudes::Type* VioModel::FaudesObject(void) const { 
  return mData->FaudesObject(); 
}


// get faudes object with write access
faudes::Type* VioModel::TakeFaudesObject(void) { 
  FD_DQT("VioModel::TakeFaudesObject(): get writable faudes object refernce");
  mFaudesLocked=true;
  return mData->FaudesObject(); 
}

// test lock
bool VioModel::FaudesLocked(void) const {
  return mFaudesLocked;
}

// access faudes name
QString VioModel::FaudesName(void) const { 
  return VioStyle::QStrFromStr(mData->FaudesObject()->Name());
}

// access faudes name
void VioModel::FaudesName(const QString& rName) {
  std::string fname = VioStyle::StrFromQStr(rName);
  if(mData->FaudesObject()->Name()==fname) return;
  mData->FaudesObject()->Name(fname);
  emit NotifyNameChange();
}

// token io: write tokenwriter
void VioModel::Write(faudes::TokenWriter& rTw) {
  FD_DQT("VioModel::FaudesWrite(): type " << mFaudesType);
  // About to write, tell lazy views to update model data
  emit NotifyFlush();
  // assemble my section tag
  QString section="Vio"+mFaudesType;
  // vio model tag
  rTw.WriteBegin(VioStyle::StrFromQStr(section));
  // if its an attribute, have extra section guaranteed
  if(mFaudesType.contains("Attribute")) rTw.WriteBegin("Attribute");
  // faudes object 

  // long term is XML ... anyway, for now its just generators 
  if( dynamic_cast<faudes::Generator*>(mData->FaudesObject()) ) 
     mData->FaudesObject()->XWrite(rTw);
  else
     mData->FaudesObject()->Write(rTw);

  // if its an attribute, have extra section guaranteed
  if(mFaudesType.contains("Attribute")) rTw.WriteEnd("Attribute");
  // vio data section
  DoVioWrite(rTw);  
  // end tag
  rTw.WriteEnd(VioStyle::StrFromQStr(section));
  FD_DQT("VioModel::FaudesWrite(): done");
}  

// token io: write to file
void VioModel::Write(const QString& rFileName) {
  try {
    faudes::TokenWriter tw(VioStyle::LfnFromQStr(rFileName));
    Write(tw);
    Modified(false);
  }
  catch (faudes::Exception&) {
    std::stringstream errstr;
    errstr << "Exception opening/writing file \"" << VioStyle::StrFromQStr(rFileName) << "\"";
    throw faudes::Exception("VioModel::Write", errstr.str(), 2);
  }
}

// token io: read tokenwriter
void VioModel::Read(faudes::TokenReader& rTr) {
  FD_DQT("VioModel::FaudesRead(): ftype " << mFaudesType << 
	 " ctype " << typeid(*(FaudesObject())).name() << " to fobject " << mData->FaudesObject());
  // virtual clear
  Clear();
  // assemble my section tag
  QString section="Vio"+mFaudesType;
  // seek my section
  rTr.ReadBegin(VioStyle::StrFromQStr(section));
  // if its an attribute, have extra section guaranteed
  if(mFaudesType.contains("Attribute")) rTr.ReadBegin("Attribute");
  // read faudes object
  mData->FaudesObject()->Read(rTr);
  // if its an attribute, have extra section guaranteed
  if(mFaudesType.contains("Attribute")) rTr.ReadEnd("Attribute");
  // vio data section
  DoVioRead(rTr);
  // my section end
  rTr.ReadEnd(VioStyle::StrFromQStr(section));
  // trigger update
  emit NotifyAnyChange();
  // done
  FD_DQT("VioModel::FaudesRead(): done");
}


// token io: read from file
void VioModel::Read(const QString& rFileName) {
  faudes::TokenReader tr(VioStyle::LfnFromQStr(rFileName));
  Read(tr);
  Modified(false);
}



// token io: faudes write to file
void VioModel::ExportFaudesFile(const QString& rFileName) const {
  try {
    // when an .ftx suffix is specified, we use explicit XML format
    if(QFileInfo(rFileName).suffix()=="ftx") {
      mData->FaudesObject()->XWrite(VioStyle::LfnFromQStr(rFileName));
    } else {
      mData->FaudesObject()->Write(VioStyle::LfnFromQStr(rFileName));
    }
  } catch (faudes::Exception& exception) {
    std::stringstream errstr;
    errstr << "Exception opening/writing file \"" << VioStyle::StrFromQStr(rFileName) << "\"";
    throw faudes::Exception("VioModel::ExportFaudesFile", errstr.str(), 2);
  }
}


// token io: faudes read from file
void VioModel::ImportFaudesFile(const QString& rFileName) {
  faudes::TokenReader tr(VioStyle::LfnFromQStr(rFileName));
  mData->FaudesObject()->Read(tr);
  DoVioUpdate();
}


// allocate visual model data
void VioModel::DoVioAllocate(void) {
  FD_DQT("VioModel::DoVioAllocate()");
  // allocal my data
  if(!mData) mData= new VioData(this);
  // only have faudes object to allocate
  DoFaudesAllocate();
}

// allocate faudes object
void VioModel::DoFaudesAllocate(void) {
  FD_DQT("VioModel::DoFaudsAllocate(): " << mFaudesType);
  // delete old faudes object
  mData->FaudesObject(0);
  // see whether we can find the type in the registry
  if(faudes::TypeRegistry::G()->Exists(VioStyle::StrFromQStr(mFaudesType))) 
  if(faudes::TypeRegistry::G()->Definition(VioStyle::StrFromQStr(mFaudesType)).Prototype()) 
    mData->FaudesObject(faudes::TypeRegistry::G()->NewObject(VioStyle::StrFromQStr(mFaudesType))); 
  // fallback (todo: or should we throw an exception?)
  if(!mData->FaudesObject()) mData->FaudesObject(new faudes::Type());   
}

// test whether we can host this faudes object
int VioModel::DoTypeCheck(const faudes::Type* fobject) const {  
  (void) fobject;
  // we host any faudes object 
  // (todo: check existence in faudes registry, check copy/assignment, check tokenio)
  return 0; 
}

// update visual data from (new) faudes object
void VioModel::DoVioUpdate(void) {
  FD_DQT("VioModel::DoVioUpdate()");
  // set debug text
  mData->mText=VioStyle::QStrFromStr(FaudesObject()->ToText());
  // notify
  emit NotifyAnyChange();
}


// token io: vio data
void VioModel::DoVioWrite(faudes::TokenWriter& rTw) const {
  FD_DQT("VioModel::DoVioWrite()");
  QByteArray buff = mData->mText.toAscii();
  rTw.WriteBinary(buff.constData(),buff.size()); 
}

// token io: vio data
void VioModel::DoVioRead(faudes::TokenReader& rTr) {
  FD_DQT("VioModel::DoVioRead()");
  // relax in universal base class
  faudes::Token token;
  rTr.Peek(token);
  // do actual read
  if(token.Type()==faudes::Token::Binary) {
    std::string rstr;
    rTr.ReadBinary(rstr);
    mData->mText= QString::fromAscii(rstr.c_str(),rstr.size());
  } else {
    mData->mText= VioStyle::QStrFromStr(mData->FaudesObject()->ToText());   // fake it for derived debugging functionality
  }
}


// vio data access: get
VioData* VioModel::Data(void) {
  FD_DQT("VioModel::Data(): retrieve all data");
  VioData* vdat= new VioData();
  vdat->FaudesObject(mData->FaudesObject()->Copy());
  vdat->mText=VioText();
  return vdat;
}

// vio data access: set (0 on accept)
int VioModel::Data(const VioData* pData) {
  FD_DQT("VioModel::Data(): set all data");
  // reject wrong data type
  if(TypeCheckData(pData)!=0) return 1;
  // clear and merge
  Clear();
  DoMergeData(pData);
  return 0;
};


// vio data access: set (1 on changes)
int VioModel::DoMergeData(const VioData* pData) {
  (void) pData;
  FD_DQT("VioModel::DoMergeData()");
  return 0;
}


// vio data access (1 on error)
int VioModel::TypeCheckData(const VioData* pData) {
  FD_DQT("VioModel::TypeCheckData()");
  (void) pData;
  return 1;
};

// set typed viodata
void VioModel::VioText(const QString& text) {
  FD_DQT("VioModel::VioText(): set text \"" << text<<"\"");
  // apply on representation data
  mData->mText=text;
  // apply on faudes object (may throw an expection)
  mData->FaudesObject()->FromString(VioStyle::StrFromQStr(text));
  // bail out on identity
  if(text==mData->mText) return;
  // notify
  emit NotifyAnyChange();
}

// get viodata
const QString& VioModel::VioText(void) const {
  return mData->mText;
}

// seletion: clear
void VioModel::SelectionClear(void) {
  FD_DQT("VioModel::SelectionClear(): #" << mSelection.size());
  bool changed= (mSelection.size()!=0);
  mSelection.clear();
  if(changed) emit NotifySelectionClear();
}

// seletion: query
bool VioModel::IsSelected(const VioElement& elem) const {
  return mSelection.contains(elem);
}

// seletion: query all
const QList<VioElement>& VioModel::Selection(void) const {
  FD_DQT("VioModel::Selection(): #" << mSelection.size());
  return mSelection;
}
 
// seletion: select
void VioModel::Select(const VioElement& elem, bool on) {
  // ignore invalid/void
  if(elem.IsVoid()) return;  
  //if(!elem.IsValid()) return;  
  // report
  FD_DQT("VioModel::Select(): #" << mSelection.size() << " " << elem.Str() << " to " << on);
  // test for actual changes
  bool contained=mSelection.contains(elem);
  bool changed= (contained!=on);
  // do it
  if(contained && !on) mSelection.removeAll(elem);
  if(!contained && on) mSelection.append(elem);
  if(!contained && on) qSort(mSelection);
  // emit
  if(changed) emit NotifySelectionElement(elem,on);
}


// query changes (dont emit signal)
bool VioModel::Modified(void) const { 
  return mModified;
};

// collect and pass on modifications of childs
void VioModel::ChildModified(bool changed) { 
  // ignre netagtives
  if(!changed) return;
  // report
  FD_DQT("VioModel::ChildModified(1): model modified " << mModified);
  Modified(true);
};

// collect and pass on modifications of childs
void VioModel::ParentModified(bool changed) { 
  // ignre positives
  if(changed) return;
  // report
  FD_DQT("VioModel::ParentModified(0): clr modified " << mModified);
  Modified(false);
};


// record changes and emit signal)
void VioModel::Modified(bool ch) { 
  // set
  if(!mModified && ch) {
    mModified=true;
    emit NotifyModified(mModified);
  }
  // clr
  if(mModified && !ch) {
    mModified=false;
    FD_DQT("VioModel::Modified(" << this << "): emit modified notification");
    emit NotifyModified(mModified);
  }
  // clear stack
  if(!ch) {
    UndoStackClear();
  }
}

// forward highlite requests to all views
void VioModel::Highlite(const VioElement& elem, bool on) {;
  FD_DQT("VioModel::Highlite(): emit notification");
  emit NotifyHighlite(elem,on);
}

// forward highlite requests
void VioModel::HighliteClear(void) {;
  FD_DQT("VioModel::HighliteClear(): emit notification");
  emit NotifyHighliteClear();
}

// forward show requests
void VioModel::Show(const VioElement& elem) {;
  FD_DQT("VioModel::Show(): emit notification");
  emit NotifyShow(elem);
  FD_DQT("VioModel::Show(): done");
}


// state based undo scheme: undo command with vio data TODO: guarded pointers
/*
class VioUndoCommand : public QUndoCommand {
public:
  VioUndoCommand(VioModel* model) : QUndoCommand() {
    FD_DQT("VioUndoCommand::VioUndoCommand()");
    pModel=model; 
    mDataT0=pModel->Data(); 
    mDataT1=0; 
    FD_DQT("VioUndoCommand::VioUndoCommand(): done");
  };
  ~VioUndoCommand(void) { 
    if(mDataT1) delete mDataT1; 
    delete mDataT0; 
  };
  void undo(void) {
    if(!pModel) return; 
    FD_DQT("VioUndoCommand::undo()");
    if(mDataT1) delete mDataT1; 
    mDataT1=pModel->Data(); 
    pModel->Data(mDataT0);};
  void redo(void) {
    if(!pModel) return; 
    FD_DQT("VioUndoCommand::redo()");
    if(mDataT1) pModel->Data(mDataT1);
  }; 
private:
  QPointer<VioModel> pModel;
  VioData* mDataT0;
  VioData* mDataT1;
};
*/

// state based undo scheme: clear current stack (use also for construct/destruct)
void VioModel::UndoStackClear(void) { 
  FD_DQT("VioModel::UndoStackClear");
  mUndoEditLevel=0;
  mUndoCurrent=-1;
  while(mUndoStack.size()>0) {
    delete mUndoStack.back();
    mUndoStack.pop_back();
  }
  msUndoDropOrder.removeAll(this);
};

// state based undo: pop (only we call this)
void VioModel::UndoStackPopBack(void) { 
  FD_DQT("VioModel::UndoStackPopBack");
  if(mUndoStack.size()==0) return;
  delete mUndoStack.back();
  mUndoStack.pop_back();
  for(int i=msUndoDropOrder.size()-1; i>=0 ; i--) {
    if(msUndoDropOrder.at(i)==this) {
      msUndoDropOrder.removeAt(i);
      break;
    }
  }
}

// state based undo: pop (others call this asynchronously)
void VioModel::UndoStackPopFront(void) { 
  FD_DQT("VioModel::UndoStackPopFront");
  if(mUndoStack.size()==0) return;
  delete mUndoStack.front();
  mUndoStack.pop_front();
  if(mUndoCurrent>=0) mUndoCurrent--;
  for(int i=0; i<msUndoDropOrder.size(); i++) {
    if(msUndoDropOrder.at(i)==this) {
      msUndoDropOrder.removeAt(i);
      break;
    }
  }
}

// state based undo: push
void VioModel::UndoStackPushBack(void) { 
  mUndoStack.push_back(Data());
  msUndoDropOrder.push_back(this);
  if(msUndoDropOrder.size()<=15) return; // TODO: style the stack limit
  VioModel* other=msUndoDropOrder.at(0);
  other->UndoStackPopFront();
}


// state based undo scheme: user starts editing
void VioModel::UndoEditStart(void) {
  FD_DQT("VioModel::UserEditStart()");
  // edit in progess
  if(mUndoEditLevel>0) {
    mUndoEditLevel++;
    return;
  }
  // chop stack and cancel undo/redo
  if(mUndoCurrent!=-1) {
    while(mUndoStack.size()>mUndoCurrent) 
      UndoStackPopBack();
    mUndoCurrent=-1;
  }
  // record state
  FD_DQT("VioModel::UserEditStart(): MarkUndoPoint #" << mUndoStack.size());
  UndoStackPushBack();
  mUndoEditLevel=1;
};

// state based undo scheme: user starts editing
void VioModel::UndoEditStop(void) {
  // ignore during undo/redo
  if(mUndoCurrent!=-1) {
    FD_DQT("VioModel::UserEditStop(): ignore during undo/redo");
    mUndoEditLevel--;
    return;
  }
  // record stop
  mUndoEditLevel--;
  FD_DQT("VioModel::UserEditStop(): level " << mUndoEditLevel);
  if(mUndoEditLevel < 0) {
    FD_DQT("VioModel::UserEditStop(): fixing unbalanced");
    mUndoEditLevel=0;
  }
};

// state based undo scheme: user starts editing
void VioModel::UndoEditCancel(void) {
  // ignore during undo/redo
  if(mUndoCurrent!=-1) {
    FD_DQT("VioModel::UserEditCancel(): ignore during undo/redo");
    mUndoEditLevel--;
    return;
  }
  // ignore unbalanced 
  if(mUndoEditLevel <= 0) {
    FD_DQT("VioModel::UserEditCancel(): ignore unbalanced");
    mUndoEditLevel=0;
    return;
  }
  // do cancel
  mUndoEditLevel--;
  // drop last entry
  if(mUndoEditLevel ==0) {
    if(mUndoStack.size()>0) {
      FD_DQT("VioModel::UserEditCancel(): drop last entry");
      UndoStackPopBack();
    }
  }
};


// state based undo scheme: undo
void VioModel::Undo(void) {
  mUndoEditLevel=0;
  // there is no more to undo
  if(mUndoCurrent==0 || mUndoStack.size()==0) 
    return;
  // record state at first undo
  if(mUndoCurrent==-1) {
    UndoStackPushBack();
    mUndoCurrent=mUndoStack.size()-1;
  }
  // undo
  if(mUndoCurrent-1 >=0) 
  if(mUndoCurrent-1 < mUndoStack.size()) 
  { 
    mUndoCurrent--;
    FD_DQT("VioModel::Undo() #" << mUndoStack.size() << " at " << mUndoCurrent); 
    Data(mUndoStack.at(mUndoCurrent));
  }
};

// state based undo scheme: redo
void VioModel::Redo(void) {
  FD_DQT("VioModel::Redo()");
  mUndoEditLevel=0;
  if(mUndoCurrent==-1) return;
  if(mUndoCurrent+1 >=0) 
  if(mUndoCurrent+1 < mUndoStack.size()) 
  {
    mUndoCurrent++;
    FD_DQT("VioModel::Redo() #" << mUndoStack.size() << " at " << mUndoCurrent); 
    Data(mUndoStack.at(mUndoCurrent));
  }
};



/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioView

****************************************************************
****************************************************************
****************************************************************
*/

// construct
VioView::VioView(QWidget* parent, VioStyle* config, bool alloc) : 
  QWidget(parent), 
  mFaudesType("Type"),
  pConfig(config),
  pModel(0), 
  mModified(false),
  mTextInfo(0),
  mTextEdit(0),
  mApplyButton(0)
{
  // make sure we are configured
  if(!pConfig) pConfig=VioStyle::G();
  mFaudesType=pConfig->FaudesType();
  // report
  FD_DQT("VioView::VioView(): type " << mFaudesType);
  // set layout
  setWindowFlags(Qt::Widget);
  //setContentsMargins(0,0,0,0);              // margin issue test: set them to 0 ...
  //QTextEdit* q= new QTextEdit("GGG",this);  // ... and test with an unmanaged widget: OK
#ifdef Q_WS_MAC 
  setContentsMargins(-6,0,-6,0);              // margin issue cheap fix: compensate offset
#endif
  mVbox = new QVBoxLayout(this);
  mVbox->setMargin(0);
  mVbox->setSpacing(0);
  mVbox->setContentsMargins(0,0,0,0);
  // allocate further data
  if(alloc) {
    DoVioAllocate();
    mTextInfo->show();
    mTextEdit->show();
    mApplyButton->show();
  }
  // done
  FD_DQT("VioView::VioView(): done");
}

// destruct
VioView::~VioView(void) {
}

// report configuration
VioStyle* VioView::Configuration(void) const { 
  return pConfig; 
};

// type we can host
const QString& VioView::FaudesType(void) const {
  return mFaudesType;
};

// set by vio model
int VioView::Model(VioModel* model) {
  FD_DQT("VioView::Model(" << model << "): type " << model->FaudesType());
  // bail out on identity
  if(model==pModel) return 0;
  // disconnect
  if(pModel) disconnect(pModel, 0, this, 0);
  // type check
  if(model) if(model->FaudesType()!= FaudesType()) model=0;
  FD_DQT("VioView::Model(" << model << "): stage 2");
  // set model
  pModel=model;
  // update visual
  DoVioUpdate();
  FD_DQT("VioView::Model(" << model << "): stage 3");
  // if void were done
  if(!pModel) return 1;
  // connect model 
  connect(pModel, SIGNAL(NotifyAnyChange(void)), this, SLOT(UpdateView(void)));
  connect(pModel, SIGNAL(NotifyNameChange(void)), this, SLOT(UpdateView(void)));
  connect(pModel, SIGNAL(NotifyFlush(void)), this, SLOT(UpdateModel())); // most models dont need this
  connect(pModel, SIGNAL(NotifyShow(const VioElement&)), this, SLOT(Show(const VioElement&)));
  connect(pModel, SIGNAL(NotifyHighlite(const VioElement&, bool)), this, SLOT(Highlite(const VioElement&,bool)));
  connect(pModel, SIGNAL(NotifyHighliteClear(void)), this, SLOT(HighliteClear(void)));
  connect(pModel, SIGNAL(NotifyModified(bool)), this, SLOT(ParentModified(bool)));
  // manage changed flag
  Modified(pModel->Modified());
  // done
  FD_DQT("VioView::Model(" << model << "): done");
  return 0;
}

// get by vio model
const VioModel* VioView::Model(void) const {
  return pModel;
}

// get faudes object
const faudes::Type* VioView::FaudesObject(void) const  {
  if(!pModel) return 0;
  return pModel->FaudesObject();
}

// update model from view slot
void VioView::UpdateModel(void) {
  FD_DQT("VioView::UpdateModel(): update model from widget");
  // catch faudes errors
  QString err="";
  try { 
    DoModelUpdate();
  } catch (faudes::Exception& fexcep) {
    err=QString("Token mismatch: ")+VioStyle::QStrFromStr(fexcep.What());
  }
  // report error
  if(err!="") {
    // figre line
    int line=-1;
    QRegExp rex("\\(#.*\\)");
    int pos= rex.indexIn(err);
    if(pos>=0) {
      int len=rex.matchedLength();
      FD_DQT("VioView::UpdateModel(): interpret err str: match: " << pos << "/" << len);
      QString numstr=err.mid(pos+2,len-3);
      bool nsok;
      line = numstr.toInt(&nsok);
      if(!nsok) line=-1;
    }
    mTextEdit->ShowLine(line);
    //emit StatusMessage(err);
    emit ErrorMessage("Updating FAUDES object: "+ err);
  }
  // set changed
  Modified(true);
}

// update view from (new) model
void VioView::UpdateView(void) {
  FD_DQT("VioView::UpdateView(): update view from model");
  DoVioUpdate();
}

// allocate my data
// note: base class widgets are used for debugging; hidden by default
void VioView::DoVioAllocate(void) {
  FD_DQT("VioView::DoVioAllocate()");
  mTextInfo = new QLabel();
  mTextInfo->setText(tr("Token editor for FAUDES type \"%1\"").arg(FaudesType()));
  mTextInfo->hide();
  mVbox->addWidget(mTextInfo);
  mTextEdit= new VioTokenEditor();
  mTextEdit->setTextInteractionFlags(Qt::TextSelectableByKeyboard | Qt::TextEditable); 
  mTextEdit->installEventFilter(this);
  QObject::connect(mTextEdit,SIGNAL(modificationChanged(bool)),this,SLOT(ChildModified(bool)));
  mTextEdit->hide();
  mVbox->addWidget(mTextEdit);
  mApplyButton= new QPushButton();
  mApplyButton->setText("Apply");
  QObject::connect(mApplyButton,SIGNAL(pressed(void)),this,SLOT(UpdateModel(void)));
  mApplyButton->hide();
  mVbox->addWidget(mApplyButton);
#ifdef FAUDES_DEBUG_VIO_WIDGETS
  mTextInfo->show();
  mTextEdit->show();
  mApplyButton->show();
#endif
  // read actions from style
  for(int i=0; i< pConfig->EditFunctions().size(); i++) {
    const VioEditFunction& vfnct=pConfig->EditFunctions().at(i);
    QAction* act=new QAction(vfnct.mName,this);
    act->setProperty("FaudesFunction",vfnct.mFFunction);
    act->setStatusTip("run faudes function "+vfnct.mName);
    act->setEnabled(true);
    QObject::connect(act,SIGNAL(triggered(bool)),this,SLOT(ApplyEditFunction(void)));
    mEditActions.append(act); 
  }
  // std actions
  mCutAction = new QAction("Cut",this);
  mCutAction->setShortcut(tr("Ctrl+X"));
  mCutAction->setStatusTip(tr("Cut the current selection's contents to the clipboard"));
  connect(mCutAction, SIGNAL(triggered()), this, SLOT(Cut()));
  mPasteAction = new QAction("Paste",this);
  mPasteAction->setShortcut(tr("Ctrl+V"));
  mPasteAction->setStatusTip(tr("Paste the clipboard's contents into the current selection"));
  connect(mPasteAction, SIGNAL(triggered()), this, SLOT(Paste()));
  mCopyAction = new QAction(tr("&Copy"), this);
  mCopyAction->setShortcut(tr("Ctrl+C"));
  mCopyAction->setStatusTip(tr("Copy the current selection's contents to the clipboard"));
  connect(mCopyAction, SIGNAL(triggered()), this, SLOT(Copy()));
  mUndoAction = new QAction("Undo",this);
  mUndoAction->setShortcut(tr("Ctrl+Z"));
  mUndoAction->setStatusTip(tr("Undo last operation"));
  connect(mUndoAction, SIGNAL(triggered()), this, SLOT(Undo()));
  mRedoAction = new QAction("Redo",this);
  mRedoAction->setShortcut(tr("Ctrl+Shift+Z"));
  mRedoAction->setStatusTip(tr("Redo last operation"));
  connect(mRedoAction, SIGNAL(triggered()), this, SLOT(Redo()));
  FD_DQT("VioView::DoVioAlocate(): done");
}

// update view from (new) model
void VioView::DoVioUpdate(void) {
  FD_DQT("VioView::DoVioUpdate(): type " << FaudesType());
  // set text
  if(pModel) mTextEdit->setPlainText(pModel->VioText());
  else mTextEdit->setPlainText("No Model");
  mTextEdit->document()->setModified(false);
}

// update model from widget
void VioView::DoModelUpdate(void) {
  FD_DQT("VioView::DoModelUpdate(): type " << FaudesType());
  // bail out if widget or model is not ready
  if(!mTextEdit || !pModel) return;
  // do the update
  pModel->VioText(mTextEdit->toPlainText()); 
}


// event filter (currently used on the textedit widget
// to ignore shortcuts and pass them on to their slots)
// THIS IS NOT THE PROPER SOLUTION TO THE ISSUE
bool VioView::eventFilter(QObject *obj, QEvent *event) {
  if(obj == mTextEdit) {
    if(event->type() == QEvent::KeyPress) {
      QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
      //FD_DQT("VioView::eventFilter: see key" << keyEvent->key());
      if(keyEvent->modifiers() & (Qt::ControlModifier | Qt::MetaModifier | Qt::MetaModifier)) {
	FD_DQT("VioView::eventFilter: catch modifier and process");
        if(keyEvent->key()==Qt::Key_C) Copy();
        if(keyEvent->key()==Qt::Key_X) Cut();
        if(keyEvent->key()==Qt::Key_V) Paste();
        return true;
      } 
    }
    return false;
  }
  return QWidget::eventFilter(obj, event);
}

// query changes (dont emit signal)
bool VioView::Modified(void) const { 
  return mModified;
};

// collect and pass on modifications of childs
void VioView::ChildModified(bool changed) { 
  // ignre netagtives
  if(!changed) return;
  // report
  FD_DQT("VioView::ChildModified(1): model modified " << mModified);
  Modified(true);
};

// collect and pass on modifications of childs
void VioView::ParentModified(bool changed) { 
  // ignre positives
  if(changed) return;
  // report
  FD_DQT("VioViewl::ParentModified(0): clr modified " << mModified);
  Modified(false);
};

// record changes and emit signal)
void VioView::Modified(bool ch) { 
  // set
  if(!mModified && ch) {
    mModified=true;
    emit NotifyModified(mModified);
  }
  // clr
  if(mModified && !ch) {
    mModified=false;
    FD_DQT("VioView::Modified(" << this << "): emit modified notification");
    emit NotifyModified(mModified);
  }
  // clr my widgets 
  if(!ch && mTextEdit) 
    mTextEdit->document()->setModified(false);
}

// forward highlite requests (-> model -> all views ??? should we better just process?) 
void VioView::Highlite(const VioElement& elem, bool on) {;
  FD_DQT("VioView::Highlite(): emit notification");
  emit NotifyHighlite(elem,on);
}

// forward highlite requests
void VioView::HighliteClear(void) {;
  FD_DQT("VioView::HighliteClear(): emit notification");
  emit NotifyHighliteClear();
}

// forward show requests
void VioView::Show(const VioElement& elem) {;
  FD_DQT("VioView::Show(): emit notification");
  emit NotifyShow(elem);
}

// vio data access
int VioView::InsertData(const VioData* pData) {
  if(!pModel) return 0;
  FD_DQT("VioView::InsertData(): insert data: " << pData->mText);
  mTextEdit->insertPlainText(pData->mText);
  return 1;
}

// vio data access
VioData* VioView::SelectionData(void) {
  if(!pModel) return 0;
  FD_DQT("VioView::SelectionData(): retrieve data");
  VioData* vdat= new VioData();
  const faudes::Type* fobj= pModel->FaudesObject();
  vdat->FaudesObject(fobj->New());
  // todo: assign interface
  vdat->mText=mTextEdit->textCursor().selectedText(); 
  FD_DQT("VioView::SelectionData(): retrieve data: " << vdat->mText);
  return vdat;
}

// vio data access
void VioView::DeleteSelection(void) {
  if(!pModel) return;
  FD_DQT("VioView::DeleteSelection(): delete selection");
  mTextEdit->textCursor().removeSelectedText();
}

// mime data access
int VioView::TestMimeData(const QMimeData* pMimeData) {
  if(!pModel) return 1;
  VioData* vdat= pModel->NewData(); 
  return vdat->TestMime(pMimeData);
}

// mime data access
int VioView::InsertMimeData(const QMimeData* pMimeData) {
  if(!pModel) return 1;
  FD_DQT("VioView::InsertMimeData()");
  VioData* vdat= pModel->NewData(); 
  vdat->FromMime(pMimeData);
  int res=InsertData(vdat);
  delete vdat;
  FD_DQT("VioView::InsertMimeData(): done");
  return res;
}

// mime data access
QMimeData* VioView::SelectionMimeData(void) {
  FD_DQT("VioView::SelectionMimeData()");
  VioData* vdat=SelectionData();
  QMimeData* mdat=vdat->ToMime();
  FD_DQT("VioView::SelectionMimeData(): size " << mdat->text().size());
  delete vdat;
  return mdat;
}


// clippboard
void VioView::Cut(void) {
  if(!pModel) return;
  if(pModel->Selection().size()==0) return;
  FD_DQT("VioView::Cut()");
  pModel->UndoEditStart();
  QMimeData* mdat=SelectionMimeData();
  QApplication::clipboard()->setMimeData(mdat);
  DeleteSelection();
  pModel->UndoEditStop();
  FD_DQT("VioView::Cut():done");
}

// clippboard
void VioView::Copy(void) {
  if(!pModel) return;
  FD_DQT("VioView::Copy()");
  QMimeData* mdat=SelectionMimeData();
  QApplication::clipboard()->setMimeData(mdat);
  FD_DQT("VioView::Copy():done");
}

// clippboard
void VioView::Paste(void) {
  if(!pModel) return;
  FD_DQT("VioView::Paste()");
  pModel->UndoEditStart();
  DeleteSelection();
  const QMimeData* mdat=  QApplication::clipboard()->mimeData();
  InsertMimeData(mdat);
  pModel->UndoEditStop();
  FD_DQT("VioView::Paste():done");
}

// pass on undo
void VioView::Undo(void) {
  if(!pModel) return;
  FD_DQT("VioView::Undo()");
  pModel->Undo();
}

// pass on undo
void VioView::Redo(void) {
  if(!pModel) return;
  FD_DQT("VioView::Redo()");
  pModel->Redo();
}

// apply edit function
void VioView::ApplyEditFunction(void) {
  FD_DQT("VioView::ApplyEditFunction()");
  if(!sender()) return;
  QString ffnct=sender()->property("FaudesFunction").toString();
  ApplyFaudesFunction(ffnct);
}

// apply faudes function
int VioView::ApplyFaudesFunction(const QString& ffnct) {
  // bail out on missing model
  if(!pModel) return 2;
  // get function from faudes registry
  FD_DQT("VioView::ApplyFaudesFunction(" << ffnct << ")");
  if(!faudes::FunctionRegistry::G()->Exists(VioStyle::StrFromQStr(ffnct))) {
    FD_DQT("VioView::ApplyFaudesFunction(" << ffnct << "): not found in faudes registry");
    return 1;
  }
  faudes::Function* fnct = faudes::NewFaudesFunction(VioStyle::StrFromQStr(ffnct));
  // lock faudes object, track name
  faudes::Type* fobject=pModel->TakeFaudesObject();
  std::string fname=fobject->Name();
  // test signatures
  for(int i=0; i<fnct->VariantsSize(); i++) {
    fnct->Variant(i);
    const faudes::Signature* signature=fnct->Variant();
    FD_DQT("VioView::ApplyFaudesFunction(" << ffnct << "): testing signature " <<
	   signature->Name() << " #" << signature->Size());
    if(signature->Size()!=1) continue;
    fnct->ParamValue(0,fobject);
    if(fnct->TypeCheck(0)) break;
    FD_DQT("VioView::ApplyFaudesFunction(" << ffnct << "): typecheck failed")
  }
  // execute
  if(fnct->TypeCheck()) {
    FD_DQT("VioView::ApplyFaudesFunction(" << ffnct << "): execute");
    try {
      fnct->Execute();
    } catch(faudes::Exception) {
    }
  }
  // return ownership
  fobject->Name(fname);
  FD_DQT("VioView::ApplyFaudesFunction(" << ffnct << "): result " << fobject->ToString());
  pModel->InsertFaudesObject(fobject);
  return 0;
}



/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioWidget

****************************************************************
****************************************************************
****************************************************************
*/

// construct
VioWidget::VioWidget(QWidget* parent, VioStyle* config, bool alloc) : 
  QWidget(parent), 
  pConfig(config),
  mModel(0),
  mView(0), 
  mModified(false)
{
  // make sure we are configures
  if(!pConfig) pConfig=VioStyle::G();
  // report
  FD_DQT("VioWidget::VioWidget(): " << pConfig->ConfigName());
  // set layout
  setContentsMargins(0,0,0,0);
  mVbox = new QVBoxLayout(this);
  mVbox->setSpacing(0);
  mVbox->setMargin(0);
   // allocate model and view
  if(alloc) DoVioAllocate();
  // fix modified flag
  mModified=false;
  // done
  FD_DQT("VioWidget::VioWidget(): done");
}

// destruct
VioWidget::~VioWidget(void) {
}

// tell configuration
VioStyle* VioWidget::Configuration(void) const { 
  return pConfig; 
};

// clear to default/empty faudes object
void VioWidget::Clear(void) {
  FD_DQT("VioWidget::Clear(): type " << FaudesType());
  mModel->Clear();
}

// type info
const QString& VioWidget::FaudesType(void) const {
  return mView->FaudesType();
}

// set by faudes object
int VioWidget::InsertFaudesObject(faudes::Type* fobject) {
  FD_DQT("VioWidget::FaudesObject(" << fobject << "): type " << FaudesType());
  int res=mModel->InsertFaudesObject(fobject);
  FD_DQT("VioWidget::FaudesObject(" << fobject << "): done " << FaudesType());
  return res;
}

// set by faudes object by copy
int VioWidget::FaudesObject(const faudes::Type* fobject) {
  FD_DQT("VioWidget::FaudesObject(" << fobject << "): type " << FaudesType());
  int res=mModel->FaudesObject(fobject);
  FD_DQT("VioWidget::FaudesObject(" << fobject << "): done " << FaudesType());
  return res;
}

// get faudes object
const faudes::Type* VioWidget::FaudesObject(void) const  {
  return mModel->FaudesObject();
}

// get faudes object
faudes::Type* VioWidget::TakeFaudesObject(void) {
  return mModel->TakeFaudesObject();
}

// set by vio model
int VioWidget::Model(VioModel* model) {
  FD_DQT("VioWidget::Model(" << model << "): type " << FaudesType());
  // bail out on identity
  if(model==mModel) return 0;
  // disconnect
  if(mModel) disconnect(mModel,0,this,0);
  // type check
  if(model->FaudesType()!= FaudesType()) return 1;
  // introduce to view
  mView->Model(model);
  // displose old model
  if(mModel) delete mModel;
  // record new model
  mModel=model;
  // done, if void
  if(!mModel) return 1;
  // take ownership
  mModel->setParent(this);
  // connect model
  QObject::connect(mModel,SIGNAL(NotifyModified(bool)),this,SLOT(ChildModified(bool)));
  QObject::connect(mModel,SIGNAL(StatusMessage(const QString&)),this,SIGNAL(StatusMessage(const QString&)));
  QObject::connect(mModel,SIGNAL(ErrorMessage(const QString&)),this,SIGNAL(ErrorMessage(const QString&)));
  QObject::connect(mModel,SIGNAL(NotifyAnyChange(void)),this,SIGNAL(NotifyAnyChange(void)));
  // done
  FD_DQT("VioWidget::Model(" << model << "): done");
  return 0;
}


// get vio model
const VioModel* VioWidget::Model(void) const {
  return mModel;
}

// get vio view
const VioView* VioWidget::View(void) const {
  return mView;
}


// allocate model/view
void VioWidget::DoVioAllocate(void) {
  // bail on double call 
  if(mView) return;
  // reprot
  FD_DQT("VioWidget::DoVioAllocate()");
  // have a view
  if(mView) delete mView;
  mView= new VioView(0,pConfig);
  mVbox->addWidget(mView);
  // connect view
  QObject::connect(mView,SIGNAL(NotifyModified(bool)),this,SLOT(ChildModified(bool)));
  QObject::connect(mView,SIGNAL(MouseClick(const VioElement&)),this,SIGNAL(MouseClick(const VioElement&)));
  QObject::connect(mView,SIGNAL(MouseDoubleClick(const VioElement&)),this,SIGNAL(MouseDoubleClick(const VioElement&)));
  QObject::connect(mView,SIGNAL(StatusMessage(const QString&)),this,SIGNAL(StatusMessage(const QString&)));
  QObject::connect(mView,SIGNAL(ErrorMessage(const QString&)),this,SIGNAL(ErrorMessage(const QString&)));
  // have a model
  VioModel* model= new VioModel(this,pConfig);
  // set it
  Model(model);
  FD_DQT("VioWidget::DoVioAllocate(): done");
}



// token io: pass through to my model
void VioWidget::Write(faudes::TokenWriter& rTw) {
  // be sure that visual rep is in sync with model
  if(mView->Modified()) mView->UpdateModel();
  // do write
  mModel->Write(rTw);
}

// token io: pass through to my model
void VioWidget::Write(const QString& rFilename) {
  // be sure that visual rep is in sync with model
  if(mView->Modified()) mView->UpdateModel();
  // do write
  mModel->Write(rFilename);
  // manage changed flag 
  Modified(false);
}

// token io: pass through to my model
void VioWidget::Read(faudes::TokenReader& rTr) {
  mModel->Read(rTr);
}

// token io: pass through to my model
void VioWidget::Read(const QString& rFilename) {
  mModel->Read(rFilename);
  // manage changed flag
  Modified(false);
}

// token io: faudes write to file
void VioWidget::ExportFaudesFile(const QString& rFileName) const {
  mModel->ExportFaudesFile(rFileName);
}



// token io: faudes read from file
void VioWidget::ImportFaudesFile(const QString& rFileName) {
  mModel->ImportFaudesFile(rFileName);
}

// contribute to menus:pass on to my view
const QList<QAction*>& VioWidget::FileActions() const { return mView->FileActions();};
const QList<QAction*>& VioWidget::EditActions() const { return mView->EditActions();};
const QList<QAction*>& VioWidget::ViewActions() const { return mView->ViewActions();};
QAction* VioWidget::CutAction() const { return mView->CutAction();};
QAction* VioWidget::CopyAction() const { return mView->CopyAction();};
QAction* VioWidget::PasteAction() const { return mView->PasteAction();};
QAction* VioWidget::UndoAction() const { return mView->UndoAction();};
QAction* VioWidget::RedoAction() const { return mView->RedoAction();};

// query changes (dont emit signal)
bool VioWidget::Modified(void) const { 
  return mModified;
};

// collect and pass on modifications of childs
void VioWidget::ChildModified(bool changed) { 
  // ignre netagtives
  if(!changed) return;
  // report
  FD_DQT("VioWidget::ChildModified(1): model modified " << mModified);
  Modified(true);
};

// record changes and emit signal)
void VioWidget::Modified(bool ch) { 
  // set
  if(!mModified && ch) {
    mModified=true;
    emit NotifyModified(mModified);
  }
  // clr
  if(mModified && !ch) {
    mModified=false;
    FD_DQT("VioWidget::Modified(" << this << "): emit modified notification");
    emit NotifyModified(mModified);
  }
  // clr my childs
  if(!ch && mModel) 
    mModel->Modified(false);
  if(!ch && mView) 
    mView->Modified(false);
}


// forward highlite requests
void VioWidget::Highlite(const VioElement& elem, bool on) {;
  FD_DQT("VioWidget::Highlite(): call model slot");
  if(mModel) mModel->Highlite(elem,on);
}

// forward highlite requests
void VioWidget::HighliteClear(void) {;
  FD_DQT("VioWidget::HighliteClear(): call model slot");
  if(mModel) mModel->HighliteClear();
}

// forward show requests
void VioWidget::Show(const VioElement& elem) {;
  FD_DQT("VioWidget::Show(): call model slot");
  if(mModel) mModel->Show(elem);
}

// forward std slots
void VioWidget::Cut(void) { mView->Cut(); };
void VioWidget::Copy(void) { mView->Copy(); };
void VioWidget::Paste(void) { mView->Paste(); };
void VioWidget::Undo(void) { mView->Undo(); };
void VioWidget::Redo(void) { mView->Redo(); };

