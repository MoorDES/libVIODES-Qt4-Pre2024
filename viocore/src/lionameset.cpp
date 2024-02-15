/* lionameset.cpp  -  qt list versions of nameset */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2010  Thomas Moor, 

*/


#include "lionameset.h"

/* 
 ******************************************
 ******************************************
 ******************************************

 implementation of LioNameSetModel

 ******************************************
 ******************************************
 ******************************************
 */


// constructor
LioNameSetModel::LioNameSetModel(VioNameSetModel* nameset) : 
  QAbstractTableModel(nameset) {
  FD_DQN("LioNameSetModel(" << nameset << ")");
  pVioNameSetModel=nameset;
  pVioNameSetConfig=nameset->NameSetConfiguration();
  // default dimensions
  mDataColumns=1;
  mFlagColumns=0;
  // get flags from config
  InsertFlags(pVioNameSetConfig->mAttribute->AttributeConfiguration()->BooleanProperties());
  // update/resize hook
  mUpdateOldRows=-1;
  mUpdateChanged=false;
  // track changes
  mModified=false;
  // connect
  QObject::connect(pVioNameSetModel,SIGNAL(NotifyAnyChange()),this,SLOT(UpdateAnyChange()));
  QObject::connect(pVioNameSetModel,SIGNAL(NotifySymbolChange(QString)),this,SLOT(UpdateSymbolChange(QString)));

};

// destructor
LioNameSetModel::~LioNameSetModel(void) {
  FD_DQN("LioNameSetModel::~LioNameSetModel()");
};

// read only access to faudes nameset 
const faudes::NameSet* LioNameSetModel::NameSet(void) const {
  return pVioNameSetModel->NameSet();
}

// convenience access to vio nameset
VioNameSetModel* LioNameSetModel::VioModel(void) {
  return pVioNameSetModel;
}

// install faudes flags
void LioNameSetModel::InsertFlags(const QList<VioBooleanProperty>& boolprops) {
  FD_DQN("LioNameSetModel::InsertFlags(#" << boolprops.size() << ")");
  for(int i=0; i<boolprops.size(); i++) {
    const VioBooleanProperty& prop = boolprops[i];
    if(!prop.mEditable) continue;
    if(prop.mShortName=="") continue;
    mFlagNames.append(prop.mShortName);
    mFlagAddresses.append(i);
  }
  mFlagColumns=mFlagNames.size();
  FD_DQN("LioNameSetModel::InsertFlags(#" << mFlagColumns << ")");
}


// tabelmodel: headers
QVariant LioNameSetModel::headerData(int section, Qt::Orientation orientation, int role) const {
  // bail out
  if(orientation != Qt::Horizontal) return QVariant();
  if(section>=columnCount())  return QVariant();
  if(role!=Qt::DisplayRole) return QVariant();
  // data header
  if(section==0) return QString(pVioNameSetConfig->mHeader);
  // flags
  int flagno=section-mDataColumns;
  return mFlagNames.at(flagno);
}

// access names by model index: validity
bool LioNameSetModel::IsSymbol(const QModelIndex& index) {
  int row=index.row();
  int col=index.column();
  FD_DQN("LioNameSetModel:IsSymbol(" << row << ", " << col << ")");
  // check for valid index
  if(!index.isValid()) return false;
  if(col<0 || col >= columnCount()) return false; 
  if(row<0 || row >= pVioNameSetModel->Size()) return false;
  // passed
  return true;
}


// access names by model index: get
const QString& LioNameSetModel::Symbol(const QModelIndex& index) {
  static QString estr;
  int row=index.row();
  int col=index.column();
  FD_DQN("LioNameSetModel::Symbol(" << row << ", " << col << ")");
  // check index and return
  if(index.isValid()) 
  if(col>=0 || col < columnCount())
  if(row>=0 || row < pVioNameSetModel->Size()) {
    return pVioNameSetModel->At(row);
  }
  // return default
  return estr;
}


// access names by model index: find
QModelIndex LioNameSetModel::ModelIndex(const QString& name) {
  int row = pVioNameSetModel->IndexOf(name);
  if(row<0) return QModelIndex();
  QModelIndex index = createIndex(row,0);  
  return index;
}


