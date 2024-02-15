/* liotsemodel.cpp  - list representation of generators */


/*
   Visual IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2011  Thomas Moor

*/

// my header
#include "viogenlist.h"

// resolve forwards
#include "liotselist.h"
#include "liotseview.h"

/* 
 ******************************************
 ******************************************
 ******************************************

 some helper for sorting lists

 ******************************************
 ******************************************
 ******************************************
 */


class VioElementOrder {
public:
 static const faudes::vGenerator* pGen;
 static bool mLessThanX1(const VioElement& elem1, const VioElement& elem2) {
   return VioStyle::SortStateName(pGen,elem1.X1()) < VioStyle::SortStateName(pGen,elem2.X1());}
 static bool mGreaterThanX1(const VioElement& elem1, const VioElement& elem2) {
   return VioStyle::SortStateName(pGen,elem1.X1()) > VioStyle::SortStateName(pGen,elem2.X1());}
 static bool mLessThanEv(const VioElement& elem1, const VioElement& elem2) {
   return VioStyle::SortEventName(pGen,elem1.Ev()) < VioStyle::SortEventName(pGen,elem2.Ev());}
 static bool mGreaterThanEv(const VioElement& elem1, const VioElement& elem2) {
   return VioStyle::SortEventName(pGen,elem1.Ev()) > VioStyle::SortEventName(pGen,elem2.Ev());}
 static bool mLessThanX2(const VioElement& elem1, const VioElement& elem2) {
   return VioStyle::SortStateName(pGen,elem1.X2()) < VioStyle::SortStateName(pGen,elem2.X2());}
 static bool mGreaterThanX2(const VioElement& elem1, const VioElement& elem2) {
   return VioStyle::SortStateName(pGen,elem1.X2()) > VioStyle::SortStateName(pGen,elem2.X2());}
};

const faudes::vGenerator* VioElementOrder::pGen=0;

/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioGeneratorListModel

****************************************************************
****************************************************************
****************************************************************
*/

// construct
VioGeneratorListModel::VioGeneratorListModel(VioGeneratorModel* parent, VioElement::EType etype) : 
  VioGeneratorAbstractModel(parent),
  mTableModel(0)
{
  FD_DQG("VioGeneratorListModel::VioGeneratorListModel() for " << VioElement::TypeStr(etype));
  pVioGeneratorModel=parent;
  mEType=etype;
  switch(mEType) {
  case VioElement::ETrans: mTableModel= new LioTList(this); break;
  case VioElement::EState: mTableModel= new LioSList(this); break;
  case VioElement::EEvent: mTableModel= new LioEList(this); break;
  default: break; //error
  }
  connect(mTableModel,SIGNAL(NotifyModified(bool)),this,SLOT(ChildModified(bool)));
}

// access as q table model
LioVList* VioGeneratorListModel::TabelModel(void) { return mTableModel; };

// create new view for this representationmodel
VioGeneratorAbstractView* VioGeneratorListModel::NewView(VioGeneratorView* parent) {
  FD_DQG("VioGeneratorListModel::NewView()");
  // create
  VioGeneratorAbstractView* view = new VioGeneratorListView(parent);
  // set model (incl connect)
  view->Model(this);
  // done
  return view;
}

// token io: vio data
void VioGeneratorListModel::DoVioWrite(faudes::TokenWriter& rTw) const {
  FD_DQG("VioGeneratorListModel::DoVioWrite()");
  // set up byte array streams
  QByteArray buff1;
  QDataStream out(&buff1,QIODevice::WriteOnly);
  // stream my list
  out << (qint32) mElementList.size();
  for(int i=0; i< mElementList.size(); i++) {
    mElementList.at(i).DoWrite(out,Generator());
    FD_DQG("VioGeneratorListModel::DoVioWrite(): " << mElementList.at(i).Str());
  }
  // token io
  rTw.WriteBegin(VioElement::TypeStr(mEType)+"List");
  rTw.WriteBinary(buff1.constData(),buff1.size());
  rTw.WriteEnd(VioElement::TypeStr(mEType)+"List");
}

