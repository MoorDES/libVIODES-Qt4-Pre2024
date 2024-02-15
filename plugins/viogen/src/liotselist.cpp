/* liotselist.cpp  - faudes lists as item models */

/*
   Graphical  IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006, 2007  Thomas Moor, Klaus Schmidt, Sebastian Perk

*/


#include "liotselist.h"
#include "viogenlist.h"


/* 
 ******************************************
 ******************************************
 ******************************************

 implementation of LioVList

 ******************************************
 ******************************************
 ******************************************
 */


// constructor
LioVList::LioVList(VioGeneratorListModel* genlist) : 
  QAbstractTableModel(genlist) {
  FD_DQG("LioVList::LioVList(" << genlist << ")");
  pVioGeneratorListModel=genlist;
  pVioGeneratorModel=genlist->GeneratorModel();
  pVioGeneratorConfig=genlist->GeneratorConfiguration();
  // empty dimensions
  mDataColumns=0;
  mFlagColumns=0;
  // update/resize hook
  mUpdateOldRows=-1;
  mUpdateChanged=false;
  // track changes
  mModified=false;
};

// destructor
LioVList::~LioVList(void) {
  FD_DQG("LioVList::~LioVList()");
}

// read only access to faudes generator 
const faudes::vGenerator* LioVList::Generator(void) const {
  return pVioGeneratorListModel->Generator();
};

// convenience access to vio generator model
VioGeneratorModel* LioVList::GeneratorModel(void) {
  return pVioGeneratorListModel->GeneratorModel();
};

// convenience access to vio generator list model
VioGeneratorListModel* LioVList::GeneratorListModel(void) {
  return pVioGeneratorListModel;
};


// install faudes flags
void LioVList::InsertFlags(const QList<VioBooleanProperty>& boolprops) {
  FD_DQG("LioVList::InsertFlags(#" << boolprops.size() << ")");
  for(int i=0; i<boolprops.size(); i++) {
    const VioBooleanProperty& prop = boolprops[i];
    if(!prop.mEditable) continue;
    if(prop.mShortName=="") continue;
    mFlagNames.append(prop.mShortName);
    mFlagAddresses.append(i);
  }
  mFlagColumns=mFlagNames.size();
  FD_DQG("LioVList::InsertFlags(#" << mFlagColumns << ")");
}


// tabelmodel: headers
QVariant LioVList::headerData(int section, Qt::Orientation orientation, int role) const {
  (void) orientation;
  if(section>=columnCount())  return QVariant();
  if(section<mDataColumns)  return QVariant();
  int flagno=section-mDataColumns;
  // display role
  if(role==Qt::DisplayRole) 
    return mFlagNames.at(flagno);
  // invalid
  return QVariant();
}

// access faudes items by model index: validity
bool LioVList::IsFaudesElement(const QModelIndex& index) {
  int row=index.row();
  int col=index.column();
  FD_DQG("LioTList::IsFaudesElement(" << row << ", " << col << ")");
  // check for valid index
  if(!index.isValid()) return false;
  if(col<0 || col >= columnCount()) return false; 
  if(row<0 || row >= pVioGeneratorListModel->Size()) return false;
  // heck for valid element
  const VioElement& elem=pVioGeneratorListModel->At(row);
  if(elem.Type()!=pVioGeneratorListModel->ElementType()) return false;
  if(!elem.IsValid()) return false;
  // passed
  return true;
}


// access faudes items by model index: get
VioElement LioVList::FaudesElement(const QModelIndex& index) {
  int row=index.row();
  int col=index.column();
  FD_DQG("LioTList::FaudesElement(" << row << ", " << col << ")");
  // check index and return
  if(index.isValid()) 
  if(col>=0 || col < columnCount())
  if(row>=0 || row < pVioGeneratorListModel->Size()) {
    return pVioGeneratorListModel->At(row);
  }
  // return default
  return VioElement();
}


// access faudes items by model index: find
QModelIndex LioVList::ModelIndex(const VioElement& elem) {
  int row = pVioGeneratorListModel->IndexOf(elem);
  if(row<0) return QModelIndex();
  QModelIndex index = createIndex(row,0);  
  return index;
}


