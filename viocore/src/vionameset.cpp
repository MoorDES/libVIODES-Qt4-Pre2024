/* vionameset.cpp  - vio nameset model and widget */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2010  Thomas Moor, 

*/


#include "vionameset.h"
#include "lionameset.h"

/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioNameSetData

****************************************************************
****************************************************************
****************************************************************
*/

// construct
VioNameSetData::VioNameSetData(QObject* parent) : VioData(parent) {
};
 
// destruct
VioNameSetData::~VioNameSetData(void) {
};

// clear
void VioNameSetData::Clear(void) {
  // clear my stuff
  mList.clear();
  // call base
  VioData::Clear();
};


// token io: write
void VioNameSetData::DoWrite(faudes::TokenWriter& rTw, const QString& ftype) const {
  FD_DQN("VioNameSetData::DoWrite()");
  // bail out on missing faudes object
  if(!mFaudesObject) return;
  // begin tag with faudes type
  faudes::Token btag;
  std::string section="NameSet";
  if(ftype!="") section=VioStyle::StrFromQStr(ftype);
  btag.SetBegin(section);
  if(mFaudesObject->Name()!="") 
    btag.InsAttributeString("name",mFaudesObject->Name());
  rTw << btag; 
  // my core
  DoWriteCore(rTw,VioStyle::QStrFromStr(section));
  // end of section
  btag.SetEnd(section);
  rTw << btag;
  // done
  FD_DQN("VioNameSetData::DoWrite()");
}

// token io: write
void VioNameSetData::DoWriteCore(faudes::TokenWriter& rTw, const QString& ftype) const {
  FD_DQN("VioNameSetData::DoWriteCore()");
  // bail out on missing faudes object
  faudes::NameSet* nset=dynamic_cast<faudes::NameSet*>(mFaudesObject);
  if(!nset) return;
  // elements from list
  for(int i=0; i<mList.size(); i++) {
    std::string elem=VioStyle::StrFromQStr(mList.at(i));
    if(!nset->Exists(elem)) continue;
    faudes::Idx eidx=nset->Index(elem);
    const faudes::AttributeVoid& attr = nset->Attribute(eidx);
    if(attr.IsDefault()) {
      faudes::Token etag;
      etag.SetEmpty("E");
      etag.InsAttributeString("name",elem);
      rTw << etag;
    } else {
      faudes::Token etag;
      etag.SetBegin("E");
      etag.InsAttributeString("name",elem);
      rTw << etag;
      attr.Write(rTw);
      etag.SetEnd("E");
      rTw << etag;
    }
  }
  // done
  FD_DQN("VioNameSetData::DoWrite()");
}



// token io: read
void VioNameSetData::DoRead(faudes::TokenReader& rTr, const QString& ftype) {
  FD_DQN("VioNameSetData::DoRead()");
  // bail out on missing faudes object
  if(!mFaudesObject) return;
  // figure my section
  std::string section="NameSet";
  if(ftype!="") section=VioStyle::StrFromQStr(ftype);
  // read begin
  faudes::Token btag;
  rTr.ReadBegin(section,btag);
  // set name
  if(btag.ExistsAttributeString("name"))
    mFaudesObject->Name(btag.AttributeStringValue("name"));
  // iterate section
  DoReadCore(rTr,VioStyle::QStrFromStr(section));
  // read end
  rTr.ReadEnd(section);
  FD_DQN("VioNameSetData::DoRead(): done");
}  


// token io: read
void VioNameSetData::DoReadCore(faudes::TokenReader& rTr, const QString& section) {
  FD_DQN("VioNameSetData::DoReadCore()");
  // bail out on missing faudes object
  faudes::NameSet* nset=dynamic_cast<faudes::NameSet*>(mFaudesObject);
  if(!nset) return;
  // iterate section
  while(!rTr.Eos(VioStyle::StrFromQStr(section))) {
    faudes::Token etag;
    rTr.Peek(etag);
    // skip non sections
    if(!etag.IsBegin()) { rTr.Get(etag); continue;}
    // skip unknown sections 
    std::string esec=etag.StringValue();
    if(esec!="E") { rTr.ReadBegin(esec); rTr.ReadEnd(esec); continue; }
    // interpret element
    rTr.Get(etag);
    std::string elem;
    // insist in name
    if(!etag.ExistsAttributeString("name")) {
      std::stringstream errstr;
      errstr << "Element tag must have a name attribute at " << rTr.FileLine();
      throw faudes::Exception("VioNameSetData::DoReadCore", errstr.str(), 50);
    }
    elem=etag.AttributeStringValue("name");
    // insist in no doublets
    if(nset->Exists(elem)) {
      std::stringstream errstr;
      errstr << "Elements must be unique at " << rTr.FileLine();
      throw faudes::Exception("VioNameSetData::DoReadCore", errstr.str(), 50);
    }
    // insert element
    faudes::Idx eidx= nset->Insert(elem);
    mList.append(VioStyle::QStrFromStr(elem));
    // attribute: 
    faudes::AttributeVoid* attrp = nset->AttributeType()->New();
    attrp->Read(rTr);
    nset->Attribute(eidx,*attrp);
    delete attrp;
    // end element
    rTr.ReadEnd("E");
  }
}



// conversion 
QMimeData* VioNameSetData::ToMime(void) {
  FD_DQN("VioNameSetData::ToMime()");
  // use tokenized name set data exchange
  faudes::TokenWriter rTw(faudes::TokenWriter::String);
  rTw.Endl(false);
  Write(rTw);
  // return as mime text
  QMimeData* mdat= new QMimeData();
  mdat->setText(rTw.Str().c_str());
  FD_DQN("VioNameSetData::ToMime(): done");
  return mdat;
}