// token io: vio data
void VioGeneratorListModel::DoVioRead(faudes::TokenReader& rTr) {
  FD_DQG("VioGeneratorListModel::DoVioRead()");
  // clear
  mElementList.clear(); // ?? this did miss ???
  // read tokens
  QByteArray buff1;
  rTr.ReadBegin(VioElement::TypeStr(mEType)+"List");
  std::string rstr;
  rTr.ReadBinary(rstr);
  buff1 = QByteArray::fromRawData(rstr.data(),rstr.size());
  // read stream: len, list of elements
  QDataStream in(&buff1,QIODevice::ReadOnly);
  qint32 len;
  VioElement elem;
  in >> len;
  FD_DQG("VioGeneratorListModel::DoVioRead(): #" << len << " elements from buffer");
  for(int i=0; i<len; i++) {
    elem.DoRead(in,Generator());
    mElementList.append(elem);
    FD_DQG("VioGeneratorListModel::DoVioRead(): " << elem.Str());
  }
  rTr.ReadEnd(VioElement::TypeStr(mEType)+"List");
  // fix my data structures
  DoFixRowMap();
}

// update notification: insert
void VioGeneratorListModel::UpdateElementIns(const VioElement& elem) { 
  FD_DQG("VioGeneratorListModel::UpdateElementIns(): " << elem.Str());
  // do nothing if tis exists
  if(Contains(elem)) return;
  // default destination: end
  int dest=mElementList.size();
  // is there a single selection to indicate destination?
  /*
  if(pVioGenerator->SelectedTrans().size()==1) 
  if(int insrow=mFaudesList.indexOf(*pVioGenerator->SelectedTrans().begin())>=0) {
    FD_DQ("LioTlist::GenSync(): destination " << insrow);
    row=insrow+1;
  }
  */
  Insert(dest,elem);
  // notify
  emit NotifyElementIns(elem);
  // pass on to my std item model
  mTableModel->UpdateReset();
  FD_DQG("VioGeneratorListModel::UpdateElementIns(): done");
}

// update notification: delete
void VioGeneratorListModel::UpdateElementDel(const VioElement& elem) { 
  FD_DQG("VioGeneratorListModel::UpdateElementDel():" << elem.Str());
  // do nothing if not exists
  if(!Contains(elem)) return;
  // do delete
  Remove(elem);
  // notify
  emit NotifyElementDel(elem);
  // pass on to my std item model
  mTableModel->UpdateAll();
}
 

// update notification: reimplement move
void VioGeneratorListModel::UpdateElementEdit(const VioElement& selem, const VioElement& delem) { 
  FD_DQG("VioGeneratorListModel::UpdateElementEdit(): " << selem.Str() << " to " << delem.Str());
  Dump();
  // find old position
  int pos=IndexOf(selem);
  if(pos>=0) At(pos,delem);
  if(pos<0) Append(delem);
  // replace by new element
  FD_DQG("VioGeneratorListModel::UpdateElementEdit(): found at " << pos);
  emit NotifyElementEdit(selem,delem);
  // pass on to my std item model
  mTableModel->UpdateRow(pos);
  if(pos<0) mTableModel->UpdateReset();
};


// update notification: properties (eg state name, attribute)
void VioGeneratorListModel::UpdateElementProp(const VioElement& elem) { 
  // do nothing if not exists
  if(!Contains(elem)) return;
  // pass on notification
  emit NotifyElementProp(elem);
  // pass on to my std item model
  mTableModel->UpdateRow(IndexOf(elem));
};

// update notification: trim
void VioGeneratorListModel::UpdateTrimElements(void) { 
  UpdateAnyChange();
};

// update notification: properties
void VioGeneratorListModel::UpdateAnyAttr(void) { 
  UpdateAnyChange();
};