// resize hook
void LioVList::PrepareResize(void) {
  if(mUpdateOldRows!=-1 && mUpdateOldRows!=rowCount()) {
    // pending prepare: total redraw on next call
    mUpdateOldRows=0;
    mUpdateChanged=true;
  } else {
    // first call since last update: try smart
    mUpdateOldRows=rowCount();
    mUpdateChanged=false;
  }
}

// resize hook: update
void LioVList::UpdateResize(void) { 
  // bail out on no pending
  if(rowCount()!=0 && rowCount()==mUpdateOldRows && !mUpdateChanged)
    return;
  // empty or content changed: force reset
  if(rowCount()<=0) {
    reset();
  }
  // sense resize: grow
  if(rowCount()>mUpdateOldRows && mUpdateOldRows>=0) {
    beginInsertRows(QModelIndex(), mUpdateOldRows, rowCount()-1);
    endInsertRows();
  }
  // sense resize: shrink
  if(rowCount()<mUpdateOldRows && rowCount()>0) {
    beginRemoveRows(QModelIndex(), rowCount(), mUpdateOldRows-1);
    endRemoveRows();
  }
  // update all
  QModelIndex index1, index2;
  index1=createIndex(0,0);
  index2=createIndex(rowCount()-1,columnCount()-1);
  emit dataChanged(index1,index2);
  // clear pending
  mUpdateOldRows=rowCount();
  mUpdateChanged=false;
}

// resize hook: update all
void LioVList::UpdateAll(void) { 
  FD_DQG("LioVList::UpdateAll(): rows " << rowCount());
  QModelIndex index1, index2;
  index1=createIndex(0,0);
  index2=createIndex(rowCount()-1,columnCount()-1);
  emit dataChanged(index1,index2);
  mUpdateChanged=false;
}

// clear all
void LioVList::UpdateReset(void) {
  FD_DQG("LioVList::UpdateReset()");
  reset();
  mUpdateOldRows=-1;
  mUpdateChanged=false;
  UpdateAll();
}

// resize hook: update row
void LioVList::UpdateRow(int row) { 
  QModelIndex index1, index2;
  index1=createIndex(row,0);
  index2=createIndex(row,columnCount()-1);
  emit dataChanged(index1,index2);
  return;
} 


// tablemodel: number of columns
int LioVList::columnCount(const QModelIndex &parent) const {
  (void) parent;
  return mDataColumns+mFlagColumns;
}

// tablemodel: number of columns
int LioVList::rowCount(const QModelIndex &parent) const {
  (void) parent;
  return pVioGeneratorListModel->Size();
}

// tablemodel: default item flags 
Qt::ItemFlags LioVList::flags(const QModelIndex &index) const {
  //FD_DQG("LioVList::flags(" << this << ")");
  // default
  Qt::ItemFlags defaultFlags = Qt::ItemIsEnabled | Qt::ItemIsDropEnabled; 
  if(index.isValid()) {
     int col=index.column();
     defaultFlags|= Qt::ItemIsDragEnabled;
    // name data ?
    if(col<mDataColumns) 
      defaultFlags |= Qt::ItemIsEditable | Qt::ItemIsSelectable; 
    // faudes flag data ?
    if(col>=mDataColumns) 
      defaultFlags |= Qt::ItemIsUserCheckable;
  }
  //FD_DQG("LioVList::flags(" << this << "): done");
  return defaultFlags;
};

// tabelmodel: add element (assume user)
bool LioVList::insertRows(int row, int count, const QModelIndex& parent) {
  FD_DQG("LioVList::insertRows(" << row << ", " << count << ")");
  // bail out on trivial
  if(count==0) return false;
  // track user edit
  Modified(true);
  pVioGeneratorModel->UndoEditStart();
  // todo: inefficient "builtin fixmap" for large count
  beginInsertRows(parent, row, row+count);
  // insert new default elements
  while(count--) {
    VioElement delem= VioElement::FromType(pVioGeneratorListModel->ElementType());
    if(pVioGeneratorListModel->ElementType()==VioElement::EState || pVioGeneratorListModel->ElementType()==VioElement::EEvent) {
      delem=pVioGeneratorModel->ElementIns(delem); 
      FD_DQG("LioVList::insertRows():" << delem.Str() << " fix");
      pVioGeneratorListModel->Remove(delem); 
    }
    pVioGeneratorListModel->Insert(row,delem); 
    FD_DQG("LioVList::insertRows():" << delem.Str() << " at " << row);
  }
  endInsertRows();
  pVioGeneratorModel->UndoEditStop();
  return true;
}

