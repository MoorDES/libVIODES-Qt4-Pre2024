/* liotseview.cpp  - view on liotselists models  */

/*
   Graphical  IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006 - 2011 Thomas Moor

*/


#include "liotseview.h"
#include "viogenerator.h"
#include "viogenlist.h"
#include "piotseview.h"


/* 
 ******************************************
 ******************************************
 ******************************************

 implementation of LioVView

 ******************************************
 ******************************************
 ******************************************
 */


// constructor
LioVView::LioVView(QWidget* parent) : 
  QTableView(parent),  
  pTableModel(0)
{
  FD_DQG("LioVView::LioVView()");
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
};

// destructor
LioVView::~LioVView(void) {
  FD_DQG("LioVView::~LioVView()");
}


// read only access to faudes generator 
const faudes::vGenerator* LioVView::Generator(void) const {
  if(!pTableModel) return 0;
  return pTableModel->Generator();
};

// convenience access to vio generator model
VioGeneratorModel* LioVView::GeneratorModel(void) {
  if(!pTableModel) return 0;
  return pTableModel->GeneratorModel();
};

// convenience access to vio generator list model
VioGeneratorListModel* LioVView::GeneratorListModel(void) {
  return pTableModel->GeneratorListModel();
};


// reimplement: set model
void LioVView::setModel(LioVList* liolist) {
  FD_DQG("LioVView::setModel()");
  // bail out ob double call
  if(model()==liolist) return;
  // disconnect
  if(pTableModel) disconnect(this,0,pTableModel->GeneratorModel(),0);
  // record refs
  pTableModel=liolist;
  pVioGeneratorModel=pTableModel->GeneratorModel();
  // call base
  QTableView::setModel(pTableModel);
  // connect: click
  connect(this,SIGNAL(MouseClick(const VioElement&)),
    pTableModel->GeneratorModel(), SIGNAL(MouseClick(const VioElement&)));
  // connect: select
  QObject::connect(selectionModel(),SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
    this, SLOT(SelectionUpdate(const QItemSelection&, const QItemSelection&)));
  // todo: connect
  //QObject::connect(pList,SIGNAL(Changed(bool)),
  //this, SIGNAL(Changed(bool)));
  // layout header after setting model: in derived classes
}


// set completers to use
void LioVView::SetStateCompleter(QCompleter* completer) {
  mStateDelegate->setCompleter(completer);
} 

// set completers to use
void LioVView::SetEventCompleter(QCompleter* completer) {
  mEventDelegate->setCompleter(completer);
}