// update notification: reimplement any change
void VioGeneratorListModel::UpdateAnyChange(void) { 
  FD_DQG("VioGeneratorListModel::UpdateAnyChange()");
  // track changes
  bool changed = false;
  // delete obsolete elements
  foreach(const VioElement& elem, mElementList) {
    if(!elem.IsValid()) continue;
    if(pVioGeneratorModel->ElementExists(elem)) continue;
    FD_DQG("VioGeneratorListModel::UpdateAnyChange(): removing " << elem.Str());
    mElementList.removeAll(elem);
    changed=true;
  }
  // default destination: end
  int dest=mElementList.size();
  // is there a single selection to indicate destination?
  /*
  if(pVioGenerator->SelectedTrans().size()==1) 
  if(int insrow=mFaudesList.indexOf(*pVioGenerator->SelectedTrans().begin())>=0) {
    FD_DQ("LioTlist::GenSync(): destination " << insrow);
    row=insrow+1;
  }
  */
  // switch element type to append ...
  switch(mEType) {
  //** ... append transitions:
  case VioElement::ETrans: {
    faudes::TransSet::Iterator tit=Generator()->TransRelBegin();
    for(; tit!=Generator()->TransRelEnd(); tit++) {
      if(mElementList.contains(VioElement::FromTrans(*tit))) continue;
      FD_DQG("VioGeneratorListModel::UpdateAnyChange(): appending " << Generator()->TStr(*tit));
      mElementList.insert(dest,VioElement::FromTrans(*tit));
      dest++;
      changed=true; 
    }
    break;
  }
  //** ... append states
  case VioElement::EState: {
    faudes::StateSet::Iterator sit=Generator()->StatesBegin();
    for(; sit!=Generator()->StatesEnd(); sit++) {
      if(mElementList.contains(VioElement::FromState(*sit))) continue;
      FD_DQG("VioGeneratorListModel::UpdateAnyChange(): appending " << Generator()->SStr(*sit));
      mElementList.insert(dest,VioElement::FromState(*sit));
      dest++;
      changed=true; 
    }
    break;
  }
  //** ... append events
  case VioElement::EEvent: {
    faudes::EventSet::Iterator eit=Generator()->AlphabetBegin();
    for(; eit!=Generator()->AlphabetEnd(); eit++) {
      if(mElementList.contains(VioElement::FromEvent(*eit))) continue;
      FD_DQG("VioGeneratorListModel::UpdateAnyChange(): appending " << Generator()->EStr(*eit));
      mElementList.insert(dest,VioElement::FromEvent(*eit));
      dest++;
      changed=true; 
    }
    break;
  }
  default:
    break;
  }  
  // fix and notify
  if(changed || true) { // broken for editing invalid transitions?
    DoFixRowMap();
    emit NotifyAnyChange();
    mTableModel->UpdateReset(); // todo: have this only on ANY change
  }
  // done
  FD_DQG("VioGeneratorListModel::UpdateAnyChange(): done with size #" << Size());
}


// update notification: reimplement
void VioGeneratorListModel::UpdateNewModel(void) { 
  FD_DQG("VioGeneratorListModel::UpdateNewModel()");
  DoVioUpdate();
}

// update visual data from (new) faudes object
void VioGeneratorListModel::DoVioUpdate(void) { 
  FD_DQG("VioGeneratorListModel::DoVioUpdate() for " << VioElement::TypeStr(ElementType()));
  // call base (fix pointers etc)
  VioGeneratorAbstractModel::DoVioUpdate();
  // clear my list
  Clear();
  // switch element type ...
  switch(mEType) {
  // ... transitions:
  case VioElement::ETrans: {
    faudes::TransSet::Iterator tit=Generator()->TransRelBegin();
    for(int count=0; tit!=Generator()->TransRelEnd(); tit++,count++) 
      Append(VioElement::FromTrans(*tit)); 
    break;
  }
  // ... states:
  case VioElement::EState: {
    faudes::StateSet::Iterator sit=Generator()->StatesBegin();
    for(int count=0; sit!=Generator()->StatesEnd(); sit++,count++) 
      Append(VioElement::FromState(*sit)); 
    break;
  }
  // ... events:
  case VioElement::EEvent: {
    faudes::EventSet::Iterator eit=Generator()->AlphabetBegin();
    for(int count=0; eit!=Generator()->AlphabetEnd(); eit++,count++) 
      Append(VioElement::FromEvent(*eit)); 
    break;
  }
  // ... void et al
  default:
    break;
  }  
  // pass on to view
  emit NotifyAnyChange();
  // report
  FD_DQG("VioGeneratorListModel::DoVioUpdate(): done #" << Size());
}