// tabelmodel: removes elements (assume user)
bool LioVList::removeRows(int row, int count, const QModelIndex& parent) {
  (void) parent;
  // bail out on trivial
  if(count==0) return false;
  FD_DQG("LioVList::removeRows(" << row << ", " << count << ")");
  // track user edit
  Modified(true);
  pVioGeneratorModel->UndoEditStart();
  // todo: inefficient "builtin fixmap" for large count
  //remove elements in list model
  QList<VioElement> rmelems;
  beginRemoveRows(parent, row, row+count-1);
  while(count--) {
    rmelems.append(pVioGeneratorListModel->At(row));
    pVioGeneratorListModel->RemoveAt(row);
  }
  endRemoveRows();
  // redraw view
  UpdateReset();
  //remove transitions in vio generator model
  foreach(const VioElement& elem, rmelems) {
    pVioGeneratorModel->ElementDel(elem);
  }
  pVioGeneratorModel->UndoEditStop();
  return true;
}


// tablemodel: user set data (flags)
bool LioVList::setData(const QModelIndex &index, const QVariant& value, int role) {
  int row=index.row();
  int col=index.column();
  // bail out on invalid data request
  if(!index.isValid()) return false;
  if(row >= rowCount()) return false;
  if(col >= columnCount()) return false;
  VioElement oelem=pVioGeneratorListModel->At(row);
  //** edit data: faudes flags 
  if(col >= mDataColumns && role == Qt::CheckStateRole) {
    FD_DQG("LioVList::setData(...): editing flag at " << row << " " << col);
    if(!pVioGeneratorModel->ElementExists(oelem)) return false;
    if(value==Qt::Checked) 
    if(!pVioGeneratorModel->ElementBooleanProperty(oelem,col-mDataColumns)) {
      FD_DQG("LioVList::setData(...): set flag " << col-mDataColumns);
      pVioGeneratorModel->UndoEditStart();
      pVioGeneratorModel->ElementBooleanProperty(oelem,col-mDataColumns,true); 
      pVioGeneratorModel->UndoEditStop();
    }
    if(value==Qt::Unchecked) 
    if(pVioGeneratorModel->ElementBooleanProperty(oelem,col-mDataColumns)) {
      FD_DQG("LioVList::setData(...): clr sflag " << col-mDataColumns);
      pVioGeneratorModel->UndoEditStart();
      pVioGeneratorModel->ElementBooleanProperty(oelem,col-mDataColumns,false); 
      pVioGeneratorModel->UndoEditStop();
    }    
    return true;
  }
  return false;
}


// tabelmodel: get data (flags)
QVariant LioVList::data(const QModelIndex &index, int role) const {
  int row=index.row();
  int col=index.column();
  // bail out on invalid data request
  if(!index.isValid()) return QVariant();
  if(row >= rowCount()) return QVariant();
  if(col >= columnCount()) return QVariant();
  // get element
  const VioElement& elem=pVioGeneratorListModel->At(row);
  // retrieve data: faudes flags
  if(col >= mDataColumns && role == Qt::CheckStateRole) {
    int flagno = col-mDataColumns;
    bool bp =  pVioGeneratorModel->ElementBooleanProperty(elem,flagno);
    if(bp) return QVariant(Qt::Checked);
    return QVariant(Qt::Unchecked);
  }
  return QVariant();
}


// drag and drop: supported action
Qt::DropActions LioVList::supportedDropActions(void) const { 
  FD_DQG("LioVList::supportedDropActions()");
  return Qt::MoveAction | Qt::CopyAction;
}

// drag and drop: supported mime types
QStringList LioVList::mimeTypes () const{
  QStringList res = QAbstractItemModel::mimeTypes();
  foreach(const QString& str, res) 
     { (void) str; FD_DQG("LioVList::mimeType(): " << VioStyle::StrFromQStr(str));}
  return res;
}