// conversion (0 on success)
int VioNameSetData::FromMime(const QMimeData* pMime) {
  FD_DQN("VioNameSetData::FromMimeData()");
  Clear();
  int res=0;
  // convert to std string (can we avoid the copy somehow??)
  std::string tstr=pMime->text().toAscii().constData();
  // convert to token stream
  faudes::TokenReader rTr(faudes::TokenReader::String, tstr);
  // read nameset data from token stream
  try {
    Read(rTr);
  } catch(faudes::Exception& exception) {
    Clear();
    res=1;
  }
  // try any alphabet
  //  ... todo ...
  return res;
}

// conversion (0 on success)
int VioNameSetData::TestMime(const QMimeData* pMime) {
  (void) pMime;
  return 1;
}


/* 
 ******************************************
 ******************************************
 ******************************************

 some helper for sorting lists

 ******************************************
 ******************************************
 ******************************************
 */


class VioStringOrder {
public:
  static bool mLessThan(const QString& elem1, const QString& elem2) {
    return VioStyle::SortName(elem1) < VioStyle::SortName(elem2);}
  static bool mGreaterThan(const QString& elem1, const QString& elem2) {
    return VioStyle::SortName(elem1) > VioStyle::SortName(elem2);}
};



/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioNameSetModel

****************************************************************
****************************************************************
****************************************************************
*/


// debugging assistant ... haha 
#define DUMP  for(int i=0; i< mList.size(); i++) { \
  FD_DQN("VioNameSetModel::DUMP(): " << VioStyle::StrFromQStr(mList.at(i)));


// construct
VioNameSetModel::VioNameSetModel(QObject* parent, VioStyle* config, bool alloc) : 
  VioModel(parent, config, false),
  mpFaudesNameSet(0),
  pNameSetStyle(0),
  mUserLayout(0)
{
  FD_DQN("VioNameSetModel::VioNameSetModel(): " << VioStyle::StrFromQStr(mFaudesType));
  // have typed style
  pNameSetStyle=dynamic_cast<VioNameSetStyle*>(pConfig);
  if(!pNameSetStyle) {
    FD_WARN("VioAttributModel::VioNameSetModel(): invalid style, using default.");
    pNameSetStyle= new VioNameSetStyle(mFaudesType);
  }
  // allocate faudes object and have default rep data
  if(alloc) {
    DoVioAllocate();
    DoVioUpdate();
  }
  // have a uniform notification
  connect(this,SIGNAL(NotifyChange(void)),this,SIGNAL(NotifyAnyChange(void)));
  connect(this,SIGNAL(NotifySymbolChange(QString)),this,SIGNAL(NotifyAnyChange(void)));
  FD_DQN("VioNameSetModel::VioNameSetModel(): done");
}

// destruct
VioNameSetModel::~VioNameSetModel(void) {
  // todo: delete style if default fall back (other VioModels have this issue too)
}

// construct on heap
VioNameSetModel* VioNameSetModel::NewModel(QObject* parent) const {
  FD_DQN("VioNameSetModel::NewModel(): type " << VioStyle::StrFromQStr(mFaudesType));
  return new VioNameSetModel(parent,pConfig);
}

// construct on data heap
VioData* VioNameSetModel::NewData(QObject* parent) const {
  FD_DQN("VioNameSetModel::NewData(): type " << VioStyle::StrFromQStr(mFaudesType));
  VioNameSetData* vdat=new VioNameSetData(parent);
  vdat->FaudesObject(mData->FaudesObject()->New());
  return vdat;
}

// construct view on heap
VioView* VioNameSetModel::NewView(QWidget* parent) const {
  FD_DQN("VioNameSetModel::NewView(): " << VioStyle::StrFromQStr(FaudesType()));
  return new VioNameSetView(parent, pConfig);
}

// construct view on heap
VioView* VioNameSetModel::NewPropertyView(QWidget* parent) const {
  FD_DQN("VioNameSetModel::NewPropertyView(): " << VioStyle::StrFromQStr(FaudesType()));
  return new VioNameSetPropertyView(parent, pConfig);
}

// construct on heap
VioWidget* VioNameSetModel::NewWidget(QWidget* parent) const {
  FD_DQN("VioNameSetMode::NewWidget(): type " << VioStyle::StrFromQStr(FaudesType()));
  return new VioNameSetWidget(parent, pConfig);
}


// allocate faudes object
void VioNameSetModel::DoFaudesAllocate(void) {
  FD_DQN("VioNameSetModel::DoFaudesAllocate()");
  // let base handle this
  VioModel::DoFaudesAllocate();
  // impose my requirements
  if(DoTypeCheck(mData->FaudesObject())) {
    FD_DQN("VioNameSetMtcStateModel::DoFaudesAllocate(): fallback ctype");
    mData->FaudesObject(new faudes::NameSet());  
  }
}

// test whether we can host this faudes object
int VioNameSetModel::DoTypeCheck(const faudes::Type* fobject) const {  
  // we host anything that casts to NameSet
  if(dynamic_cast<const faudes::NameSet*>(fobject)) return 0;
  return 1; 
}

// allocate visual model data
void VioNameSetModel::DoVioAllocate(void) {
  FD_DQN("VioNameSetModel::DoVioAllocate()");
  // prepare data container
  mpNameSetData= new VioNameSetData(this);
  mData=mpNameSetData;
  // let base allocate faudes object
  VioModel::DoVioAllocate();
  // dont have anything to allocat, just clear my list
  mpNameSetData->mList.clear();
  mRowMap.clear();
  // and have a layout
  mUserLayout = new VioNameSetLayout(this);
  if(pNameSetStyle->mLayoutFlags & VioNameSetStyle::Decorate)
    mUserLayout->mPropBuiltIn=true;
}

// update visual data from (new) faudes object
void VioNameSetModel::DoVioUpdate(void) {
  FD_DQN("VioNameSetModel::DoVioUpdate()");
  // have typed reference
  mpFaudesNameSet = dynamic_cast<faudes::NameSet*>(mData->FaudesObject());
  FD_DQ("VioNameSetModel::DoVioUpdate(): " << mpFaudesNameSet->ToString());
  // fix internal data
  DoFixList();
  // update
#ifdef FAUDES_DEBUG_VIO_WIDGETS
  // call base (incl set text and notify)
  VioModel::DoVioUpdate();
#else
  // we notify
  emit VioModel::NotifyAnyChange();
#endif
}