// fix internal data
void  VioGeneratorListModel::DoFixRowMap(void) {
  mElementRowMap.clear();
  for(int count=0; count < mElementList.size(); count++)
    mElementRowMap[mElementList[count]]=count; 
}

// console dump
void  VioGeneratorListModel::Dump(void) {
#ifdef FAUDES_DEBUG_VIO_GENERATOR
  for(int i = 0; i< mElementList.size(); i++ ) 
    FD_DQG("ListDump:" << mElementList.at(i).Str() << " [" << IndexOf(mElementList.at(i)) << "/" << mElementRowMap.size() << "]");
#endif 
}

// clear all
void VioGeneratorListModel::Clear(void) {
  mElementList.clear();
  mElementRowMap.clear();
}

// local edit: get size 
int VioGeneratorListModel::Size(void) const {
  return mElementList.size();
}

// local edit: get element by position
const VioElement& VioGeneratorListModel::At(int pos) const {
  static VioElement velem;
  if(pos<0 || pos >= mElementList.size()) return velem;
  return mElementList.at(pos);
}

// local edit: set element by position
bool VioGeneratorListModel::At(int pos, const VioElement& elem) {
  if(mEType!=elem.Type()) return false;
  if(mElementRowMap.contains(elem)) return false;
  if(pos<0 || pos >= mElementList.size()) return false;
  const VioElement& old=mElementList.at(pos);
  mElementRowMap.remove(old);
  mElementList[pos]=elem;
  mElementRowMap[elem]=pos;
  Dump();
  return true;
}

// local edit: apend element
bool VioGeneratorListModel::Append(const VioElement& elem) {
  return Insert(mElementList.size(),elem);
}

// local edit: insert
bool VioGeneratorListModel::Insert(int pos, const VioElement& elem) {
  if(mEType!=elem.Type()) return false;
  if(mElementRowMap.contains(elem)) return false;
  if(pos<0) return false;
  if(pos > mElementList.size()) return false;
  QMap<VioElement,int>::iterator lit;
  for(lit=mElementRowMap.begin(); lit!=mElementRowMap.end();lit++) {
    if(lit.value()>=pos) lit.value()++;
  }
  mElementList.insert(pos,elem);
  mElementRowMap[elem]=pos;
  Dump();
  return true;
}

// local edit: remove
bool VioGeneratorListModel::Remove(const VioElement& elem) {
  int pos=IndexOf(elem);
  return RemoveAt(pos);
}

// local edit: remove
bool  VioGeneratorListModel::RemoveAt(int pos) {
  if(pos<0 || pos >= mElementList.size()) return false;
  const VioElement& elem=mElementList.at(pos);
  mElementRowMap.remove(elem);
  QMap<VioElement,int>::iterator lit;
  for(lit=mElementRowMap.begin(); lit!=mElementRowMap.end();lit++) {
    if(lit.value()>=pos) lit.value()--;
  }
  mElementList.removeAt(pos);
  Dump();
  return true;
}

// local edit move
bool VioGeneratorListModel::Move(int from, int to) {
  if(from<0 || from >= mElementList.size()) return false;
  if(to<0 || to >= mElementList.size()) return false;
  mElementList.move(from,to);
  for(int i=from; i<to; i++) 
    mElementRowMap[mElementList.at(i)]=i;
  for(int i=to; i<from; i++) 
    mElementRowMap[mElementList.at(i)]=i;
  Dump();
  return true;
}

// local edit: find
int VioGeneratorListModel::IndexOf(const VioElement& elem) const {
  if(mEType!=elem.Type()) return -1;
  QMap<VioElement,int>::const_iterator lit;
  lit=mElementRowMap.constFind(elem);
  if(lit==mElementRowMap.end()) return -1;
  return lit.value();
}
  
// local edit: contains
bool VioGeneratorListModel::Contains(const VioElement& elem) const {
  if(mEType!=elem.Type()) return false;
  QMap<VioElement,int>::const_iterator lit;
  lit=mElementRowMap.constFind(elem);
  if(lit==mElementRowMap.end()) return false;
  return true;
}