// drag and drop: drag hack
QMimeData * LioVList::mimeData(const QModelIndexList & indexes) const {
  FD_DQG("LioVList::mimeData(...)");
  QMimeData* res=QAbstractItemModel::mimeData(indexes);
  *(const_cast<QByteArray*>(&mDragData)) = res->data("application/x-qabstractitemmodeldatalist");
  *(const_cast<QModelIndexList*>(&mDragIndexes)) = indexes;
  return res;
}

// drag and drop: drop hack
bool LioVList::dropMimeData(const QMimeData *data, Qt::DropAction action, 
  int row, int column, const QModelIndex& parent) {
  (void) column;
  // fix destination
  if(row==-1) {
   if(parent.isValid())
     row = parent.row();
   else
     row = rowCount()-1;
  }
  FD_DQG("LioVList::dropMimeData(...): destination " << row);
  // report action
  if(action & Qt::MoveAction)  { FD_DQG("LioVList::dropMimeData(...): move action"); };
  if(action & Qt::CopyAction)  { FD_DQG("LioVList::dropMimeData(...): copy action"); };
  // check for hacked index data
  QByteArray dragdata = data->data("application/x-qabstractitemmodeldatalist");
  if((dragdata==mDragData) && (action & Qt::MoveAction)) {
    // collect all source
    FD_DQG("LioVList::dropMimeData(...): do move");
    QList<int> sourcelist;
    foreach(const QModelIndex& index,mDragIndexes) {
      if(!index.isValid()) continue;
      int sourcerow=index.row();
      if(!sourcelist.contains(sourcerow)) {
        FD_DQG("LioVList::dropMimeData(...): do move " << sourcerow << " to " << row);
        sourcelist.append(sourcerow);
      }
    }
    // sort and doit
    qSort(sourcelist);
    UserInternalMove(sourcelist,row);
    reset();
    return true;
  }
  return false;
}



// drag and drop: internal move by row number
void  LioVList::UserInternalMove(const QList<int>& sourcelist, int dest) {
  // track interaction
  Modified(true);
  pVioGeneratorModel->UndoEditStart();
  // doit
  QList<int>::const_iterator sit=sourcelist.begin();
  int soff=0;  
  for(;sit!=sourcelist.end();sit++) {
    int srow=*sit;
    if(srow >= rowCount()) continue;
    if(srow <0 ) continue;
    FD_DQG("LioVList::UserInternalMove(...): rows " << srow << " --> " << dest );
    pVioGeneratorListModel->Move(srow+soff, dest);
    if(dest>srow+soff) soff--;
    else dest++; 
  }
  UpdateAll();
  pVioGeneratorModel->UndoEditStop();
}


// query changes (dont emit signal)
bool LioVList::Modified(void) const { 
  return mModified;
};

// collect and pass on modifications of childs
void LioVList::ChildModified(bool changed) { 
  // ignre netagtives
  if(!changed) return;
  // report
  FD_DQT("LioVList::ChildModified(1): model modified " << mModified);
  Modified(true);
};

// record changes and emit signal)
void LioVList::Modified(bool ch) { 
  // set
  if(!mModified && ch) {
    mModified=true;
    emit NotifyModified(mModified);
  }
  // clr
  if(mModified && !ch) {
    mModified=false;
    FD_DQG("LioVList::LioVList(" << this << "): emit modified notification");
    emit NotifyModified(mModified);
  }
}



/* 
 ******************************************
 ******************************************
 ******************************************

 implementation of LioTList

 ******************************************
 ******************************************
 ******************************************
 */


// constructor
LioTList::LioTList(VioGeneratorListModel* genlist) : LioVList(genlist) {
  FD_DQG("LioTList::LioTList(" << genlist << ")");
  mDataColumns=3;
  InsertFlags(pVioGeneratorConfig->mTransAttribute->AttributeConfiguration()->BooleanProperties());
  UpdateReset();
};


// destructor
LioTList::~LioTList(void) {
  FD_DQG("LioTList::~LioTList()");
}

// tabelmodel: headers
QVariant LioTList::headerData(int section, Qt::Orientation orientation, int role) const {
  // bail out
  if(orientation != Qt::Horizontal) return QVariant();
  if(section>=columnCount())  return QVariant();
  // display role
  if(role==Qt::DisplayRole) {
    switch(section) {
    case 0: return QString("X1");
    case 1: return QString("Ev");
    case 2: return QString("X2");
    }
  }
  // call base
  return LioVList::headerData(section,orientation,role);
} 