// update set from internal list model (dont emit signal)
void VioNameSetModel::DoFaudesUpdate(void) {
  FD_DQN("VioNameSetModel::DoFaudesUpdate()");
  // apply to faudes
  mpFaudesNameSet->Clear();
  foreach(const QString& name,mpNameSetData->mList) {
    mpFaudesNameSet->Insert(VioStyle::StrFromQStr(name));
  }
  mpFaudesNameSet->Name("NameSet");
#ifdef FAUDES_DEBUG_VIO_WIDGETS
  // update debugging stuff
  //VioModel::DoVioUpdate(); not functional!
#endif
}


// token io: implementation
void VioNameSetModel::DoVioWrite(faudes::TokenWriter& rTw) const {
  FD_DQN("VioNameSetModel::DoVioWrite()");
  // set up byte array streams
  QByteArray buff1;
  QDataStream out(&buff1,QIODevice::WriteOnly);
  // stream my list
  out << (qint32) mpNameSetData->mList.size();
  for(int i=0; i< mpNameSetData->mList.size(); i++) {
    out << mpNameSetData->mList.at(i);
  }
  // token io
  rTw.WriteBegin("VioData");
  rTw.WriteBinary(buff1.constData(),buff1.size());
  rTw.WriteEnd("VioData");
  // write layout
  mUserLayout->Write(rTw);
}

// token io: implementation 
void VioNameSetModel::DoVioRead(faudes::TokenReader& rTr) {
  FD_DQN("VioNameSetModel::DoVioRead(): for #" << mpFaudesNameSet->Size() << " faudes symbols");
  // clear 
  mpNameSetData->mList.clear();
  // read tokens
  QByteArray buff1;
  rTr.ReadBegin("VioData");
  std::string rstr;
  rTr.ReadBinary(rstr);
  buff1 = QByteArray::fromRawData(rstr.data(),rstr.size());
  // reconstruct list
  QDataStream in(&buff1,QIODevice::ReadOnly);
  qint32 len;
  QString name;
  in >> len;
  for(int i=0; i<len; i++) {
    in >> name;
    mpNameSetData->mList.append(name);
  }
  // read end token AFTER processing stream
  rTr.ReadEnd("VioData");
  // fix my data structures
  DoFixRowMap();
  DoFixList();
  // read layout
  mUserLayout->Read(rTr);
  // report
  FD_DQN("VioNameSetModel::DoVioRead(): found #" << mpNameSetData->mList.size() << " vio symbols");
}


// set/get default layout
const VioNameSetLayout& VioNameSetModel::Layout(void) { return *mUserLayout; };
void VioNameSetModel::Layout(const VioNameSetLayout& layout) { *mUserLayout=layout; };
 
// edit: clear all
void VioNameSetModel::Clear(void) {
  mpNameSetData->mList.clear();
  mRowMap.clear();
  mpFaudesNameSet->Clear();
  emit VioModel::NotifyAnyChange();
}

// edit: get size 
int VioNameSetModel::Size(void) const {
  return mpNameSetData->mList.size();
}


// edit: imndex by symbol
faudes::Idx VioNameSetModel::Index(const QString& name) const {
  return mpFaudesNameSet->Index(VioStyle::StrFromQStr(name));
}

// edit: element by symbol
VioElement VioNameSetModel::Element(const QString& name) const {
  return VioElement::FromEvent(Index(name));
}


// edit: symbol by name
QString VioNameSetModel::SymbolicName(faudes::Idx idx) const {
  return VioStyle::QStrFromStr(mpFaudesNameSet->SymbolicName(idx));
};

// edit: get by position
const QString& VioNameSetModel::At(int pos) const {
  static QString empty;
  if(pos<0 || pos >= mpNameSetData->mList.size()) return empty;
  return mpNameSetData->mList.at(pos);
}

// edit: set by position
bool VioNameSetModel::At(int pos, const QString& name) {
  if(mRowMap.contains(name)) return false;
  if(pos<0 || pos >= mpNameSetData->mList.size()) return false;
  const QString& old=mpNameSetData->mList.at(pos);
  VioElement oelem=Element(old);
  bool sel = IsSelected(oelem);
  mRowMap.remove(old);
  mpFaudesNameSet->Erase(VioStyle::StrFromQStr(old));
  mpNameSetData->mList[pos]=name;
  mRowMap[name]=pos;
  mpFaudesNameSet->Insert(VioStyle::StrFromQStr(name));
  Modified(true);
  if(sel) Select(oelem,false);
  emit NotifySymbolChange(name);
  return true;
}

// edit: rename
bool VioNameSetModel::ReName(const QString& oldname, const QString& newname) {
  int pos=IndexOf(oldname);
  if(pos<0) return false;
  return At(pos,newname);
}

// edit: apend 
bool VioNameSetModel::Append(const QString& name) {
  return Insert(mpNameSetData->mList.size(),name);
}

// edit: insert
bool VioNameSetModel::Insert(int pos, const QString& name) {
  if(mRowMap.contains(name)) return false;
  if(pos<0) return false;
  if(pos > mpNameSetData->mList.size()) return false;
  QMap<QString,int>::iterator lit;
  for(lit=mRowMap.begin(); lit!=mRowMap.end();lit++) {
    if(lit.value()>=pos) lit.value()++;
  }
  mpNameSetData->mList.insert(pos,name);
  mRowMap[name]=pos;
  mpFaudesNameSet->Insert(VioStyle::StrFromQStr(name));
  Modified(true);
  emit NotifySymbolChange(name);
  return true;
}