// editing model: selection
void LioVView::UpdateSelectionElement(const VioElement& elem, bool on) {
  if(!pTableModel) return;
  FD_DQG("LioVView::UpdateSelectionElement(" <<elem.Str() << ", " << on << ")");
  QModelIndex index= pTableModel->ModelIndex(elem);
  if(!index.isValid()) return;
  if(on) selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
  else selectionModel()->select(index, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
  FD_DQG("LioVView::UpdateSelectionElement(" <<elem.Str() << ", " << on << "): done");
}

// editing model: selection
void LioVView::UpdateSelectionClear(void) {
  clearSelection();
}

// editing model: selection
void LioVView::UpdateSelectionAny(void) {
  clearSelection();
  if(!GeneratorModel()) return;
  const QList<VioElement> gensel=GeneratorModel()->Selection();
  foreach(const VioElement& elem,gensel) {
    QModelIndex index= pTableModel->ModelIndex(elem);
    if(!index.isValid()) continue;
    selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
  }
}


// show/hide sort indicator
void LioVView::FixSortIndicator(int col) {
  if(col>=mSortEnabled) header()->setSortIndicatorShown(false); 
  else header()->setSortIndicatorShown(true); 
}

// configuration limit sorted cols
void LioVView::SetSortEnabled(int col) {
  mSortEnabled=col;
  QObject::connect(header(), SIGNAL(sectionPressed(int)), 
    this, SLOT(FixSortIndicator(int)));
}

// selected rows
QList<int> LioVView::SelectedRows(void) {
  FD_DQG("LioVView::SelectedRows(): from sel model " << selectionModel());
  QModelIndexList selectedindexes=selectionModel()->selectedIndexes();
  FD_DQG("LioVView::SelectedRows(): indexes #" << selectedindexes.size());
  QList<int> selectedrows;
  foreach(const QModelIndex& index, selectedindexes) {
    if(!index.isValid()) continue;
    if(index.row()<0) continue;
    if(index.row()>=model()->rowCount()) continue;
    if(selectedrows.contains(index.row())) continue;
    selectedrows.append(index.row());
  }
  FD_DQG("LioVView::SelectedRows(): rows #" << selectedrows.size());
  qSort(selectedrows);
  return selectedrows;
}

// we can select an element transition
void LioVView::UserSelect(const QModelIndex& index, bool on) {
  FD_DQG("LioVView::UserSelect()");
  if(!index.isValid()) return;
  if(selectionModel()->isSelected(index) == on) return;
  if(on) selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
  else   selectionModel()->select(index, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
}

// user deleting selection
void LioVView::UserDelSelection(void) {
  FD_DQG("LioVView::UserDelSelection()");
  // remove in my model
  QList<int> selectedrows=SelectedRows();
  if(selectedrows.size()==0) return;
  pVioGeneratorModel->UndoEditStart();
  int off=0;
  QList<int>::iterator rit=selectedrows.begin();
  for(;rit!=selectedrows.end();rit++) {
    model()->removeRow(*rit+off); // incl generator collback
    off--;
  }
  pVioGeneratorModel->UndoEditStop();
}

// user ins element
void LioVView::UserInsElement(void) {
  FD_DQG("LioVView::UserInsElement()");
  // figure location
  int row = -1;
  QModelIndex index = currentIndex();
  if(index.isValid()) row=index.row();
  if(row<0 || row> model()->rowCount()) row=model()->rowCount();
  pVioGeneratorModel->SelectionClear();
  model()->insertRow(row);
  setCurrentIndex(model()->index(row,0));
}


// get key events
void LioVView::keyPressEvent(QKeyEvent *event) {
  FD_DQG("LioVView::keyPressEvent(...): " << event->key());
  // bail out
  if(!pVioGeneratorModel) return;
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
  FD_DQG("LioVView::keyPressEvent(...): row " << row << " col " << column);
  // switch: ignore tab navigation
  if(event->key() == Qt::Key_Tab) {
    event->ignore();
    return;
  } 
  // switch: have space navigation
  if(event->key() == Qt::Key_Space) {
    FD_DQN("LioNameSetView::keyPressEvent(...): space navigation");
    // figure next field (skip checkboxes)
    int erow=row;
    int ecol=column+1;
    QModelIndex edix=model()->index(erow,ecol);
    if( edix.isValid() && ! ( model()->flags(edix) & Qt::ItemIsEditable) ) {
      erow=erow+1;
      ecol=0;
    }
    if(ecol>=model()->columnCount()) {
      erow=erow+1;
      ecol=0;
    }
    QModelIndex next=model()->index(erow,ecol);
    if(!next.isValid()) next=model()->index(0,0);
    if(next.isValid()) {
      GeneratorModel()->SelectionClear();
      setCurrentIndex(next);
    }
    event->accept();
    return;
  } 
  // switch: explicit insert a line
  if( (event->key() == Qt::Key_Insert) || 
      ( (event->key() == Qt::Key_Return) && (event->modifiers() & Qt::ShiftModifier ) ) ) {
    FD_DQG("LioVView::keyPressEvent(...): Insert row " << row);
    mInsertMode=false;
    // retrieve row
    const QList<VioElement> gensel=GeneratorModel()->Selection();
    if(row<0 && gensel.size()==1)  { 
      QModelIndex index= pTableModel->ModelIndex(gensel.at(0));
      row=index.row();
      FD_DQG("LioVView::keyPressEvent(...): Insert below " << gensel.at(0).Str() << " at row " << row );
    }
    pVioGeneratorModel->SelectionClear();
    model()->insertRow(row+1);
    setCurrentIndex(model()->index(row+1,0));
    QTableView::edit(currentIndex());
    event->accept();
    return;
  } 
  // switch: edit current item
  /*
  if( (event->key() == Qt::Key_Return) ) {
    FD_DQG("LioVView::keyPressEvent(...): Next Row");
    QModelIndex next=model()->index(row,column);
    FD_DQG("LioVView::keyPressEvent(...): Next Row ** " << model());
    if(next.isValid() && (model()->flags(next) & Qt::ItemIsEditable) ) {      
      FD_DQG("LioVView::keyPressEvent(...): Next Row at jj " << row);
      pVioGeneratorModel->SelectionClear();
      FD_DQG("LioVView::keyPressEvent(...): Next Row at " << row);
      setCurrentIndex(next);
      QTableView::edit(next);
      FD_DQG("LioVView::keyPressEvent(...): Next Row done ");
    }
    FD_DQG("LioVView::keyPressEvent(...): Next Row ignored ");
    event->accept();
    return;
  } 
  */
  // switch: delete selection
  if( (event->key() == Qt::Key_Delete)  || (event->key() == Qt::Key_Backspace) ) {
    FD_DQG("LioVView::keyPressEvent(...): Delete");
    mInsertMode=false;
    int current=currentIndex().row();
    UserDelSelection();
    pVioGeneratorModel->SelectionClear();
    int next=current -1;
    if(next<0) next=0;
    setCurrentIndex(model()->index(next,0));
    event->accept();
    return;
  } 
  // switch: call base for navigation etc)
  QTableView::keyPressEvent(event);
  mInsertMode=false;
}  

// edit hook
bool LioVView::edit(const QModelIndex& index, EditTrigger trigger, QEvent* event) {
  FD_DQG("LioVView::edit(): at (" << index.row() << ", " << index.column() << ")");
  // sense click to void (todo: put this to focusIn()?)
  if(!index.isValid()) {
    EmitMouseClick(index);
  }
  // record for later use
  mEditIndex=index;
  return QTableView::edit(index,trigger,event);
};


// close edit hook
void LioVView::closeEditor(QWidget* editor, QAbstractItemDelegate::EndEditHint hint) {
  FD_DQG("LioVView::closeEdit(): editor " << editor << " hint: " << hint);
  // edit next
  if(hint==QAbstractItemDelegate::EditNextItem) { 
    // find next item
    QModelIndex cur=mEditIndex; // currentIndex();  
    FD_DQG("LioVView::closeEdit(): edit next at (" << cur.row() << ", " << cur.column() << ")");
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
    /* 
    if(erow>=model()->rowCount()) {
      ecol=0; erow=0;
    }
    */
    // doit
    QTableView::closeEditor(editor,QAbstractItemDelegate::NoHint);
    QModelIndex next=model()->index(erow,ecol);
    FD_DQG("LioVView::closeEdit(): editor " << editor << " running base::edit on (" << next.row() << ", " << next.column() <<")");
    setCurrentIndex(next); 
    QTableView::edit(next);
    FD_DQG("LioVView::closeEdit(): editor " << editor << " done");
    return;
  }
  // escape
  if(hint==QAbstractItemDelegate::RevertModelCache) { 
    // find the item
    QModelIndex cur=mEditIndex; // currentIndex();
    FD_DQG("LioVView::closeEdit(): revert at (" << cur.row() << ", " << cur.column() << ")");
    if(mInsertMode) {
      model()->removeRow(cur.row());
      setCurrentIndex(cur); 
    }
    mInsertMode=false;
    QTableView::closeEditor(editor, hint);
    FD_DQG("LioVView::closeEdit(): editor " << editor << " done");
    return;
  }  
  // other cases
  mInsertMode=false;
  QTableView::closeEditor(editor, hint);
  FD_DQG("LioVView::closeEdit(): editor " << editor << " done");
}

// selection was updated
void LioVView::SelectionUpdate(const QItemSelection& selected, const QItemSelection& deselected) {
  FD_DQG("LioVView::SelectionUpdate");
  // no model
  if(!pVioGeneratorModel) return;
  if(!pTableModel) return;
  // select
  foreach(QModelIndex index, selected.indexes())
    pVioGeneratorModel->Select(pTableModel->FaudesElement(index),true);
  // deselect
  foreach(QModelIndex index, deselected.indexes()) 
    pVioGeneratorModel->Select(pTableModel->FaudesElement(index),false);
}


// track focus
void LioVView::focusInEvent(QFocusEvent* event) {
  (void) event;
  FD_DQG("LioVView::focusInEvent()");
}
void LioVView::focusOutEvent(QFocusEvent* event) {
  (void) event;
  FD_DQG("LioVView::focusOutEvent()");
};


// emit mouse press to model
void LioVView::EmitMouseClick(const QModelIndex& index) {
  if(!pTableModel) return;
  FD_DQG("LioVView::EmitMouseClick(...): row " << index.row() << " col " << index.column());
  VioElement elem= pTableModel->FaudesElement(index);
  FD_DQG("LioVView::EmitMouseClick(...): emit MouseClick " << elem.Str());
  emit MouseClick(elem);  
}


// contextmenu
void LioVView::ContextMenuAtPoint(QPoint pos) {
  FD_DQG("LioVView::ContextMenuAtPoint("<< pos.x()<<", "<<pos.y() << ")");
  QModelIndex index=indexAt(pos);
  if(QTableView* widget=qobject_cast<QTableView*>(sender())) {
    pos=widget->viewport()->mapToGlobal(pos);
  } 
  //else if(QWidget* widget=qobject_cast<QWidget*>(sender())) {
  //  pos=widget->mapToGlobal(pos);
  //}
  if(index.isValid()) {
    ContextMenu(pos,index);
    return;
  }
  // generic insert menu
  FD_DQG("LioVView::ContextMenuAtPoint( no index)");
  setCurrentIndex(index);
  QMenu* menu = new QMenu("Insert");
  QAction* insaction= menu->addAction("Insert Line (Shift+Return)");
  QAction *selaction = menu->exec(pos);
  if(selaction==insaction){
    UserInsElement();
  }
  // done
  delete menu;
  FD_DQG("LioTView::transContextMenu: done ");
}


// contextmenu
void LioVView::ContextMenu(QPoint pos, const QModelIndex& index) {
  (void) pos;
  (void) index;
  FD_DQG("LioVView::ContextMenuAt("<< index.row() <<", "<<index.column() << "): dummy");
}

/* 
 ******************************************
 ******************************************
 ******************************************

 implementation of LioTView

 ******************************************
 ******************************************
 ******************************************
 */


// constructor
LioTView::LioTView(QWidget* parent) : LioVView(parent) {
  FD_DQG("LioTView::LioTView()");
  setItemDelegateForColumn(0,mStateDelegate);
  setItemDelegateForColumn(1,mEventDelegate);
  setItemDelegateForColumn(2,mStateDelegate);
  QObject::connect(this,SIGNAL(pressed(const QModelIndex&)), 
    this, SLOT(EmitMouseClick(const QModelIndex&)));
  verticalHeader()->hide();
};

// destructor
LioTView::~LioTView(void) {
  FD_DQG("LioTView::~LioTView()");
}


// reimplement: set model
void LioTView::setModel(LioVList* liolist) {
  FD_DQG("LioTView::setModel()");
  // bail out on double call
  if(model()==liolist) return;
  // bail out on type mismatch
  if(!qobject_cast<LioTList*>(liolist)) return;
  if(model()==liolist) return;
  // call base
  LioVView::setModel(liolist);
  // layout header after setting model
  header()->setResizeMode(QHeaderView::Interactive);
  header()->setStretchLastSection(true); 
  SetSortEnabled(3);
}




// we can select a transition (us notifying viogenerator)
/*
void LioTView::SelectTrans(const QModelIndex& index, bool on) {
  FD_DQG("LioTView::SelectTrans(" <<index.row() << ", " << index.column() << ", " << on <<")");
  if(mTransList->hasFaudesItem(index)) {
    faudes::Transition ftrans = mTransList->faudesItem(index);
    pVioGenerator->SelectTrans(ftrans,on);
  }
}
*/

// contextmenu
void LioTView::ContextMenu(QPoint pos, const QModelIndex& index) {
  FD_DQG("LioTView::ContextMenuAt("<< index.row() <<", "<<index.column() << "): dummy");
  if(!index.isValid()) return; 
  // setup property widget menu
  QMenu* menu = new QMenu("Transition");
  PioTProp* transwidget = new PioTProp(menu,pVioGeneratorModel->Configuration());
  transwidget->GeneratorModel(pVioGeneratorModel);
  transwidget->ShowSelection();
  QWidgetAction* transaction= new QWidgetAction(menu);
  transaction->setDefaultWidget(transwidget);
  menu->addAction(transaction);
  // add my buttons
  menu->addSeparator();
  QAction* deleteaction= menu->addAction("Delete Transition(s)");
  QAction* insaction= menu->addAction("Insert Transition");
  // run menu
  QAction *selaction = menu->exec(pos);
  if(selaction==deleteaction){
    UserDelSelection();
  }
  if(selaction==insaction){
    UserInsElement();
  }
  // done
  delete menu;
  FD_DQG("LioTView::transContextMenu: done ");
}





/* 
 ******************************************
 ******************************************
 ******************************************

 implementation of LioSView

 ******************************************
 ******************************************
 ******************************************
 */

// constructor
LioSView::LioSView(QWidget* parent) : LioVView(parent) {
  FD_DQG("LioSView::LioSView()");
  setItemDelegateForColumn(0,mStateDelegate);
  QObject::connect(this,SIGNAL(pressed(const QModelIndex&)), 
    this, SLOT(EmitMouseClick(const QModelIndex&)));
  verticalHeader()->hide();
};

// destructor
LioSView::~LioSView(void) {
  FD_DQG("LioSView::~LioSView()");
}


// reimplement: set model
void LioSView::setModel(LioVList* liolist) {
  FD_DQG("LioSView::setModel()");
  // bail out on double call
  if(model()==liolist) return;
  // bail out on type mismatch
  if(!qobject_cast<LioSList*>(liolist)) return;
  // call base
  LioVView::setModel(liolist);
  // layout header after setting model
  header()->setResizeMode(0,QHeaderView::Stretch);
  for(int col=1; col<pTableModel->columnCount(); col++)
    header()->setResizeMode(col,QHeaderView::ResizeToContents);
  SetSortEnabled(1);
}

// contextmenu
void LioSView::ContextMenu(QPoint pos, const QModelIndex& index) {
  FD_DQG("LioSView::ContextMenuAt("<< index.row() <<", "<<index.column() << "): dummy");
  if(!index.isValid()) return;
  // setup property widget menu
  QMenu* menu = new QMenu("State");
  PioSProp* statewidget = new PioSProp(menu,pVioGeneratorModel->Configuration());
  statewidget->GeneratorModel(pVioGeneratorModel);
  statewidget->ShowSelection();
  QWidgetAction* stateaction= new QWidgetAction(menu);
  stateaction->setDefaultWidget(statewidget);
  menu->addAction(stateaction);
  // add my buttons
  menu->addSeparator();
  QAction* deleteaction= menu->addAction("Delete State(s)");
  QAction* insaction= menu->addAction("Insert State");
  // run menu
  QAction *selaction = menu->exec(pos);
  if(selaction==deleteaction){
    UserDelSelection();
  }
  if(selaction==insaction){
    UserInsElement();
  }
  // done
  delete menu;
  FD_DQG("LioSView::transContextMenu: done ");
}


/* 
 ******************************************
 ******************************************
 ******************************************

 implementation of LioEView

 ******************************************
 ******************************************
 ******************************************
 */

// constructor
LioEView::LioEView(QWidget* parent) : LioVView(parent) {
  FD_DQG("LioEView::LioEView()");
  setItemDelegateForColumn(0,mEventDelegate);
  QObject::connect(this,SIGNAL(pressed(const QModelIndex&)), 
    this, SLOT(EmitMouseClick(const QModelIndex&)));
  verticalHeader()->hide();
};

// destructor
LioEView::~LioEView(void) {
  FD_DQG("LioEView::~LioEView()");
}


// reimplement: set model
void LioEView::setModel(LioVList* liolist) {
  FD_DQG("LioEView::setModel()");
  // bail out on double call
  if(model()==liolist) return;
  // bail out on type mismatch
  if(!qobject_cast<LioEList*>(liolist)) return;
  // call base
  LioVView::setModel(liolist);
  // layout header after setting model
  header()->setResizeMode(0,QHeaderView::Stretch);
  for(int col=1; col<pTableModel->columnCount(); col++)
    header()->setResizeMode(col,QHeaderView::ResizeToContents);
  SetSortEnabled(1);
}


// contextmenu
void LioEView::ContextMenu(QPoint pos, const QModelIndex& index) {
  FD_DQG("LioEView::ContextMenuAt("<< index.row() <<", "<<index.column() << "): dummy");
  if(!index.isValid()) return;
  // setup property widget menu
  QMenu* menu = new QMenu("Event");
  PioEProp* eventwidget = new PioEProp(menu,pVioGeneratorModel->Configuration());
  eventwidget->GeneratorModel(pVioGeneratorModel);
  eventwidget->ShowSelection();
  QWidgetAction* eventaction= new QWidgetAction(menu);
  eventaction->setDefaultWidget(eventwidget);
  menu->addAction(eventaction);
  // add my buttons
  menu->addSeparator();
  QAction* deleteaction= menu->addAction("Delete Event(s)");
  QAction* insaction= menu->addAction("Insert Event");
  // run menu
  QAction *selaction = menu->exec(pos);
  if(selaction==deleteaction){
    UserDelSelection();
  }
  if(selaction==insaction){
    UserInsElement();
  }
  // done
  delete menu;
  FD_DQG("LioEView::transContextMenu: done ");
}