// tablemodel: sort
void LioTList::sort(int column, Qt::SortOrder order) {
  FD_DQG("LioTList::sort(...)");
  if(column==0) {
    if(order==Qt::AscendingOrder) pVioGeneratorListModel->SortAscendingX1();
    else pVioGeneratorListModel->SortDescendingX1();
  }
  if(column==1) {
    if(order==Qt::AscendingOrder) pVioGeneratorListModel->SortAscendingEv();
    else pVioGeneratorListModel->SortDescendingEv();
  }
  if(column==2) {
    if(order==Qt::AscendingOrder) pVioGeneratorListModel->SortAscendingX2();
    else pVioGeneratorListModel->SortDescendingX2();
  }
  reset();
  // track user edit
  Modified(true);
}
  

// tabelmodel: actual data
QVariant LioTList::data(const QModelIndex &index, int role) const {
  int row=index.row();
  int col=index.column();
  // bail out on invalid data request
  if(!index.isValid()) return QVariant();
  if(row >= rowCount()) return QVariant();
  if(col >= columnCount()) return QVariant();
  // get transition
  const VioElement& telem=pVioGeneratorListModel->At(row);
  // retrieve data 
  if(role == Qt::DisplayRole) {
    //FD_DQG("LioTList::data(" << row << "/" << rowCount()<< "): trans " << Generator()->TStr(telem.Trans()));
    switch(col) {
    case 0: return VioStyle::DispStateName(Generator(),telem.Trans().X1);
    case 1: return VioStyle::DispEventName(Generator(),telem.Trans().Ev);
    case 2: return VioStyle::DispStateName(Generator(),telem.Trans().X2);
    }
  }
  // retrieve color
  if(role == Qt::ForegroundRole) {
    if(telem.IsValid()) return VioStyle::Color(VioBlack);
    return VioStyle::Color(VioRed);
  }
  // retrieve data: faudes flags
  if(col >= mDataColumns && role == Qt::CheckStateRole) {
    return LioVList::data(index,role);
  }
  return QVariant();
}