// edit: remove
bool VioNameSetModel::Remove(const QString& name) {
  int pos=IndexOf(name);
  return RemoveAt(pos);
}

// edit: remove
bool  VioNameSetModel::RemoveAt(int pos) {
  if(pos<0 || pos >= mpNameSetData->mList.size()) return false;
  const QString& name=mpNameSetData->mList.at(pos);
  VioElement elem=Element(name);
  bool sel = IsSelected(elem);
  mRowMap.remove(name);
  QMap<QString,int>::iterator lit;
  for(lit=mRowMap.begin(); lit!=mRowMap.end();lit++) {
    if(lit.value()>=pos) lit.value()--;
  }
  mpNameSetData->mList.removeAt(pos);
  mpFaudesNameSet->Erase(VioStyle::StrFromQStr(name));
  Modified(true);
  if(sel) Select(elem,false);
  emit NotifySymbolChange(name);
  return true;
}

// edit move
bool VioNameSetModel::Move(int from, int to) {
  if(from<0 || from >= mpNameSetData->mList.size()) return false;
  if(to<0 || to >= mpNameSetData->mList.size()) return false;
  mpNameSetData->mList.move(from,to);
  for(int i=from; i<to; i++) 
    mRowMap[mpNameSetData->mList.at(i)]=i;
  for(int i=to; i<from; i++) 
    mRowMap[mpNameSetData->mList.at(i)]=i;
  Modified(true);
  emit NotifyChange();
  return true;
}

// edit: find
int VioNameSetModel::IndexOf(const QString& name) const {
  QMap<QString,int>::const_iterator lit;
  lit=mRowMap.constFind(name);
  if(lit==mRowMap.end()) return -1;
  return lit.value();
}
  
// edit: contains
bool VioNameSetModel::Exists(const QString& name) const {
  QMap<QString,int>::const_iterator lit;
  lit=mRowMap.constFind(name);
  if(lit==mRowMap.end()) return false;
  return true;
}

// edit: new symbol 
QString VioNameSetModel::UniqueSymbol(const QString& name) {
  std::string usym=VioStyle::StrFromQStr(name);
  if(usym=="") usym=VioStyle::StrFromQStr(NameSetConfiguration()->mDefSymbol);
  usym=mpFaudesNameSet->SymbolTablep()->UniqueSymbol(usym);
  return VioStyle::QStrFromStr(usym);
}


// sorting
void VioNameSetModel::SortAscending(void) {
  FD_DQN("VioNameSetModel::SortAscendingEv()");
  qStableSort(mpNameSetData->mList.begin(), mpNameSetData->mList.end(), VioStringOrder::mLessThan);
  DoFixRowMap();
  emit NotifyChange();
  FD_DQN("VioNameSetModel::SortAscendingEv(): done");
}

// sorting
void VioNameSetModel::SortDescending(void) {
  FD_DQN("VioNameSetModel::SortDescending()");
  qStableSort(mpNameSetData->mList.begin(), mpNameSetData->mList.end(), VioStringOrder::mGreaterThan);
  DoFixRowMap();
  emit NotifyChange();
  FD_DQN("VioNameSetModel::SortDescending(): done");
}


// edit: set attribute 
bool VioNameSetModel::Attribute(const QString& name, const faudes::AttributeVoid& attr) {
  FD_DQN("VioNameSetModel::Attribute("<< VioStyle::StrFromQStr(name) << ", " << attr.ToString() << " type " << typeid(&attr).name()<< ")");
  // if elem non existent, do nothing
  if(!Exists(name)) return false;
  // bail on on identical
  if(Attribute(name).Equal(attr)) return false;  
  // figure and set attribute
  try {
    faudes::Idx idx=mpFaudesNameSet->Index(VioStyle::StrFromQStr(name));
    mpFaudesNameSet->Attribute(idx,attr);
  } catch(faudes::Exception& exception) {
  } 
  FD_DQN("VioNameSetModel::Attribute(...): done");
  Modified(true);
  emit NotifySymbolChange(name);
  return true;
}

// edit: test attribute (true for equal) 
bool VioNameSetModel::AttributeTest(const QString& name, const faudes::AttributeVoid& attr) {
  FD_DQN("VioNameSetModel::AttributeTest("<< VioStyle::StrFromQStr(name) << ", " << attr.ToString() << " type " << typeid(&attr).name()<< ")");
  // if elem non existent, do nothing
  if(!Exists(name)) return false;
  // ask libfaudes
  return Attribute(name).Equal(attr);  
}

// edit: query attribute
const faudes::AttributeFlags& VioNameSetModel::Attribute(const QString& name) const {
  // default result
  static const faudes::AttributeFlags defres;
  // figure attribute
  const faudes::AttributeVoid* attr=0; 
  if(Exists(name)) {
    faudes::Idx idx=mpFaudesNameSet->Index(VioStyle::StrFromQStr(name));
    try {
      attr = &mpFaudesNameSet->Attribute(idx);
    } catch(faudes::Exception& exception) {
    } 
  }
  // try to cast
  const faudes::AttributeFlags* fattr=dynamic_cast<const faudes::AttributeFlags*>(attr);
  // use default on failure
  if(!fattr) fattr = &defres;
  // done
  FD_DQN("VioNameSetModel::Attribute(\""<< VioStyle::StrFromQStr(name) << "\") : " << fattr->ToString() << " type " << typeid(*fattr).name());
  return *fattr;
}



// query boolean properts definitions
const QList<VioBooleanProperty>& VioNameSetModel::BooleanProperties(void) const {
  return *NameSetConfiguration()->mAttribute->BooleanProperties();
}