// sorting
void VioGeneratorListModel::SortAscendingX1(void) {
  FD_DQG("VioGeneratorListModel::SortAscendingX1()");
  if(!(VioElementOrder::pGen=Generator())) return;
  qStableSort(mElementList.begin(), mElementList.end(), VioElementOrder::mLessThanX1);
  DoFixRowMap();
  FD_DQG("VioGeneratorListModel::SortAscendingX1(): done");
}

// sorting
void VioGeneratorListModel::SortDescendingX1(void) {
  FD_DQG("VioGeneratorListModel::SortDescendingX1()");
  if(!(VioElementOrder::pGen=Generator())) return;
  qStableSort(mElementList.begin(), mElementList.end(), VioElementOrder::mGreaterThanX1);
  DoFixRowMap();
  FD_DQG("VioGeneratorListModel::SortDescendingX1(): done");
}


// sorting
void VioGeneratorListModel::SortAscendingX2(void) {
  FD_DQG("VioGeneratorListModel::SortAscendingX2()");
  if(!(VioElementOrder::pGen=Generator())) return;
  qStableSort(mElementList.begin(), mElementList.end(), VioElementOrder::mLessThanX2);
  DoFixRowMap();
  FD_DQG("VioGeneratorListModel::SortAscendingX2(): done");
}


// sorting
void VioGeneratorListModel::SortDescendingX2(void) {
  FD_DQG("VioGeneratorListModel::SortDescendingX2()");
  if(!(VioElementOrder::pGen=Generator())) return;
  qStableSort(mElementList.begin(), mElementList.end(), VioElementOrder::mGreaterThanX2);
  DoFixRowMap();
  FD_DQG("VioGeneratorListModel::SortDescendingX2(): done");
}


// sorting
void VioGeneratorListModel::SortAscendingEv(void) {
  FD_DQG("VioGeneratorListModel::SortAscendingEv()");
  if(!(VioElementOrder::pGen=Generator())) return;
  qStableSort(mElementList.begin(), mElementList.end(), VioElementOrder::mLessThanEv);
  DoFixRowMap();
  FD_DQG("VioGeneratorListModel::SortAscendingEv(): done");
}

// sorting
void VioGeneratorListModel::SortDescendingEv(void) {
  FD_DQG("VioGeneratorListModel::SortDescendingEv()");
  if(!(VioElementOrder::pGen=Generator())) return;
  qStableSort(mElementList.begin(), mElementList.end(), VioElementOrder::mGreaterThanEv);
  DoFixRowMap();
  FD_DQG("VioGeneratorListModel::SortDescendingEv(): done");
}


// record changes and emit signal
void VioGeneratorListModel::Modified(bool ch) { 
  // call base
  VioGeneratorAbstractModel::Modified(ch);
  // pass on clr to childs
  if(!ch) {
    if(mTableModel) mTableModel->Modified(false);
  }
}


/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioGeneratorListView

****************************************************************
****************************************************************
****************************************************************
*/


// construct/destruct
VioGeneratorListView::VioGeneratorListView(VioGeneratorView* parent) :
  VioGeneratorAbstractView(parent),
  pGeneratorListModel(0),
  mEType(VioElement::EVoid),
  mTableView(0)
{
  FD_DQG("VioGeneratorListView::VioGeneratorListView("<< parent <<")");
}

// set model (incl typecheck)
int VioGeneratorListView::Model(VioGeneratorAbstractModel* model) {
  FD_DQG("VioGeneratorListView::Model("<< model <<")");
  // bail on double set
  if(model==pGeneratorAbstractModel) return 0;
  // typecheck (include etype)
  if(!qobject_cast<VioGeneratorListModel*>(model)) return 1;
  // my disconnects
  if(pGeneratorModel) {
    disconnect(pGeneratorModel,0,mTableView,0);
  }
  // call base: set, connect, update
  VioGeneratorAbstractView::Model(model);
  // additional connects
  connect(pGeneratorModel,SIGNAL(NotifySelectionElement(const VioElement&,bool)),
    mTableView,SLOT(UpdateSelectionElement(const VioElement&,bool)));
  connect(pGeneratorModel,SIGNAL(NotifySelectionClear(void)),
    mTableView,SLOT(UpdateSelectionClear(void)));
  connect(pGeneratorModel,SIGNAL(NotifySelectionAny(void)),
    mTableView,SLOT(UpdateSelectionAny(void)));
  // ok;
  FD_DQG("VioGeneratorListView::Model("<< model <<"): done");
  return 0;
}