// tabelmodel: user set data 
bool LioTList::setData(const QModelIndex &index, const QVariant& value, int role) {
  int row=index.row();
  int col=index.column();
  // bail out on invalid data request
  if(!index.isValid()) return false;
  if(row >= rowCount()) return false;
  if(col >= columnCount()) return false;
  VioElement otelem=pVioGeneratorListModel->At(row);
  faudes::Transition nftrans=otelem.Trans();
  //** edit data: rename stateA 
  if(col==0 && role == Qt::EditRole) {
    // get old/new stateA
    std::string ofname=Generator()->StateName(otelem.Trans().X1);
    std::string nfname=VioStyle::StrFromQStr(value.toString());
    FD_DQG("LioTList::setData(...): changing state A \"" << ofname << "\" in " << otelem.Str() << 
      " to \"" << nfname << "\"?");
    VioElement x1elem=VioElement::FromState(
       VioStyle::IdxFromSymbol(nfname,&Generator()->StateSymbolTable()));
    // state does not exist, name is valid: insert state
    if(!pVioGeneratorModel->ElementExists(x1elem) && faudes::SymbolTable::ValidSymbol(nfname)) {
      x1elem=pVioGeneratorModel->ElementIns(VioElement::FromState(0));
      pVioGeneratorModel->ElementName(x1elem,VioStyle::QStrFromStr(nfname));
    } 
    // state does not exist, index from fake symbol is valid 
    if(!pVioGeneratorModel->ElementExists(x1elem) && x1elem.IsValid()) {
      pVioGeneratorModel->ElementIns(x1elem);
    }
    // still does not exist: bail out
    if(!pVioGeneratorModel->ElementExists(x1elem)) {
      return false;
    }
    // set up transition
    nftrans.X1=x1elem.State();
  }
  //** edit data: rename event
  if(col==1 && role == Qt::EditRole) {
    // get old/new name
    std::string ofname=Generator()->EventName(otelem.Trans().Ev);
    std::string nfname=VioStyle::StrFromQStr(value.toString());
    FD_DQG("LioTList::setData(...): renaming event " << ofname << " to " << nfname << "?");
    if(!faudes::SymbolTable::ValidSymbol(nfname)) return false;
    VioElement evelem = VioElement::FromEvent(
       VioStyle::IdxFromSymbol(nfname,Generator()->EventSymbolTablep()));
    // event does not exist: insert 
    if(!pVioGeneratorModel->ElementExists(evelem)) {
      evelem=pVioGeneratorModel->ElementIns(VioElement::FromEvent(0));
      evelem=pVioGeneratorModel->ElementName(evelem,VioStyle::QStrFromStr(nfname));
    } 
    // set up new transition
    nftrans.Ev=evelem.Event();
  }
  //** edit data: rename stateB 
  if(col==2 && role == Qt::EditRole) {
    // get old/new stateB
    std::string ofname=Generator()->StateName(otelem.Trans().X2);
    std::string nfname=VioStyle::StrFromQStr(value.toString());
    FD_DQG("LioTList::setData(...): changing state B \"" << ofname << "\" in " << otelem.Str() << 
      " to \"" << nfname << "\"?");
    VioElement x2elem=VioElement::FromState(
       VioStyle::IdxFromSymbol(nfname,&Generator()->StateSymbolTable()));
    // state does not exist, name is valid: insert state
    if(!pVioGeneratorModel->ElementExists(x2elem) && faudes::SymbolTable::ValidSymbol(nfname)) {
      x2elem=pVioGeneratorModel->ElementIns(VioElement::FromState(0));
      pVioGeneratorModel->ElementName(x2elem,VioStyle::QStrFromStr(nfname));
    } 
    // state does not exist, index from fake symbol is valid 
    if(!pVioGeneratorModel->ElementExists(x2elem) && x2elem.IsValid()) {
      pVioGeneratorModel->ElementIns(x2elem);
    }
    // still does not exist: bail out
    if(!pVioGeneratorModel->ElementExists(x2elem)) {
      return false;
    }
    // set up transition
    nftrans.X2=x2elem.State();
  }
  //** edit data: faudes flags 
  if(col >= mDataColumns && role == Qt::CheckStateRole) {
    return LioVList::setData(index, value,role);
  }
  //** doit: move old to new transition
  if(otelem.Trans()!=nftrans) {
    FD_DQG("LioTList::setData(...): moving " << otelem.Str() << " to " << nftrans.Str() );
    pVioGeneratorModel->UndoEditStart();
    pVioGeneratorListModel->RemoveAt(row);  
    pVioGeneratorListModel->Insert(row,VioElement::FromTrans(nftrans));    
    pVioGeneratorModel->ElementEdit(otelem,VioElement::FromTrans(nftrans)); // incl callback myself
    pVioGeneratorModel->UndoEditStop();
  } 
  // emit data changed
  QModelIndex index1=createIndex(index.row(),0);
  QModelIndex index2=createIndex(index.row(),columnCount()-1);
  emit dataChanged(index1,index2);
  return true;
}



/* 
 ******************************************
 ******************************************
 ******************************************

 implementation of LioSList

 ******************************************
 ******************************************
 ******************************************
 */

// constructor
LioSList::LioSList(VioGeneratorListModel* genlist) : LioVList(genlist) {
  FD_DQG("LioSList::LioSList(" << genlist << ")");
  mDataColumns=1;
  InsertFlags(pVioGeneratorConfig->mStateAttribute->AttributeConfiguration()->BooleanProperties());
  //mCompleter = new VioSymbolCompleter(this);
  //mCompleter->setSymbolWorld(this,0);
  UpdateReset();
};


// destructor
LioSList::~LioSList(void) {
  FD_DQG("LioSList::~LioSList()");
}

// tabelmodel: headers
QVariant LioSList::headerData(int section, Qt::Orientation orientation, int role) const {
  // bail out
  if(orientation != Qt::Horizontal) return QVariant();
  if(section>=columnCount())  return QVariant();
  // display role
  if(role==Qt::DisplayRole) {
    switch(section) {
    case 0: return QString("State Names");
    }
  }
  // call base
  return LioVList::headerData(section,orientation,role);
} 