// query boolean property
bool VioNameSetModel::BooleanProperty(const QString& name,int prop) const {
  const faudes::AttributeFlags& attr=Attribute(name);
  FD_DQN("VioNameSetModel::BooleanProperty("<< VioStyle::StrFromQStr(name) << "): flags " << faudes::ToStringInteger16(attr.mFlags));
  const QList<VioBooleanProperty>& props= BooleanProperties();
  if(prop<0 || prop>=props.size()) return false;
  bool bp =  props.at(prop).Test(attr.mFlags);
  FD_DQN("VioNameSetModel::BooleanProperty("<< VioStyle::StrFromQStr(name) << ", " << prop << "): val " << bp);
  return bp;
}

// edit boolean property
void VioNameSetModel::BooleanProperty(const QString& name, int prop, bool val) {
  const QList<VioBooleanProperty>& props= BooleanProperties();
  if(prop<0 || prop>=props.size()) return;
  faudes::AttributeFlags* attr=Attribute(name).Copy();
  if(val) props.at(prop).Set(attr->mFlags);
  else props.at(prop).Clr(attr->mFlags);
  Attribute(name,*attr);
  delete attr;
}


// fix internal data
void  VioNameSetModel::DoFixRowMap(void) {
  mRowMap.clear();
  for(int count=0; count < mpNameSetData->mList.size(); count++)
    mRowMap[mpNameSetData->mList[count]]=count; 
}


// fix internal data
void  VioNameSetModel::DoFixList(void) {
  // carefull upate: remove items from my list, that are not in the faudes set
  bool nf=false;
  for(int i=0; i<mpNameSetData->mList.size(); i++) {
    if(mpFaudesNameSet->Exists(VioStyle::StrFromQStr(mpNameSetData->mList.at(i)))) continue;
    mpNameSetData->mList.removeAt(i);
    nf=true;
    i--;
  }  
  if(nf) DoFixRowMap();
  // carefull update: append items, that not in my list
  faudes::NameSet::Iterator nit=mpFaudesNameSet->Begin();
  faudes::NameSet::Iterator nit_end=mpFaudesNameSet->End();
  for(;nit!=nit_end;nit++) {
    QString name=VioStyle::QStrFromStr(mpFaudesNameSet->SymbolicName(*nit));
    if(Exists(name)) continue;
    mRowMap[name]=mpNameSetData->mList.size();
    mpNameSetData->mList.append(name);
  }
}


// select (only existing items)
void VioNameSetModel::Select(const VioElement& elem, bool on) {
  // ignore all but events
  if(!elem.IsEvent() && on) return;
  // ignore nonexistent
  if(!mpFaudesNameSet->Exists(elem.Event()) && on) return;
  // let base select
  VioModel::Select(elem,on);
}

// get all data
VioData* VioNameSetModel::Data(void) {
  FD_DQN("VioNameSetModel::Data(): get");
  VioNameSetData* ndat= new VioNameSetData();
  // copy faudes object
  ndat->FaudesObject(mpFaudesNameSet->Copy());
  // get all data
  ndat->mList=mpNameSetData->mList;
  // done
  return ndat;
}


// get selection data
VioData* VioNameSetModel::SelectionData(void) {
  FD_DQN("VioNameSetModel::SelectionData()");
  VioNameSetData* ndat= new VioNameSetData();
  // copy selected portion of faudes set
  faudes::NameSet* nset = mpFaudesNameSet->New();
  foreach(VioElement elem, mSelection) {
    if(!elem.IsEvent()) continue;
    if(nset->SymbolicName(elem.Event())=="") continue;
    nset->Insert(elem.Event());
    nset->Attribute(elem.Event(), mpFaudesNameSet->Attribute(elem.Event())); 
  }  
  ndat->FaudesObject(nset);
  // get all data
  ndat->mList=mpNameSetData->mList;
  FD_DQN("VioNameSetModel::SelectionData(): #" << nset->Size());
  return ndat;
}

// test data (return 0 on accept)
int VioNameSetModel::TypeCheckData(const VioData* pData) {
  FD_DQN("VioNameSetModel::TypeCheckData(): strict");
  const VioNameSetData* ndat= qobject_cast<const VioNameSetData*>(pData);
  const faudes::NameSet* nset = dynamic_cast<const faudes::NameSet*>(pData->FaudesObject());
  if(nset && ndat) { 
    FD_DQN("VioNameSetModel::TypeCheckData(): ok #"  << nset->Size());
    return 0;
  }
  return 1;
}


// merge data (return 1 on changes, assume type ok, select new items)
int VioNameSetModel::DoMergeData(const VioData* pData) {
  FD_DQN("VioNameSetModel::DoMergeData()");
  const VioNameSetData* ndat= qobject_cast<const VioNameSetData*>(pData);
  const faudes::NameSet* nset = dynamic_cast<const faudes::NameSet*>(pData->FaudesObject());
  // track changes
  bool changed=false;
  VioElement selelem;
  // insert events to faudes object
  faudes::NameSet::Iterator eit=nset->Begin();
  for(;eit!=nset->End(); eit++) {
    mpFaudesNameSet->Insert(*eit);
    mpFaudesNameSet->Attribute(*eit, nset->Attribute(*eit));
    if(selelem.Type()==VioElement::EEvent || selelem.Type()==VioElement::EVoid) {
      selelem=VioElement::FromEvent(*eit);
      mSelection.append(selelem);
    }
    changed=true;
  }
  // insert events to vio list
  for(int i=0; i<ndat->mList.size(); i++) {
    std::string fname=VioStyle::StrFromQStr(ndat->mList.at(i));
    if(!nset->Exists(fname)) continue;
    if(mpNameSetData->mList.contains(ndat->mList.at(i))) continue;
    mpNameSetData->mList.append(ndat->mList.at(i));
    changed=true;
  }
  // fix me
  if(changed) {
    DoFixRowMap();
    DoFixList();
  }
  FD_DQN("VioNameSetModel::DoMargeData(): insert abstract model data #"  << ndat->mList.size());  
 return changed;
}