// resize hook
void LioNameSetModel::PrepareResize(void) {
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
void LioNameSetModel::UpdateResize(void) { 
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
void LioNameSetModel::UpdateAll(void) { 
  FD_DQN("LioNameSetModel::UpdateAll(): rows " << rowCount());
  QModelIndex index1, index2;
  index1=createIndex(0,0);
  index2=createIndex(rowCount()-1,columnCount()-1);
  emit dataChanged(index1,index2);
  mUpdateChanged=false;
}

// clear all
void LioNameSetModel::UpdateReset(void) {
  FD_DQN("LioNameSetModel::UpdateReset()");
  reset();
  mUpdateOldRows=-1;
  mUpdateChanged=false;
  UpdateAll();
}

// resize hook: update row
void LioNameSetModel::UpdateRow(int row) { 
  QModelIndex index1, index2;
  index1=createIndex(row,0);
  index2=createIndex(row,columnCount()-1);
  emit dataChanged(index1,index2);
  return;
} 

// vio model changed 
void LioNameSetModel::UpdateAnyChange(void) { 
  FD_DQN("LioNameSetModel::UpdateAnyChange()");
  UpdateAll();
}

// vio model changed 
void LioNameSetModel::UpdateSymbolChange(QString name) { 
  FD_DQN("LioNameSetModel::UpdateSymbolChange()");
  QModelIndex index = ModelIndex(name);
  if(index.isValid())
  if(index.row()==pVioNameSetModel->IndexOf(name)) {
      UpdateRow(index.row());
      return;
  }
  UpdateAll();
}


// tablemodel: sort
void LioNameSetModel::sort(int column, Qt::SortOrder order) {
  FD_DQN("LioNameSetModel::sort(...)");
  if(column==0) {
    if(order==Qt::AscendingOrder) pVioNameSetModel->SortAscending();
    else pVioNameSetModel->SortDescending();
  }
  reset();
  // track user edit
  Modified(true);
}
  

// tablemodel: number of columns
int LioNameSetModel::columnCount(const QModelIndex &parent) const {
  (void) parent;
  return mDataColumns+mFlagColumns;
}

// tablemodel: number of columns
int LioNameSetModel::rowCount(const QModelIndex &parent) const {
  (void) parent;
  return pVioNameSetModel->Size();
}

// tablemodel: default item flags 
Qt::ItemFlags LioNameSetModel::flags(const QModelIndex &index) const {
  //FD_DQN("LioNameSetModel::flags(" << this << ")");
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
  //FD_DQN("LioNameSetModel::flags(" << this << "): done");
  return defaultFlags;
};

// tabelmodel: user add element
bool LioNameSetModel::insertRows(int row, int count, const QModelIndex& parent) {
  FD_DQN("LioNameSetModel::insertRows(" << row << ", " << count << ")");
  // ignore trivial
  if(count==0) return false;
  // track user edit
  Modified(true);
  pVioNameSetModel->UndoEditStart();
  // todo: inefficient "buitin fixmap" for large count
  beginInsertRows(parent, row, row+count);
  // insert new default elements
  while(count--) {
    QString dname = pVioNameSetModel->UniqueSymbol();
    pVioNameSetModel->Insert(row,dname); 
    FD_DQN("LioNameSetModel::insertRows():" << VioStyle::StrFromQStr(dname) << " at " << row);
  }
  endInsertRows();
  // track user edit
  pVioNameSetModel->UndoEditStop();
  return true;
}

// tabelmodel: user removes transitions
bool LioNameSetModel::removeRows(int row, int count, const QModelIndex& parent) {
  (void) parent;
  FD_DQN("LioNameSetModel::removeRows(" << row << ", " << count << ")");
  // ignore trivial
  if(count==0) return false;
  // track user edit
  Modified(true);
  pVioNameSetModel->UndoEditStart();
  // todo: inefficient "builtin fixmap" for large count
  //remove elements in list model
  beginRemoveRows(parent, row, row+count-1);
  while(count--) {
    pVioNameSetModel->RemoveAt(row);
  }
  endRemoveRows();
  // redraw view
  UpdateAll();
  // track user edit
  pVioNameSetModel->UndoEditStop();
  return true;
}


// tablemodel: user set data 
bool LioNameSetModel::setData(const QModelIndex &index, const QVariant& value, int role) {
  int row=index.row();
  int col=index.column();
  // bail out on invalid data request
  if(!index.isValid()) return false;
  if(row >= rowCount()) return false;
  if(col >= columnCount()) return false;
  const QString& oname=pVioNameSetModel->At(row);
  FD_DQN("LioNameSetModel::setData(...): on " << VioStyle::StrFromQStr(oname));
  //** edit data: rename event 
  if(col==0 && role == Qt::EditRole) {
    // get old/new event names
    QString nname=value.toString();
    FD_DQN("LioNameSetModel::setData(...): renaming " << 
	   VioStyle::StrFromQStr(oname) << " to " << VioStyle::StrFromQStr(nname) << " (?)");
    // nothing going on
    if(oname==nname) return true;
    if(!VioStyle::ValidSymbol(nname)) return false;
    // avoid doublets
    if(VioModel()->Exists(nname)) {
      nname=VioModel()->UniqueSymbol(nname);
      if(nname==VioModel()->UniqueSymbol(oname)) return false;
    }
    // doit (this will update other views)
    FD_DQN("LioNameSetModel::setData(...): renaming " << 
      VioStyle::StrFromQStr(oname) << " to " << VioStyle::StrFromQStr(nname) << "(!)");
    pVioNameSetModel->UndoEditStart();
    pVioNameSetModel->At(row,nname);
    pVioNameSetModel->UndoEditStop();
    return true;
  }
  //** edit data: faudes flags 
  if(col >= mDataColumns && role == Qt::CheckStateRole) {
    FD_DQN("LioNameSetModel::setData(...): editing flag at " << row << " " << col);
    if(!pVioNameSetModel->Exists(oname)) return false;
    if(value==Qt::Checked) 
    if(!pVioNameSetModel->BooleanProperty(oname,mFlagAddresses.at(col-mDataColumns))) {
      FD_DQN("LioNameSetModel::setData(...): set flag " << col-mDataColumns);
      pVioNameSetModel->UndoEditStart();
      pVioNameSetModel->BooleanProperty(oname,mFlagAddresses.at(col-mDataColumns),true); 
      pVioNameSetModel->UndoEditStop();
    }
    if(value==Qt::Unchecked) 
    if(pVioNameSetModel->BooleanProperty(oname,mFlagAddresses.at(col-mDataColumns))) {
      FD_DQN("LioNameSetModel::setData(...): clr sflag " << col-mDataColumns);
      pVioNameSetModel->UndoEditStart();
      pVioNameSetModel->BooleanProperty(oname,mFlagAddresses.at(col-mDataColumns),false); 
      pVioNameSetModel->UndoEditStop();
    }
    return true;
  }
  return false;
}


// tabelmodel: get data (flags)
QVariant LioNameSetModel::data(const QModelIndex &index, int role) const {
  int row=index.row();
  int col=index.column();
  // bail out on invalid data request
  if(!index.isValid()) return QVariant();
  if(row >= rowCount()) return QVariant();
  if(col >= columnCount()) return QVariant();
  // get symbol
  const QString& oname=pVioNameSetModel->At(row);
  //FD_DQN("LioNameSetModel::data(...): row " << row << " symbol " << VioStyle::StrFromQStr(oname));
  // retrieve data: symbol 
  if(role == Qt::DisplayRole && col==0) return oname;
  // retrieve color
  if(role == Qt::ForegroundRole) {
    //if(evelem.IsValid()) return VioStyle::Color(VioBlack);
    //return VioStyle::Color(VioRed);
    return VioStyle::Color(VioBlack);
  }
  // retrieve data: faudes flags
  if(col >= mDataColumns && role == Qt::CheckStateRole) {
    int flagno = col-mDataColumns;
    bool bp =  pVioNameSetModel->BooleanProperty(oname,mFlagAddresses.at(flagno));
    if(bp) return QVariant(Qt::Checked);
    return QVariant(Qt::Unchecked);
  }
  return QVariant();
}


// drag and drop: supported action
Qt::DropActions LioNameSetModel::supportedDropActions(void) const { 
  FD_DQN("LioNameSetModel::supportedDropActions()");
  return Qt::MoveAction | Qt::CopyAction;
}

// drag and drop: supported mime types
QStringList LioNameSetModel::mimeTypes () const{
  QStringList res = QAbstractItemModel::mimeTypes();
  foreach(const QString& str, res) 
     { (void) str; FD_DQN("LioNameSetModel::mimeType(): " << VioStyle::StrFromQStr(str));}
  return res;
}

// drag and drop: drag hack
QMimeData * LioNameSetModel::mimeData(const QModelIndexList & indexes) const {
  FD_DQN("LioNameSetModel::mimeData(...)");
  QMimeData* res=QAbstractItemModel::mimeData(indexes);
  *(const_cast<QByteArray*>(&mDragData)) = res->data("application/x-qabstractitemmodeldatalist");
  *(const_cast<QModelIndexList*>(&mDragIndexes)) = indexes;
  return res;
}

// drag and drop: drop hack
bool LioNameSetModel::dropMimeData(const QMimeData *data, Qt::DropAction action, 
  int row, int column, const QModelIndex& parent) {
  (void) column;
  // fix destination
  if(row==-1) {
   if(parent.isValid())
     row = parent.row();
   else
     row = rowCount()-1;
  }
  FD_DQN("LioNameSetModel::dropMimeData(...): destination " << row);
  // report action
  if(action & Qt::MoveAction)   { FD_DQN("LioNameSetModel::dropMimeData(...): move action"); }
  if(action & Qt::CopyAction)   { FD_DQN("LioNameSetModel::dropMimeData(...): copy action"); }
  // check for hacked index data
  QByteArray dragdata = data->data("application/x-qabstractitemmodeldatalist");
  if((dragdata==mDragData) && (action & Qt::MoveAction)) {
    // collect all source
    FD_DQN("LioNameSetModel::dropMimeData(...): do move");
    QList<int> sourcelist;
    foreach(const QModelIndex& index,mDragIndexes) {
      if(!index.isValid()) continue;
      int sourcerow=index.row();
      if(!sourcelist.contains(sourcerow)) {
        FD_DQN("LioNameSetModel::dropMimeData(...): do move " << sourcerow << " to " << row);
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
void  LioNameSetModel::UserInternalMove(const QList<int>& sourcelist, int dest) {
  // track interaction
  Modified(true);
  pVioNameSetModel->UndoEditStart();
  // doit
  QList<int>::const_iterator sit=sourcelist.begin();
  int soff=0;  
  for(;sit!=sourcelist.end();sit++) {
    int srow=*sit;
    if(srow >= rowCount()) continue;
    if(srow <0 ) continue;
    FD_DQN("LioNameSetModel::InternalMove(...): rows " << srow << " --> " << dest );
    pVioNameSetModel->Move(srow+soff, dest);
    if(dest>srow+soff) soff--;
    else dest++; 
  }
  UpdateAll();
  Modified(true);
  pVioNameSetModel->UndoEditStop();
}


// query changes (dont emit signal)
bool LioNameSetModel::Modified(void) const { 
  return mModified;
};

// collect and pass on modifications of childs
void LioNameSetModel::ChildModified(bool changed) { 
  // ignre netagtives
  if(!changed) return;
  // report
  FD_DQN("LioNameSetModel::ChildModified(1): model modified " << mModified);
  Modified(true);
};

// record changes and emit signal)
void LioNameSetModel::Modified(bool ch) { 
  // set
  if(!mModified && ch) {
    mModified=true;
    emit NotifyModified(mModified);
  }
  // clr
  if(mModified && !ch) {
    mModified=false;
    FD_DQN("LioNameSetModel::LioVList(" << this << "): emit modified notification");
    emit NotifyModified(mModified);
  }
}



/* 
 ******************************************
 ******************************************
 ******************************************

 implementation of LioNameSetView

 ******************************************
 ******************************************
 ******************************************
 */


// constructor
LioNameSetView::LioNameSetView(QWidget* parent) : 
  QTableView(parent),  
  pLioModel(0),
  pVioModel(0)
{
  FD_DQN("LioNameSetView::LioNameSetView()");
  //setRootIsDecorated(false); // treeview version
  setShowGrid(false);          // tableview only
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setAlternatingRowColors(true);
  setSelectionMode(QAbstractItemView::ExtendedSelection);
  setSelectionBehavior(QAbstractItemView::SelectRows);
  setSortingEnabled(true);
  setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked |
    QAbstractItemView::EditKeyPressed | QAbstractItemView::AnyKeyPressed);
  setDragDropMode(QAbstractItemView::InternalMove);
  setDragEnabled(true);
  setAcceptDrops(true);
  setDropIndicatorShown(true);
  setDragDropOverwriteMode(false);
  mStateDelegate = new VioSymbolDelegate(this);
  mStateDelegate->setSymbolMode(VioSymbol::FakeSymbols);
  mEventDelegate = new VioSymbolDelegate(this);
  // layout
  //setFrameStyle(QFrame::StyledPanel); // better on osx/win/kde
  setFrameStyle(QFrame::NoFrame);       // better on winnt (aka winxp-classic)
  setMinimumSize(QSize(200,200));
  setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
  // context menu
  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this,SIGNAL(customContextMenuRequested(QPoint)),
      this,SLOT(ContextMenuAtPoint(QPoint)));
  // behavioural state
  mSortEnabled=1;
  mMuteSelection=false;
  mInsertMode=false;
  // tailor for nameset
  setItemDelegateForColumn(0,mEventDelegate);
  QObject::connect(this,SIGNAL(pressed(const QModelIndex&)), 
    this, SLOT(EmitMouseClick(const QModelIndex&)));
  verticalHeader()->hide();
};

// destructor
LioNameSetView::~LioNameSetView(void) {
  FD_DQN("LioNameSetView::~LioNameSetView()");
}


// read only access to faudes generator 
const faudes::NameSet* LioNameSetView::NameSet(void) const {
  if(!pVioModel) return 0;
  return pVioModel->NameSet();
};

// convenience access to vio generator model
VioNameSetModel* LioNameSetView::VioModel(void) {
  return pVioModel;
};

// reimplement: set model
void LioNameSetView::setModel(LioNameSetModel* liomodel) {
  FD_DQN("LioNameSetView::setModel()");
  // bail out ob double call
  if(model()==liomodel) return;
  // disconnect
  if(pVioModel) disconnect(this,0,pVioModel,0);
  if(pVioModel) disconnect(pVioModel,0,this,0);
  // record refs
  pLioModel=liomodel;
  pVioModel=pLioModel->VioModel();
  // call base
  QTableView::setModel(pLioModel);
  // connect: click
  connect(this,SIGNAL(MouseClick(const VioElement&)),
	  pVioModel,SIGNAL(MouseClick(const VioElement&)));
  // connect: select
  QObject::connect(selectionModel(),SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
    this, SLOT(SelectionUpdate(const QItemSelection&, const QItemSelection&)));
  QObject::connect(pVioModel,SIGNAL(NotifySelectionElement(const VioElement&, bool)),
    this, SLOT(UpdateSelectionElement(const VioElement&, bool)));
  QObject::connect(pVioModel,SIGNAL(NotifySelectionClear()),
    this, SLOT(UpdateSelectionClear()));
  QObject::connect(pVioModel,SIGNAL(NotifySelectionAny()),
    this, SLOT(UpdateSelectionAny()));
  // connect: modified
  //QObject::connect(pList,SIGNAL(Changed(bool)),
  //this, SIGNAL(Changed(bool)));
  // layout header after setting model
  header()->setResizeMode(0,QHeaderView::Stretch);
  for(int col=1; col<pLioModel->columnCount(); col++)
    header()->setResizeMode(col,QHeaderView::ResizeToContents);
  SetSortEnabled(1);
  FD_DQN("LioNameSetView::setModel(): done");
}


// set completers to use
void LioNameSetView::SetStateCompleter(QCompleter* completer) {
  mStateDelegate->setCompleter(completer);
} 

// set completers to use
void LioNameSetView::SetEventCompleter(QCompleter* completer) {
  mEventDelegate->setCompleter(completer);
}


// editing model: selection
void LioNameSetView::UpdateSelectionElement(const VioElement& elem, bool on) {
  if(!pLioModel) return;
  if(elem.IsValid()) return;
  if(elem.IsEvent()) return;
  QString name = pVioModel->SymbolicName(elem.Ev());
  FD_DQN("LioNameSetView::UpdateSelectionElement(" << VioStyle::StrFromQStr(name) << ", " << on << ")");
  QModelIndex index= pLioModel->ModelIndex(name);
  if(!index.isValid()) return;
  if(on) selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
  else selectionModel()->select(index, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
  FD_DQN("LioNameSetView::UpdateSelectionSymbol(" << VioStyle::StrFromQStr(name) << ", " << on << "): done");
}

// editing model: selection
void LioNameSetView::UpdateSelectionClear(void) {
  clearSelection();
}

// editing model: selection
// watch out: we must not clear out selection model, since this
// will be signaled to the vionamesetmodel ...
void LioNameSetView::UpdateSelectionAny(void) {
  if(!pVioModel) return;
  clearSelection(); // TODO
  FD_DQN("LioNameSetView::UpdateSelectionAny(): A");
  const QList<VioElement> gensel=pVioModel->Selection();
  foreach(const VioElement& elem,gensel) {
    QString name=pVioModel->SymbolicName(elem.Ev());
    QModelIndex index= pLioModel->ModelIndex(name);
    if(!index.isValid()) continue;
    selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
  }
  FD_DQN("LioNameSetView::UpdateSelectionAny(): B");
  QModelIndexList selectedindexes=selectionModel()->selectedIndexes();
  foreach(const QModelIndex& index, selectedindexes) {
    if(!index.isValid()) continue;
    QString selname = pLioModel->Symbol(index);
    VioElement selelem = VioElement::FromEvent(VioModel()->IndexOf(selname));
    if(!VioModel()->IsSelected(selelem))
      selectionModel()->select(index,QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
  }
}


// show/hide sort indicator
void LioNameSetView::FixSortIndicator(int col) {
  if(col>=mSortEnabled) header()->setSortIndicatorShown(false); 
  else header()->setSortIndicatorShown(true); 
}

// configuration limit sorted cols
void LioNameSetView::SetSortEnabled(int col) {
  mSortEnabled=col;
  QObject::connect(header(), SIGNAL(sectionPressed(int)), 
    this, SLOT(FixSortIndicator(int)));
}

// selected rows
QList<int> LioNameSetView::SelectedRows(void) {
  FD_DQN("LioNameSetView::SelectedRows(): from sel model " << selectionModel());
  QModelIndexList selectedindexes=selectionModel()->selectedIndexes();
  FD_DQN("LioNameSetView::SelectedRows(): indexes #" << selectedindexes.size());
  QList<int> selectedrows;
  foreach(const QModelIndex& index, selectedindexes) {
    if(!index.isValid()) continue;
    if(index.row()<0) continue;
    if(index.row()>=model()->rowCount()) continue;
    if(selectedrows.contains(index.row())) continue;
    selectedrows.append(index.row());
  }
  FD_DQN("LioNameSetView::SelectedRows(): rows #" << selectedrows.size());
  qSort(selectedrows);
  return selectedrows;
}

// user deleting selection
void LioNameSetView::UserDelSelection(void) {
  FD_DQN("LioNameSetView::UserDelSelection()");
  QList<int> selectedrows=SelectedRows();
  if(selectedrows.size()==0) return;
  pVioModel->UndoEditStart();
  int off=0;
  QList<int>::iterator rit=selectedrows.begin();
  for(;rit!=selectedrows.end();rit++) {
    model()->removeRow(*rit+off); // incl alphabet collback
    off--;
  }
  pVioModel->UndoEditStop();
}


// user ins element
void LioNameSetView::UserInsert(void) {
  FD_DQN("LioNameSetView::UserInsert()");
  int row = -1;
  QModelIndex index = currentIndex();
  if(index.isValid())row=index.row();
  if(row<0 || row> model()->rowCount()) row=model()->rowCount();
  pVioModel->SelectionClear();
  model()->insertRow(row);
  setCurrentIndex(model()->index(row,0));
}


// get key events
void LioNameSetView::keyPressEvent(QKeyEvent *event) {
  FD_DQN("LioNameSetView::keyPressEvent(...): " << event->key());
  // bail out
  if(!pLioModel) return;
  // inactive
  /*
  QTableView::keyPressEvent(event);
  mInsertMode=false;
  return;
  */
  // figure where
  int row = -1;
  int column = -1;
  QModelIndex index = currentIndex();
  if(index.isValid()) row=index.row();
  column=index.column();
  FD_DQN("LioNameSetView::keyPressEvent(...): row " << row << " col " << column);
  // switch: ingnore tab navigation
  if(event->key() == Qt::Key_Tab) {
    event->ignore();
    return;
  } 
  // switch: have space navigation
  if(event->key() == Qt::Key_Space) {
    FD_DQN("LioNameSetView::keyPressEvent(...): space navigation");
    QModelIndex next=model()->index(row+1,0);
    if(!next.isValid()) next=model()->index(0,0);
    if(next.isValid()) {
      pVioModel->SelectionClear();
      setCurrentIndex(next);
    }
    event->accept();
    return;
  } 
  // switch: explicitly insert a line
  if( (event->key() == Qt::Key_Insert) || 
      ( (event->key() == Qt::Key_Return) && (event->modifiers() & Qt::ShiftModifier ) ) ) {
    FD_DQN("LioNameSetView::keyPressEvent(...): Insert row " << row);
    mInsertMode=false;
    // retrieve row
    const QList<VioElement> gensel=pVioModel->Selection();
    if(row<0 && gensel.size()==1)  { 
      QString name=pVioModel->SymbolicName(gensel.at(0).Ev());
      QModelIndex index= pLioModel->ModelIndex(name);
      row=index.row();
      FD_DQN("LioNameSetView::keyPressEvent(...): Insert below " << gensel.at(0).Str() << " at row " << row );
    }
    pVioModel->SelectionClear();
    model()->insertRow(row+1);
    setCurrentIndex(model()->index(row+1,0));
    QTableView::edit(currentIndex());
    event->accept();
    return;
  } 
  // switch: edit current item
  /*
  if( (event->key() == Qt::Key_Return) || (event->key() == Qt::Key_Space) ) {
    FD_DQN("LioNameSetView::keyPressEvent(...): Edit item");
    QModelIndex next=model()->index(row,column);
    FD_DQN("LioNameSetView::keyPressEvent(...): Next Row ** " << model());
    if(next.isValid() && (model()->flags(next) & Qt::ItemIsEditable) ) {      
      FD_DQN("LioNameSetView::keyPressEvent(...): Next Row at jj " << row);
      pVioModel->SelectionClear();
      FD_DQN("LioNameSetView::keyPressEvent(...): Next Row at " << row);
      setCurrentIndex(next);
      QTableView::edit(next);
      event->accept();
      FD_DQN("LioNameSetView::keyPressEvent(...): Next Row done ");
      return;
    }
    FD_DQN("LioNameSetView::keyPressEvent(...): Next Row -- ");
  } 
  */
  // switch: delete selection
  if( (event->key() == Qt::Key_Delete)  || (event->key() == Qt::Key_Backspace) ) {
    FD_DQN("LioNameSetView::keyPressEvent(...): Delete");
    mInsertMode=false;
    int current=currentIndex().row();
    UserDelSelection();
    pVioModel->SelectionClear();
    int next=current -1;
    if(next<0) next=0;
    setCurrentIndex(model()->index(next,0));
    event->accept();
    return;
  } 
  // switch: call base (navigation, start editing etc)
  QTableView::keyPressEvent(event);
  mInsertMode=false;
}  

// edit hook
bool LioNameSetView::edit(const QModelIndex& index, EditTrigger trigger, QEvent* event) {
  FD_DQN("LioNameSetView::edit(): at (" << index.row() << ", " << index.column() << ")");
  // sense clock to void (todo: put this to focusIn()?)
  if(!index.isValid()) {
    EmitMouseClick(index);
  }
  // record for later use
  mEditIndex=index;
  return QTableView::edit(index,trigger,event);
};


// close edit hook
void LioNameSetView::closeEditor(QWidget* editor, QAbstractItemDelegate::EndEditHint hint) {
  FD_DQN("LioNameSetView::closeEdit(): editor " << editor << " hint: " << hint);
  // edit next
  if(hint==QAbstractItemDelegate::EditNextItem) { 
    // find next item
    QModelIndex cur=mEditIndex; // currentIndex();  
    FD_DQN("LioNameSetView::closeEdit(): edit next at (" << cur.row() << ", " << cur.column() << ")");
    int erow=cur.row();
    int ecol=cur.column() +1;
    // skip checkboxes
    QModelIndex edix=model()->index(erow,ecol);
    if( edix.isValid() && ! ( model()->flags(edix) & Qt::ItemIsEditable) ) {
      erow=erow+1;
      ecol=0;
    }
    // move to next line
    if(ecol>=model()->columnCount()) {
      erow=erow+1;
      ecol=0;
    }
    // version1: auto insert
    mInsertMode=false;
    if(erow>=model()->rowCount()) {
      mInsertMode=true;
      erow=model()->rowCount();
      model()->insertRow(erow);
      ecol=0;
    }
    // version2: wrap end of list
    /*if(erow>=model()->rowCount()) {
      ecol=0; erow=0;
    }*/
    // doit
    QTableView::closeEditor(editor, QAbstractItemDelegate::NoHint);
    QModelIndex next=model()->index(erow,ecol);
    FD_DQN("LioNameSetView::closeEdit(): editor " << editor << 
       " running base::edit on (" << next.row() << ", " << next.column() <<")");
    setCurrentIndex(next); 
    QTableView::edit(next);
    FD_DQN("LioNameSetViewiew::closeEdit(): editor " << editor << " done im " << mInsertMode);
    return;
  }
  // escape
  if(hint==QAbstractItemDelegate::RevertModelCache) { 
    // find next item
    QModelIndex cur=mEditIndex; // currentIndex();
    FD_DQN("LioNameSetView::closeEdit(): revert at (" << cur.row() << ", " << cur.column() << ") with im " << mInsertMode);
    if(mInsertMode) {
      model()->removeRow(cur.row());
      setCurrentIndex(cur); 
    }
    mInsertMode=false;
    QTableView::closeEditor(editor, hint);
    FD_DQN("LioNameSetView::closeEdit(): editor " << editor << " done");
    return;
  }  
  // other cases
  mInsertMode=false;
  QTableView::closeEditor(editor, hint);
  FD_DQN("LioNameSetView::closeEdit(): editor " << editor << " done");
}

// selection was updated
void LioNameSetView::SelectionUpdate(const QItemSelection& selected, const QItemSelection& deselected) {
  // no model
  if(!pVioModel) return;
  if(!pLioModel) return;
  FD_DQN("LioNameSetView::SelectionUpdate(): on #" << selected.size() << " off #" << deselected.size());
  // select
  foreach(QModelIndex index, selected.indexes())
    pVioModel->Select(VioElement::FromEvent(pVioModel->Index(pLioModel->Symbol(index))),true);
  // deselect
  foreach(QModelIndex index, deselected.indexes()) 
    pVioModel->Select(VioElement::FromEvent(pVioModel->Index(pLioModel->Symbol(index))),false);
}


// track focus
void LioNameSetView::focusInEvent(QFocusEvent* event) {
  FD_DQN("LioNameSetView::focusInEvent()");
  QTableView::focusInEvent(event);
}
void LioNameSetView::focusOutEvent(QFocusEvent* event) {
  FD_DQN("LioNameSetView::focusOutEvent()");
  QTableView::focusInEvent(event);
};


// emit mouse press to model
void LioNameSetView::EmitMouseClick(const QModelIndex& index) {
  if(!pLioModel) return;
  FD_DQN("LioNameSetView::EmitMouseClick(...): row " << index.row() << " col " << index.column());
  QString name = pLioModel->Symbol(index);
  VioElement elem= VioElement::FromEvent(pVioModel->Index(name));
  FD_DQN("LioNameSetView::EmitMouseClick(...): emit MouseClick " << elem.Str());
  emit MouseClick(elem);  
}


// contextmenu
void LioNameSetView::ContextMenuAtPoint(QPoint pos) {
  FD_DQN("LioNameSetView::ContextMenuAtPoint("<< pos.x()<<", "<<pos.y() << ")");
  QModelIndex index=indexAt(pos);
  if(QTableView* widget=qobject_cast<QTableView*>(sender())) {
    pos=widget->viewport()->mapToGlobal(pos);
  } 
  //else if(QWidget* widget=qobject_cast<QWidget*>(sender())) {
  //  pos=widget->mapToGlobal(pos);
  //}
  ContextMenu(pos,index);
}

// contextmenu
void LioNameSetView::ContextMenu(QPoint pos, const QModelIndex& index) {
  FD_DQN("LioNameSetView::ContextMenuAt("<< index.row() <<", "<<index.column() << ")");
  // generic insert
  if(!index.isValid()) {
    setCurrentIndex(index);
    QMenu* menu = new QMenu("Insert");
    QAction* insaction= menu->addAction("Insert Line (Shift+Return)");
    QAction *selaction = menu->exec(pos);
    if(selaction==insaction){
      UserInsert();
    }
    delete menu;
    return;
  }
  // setup property widget menu
  QMenu* menu = new QMenu(pVioModel->NameSetConfiguration()->mHeader);
  PioNameSetView* eventwidget = new PioNameSetView(menu,pVioModel->Configuration());
  eventwidget->Model(pVioModel);
  eventwidget->ShowSelection();
  QWidgetAction* eventaction= new QWidgetAction(menu);
  eventaction->setDefaultWidget(eventwidget);
  menu->addAction(eventaction);
  // add my buttons
  menu->addSeparator();
  QAction* deleteaction= menu->addAction("Delete");
  QAction* insaction= menu->addAction("Insert");
  // run menu
  QAction *selaction = menu->exec(pos);
  if(selaction==deleteaction){
    UserDelSelection();
  }
  if(selaction==insaction){
    UserInsert();
  }
  // done
  delete menu;
  FD_DQN("LioNameSetView::ContextMenu: done ");

}



/*
 *******************************************************
 *******************************************************

 PioNameSetView implementation

 *******************************************************
 *******************************************************
 */

// construct base
PioNameSetView::PioNameSetView(QWidget* parent, VioStyle* config) : QWidget(parent) {
 
  FD_DQN("PioNameSetView(parent " << parent <<" ,config " << config <<")");

  // typed version of configuration
  pNameSetConfig = dynamic_cast<VioNameSetStyle*>(config);

  // not connected
  pVioModel=0;
  mSymbol="";

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

  // connect name chyange
  QObject::connect(mEditName,SIGNAL(returnPressed(void)),this,SLOT(UpdateModel(void)));

  // have attribute widget
  if(pNameSetConfig)
    mAttribute= qobject_cast<VioAttributeWidget*>(pNameSetConfig->mAttribute->NewWidget());
#ifdef FAUDES_DEBUG_VIO_NAMESET 
  if(!mAttribute || !pNameSetConfig) 
    FD_DQN("PioNameSetView: cannot cast attribute widget for config " << pNameSetConfig);
#endif

  // fallback
  if(!mAttribute) mAttribute= new VioAttributeWidget();

  // connect attribute change
  QObject::connect(mAttribute,SIGNAL(NotifyAnyChange(void)),this,SLOT(UpdateModel(void)));

  // overall layout
  mVbox = new QVBoxLayout(this);
  mVbox->setMargin(0);
  mVbox->setSpacing(0);
  mVbox->addLayout(hbox);
  mVbox->addSpacing(10);
  mVbox->addWidget(mAttribute);

  // initialise update block
  mBlockModelUpdate=false;

  // clear view
  DoClear();

  // report
  FD_DQN("PioNameSetView(...): done");
};

// destruct
PioNameSetView::~PioNameSetView(void) {
  FD_DQN("PioNameSetView::~PioNameSetView()");
}

// set vionameset
void PioNameSetView::Model(VioNameSetModel* model) {
  FD_DQN("PioNameSetView::Model()");
  if(model) FD_DQN("PioNameSetView::Model(): ctype " << typeid(*model).name());
  // disconnect
  if(pVioModel)
    disconnect(pVioModel, 0, this, 0);
  // record
  pVioModel=model;
  mAttribute->Model()->Context(model);
  // connect model change
  connect(pVioModel,SIGNAL(NotifyChange(void)),this,SLOT(UpdateView(void)));
  connect(pVioModel,SIGNAL(NotifySelectionChange(void)),this,SLOT(ShowSelection(void)));
  // clear 
  DoClear();
  // show
  Show(VioElement());
  //ShowSelection();
}

// get viogenerator
const VioNameSetModel* PioNameSetView::Model(void) const {
  return pVioModel;
}

// read only access to faudes name set 
const faudes::NameSet* PioNameSetView::NameSet(void) const {
  if(!pVioModel) return 0;
  return pVioModel->NameSet();
}


// set symbol mode of name specifier
void PioNameSetView::SymbolMode(VioSymbol::Mode mode) {
  mEditName->setSymbolMode(mode);
}  


// get faudes symbol that is currently displyed
const QString& PioNameSetView::Symbol(void) const { 
  return mSymbol; 
};

// get faudes index that is currently displyed
faudes::Idx PioNameSetView::Idx(void) const { 
  if(!pVioModel) return 0;
  return pVioModel->Index(mSymbol);
};

// get faudes element that is currently displyed
VioElement PioNameSetView::Element(void) const { 
  return VioElement::FromEvent(Idx());
};


// get symbolic name from editor
QString PioNameSetView::Name(void) {
  return mEditName->symbol();
}


// set symbolic name   
void PioNameSetView::Name(const QString& qname) {
  mEditName->setText(qname);
  mEditName->setEnabled(!qname.startsWith("< sel"));
}

// set attribute (silent)
void PioNameSetView::Attribute(const faudes::AttributeVoid& attr) {
  // bail out 
  if(!pVioModel) return;
  if(!mAttribute) return;
  // silent set
  bool old=mBlockModelUpdate; 
  mBlockModelUpdate=true;
  mAttribute->FaudesObject(&attr); // const is crucial here
  mBlockModelUpdate=old;
}

// set attribute from selection
void PioNameSetView::AttributeFromSelection(void) {
  // bail out 
  if(!pVioModel) return;
  if(!mAttribute) return;
  FD_DQN("PioNameSetView::AttributeFromSelection(): #" << pVioModel->Selection().size());
  // silent set
  bool old=mBlockModelUpdate; 
  mBlockModelUpdate=true;
  mAttribute->Model()->MergeClear();
  foreach(VioElement elem,pVioModel->Selection()) {
    if(elem.Type()!=VioElement::EEvent) continue;
    faudes::Idx index=elem.Ev();
    QString sname = pVioModel->SymbolicName(index);
    if(sname=="") continue;
    const faudes::AttributeFlags& attr = pVioModel->Attribute(sname);
    mAttribute->Model()->MergeInsert(&attr); 
  }
  mAttribute->Model()->MergeDone();
  mBlockModelUpdate=old;
}

// set attribute to selection
void PioNameSetView::AttributeToSelection(void) {
  // bail out 
  if(!pVioModel) return;
  if(!mAttribute) return;
  FD_DQN("PioNameSetView::AttributeToSelection(): #" << pVioModel->Selection().size());
  // block update
  bool old=mBlockModelUpdate; 
  mBlockModelUpdate=true;
  // loop selections
  QMap<VioElement,faudes::AttributeFlags*> elem2attr;
  foreach(VioElement elem,pVioModel->Selection()) {
    if(elem.Type()!=VioElement::EEvent) continue;
    faudes::Idx index=elem.Ev();
    QString sname = pVioModel->SymbolicName(index);
    if(sname=="") continue;
    faudes::AttributeFlags* attr = pVioModel->Attribute(sname).Copy();
    mAttribute->Model()->MergeAssign(attr); 
    //elem2attr[elem]=attr;
    pVioModel->Attribute(sname,*attr);
    delete attr;
  }
  /*
  QMap<VioElement, faudes::AttributeFlags*>::const_iterator i = elem2attr.constBegin();
  for(; i != elem2attr.constEnd();i++) {
    faudes::Idx index=i.key().Ev();
    QString sname = pVioModel->SymbolicName(index);
    if(sname=="") continue;
    pVioModel->Attribute(sname,*i.value());
    delete i.value();
  }
  */
  FD_DQN("PioNameSetView::AttributeToSelection(): done");
  mBlockModelUpdate=old;
}

// set model from visual representation
void PioNameSetView::UpdateModel(void) {
  // bail out on block
  if(mBlockModelUpdate) return;
  // call virtual
  FD_DQN("PioNameSetView::UpdateModel()");
  DoModelUpdate();
}

// set view from model
void PioNameSetView::UpdateView(void) {
  FD_DQN("PioNameSetView::UpdateView()");
  if(mBlockModelUpdate) return;
  DoVioUpdate();
}

// set values from faudes element if connected to viogenerator, 
void PioNameSetView::Show(const VioElement& elem) {
  // bail out in no model
  if(!pVioModel) return; 
  // ignore non event elements
  if(!elem.IsEvent()) return;
  // set it
  //FD_DQN("PioNameSetView::Show(): " << pVioModel->ElementStr(elem));
  mSymbol=pVioModel->SymbolicName(elem.Ev());
  UpdateView();
}

// set values from selection (defaults to single element)
void PioNameSetView::ShowSelection(void) {
  // bail out in no model
  if(!pVioModel) return; 
  // default implementation
  FD_DQN("PioNameSetView::ShowSelection(): #" << pVioModel->Selection().size());
  mSymbol="";
  if(pVioModel->Selection().size() ==1) 
    if(pVioModel->Selection().at(0).Type()==VioElement::EEvent)
      mSymbol=pVioModel->SymbolicName(
	pVioModel->Selection().at(0).Event());
  UpdateView();  
}


// clear view
void PioNameSetView::DoClear(void) {
  mSymbol="";
  Name("< none >");
  // silent clr
  bool old=mBlockModelUpdate; 
  mBlockModelUpdate=true;
  mAttribute->Clear();
  mBlockModelUpdate=old;
}


// update view for a given element
void PioNameSetView::DoVioUpdate(void) {
  // bail out in no model
  if(!pVioModel) return; 
  FD_DQN("PioNameSetView::DoVioUpdate() from "<< pVioModel << " with idx " << Idx());
  // set my data: if its one element
  if(pVioModel->Exists(mSymbol)) {
    //mEditName->setCompleter(pVioGenerator->EventCompleter());
    Name(VioStyle::DispEventName(NameSet(),Idx()));
    const faudes::AttributeFlags& attr=pVioModel->Attribute(mSymbol);
    Attribute(attr);
  }
  if(!pVioModel->Exists(mSymbol)) {
    DoClear();
  }  
  // set my data: if its the selection
  if(mSymbol=="") {
    Name("< selection >");    
    AttributeFromSelection();
  }
  FD_DQN("PioNameSetView::DoVioUpdate(): done");
}

// update model
void PioNameSetView::DoModelUpdate(void) {
  // if disconnected, emit uniform signal and return 
  if(!pVioModel) {
    emit NotifyModified(true);
    return;
  }
  // else update model
  bool changed=false;
  // FD_DQN("PioNameSetView::DoModelUpdate: " << pVioModel->ElementStr(mElement));
  // change of name aka move event
  if(pVioModel->Exists(mSymbol)) {
    // get possibly new event
    QString ofname= mSymbol;
    QString nfname= Name();
    // insist in valid symbol
    if(!VioStyle::ValidSymbol(nfname)) nfname=ofname;
    FD_DQN("PioNameSetView::PropertyChanged: (new) event " << VioStyle::StrFromQStr(nfname));
    // avoid doublets
    if(nfname!=ofname && pVioModel->Exists(nfname)) {
      nfname=pVioModel->UniqueSymbol(nfname);
      if(nfname==pVioModel->UniqueSymbol(ofname)) nfname=ofname;
    }
    // actually rename event
    if(nfname != ofname) {
      changed |= pVioModel->ReName(ofname,nfname);
    }
  }
  // change one  attribute 
  if(pVioModel->Exists(mSymbol)) 
  if(!pVioModel->AttributeTest(mSymbol,*mAttribute->Attribute())){
    pVioModel->UndoEditStart();
    changed|= pVioModel->Attribute(mSymbol,*mAttribute->Attribute());
    pVioModel->UndoEditStop();
  }
  // apply to selection
  if(mSymbol=="") {
    pVioModel->UndoEditStart();
    AttributeToSelection();
    changed=true;
    pVioModel->UndoEditStop();
  }
  FD_DQN("PioNameSetView::DoModelUpdate: done with changed " << changed);
  if(changed) emit NotifyModified(true);
};