// tablemodel: sort
void LioSList::sort(int column, Qt::SortOrder order) {
  FD_DQG("LioSList::sort(...)");
  if(column==0) {
    if(order==Qt::AscendingOrder) pVioGeneratorListModel->SortAscendingX1();
    else pVioGeneratorListModel->SortDescendingX1();
  }
  reset();
  // track user edit
  Modified(true);
}
  

// tablemodel: actual data
QVariant LioSList::data(const QModelIndex &index, int role) const {
  int row=index.row();
  int col=index.column();
  // bail out on invalid data request
  if(!index.isValid()) return QVariant();
  if(row >= rowCount()) return QVariant();
  if(col >= columnCount()) return QVariant();
  // get state
  const VioElement& selem=pVioGeneratorListModel->At(row);
  // retrieve data 
  if(role == Qt::DisplayRole) {
    //FD_DQG("LioSList::data(" << row << "/" << rowCount()<< "): state " << Generator()->SStr(selem.State()));
    switch(col) {
    case 0: return VioStyle::DispStateName(Generator(),selem.State());
    }
  }
  // retrieve color
  if(role == Qt::ForegroundRole) {
    if(selem.IsValid()) return VioStyle::Color(VioBlack);
    return VioStyle::Color(VioRed);
  }
  // retrieve data: faudes flags
  if(col >= mDataColumns && role == Qt::CheckStateRole) {
    return LioVList::data(index,role);
  }
  return QVariant();
}

// tablemodel: user set data 
bool LioSList::setData(const QModelIndex &index, const QVariant& value, int role) {
  int row=index.row();
  int col=index.column();
  // bail out on invalid data request
  if(!index.isValid()) return false;
  if(row >= rowCount()) return false;
  if(col >= columnCount()) return false;
  VioElement oselem=pVioGeneratorListModel->At(row);
  //** edit data: rename state 
  if(col==0 && role == Qt::EditRole) {
    // get old/new state names
    std::string ofname=Generator()->StateName(oselem.State());
    std::string nfname=VioStyle::StrFromQStr(value.toString());
    FD_DQG("LioSList::setData(...): editing state " << Generator()->SStr(oselem.State()) << " to " << nfname << "?");
    // nothing going on
    if(ofname==nfname) return true;
    // remove state name
    if(nfname==""){ 
      FD_DQG("LioSList::setData(...): removing state name " << ofname);
      pVioGeneratorModel->ElementName(oselem,""); // callback: sync
      return true;
    }
    // rename state
    if(faudes::SymbolTable::ValidSymbol(nfname)) {
      if(Generator()->ExistsState(nfname)) {
        nfname=Generator()->UniqueStateName(nfname);
        if(nfname==Generator()->UniqueStateName(ofname)) return false;
      }
      FD_DQG("LioSList::setData(...): renaming state " << ofname << " to " << nfname );
      pVioGeneratorModel->UndoEditStart();
      pVioGeneratorModel->ElementName(oselem,VioStyle::QStrFromStr(nfname)); // callback: sync
      pVioGeneratorModel->UndoEditStop();
      return true;
    }
    // renumber state 
    if(VioStyle::ValidFakeSymbol(nfname)) {
      faudes::Idx nfstate=VioStyle::IdxFromSymbol(nfname);
      FD_DQG("LioSList::setData(...): moving state " << oselem.State() << " to " << nfstate);
      if(nfstate==0 || Generator()->ExistsState(nfstate)) return false;
      pVioGeneratorModel->UndoEditStart();
      pVioGeneratorModel->ElementEdit(oselem,VioElement::FromState(nfstate)); // callback: sync
      pVioGeneratorModel->UndoEditStop();
      return true;
    }
  }
  //** edit data: faudes flags 
  if(col >= mDataColumns && role == Qt::CheckStateRole) {
    return LioVList::setData(index, value,role);
  }
  // emit data changed
  /*
  QModelIndex index1=createIndex(index.row(),0);
  QModelIndex index2=createIndex(index.row(),columnCount()-1);
  emit dataChanged(index1,index2);
  return true;
  */
  return false;
}




/* 
 ******************************************
 ******************************************
 ******************************************

 implementation of LioEList

 ******************************************
 ******************************************
 ******************************************
 */