// vio data access: set (0 on accept)
int VioNameSetModel::Data(const VioData* pData) {
  FD_DQN("VioNameSetModel::Data(): set all data");
  // reject wrong data type
  if(TypeCheckData(pData)!=0) return 1;
  // record selected singleton
  QString selname="";
  if(Selection().size()==1) {
    selname = SymbolicName(Selection().at(0).Event());
    FD_DQN("VioNameSetModel::Data(): recover selection " << VioStyle::StrFromQStr(selname));
  }
  // clear and merge
  Clear(); 
  bool changed=DoMergeData(pData);
  if(changed) {
    //Modified(true);
    emit NotifyAnyChange();       // universal update
  }
  // reconstruct selection if possible
  mSelection.clear();
  if(Exists(selname)) {  
    faudes::Idx selidx=IndexOf(selname);
    mSelection.append(VioElement::FromEvent(selidx));
  }
  emit NotifySelectionChange();    // universal selection change
  FD_DQN("VioNameSetModel::Data(): done sel #" << Selection().size());
  return 0;
};


// insert data (return 0 on accept)
int VioNameSetModel::InsertData(const VioData* pData) {
  FD_DQN("VioNameSetModel::InsertData(): test types");
  if(TypeCheckData(pData)!=0) return 1;
  // do the insert (incl selection)
  bool changed=DoMergeData(pData);
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
void VioNameSetModel::DeleteSelection(void) {
  FD_DQN("VioNameSetModel::DeleteSelection()");
  foreach(VioElement elem, mSelection) {
    if(!elem.IsEvent()) continue;
    Remove(VioStyle::QStrFromStr(mpFaudesNameSet->SymbolicName(elem.Event())));
  }
  SelectionClear(); // should we keep an selection pint for insertion?
  FD_DQN("VioNameSetModel::DeleteSelection(): done ");
}


/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioNameSetLayout

****************************************************************
****************************************************************
****************************************************************
*/

// defaults
VioNameSetLayout::VioNameSetLayout(QObject* parent) : QObject(parent) {
  mPropBuiltIn=false;
  mPropSize=150;
  mListSize=150;
}

// assignment
VioNameSetLayout& VioNameSetLayout::operator=(const VioNameSetLayout& rSrc) {
  mPropBuiltIn=rSrc.mPropBuiltIn;
  mPropSize=rSrc.mPropSize;
  mListSize=rSrc.mListSize;
  return *this;
}  

// write layout
void VioNameSetLayout::Write(faudes::TokenWriter& rTw) const {
  rTw.WriteBegin("VioLayout");
  rTw.WriteInteger(mPropBuiltIn);
  rTw.WriteInteger(mPropSize);
  rTw.WriteInteger(mListSize);
  rTw.WriteEnd("VioLayout");
}

// read layout 
void VioNameSetLayout::Read(faudes::TokenReader& rTr) {
  try {
    rTr.ReadBegin("VioLayout");
    mPropBuiltIn= rTr.ReadInteger();
    mPropSize= rTr.ReadInteger();
    mListSize= rTr.ReadInteger();
    rTr.ReadEnd("VioLayout");
  } catch(faudes::Exception expection) {
  } 
}

/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioNameSetView

****************************************************************
****************************************************************
****************************************************************
*/

// construct
VioNameSetView::VioNameSetView(QWidget* parent, VioStyle* config, bool alloc) : 
  VioView(parent,config,false),
  pNameSetModel(0),
  mUserLayout(0),
  mListModel(0),
  mSplitter(0),
  mListView(0),
  mPropView(0),
  mPropAction(0)
{
  FD_DQN("VioNameSetView::VioNameSetView(): " << VioStyle::StrFromQStr(FaudesType()));
  // have typed style
  pNameSetStyle=dynamic_cast<VioNameSetStyle*>(pConfig);
  if(!pNameSetStyle) {
    FD_WARN("VioAttributView::VioNameSetModel(): invalid style, using default.");
    pNameSetStyle= new VioNameSetStyle(mFaudesType);
  }
  // my alloc
  if(alloc) DoVioAllocate();
  FD_DQN("VioNameSetView::VioNameSetView(): done");
}

// destruct
VioNameSetView::~VioNameSetView(void) {
  FD_DQN("VioNameSetView::~VioNameSetView()");
}

// typed faudes object access
const faudes::NameSet* VioNameSetView::NameSet(void) const {
  if(!pNameSetModel) return 0;
  return pNameSetModel->NameSet();
}

// allocate my data
void VioNameSetView::DoVioAllocate(void) {
  FD_DQN("VioNameSetView::DoVioAllocate(): create layout");
  // allocate base
  VioView::DoVioAllocate();
  // my widgets
  mListView = new LioNameSetView();
  if(pNameSetStyle->mLayoutFlags & VioNameSetStyle::Properties) 
    mPropView= new VioNameSetPropertyView(0,pNameSetStyle);
  // std splitter
  if(~(pNameSetStyle->mLayoutFlags & VioNameSetStyle::Decorate)) {
    mSplitter=new QSplitter();
    mSplitter->addWidget(mListView);
    if(mPropView) mSplitter->addWidget(mPropView);
    mVbox->addWidget(mSplitter);
  }
  // decorated splitter
  if(pNameSetStyle->mLayoutFlags & VioNameSetStyle::Decorate) {
    QGroupBox* gb1 = new QGroupBox(pNameSetStyle->mHeader);
    QVBoxLayout* vb1 = new QVBoxLayout(gb1);
    vb1->setMargin(0);
    vb1->addWidget(mListView);
    mSplitter->addWidget(gb1);
    if(mPropView) {
      QGroupBox* gb2 = new QGroupBox("Properties");
      QVBoxLayout* vb2 = new QVBoxLayout(gb2);
      vb2->setMargin(0);
      vb2->addWidget(mPropView);
      mSplitter->addWidget(gb2);
    }
  }
  // splitter orientation
  if(pNameSetStyle->mLayoutFlags & VioNameSetStyle::PropH) 
    mSplitter->setOrientation(Qt::Horizontal);
  if(pNameSetStyle->mLayoutFlags & VioNameSetStyle::PropV) 
    mSplitter->setOrientation(Qt::Vertical);
  // property action
  if(mPropView) {
    mPropAction=new QAction("&Properties",this);
    mPropAction->setStatusTip("show/hide property view");
    mPropAction->setCheckable(true);
    mPropAction->setEnabled(true);
    mPropAction->setChecked(!mPropView->isHidden());
    mViewActions.append(mPropAction); 
    connect(mPropAction, SIGNAL(triggered(bool)), this, SLOT(ShowPropertyView(bool)));
  }
  // connect
  connect(this, SIGNAL(NotifyShow(const VioElement&)), mListView, 
	  SLOT(Show(const VioElement&)));
  // have a user layout
  mUserLayout= new VioNameSetLayout(this);
 // debugging widget
#ifdef FAUDES_DEBUG_VIO_WIDGETS
  mTextInfo->setText(QString("NameSet \"%1\"").arg(FaudesType()));
  mTextEdit->setReadOnly(true);
  mApplyButton->hide();
#endif
  FD_DQN("VioNameSetView::DoVioAllocate(): done");
}

// update view from (new) model
void VioNameSetView::DoVioUpdate(void) {
  FD_DQN("VioNameSetView::DoVioUpdate(): type " << VioStyle::StrFromQStr(FaudesType()));
  // disconnect 
  /*
  if(pNameSetModel) {
    disconnect(pNameSetModel, 0 , mListView, SLOT(UpdateSelectionAny(void)));
  }
  */
  // bail out on void
  if(!pModel) return; 
  // have typed model reference (incl set to 0)
  pNameSetModel=qobject_cast<VioNameSetModel*>(pModel);
  // bail out on no model
  if(!pNameSetModel) return;
  // have my item model
  if(mListModel) mListModel->deleteLater();
  mListModel =new LioNameSetModel(pNameSetModel);
  mListView->setModel(mListModel);
  // set propertyview
  if(mPropView) mPropView->Model(pNameSetModel);
#ifdef FAUDES_DEBUG_VIO_WIDGETS
  mTextEdit->setPlainText(VioStyle::QStrFromStr(pModel->FaudesObject()->ToText()));
#endif
  // get and set user layout
  *mUserLayout =pNameSetModel->Layout();
  UpdateUserLayout(); 
  // follow selection
  /* NOT FUNCTIONAL --- TODO
  connect(pModel, SIGNAL(NotifySelectionChange(void)), 
    mListView, SLOT(UpdateSelectionAny(void))); 
  */
  FD_DQN("VioNameSetView::DoVioUpdate(): done");
} 

// update model from view
void VioNameSetView::DoModelUpdate(void) {
  FD_DQN("VioNameSetView::DoModelUpdate(): type " << VioStyle::StrFromQStr(FaudesType()));
  // bail out if widget is not ready or model not there
  if(!mListView || !pModel) return;
  // no need: allways in sync?
  SaveUserLayout();
}

// directed data access
int VioNameSetView::InsertData(const VioData* pData) {
  if(!pNameSetModel) return 1; 
  return pNameSetModel->InsertData(pData);
}

// directed data access
VioData* VioNameSetView::SelectionData(void) {
  FD_DQN("VioNameSetView::SelectionData()");
  if(!pNameSetModel) return 0; 
  return pNameSetModel->SelectionData();
}

// directed data access
void VioNameSetView::DeleteSelection(void) {
  if(pNameSetModel) pNameSetModel->DeleteSelection();
}


// show/hide views from layout
void VioNameSetView::UpdateUserLayout(void) {
  FD_DQN("VioNameSetView::UpdateUserLayout()");
  // bail out on invalid
  if(!mSplitter) return;
  if(mSplitter->count()!=2) return;
  // doit
  if(mPropView) {
    mSplitter->widget(1)->setVisible(mUserLayout->mPropBuiltIn);
    mPropAction->setChecked(mUserLayout->mPropBuiltIn);
  }
  // propview size
  int w;
  if(mPropView) {
    QList<int> sz = mSplitter->sizes();
    w = sz[0]+sz[1];
    if(w==0) w = mUserLayout->mPropSize+mUserLayout->mListSize;
    sz[0]= w - mUserLayout->mPropSize;
    if(sz[0]<0) sz[0]=0;
    sz[1]=w-sz[0];
    mSplitter->setSizes(sz);
  }
  // done
  FD_DQN("VioNameSetView::UpdateUserLayout(): done " << w);
}

// retrieve layout
void VioNameSetView::SaveUserLayout(void) {
  FD_DQN("VioNameView::SaveUserLayout()");
  // bail out on invalid
  if(!mSplitter) return;
  if(mSplitter->count()!=2) return;
  QList<int> sz = mSplitter->sizes();
  // show/hide 
  if(mPropView) {
    mUserLayout->mPropBuiltIn=mSplitter->widget(1)->isVisible();
    if(mUserLayout->mPropBuiltIn) {
      mUserLayout->mPropSize=sz[1];
      mUserLayout->mListSize=sz[0];
    }
  }
  // save to model
  if(pNameSetModel) pNameSetModel->Layout(*mUserLayout);
}

// show/hide  abstract view
void::VioNameSetView::ShowPropertyView(bool on) {
  // bail out on invalid
  if(!mPropView) return;
  if(mSplitter->count()!=2) return;
  // get data
  QList<int> sizes = mSplitter->sizes();
  FD_DQN("VioNameSetView::ShowPropertyView(): " << on);
  if(!on && mSplitter->widget(1)->isVisible()) {
    mUserLayout->mListSize=sizes[0];
    mUserLayout->mPropSize=sizes[1];
    mSplitter->widget(1)->setVisible(false);
  } 
  if(on && !mSplitter->widget(1)->isVisible()) {
    QList<int> sizes = mSplitter->sizes();
    sizes[0]= mUserLayout->mListSize;
    sizes[1]= mUserLayout->mPropSize;
    if(sizes[1]==0) sizes[1]=150;
    mSplitter->widget(1)->setVisible(true);
    mSplitter->setSizes(sizes);
  }
  mPropAction->setChecked(on);
  // record layout
  SaveUserLayout();
}
 


/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioNameSetWidget

****************************************************************
****************************************************************
****************************************************************
*/

// construct
VioNameSetWidget::VioNameSetWidget(QWidget* parent, VioStyle* config, bool alloc) : 
  VioWidget(parent,config,false),
  pNameSetModel(0) 
{
  FD_DQN("VioNameSetWidget::VioNameSetWidget(): " << VioStyle::StrFromQStr(pConfig->ConfigName()));
  // allocate model and view
  if(alloc) {
    // have view
    mView= new VioNameSetView(0,pConfig);
    mVbox->addWidget(mView);
    // fix typed refs and connect to widget
    DoVioAllocate();
    // have a model and set
    Model(new VioNameSetModel(this,pConfig));
  }
  // fix modified flag
  mModified=false;
  FD_DQN("VioNameSetWidget::VioNameSetWidget(): done");
}

// destruct
VioNameSetWidget::~VioNameSetWidget(void) {
  FD_DQN("VioNameSetWidget::~VioNameSetWidget()");
}

// fix view
void VioNameSetWidget::DoVioAllocate(void) {
  // connect view
  QObject::connect(mView,SIGNAL(NotifyModified(bool)),this,SLOT(ChildModified(bool)));
  QObject::connect(mView,SIGNAL(MouseClick(const VioElement&)),this,SIGNAL(MouseClick(const VioElement&)));
  QObject::connect(mView,SIGNAL(MouseDoubleClick(const VioElement&)),this,SIGNAL(MouseDoubleClick(const VioElement&)));
}

// set by vio model
int VioNameSetWidget::Model(VioModel* model) {
  FD_DQN("VioNameSetWidget::Model(" << model << "): type " << VioStyle::StrFromQStr(FaudesType()));
  // bail out on identity
  if(model==mModel) return 0;
  // call base to set view and connects
  int res=VioWidget::Model(model);
  // update typed ref
  pNameSetModel=qobject_cast<VioNameSetModel*>(mModel);
  FD_DQN("VioNameSetWidget::Model(" << model << "): done");  
  return res;
}


// typed faudes object access
const faudes::NameSet* VioNameSetWidget::NameSet(void) const {
  return pNameSetModel->NameSet();
}



/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioNameSetPropertyView

****************************************************************
****************************************************************
****************************************************************
*/

// construct 
VioNameSetPropertyView::VioNameSetPropertyView(QWidget* parent, VioStyle* config, bool alloc) : 
  VioView(parent,config,false), 
  pNameSetModel(0),
  mProperties(0) 
{
  FD_DQN("VioNameSetPropertyView::VioNameSetPropertyView(): " << VioStyle::StrFromQStr(FaudesType()));
  if(alloc) DoVioAllocate();
  FD_DQN("VioNameSetPropertyView::VioNameSetPropertyView(): done");
}

// destruct
VioNameSetPropertyView::~VioNameSetPropertyView(void) {
}

// allocate visual items
void VioNameSetPropertyView::DoVioAllocate(void) {
  FD_DQN("VioNameSetPropertyView::DoVioAllocate(): layout for type " << VioStyle::StrFromQStr(FaudesType()));
  // allocate base
  VioView::DoVioAllocate();
  // debugging widget (turn of
#ifdef FAUDES_DEBUG_VIO_WIDGETS
  mTextInfo->setText(tr("Properties"));
  mTextEdit->hide();
  mApplyButton->hide();
#endif
  // my box
  mProperties = new PioNameSetView(0,pConfig);
  //mProperties->setMinimumSize(QSize(100,100));
  //mProperties->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Expanding);
  mVbox->addWidget(mProperties);
  mVbox->addStretch(1);
}


// update widget from (new) model
void VioNameSetPropertyView::DoVioUpdate(void) {
  // bail out on void
  if(!pModel) return; 
  // disconnect 
  if(pModel) {
    disconnect(pModel, 0 , this, SLOT(UpdateSelectionChange(void)));
  }
  // call base
  VioView::DoVioUpdate();
  FD_DQN("VioNameSetPropertyView::DoVioUpdate(): sensed model " << VioStyle::StrFromQStr(FaudesType()));
  pNameSetModel = qobject_cast<VioNameSetModel*>(pModel);
  mProperties->Model(pNameSetModel);
  // follow selection
  connect(pModel, SIGNAL(NotifySelectionChange(void)), 
    this, SLOT(UpdateSelectionChange(void))); 
  disconnect(pModel, 0, this, SLOT(Show(const VioElement&)));
  // now update
  FD_DQN("VioNameSetPropertyView::DoVioUpdate(): update view for type " << VioStyle::StrFromQStr(FaudesType()));
  Show(VioElement());
}


// update model from widget
void VioNameSetPropertyView::DoModelUpdate(void) {
  FD_DQN("VioNameSetPropertyView::DoModelUpdate(): type " << VioStyle::StrFromQStr(FaudesType()));
  // bail out if widget is not ready
  if(!mProperties) return;
  // todo
}


// reimplement  show element
void VioNameSetPropertyView::Show(const VioElement& elem) {
  FD_DQN("VioNameSetPropertyView::Show(): " << elem.Str());
  // bail out on not ready
  if(!mProperties) return;
  // tell attribute widgets
  mProperties->Show(elem);
  FD_DQN("VioNameSetPropertyView::Show(): done " << elem.Str());
}


// reimplement to follow selection
void VioNameSetPropertyView::UpdateSelectionChange(void) {
  // bail out on not ready
  if(!mProperties) return;
  FD_DQN("VioNameSetPropertyView::UpdateSelectionChange()");
}