// get model and friends: representation model
const VioGeneratorListModel* VioGeneratorListView::Model(void) const {
  return pGeneratorListModel;
}

// update notification: default to update all
void VioGeneratorListView::UpdateElementIns(const VioElement& elem) 
  { (void) elem; UpdateAnyChange();};
void VioGeneratorListView::UpdateElementDel(const VioElement& elem) 
  { (void) elem; UpdateAnyChange();};
void VioGeneratorListView::UpdateElementEdit(const VioElement& selem, const VioElement& delem) 
  { (void) selem; (void) delem; UpdateAnyChange();};
void VioGeneratorListView::UpdateElementProp(const VioElement& elem)
  { (void) elem; UpdateAnyChange();};
void VioGeneratorListView::UpdateTrimElements(void) 
  { UpdateAnyChange();};
void VioGeneratorListView::UpdateAnyAttr(void) 
  { UpdateAnyChange();};


// update notification: any editing in list model
void VioGeneratorListView::UpdateAnyChange(void) { 
  FD_DQG("VioGeneratorListView::UpdateAnyChange()");
  // tell my table model (should we do this in Gen.ListModel ?)
  pTableModel->UpdateAll(); 
  // debugging widgets
#ifdef FAUDES_DEBUG_VIO_WIDGETS
  mTextInfo->setText(
    tr("%1List").arg(
    VioStyle::QStrFromStr(VioElement::TypeStr(mEType))));
  switch(mEType) {
  case VioElement::ETrans:
    mTextEdit->setPlainText(VioStyle::QStrFromStr(Generator()->TransRelToText()));
    break;
  case VioElement::EState:
    mTextEdit->setPlainText(VioStyle::QStrFromStr(Generator()->StatesToText()));
    break;
  case VioElement::EEvent:
    mTextEdit->setPlainText(VioStyle::QStrFromStr(Generator()->Alphabet().ToText()));
    break;
  default:
    break;
  }
#endif
}

// update notification: new list model
void VioGeneratorListView::UpdateNewModel(void) { 
  FD_DQG("VioGeneratorListView::UpdateAnyChange()");
  DoVioUpdate();
}


// update from model
void VioGeneratorListView::DoVioUpdate(void) {
  FD_DQG("VioGeneratorListView::DoVioUpdate("<<this<<")");
  // have typed ref (model could have changed)
  pGeneratorListModel=qobject_cast<VioGeneratorListModel*>(pGeneratorAbstractModel);
  pTableModel=pGeneratorListModel->TabelModel(); 
  // first time 
  if(mEType==VioElement::EVoid) {
    // set type
    mEType=pGeneratorListModel->ElementType();
    FD_DQG("VioGeneratorListView::DoVioUpdate("<<this<<"): setup layout for " << VioElement::TypeStr(mEType));
    // have q table view
    switch(mEType) {
    case VioElement::ETrans: mTableView = new LioTView(); setWindowTitle("Transitions"); break;
    case VioElement::EState: mTableView = new LioSView(); setWindowTitle("States"); break;
    case VioElement::EEvent: mTableView = new LioEView(); setWindowTitle("Alphabet"); break;
    default: break; // error
    }
    // add view to layout
    if(mTableView) {
      mTableView->setModel(pTableModel);
      mVbox->addWidget(mTableView);
    }
  }
  // etype may not change
  if(mEType != pGeneratorListModel->ElementType()) {}; // error
  // trigger update
  pTableModel->UpdateReset();  // updating must be routed through model !!! todo
  // debugging widgets
  UpdateAnyChange();
}