// constructor
LioEList::LioEList(VioGeneratorListModel* genlist) : LioVList(genlist) {
  FD_DQG("LioEList::LioEList(" << genlist << ")");
  mDataColumns=1;
  InsertFlags(pVioGeneratorConfig->mEventAttribute->AttributeConfiguration()->BooleanProperties());
  //mCompleter = new VioSymbolCompleter(this);
  //mCompleter->setSymbolWorld(this,0);
  UpdateReset();
};


// destructor
LioEList::~LioEList(void) {
  FD_DQG("LioEList::~LioEList()");
}

// tabelmodel: headers
QVariant LioEList::headerData(int section, Qt::Orientation orientation, int role) const {
  // bail out
  if(orientation != Qt::Horizontal) return QVariant();
  if(section>=columnCount())  return QVariant();
  // display role
  if(role==Qt::DisplayRole) {
    switch(section) {
    case 0: return QString("Event Names");
    }
  }
  // call base
  return LioVList::headerData(section,orientation,role);
} 

// tablemodel: sort
void LioEList::sort(int column, Qt::SortOrder order) {
  FD_DQG("LioEList::sort(...)");
  if(column==0) {
    if(order==Qt::AscendingOrder) pVioGeneratorListModel->SortAscendingEv();
    else pVioGeneratorListModel->SortDescendingEv();
  }
  reset();
  // track user edit
  Modified(true);
}
  

// tablemodel: actual data
QVariant LioEList::data(const QModelIndex &index, int role) const {
  int row=index.row();
  int col=index.column();
  // bail out on invalid data request
  if(!index.isValid()) return QVariant();
  if(row >= rowCount()) return QVariant();
  if(col >= columnCount()) return QVariant();
  // get event
  const VioElement& evelem=pVioGeneratorListModel->At(row);
  // retrieve data 
  if(role == Qt::DisplayRole) {
    //FD_DQG("LioEList::data(" << row << "/" << rowCount()<< "): event " << Generator()->EStr(evelem.Event()));
    switch(col) {
    case 0: return VioStyle::DispEventName(Generator(),evelem.Event());
    }
  }
  // retrieve color
  if(role == Qt::ForegroundRole) {
    if(evelem.IsValid()) return VioStyle::Color(VioBlack);
    return VioStyle::Color(VioRed);
  }
  // retrieve data: faudes flags
  if(col >= mDataColumns && role == Qt::CheckStateRole) {
    return LioVList::data(index,role);
  }
  return QVariant();
}

// tablemodel: user set data 
bool LioEList::setData(const QModelIndex &index, const QVariant& value, int role) {
  int row=index.row();
  int col=index.column();
  // bail out on invalid data request
  if(!index.isValid()) return false;
  if(row >= rowCount()) return false;
  if(col >= columnCount()) return false;
  VioElement oevelem=pVioGeneratorListModel->At(row);
  //** edit data: rename event 
  if(col==0 && role == Qt::EditRole) {
    // get old/new event names
    std::string ofname=Generator()->EventName(oevelem.Event());
    std::string nfname=VioStyle::StrFromQStr(value.toString());
    FD_DQG("LioEList::setData(...): editing event " << Generator()->EStr(oevelem.Event()) << " to " << nfname << "?");
    // nothing going on
    if(ofname==nfname) return true;
    if(!faudes::SymbolTable::ValidSymbol(nfname)) return false;
    // avoid doublets
    if(Generator()->ExistsEvent(nfname)) {
      FD_DQG("LioEList::setData(...): new name exists");
      nfname=Generator()->UniqueEventName(nfname);
      if(nfname==Generator()->UniqueEventName(ofname)) return false;
    }
    // doit (this will update myself and other views)
    FD_DQG("LioEList::setData(...): renaming " << ofname << " to " << nfname << "!");
    pVioGeneratorModel->UndoEditStart();
    pVioGeneratorModel->ElementName(oevelem,VioStyle::QStrFromStr(nfname));
    pVioGeneratorModel->UndoEditStop();
  }
  //** edit data: faudes flags 
  if(col >= mDataColumns && role == Qt::CheckStateRole) {
    return LioVList::setData(index, value,role);
  }
  // emit data changed
  QModelIndex index1=createIndex(index.row(),0);
  QModelIndex index2=createIndex(index.row(),columnCount()-1);
  emit dataChanged(index1,index2);
  return true;
}


