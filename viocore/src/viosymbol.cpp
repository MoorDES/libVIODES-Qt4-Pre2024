/* viosymbol.cpp  - symbol line editors and friends  */

/*
   Graphical  IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2008  Thomas Moor, Klaus Schmidt, Sebastian Perk

*/


#include "viosymbol.h"



/* 
 ******************************************
 ******************************************
 ******************************************

 implementation of symbol validator

 ******************************************
 ******************************************
 ******************************************
 */

// construct
VioSymbolValidator::VioSymbolValidator(QObject* parent) :  
  QValidator(parent), mSymbolMode(VioSymbol::DefaultMode), pCompleter(0) {
};

// destruct
VioSymbolValidator::~VioSymbolValidator(void) {  
  FD_DQ("VioSymbolValidator::~VioSymbolValidator("<<this<<")");
};

// enable fake / known / etc
void VioSymbolValidator::setSymbolMode(VioSymbol::Mode mode) {
  mSymbolMode=mode;
}

// set completer to validate known symbols
void VioSymbolValidator::setCompleter(QCompleter* completer) {
  pCompleter=completer;
}

// validate
VioSymbolValidator::State VioSymbolValidator::validate(QString& input, int & pos) const {
  (void) pos;
  // have copy
  QString cand=input;
  // validate any string
  if(mSymbolMode & VioSymbol::AnyString) {
    if(cand.length()>0) return Acceptable;
    return Intermediate;
  }
  // validate fake symbol
  if(cand.indexOf("#")==0 && (mSymbolMode & VioSymbol::FakeSymbols)) {
    cand.remove(0,1);
    cand=cand.trimmed();
    if(cand=="") return Intermediate;
    bool ok=true;
    cand.toULong(&ok,10);
    if(ok) return Acceptable;
    return Invalid;
  }
  // validate faudes symbol 
  if(!(mSymbolMode & VioSymbol::KnownSymbolsOnly)) {
    if(input=="") return Intermediate;
    if(faudes::SymbolTable::ValidSymbol(VioStyle::StrFromQStr(input))) return Acceptable;
    return Invalid;
  }
  // validate known faudes symbol 
  if(input=="") return Intermediate;
  if(!faudes::SymbolTable::ValidSymbol(VioStyle::StrFromQStr(input))) return Invalid;
  if(pCompleter) 
    if(QStringListModel* strlist=qobject_cast<QStringListModel*>(pCompleter->model())) {
      if(strlist->stringList().contains(input)) 
        return Acceptable;
    }
  // todo: use completer to figure intermediate
  return Intermediate;
};


/* 
 ******************************************
 ******************************************
 ******************************************

 implementation of symbol line edit

 ******************************************
 ******************************************
 ******************************************
 */


// construct
VioSymbolEdit::VioSymbolEdit(QWidget *parent) : 
  QWidget(parent), pSymbolTable(0), mSymbolMode(VioSymbol::DefaultMode), pCompleter(0)  
{
  FD_DQ("VioSymbolEdit::VioSymbolEdit("<<this<<")");
  // stacked widget with line edit default
  mStack = new QStackedWidget();
  mLineEdit=new QLineEdit();
  mComboBox=new QComboBox();
  mComboBox->setEditable(true);
  mStack->addWidget(mComboBox);
  mStack->addWidget(mLineEdit);
  mVbox = new QVBoxLayout(this);
  mVbox->setMargin(0);
  mVbox->setSpacing(0);
  mVbox->addWidget(mStack);
  // have focus (will pass it to proxy anyway)
  setFocusPolicy(Qt::StrongFocus);
  // have my own validator
  mValidator= new VioSymbolValidator(this);
  // further initialisation by set up mode
  setSymbolMode(VioSymbol::DefaultMode);  
  // record my palette
  mPalette= mLineEdit->palette();
  // connect signals to uniform extern interface
  //connect(mLineEdit,SIGNAL(textEdited(const QString&)),this,SIGNAL(textChanged(const QString)));
  connect(mLineEdit,SIGNAL(returnPressed(void)),this,SIGNAL(returnPressed(void)));
  connect(mComboBox,SIGNAL(editTextChanged(const QString&)),this,SIGNAL(textChanged(const QString)));
  connect(mComboBox,SIGNAL(activated(const QString&)),this,SIGNAL(textChanged(const QString)));
  connect(mComboBox,SIGNAL(activated(const QString&)),this,SIGNAL(returnPressed()));
  connect(mComboBox->lineEdit(),SIGNAL(returnPressed()),this,SIGNAL(returnPressed()));
  // connect for internal validation
  //connect(mLineEdit,SIGNAL(textEdited(const QString&)),this,SLOT(validate()));
  connect(mComboBox->lineEdit(),SIGNAL(textEdited(const QString&)),this,SLOT(validate()));
  FD_DQ("VioSymbolEdit::VioSymbolEdit(): done");
};


// destruct
VioSymbolEdit::~VioSymbolEdit(void) {  
  FD_DQ("VioSymbolEdit::~VioSymbolEdit("<<this<<")");
};

// set/initialise symbolmode
void VioSymbolEdit::setSymbolMode(VioSymbol::Mode mode) {
  FD_DQ("VioSymbolEdit::setSymbolMode(" << mode << ")");
  // record mode
  mSymbolMode=mode;
  // pass on to my validator and completer
  mValidator->setSymbolMode(mode);
  /*
  if(VioSymbolCompleter* vscompleter=qobject_cast<VioSymbolCompleter*>(pCompleter)) {
    vscompleter->setSymbolMode(mode);
  }
  */
  // remove completer if no such
  if(!pCompleter) {  
    mLineEdit->setCompleter(0);
    mComboBox->lineEdit()->setCompleter(0);
  }
  // let validator know valid symbols
  if(pCompleter)   
  if(mSymbolMode & VioSymbol::KnownSymbolsOnly) {
    mValidator->setCompleter(pCompleter);
  }
  // remove previous event filters from my editor widgets
  mLineEdit->removeEventFilter(this);
  mComboBox->removeEventFilter(this);
  mComboBox->view()->removeEventFilter(this);
  mComboBox->lineEdit()->removeEventFilter(this);
  // initialise edit widget and put to front
  if(!(mSymbolMode & VioSymbol::ComboBox)) {
    if(pCompleter)  mLineEdit->setCompleter(pCompleter);
    mLineEdit->setValidator(mValidator);
    mLineEdit->setReadOnly(mSymbolMode & VioSymbol::ReadOnly);
    mLineEdit->installEventFilter(this);
    mLineEdit->setMinimumHeight(20);
    mStack->setCurrentWidget(mLineEdit);  
  }
  if(mSymbolMode & VioSymbol::ComboBox) {
    if(pCompleter)   mComboBox->lineEdit()->setCompleter(pCompleter);
    mComboBox->lineEdit()->setValidator(mValidator);
    mComboBox->lineEdit()->setReadOnly(mSymbolMode & VioSymbol::ReadOnly);
    mComboBox->setEnabled(!(mSymbolMode & VioSymbol::ReadOnly));
    mComboBox->setDuplicatesEnabled(false);
    mComboBox->view()->setMinimumHeight(20);
    mComboBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    mComboBox->installEventFilter(this);
    mComboBox->view()->installEventFilter(this);
    mComboBox->lineEdit()->installEventFilter(this);
    mStack->setCurrentWidget(mComboBox);
  }
  // focus proxy
  setFocusProxy(mStack->currentWidget());
  FD_DQ("VioSymbolEdit::setSymbolMode(): done");
}


// set completer
void VioSymbolEdit::setCompleter(QCompleter* completer) {
  pCompleter=completer;
  setSymbolMode(mSymbolMode);
}


// set symbol table
void VioSymbolEdit::setSymbolTable(faudes::SymbolTable* symtab) {
  pSymbolTable=symtab;
  setSymbolMode(mSymbolMode);
}

// set text
void VioSymbolEdit::setText(QString symbol) {
  FD_DQ("VioSymbolEdit::setText(" << VioStyle::StrFromQStr(symbol) << ")");
  if(!(mSymbolMode & VioSymbol::ComboBox)) 
    mLineEdit->setText(symbol);
  else {
    int pos=mComboBox->findText(symbol);
    if(pos<0) mComboBox->addItem(symbol);
    pos=mComboBox->findText(symbol);
    if(pos>=0) mComboBox->setCurrentIndex(pos);
  }
  //validate();
  FD_DQ("VioSymbolEdit::setText(): done");
}
  
// set text
void VioSymbolEdit::setSymbol(QString symbol) {
  // totally relaxed
  if(mSymbolMode & VioSymbol::AnyString) {
    setText(symbol);
    return;
  }
  // incl fake symbols
  if(mSymbolMode & VioSymbol::FakeSymbols) {
    if(!VioStyle::ValidFakeSymbol(symbol)) symbol="";
    setText(symbol);
    return;
  }
  // strict
  if(!VioStyle::ValidSymbol(symbol)) symbol="";
  setText(symbol);
}

// set text
void VioSymbolEdit::setSymbol(const std::string& symbol) {
  setSymbol(VioStyle::QStrFromStr(symbol));
}

// set text
void VioSymbolEdit::setIndex(faudes::Idx idx) {
  setText(VioStyle::SymbolFromIdx(idx,pSymbolTable));
}

// get text
QString VioSymbolEdit::text(void) {
  QString res;
  if(!(mSymbolMode & VioSymbol::ComboBox)) 
    res=mLineEdit->text();
  else 
    res=mComboBox->currentText();
  FD_DQ("VioSymbolEdit::text("<< this << "):" << VioStyle::StrFromQStr(res));
  return res;
}

// get text
QString VioSymbolEdit::symbol(void) {
  return text();
}

// get index
faudes::Idx VioSymbolEdit::index(void) {
  return VioStyle::IdxFromSymbol(symbol(),pSymbolTable);  
}

// set/get model index (for reference only)
void VioSymbolEdit::setModelIndex(const QModelIndex& index) {
  mModelIndex=index;
}
QModelIndex VioSymbolEdit::modelIndex(void) {
  return mModelIndex;
}

// selcet all
void VioSymbolEdit::selectAll(void) {
  mLineEdit->selectAll();
  mComboBox->lineEdit()->selectAll();
}

// since i have a focus proxy, i never get a key
void VioSymbolEdit::keyPressEvent(QKeyEvent* event){
  FD_DQ("VioSymbolEdit::keyPressEvent(" << this << " ): "  << event->key());
  QWidget::keyPressEvent(event);
}

// since i have a focus proxy, i never get focus
void VioSymbolEdit::focusInEvent(QFocusEvent *event) {
  FD_DQ("VioSymbolEdit::focusInEvent(" << this << " ): updating");
  if(VioSymbolCompleter* vscompleter= qobject_cast<VioSymbolCompleter*>(pCompleter)) 
    vscompleter->Update();
  QWidget::focusInEvent(event);
}; 


// debug
bool VioSymbolEdit::event(QEvent* ev) {
  //FD_DQ("VioSymbolEdit::event()");
  return QWidget::event(ev);
}

// provide event filter for my editor widgets (lineedit and combobox) to sense focus in
bool VioSymbolEdit::eventFilter(QObject *obj, QEvent *event) {

  // run original filter
  // return QWidget::eventFilter(obj,event);

  // other objects use whatever this stacked widget wants to filter
  if((obj != mLineEdit) && (obj != mComboBox) && (obj != mComboBox->view()) && (obj != mComboBox->lineEdit()) ) 
    return QWidget::eventFilter(obj, event);
  // if it is a navigation key: filter out, pass on to delegate for navigation
  if(event->type() == QEvent::KeyPress) {
    QKeyEvent *keyevent = static_cast<QKeyEvent*>(event);
    if((keyevent->key() == Qt::Key_Space) && !(mSymbolMode & VioSymbol::AnyString)) {
      FD_DQ("VioSymbolEdit::eventFilter(): pass on navigation");
      event->ignore();
      return true;
    }
    if((keyevent->key() == Qt::Key_Left) && (obj == mLineEdit)) {
      if(mLineEdit->cursorPosition()<=0) {
        FD_DQ("VioSymbolEdit::eventFilter(): pass on navigation");
        event->ignore();
        return true;
      }
    }
    if((keyevent->key() == Qt::Key_Right) && (obj == mLineEdit)) {
      if(mLineEdit->cursorPosition()>=mLineEdit->text().length()) {
        FD_DQ("VioSymbolEdit::eventFilter(): pass on navigation");
        event->ignore();
        return true;
      }
    }
  }
  // if it some other key: report and accept
  if(event->type() == QEvent::KeyPress) {
    //QKeyEvent *keyevent = static_cast<QKeyEvent*>(event);
    //FD_DQ("VioSymbolEdit::eventFilter(): got key " << keyevent->key());
    event->accept();
    return false;
  }
  // if it is a focus in, update their completers, pass on to editor widget
  if(event->type() == QEvent::FocusIn) {
    QFocusEvent *focusevent = static_cast<QFocusEvent*>(event);
    if(focusevent->gotFocus()) {
      if(VioSymbolCompleter* vscompleter= qobject_cast<VioSymbolCompleter*>(pCompleter)) {
        FD_DQ("VioSymbolEdit::eventFilter(): got focus: update completer");
        vscompleter->Update();
        if(obj == mComboBox) {
          // note: this is ineffivcient ... should have fake model on combo model
          FD_DQ("VioSymbolEdit::eventFilter(): got focus: update combobox: " <<
	    " keep " << VioStyle::StrFromQStr(mComboBox->currentText()) << 
            " expecting #" << vscompleter->symbolWorld()->stringList().size());
	  foreach(QString entry, vscompleter->symbolWorld()->stringList()) 
            if(mComboBox->findText(entry)<0) mComboBox->addItem(entry);
        } 
      }    
    };    
    //event->ignore();
    return QWidget::eventFilter(obj,event);
  }
  // if it is a focus out, report
  if(event->type() == QEvent::FocusOut) {
    //QFocusEvent *focusevent = static_cast<QFocusEvent*>(event);
    //FD_DQ("VioSymbolEdit::eventFilter(): lost focus: why? (4<>closed popup)" << focusevent->reason());
  }
  // run original filter
  return QWidget::eventFilter(obj,event);
}


// be neat
bool VioSymbolEdit::validate(void) {
  // are we valid?
  QString line=text();
  int pos;
  bool valid=  (mValidator->validate(line,pos)==QValidator::Acceptable);
  // are we known to the completer?
  bool known=false;
  if(pCompleter) {
    if(QStringListModel* strlist=qobject_cast<QStringListModel*>(pCompleter->model())) {
      if(strlist->stringList().contains(text())) known=true;
    }
  }
  // report
  FD_DQ("VioSymbolEdit::validate(); " << VioStyle::StrFromQStr(text()) << " valid " << valid << " known " << known );
  // set colors
  QPalette pal=mPalette;
  if(!valid) pal.setColor(QPalette::Text, VioStyle::Color(VioRed));  
  if(!known && (mSymbolMode & VioSymbol::KnownSymbolsOnly)) pal.setColor(QPalette::Text, VioStyle::Color(VioGreen));  
  mLineEdit->setPalette(pal);
  mComboBox->setPalette(pal);
  return valid;
}


/* 
 ******************************************
 ******************************************
 ******************************************

 implementation of symbol completer

 ******************************************
 ******************************************
 ******************************************
 */

// construct
VioSymbolCompleter::VioSymbolCompleter(QObject *parent) : QCompleter(parent) {
  mSymbolWorld= new QStringListModel(0);
  setModel(mSymbolWorld);
  pSymbolSource=0;
  setCompletionMode(QCompleter::InlineCompletion);
}

// destruct
VioSymbolCompleter::~VioSymbolCompleter(void) {
  FD_DQ("VioSymbolCompleter:::~VioSymbolCompleter()");
  setModel(0);
  delete mSymbolWorld;
  FD_DQ("VioSymbolCompleter:::~VioSymbolCompleter(): done");
}

// source: list of strings
void VioSymbolCompleter::setSymbolWorld(const QStringList& rStringList) {
  FD_DQ("VioSymbolCompleter::setSymbolWorld("<<this << "): from stringlist");
  clrSymbolWorld();
  mSymbolWorld->setStringList(rStringList);
  setModel(mSymbolWorld);
  FD_DQ("VioSymbolCompleter::setSymbolWorld("<<this << "): from stringlist: done #" << rStringList.size());
};

// source: event set
void VioSymbolCompleter::setSymbolWorld(const faudes::EventSet& rEventSet) {
  FD_DQ("VioSymbolCompleter::setSymbolWorld("<<this << "): from eventset");
  QStringList strings;
  VioStyle::EventsQStrList(strings,&rEventSet);
  setSymbolWorld(strings);
};


// source: other model
void VioSymbolCompleter::setSymbolWorld(QAbstractItemModel* pStringModel, int col) {
  clrSymbolWorld();
  pSymbolSource=pStringModel;
  mSymbolSourceColumn=col;
  Update();
};

// update from source
void VioSymbolCompleter::Update(void) {
  if(!pSymbolSource) return;
  FD_DQ("VioSymbolCompleter::Update("<<this << "): from model with #" << pSymbolSource->rowCount()
	<< " at column " << mSymbolSourceColumn);
  // careful resize/update since my model can be usd somewhere else to
  if(mSymbolWorld->rowCount() > pSymbolSource->rowCount())
    mSymbolWorld->removeRows(pSymbolSource->rowCount(),
      mSymbolWorld->rowCount()-pSymbolSource->rowCount());
  if(mSymbolWorld->rowCount() < pSymbolSource->rowCount())
    mSymbolWorld->insertRows(mSymbolWorld->rowCount(),
      pSymbolSource->rowCount()-mSymbolWorld->rowCount());
  QModelIndex source, dest;
  int count=0;
  for(int row=0; row < pSymbolSource->rowCount(); row++) {
    source=pSymbolSource->index(row,mSymbolSourceColumn);
    if(!source.isValid()) continue;
    QVariant symbol= pSymbolSource->data(source);
    if(!VioStyle::ValidSymbol(symbol.toString())) continue;
    dest = mSymbolWorld->index(count);
    if(!dest.isValid()) continue;
    mSymbolWorld->setData(dest,symbol);
    count++;
    FD_DQ("VioSymbolCompleter::Update("<<this << "): symbol " << 
	  VioStyle::StrFromQStr(symbol.toString()));
  } 
  // todo: we should (optionaly) run sort on it ...  
  mSymbolWorld->removeRows(count,mSymbolWorld->rowCount()-count);
  setModel(mSymbolWorld);
  FD_DQ("VioSymbolCompleter::Update("<<this << "): results in #" << mSymbolWorld->rowCount());
}

// release mem
void VioSymbolCompleter::clrSymbolWorld(void) {
  setModel(0);
  mSymbolWorld->removeRows(0,mSymbolWorld->rowCount());
  pSymbolSource=0;
};




/* 
 ******************************************
 ******************************************
 ******************************************

 implementation of symbol delegate

 ******************************************
 ******************************************
 ******************************************
 */



// construct
VioSymbolDelegate::VioSymbolDelegate(QObject *parent) 
  : QItemDelegate(parent) , pCompleter(0), mSymbolMode(VioSymbol::DefaultMode)
{
  FD_DQ("VioSymbolDelegate::VioSymbolDelegate("<<this << ")");
}

// set symbolmode
void VioSymbolDelegate::setSymbolMode(VioSymbol::Mode mode) {
  mSymbolMode=mode;
}


// tell
QString VioSymbolDelegate::symbol(QWidget *editor) {
  VioSymbolEdit* symedit = static_cast<VioSymbolEdit*>(editor);
  return symedit->symbol();
}

// tell
faudes::Idx VioSymbolDelegate::index(QWidget *editor) {
  VioSymbolEdit* symedit = static_cast<VioSymbolEdit*>(editor);
  return symedit->index();
}

// tell
QModelIndex VioSymbolDelegate::modelIndex(QWidget *editor) {
  VioSymbolEdit* symedit = static_cast<VioSymbolEdit*>(editor);
  return symedit->modelIndex();
}

// set completor
void VioSymbolDelegate::setCompleter(QCompleter* completer) {
  FD_DQ("VioSymbolDelegate::setCompleter("<<this << "): " << completer);
  pCompleter=completer;
}

// create editor widget
QWidget *VioSymbolDelegate::createEditor(QWidget *parent,
  const QStyleOptionViewItem& option,
  const QModelIndex& index) const
{
  (void) option; (void) index;
  FD_DQ("VioSymbolDelegate::createEditor("<<this << ")");
  VioSymbolEdit*  editor = new VioSymbolEdit(parent);
  QString symbol = index.model()->data(index, Qt::DisplayRole).toString();
  editor->setSymbolMode(mSymbolMode);
  editor->setCompleter(pCompleter);
  editor->setModelIndex(index);
#ifdef FAUDES_DEBUG_VIO
  if(pCompleter)  
    FD_DQ("VioSymbolDelegate::createEditor("<<this << "): " << editor << " with completer " << pCompleter 
	<< " #" << pCompleter->model()->rowCount());
#endif
  return editor;
}

// set line edit with data
void VioSymbolDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
  QString symbol = index.model()->data(index, Qt::DisplayRole).toString();
  VioSymbolEdit* symedit = static_cast<VioSymbolEdit*>(editor);
  symedit->setSymbol(symbol);
  symedit->selectAll();
}

// set model from line edit
void VioSymbolDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
  const QModelIndex &index) const
{
  VioSymbolEdit* symedit = static_cast<VioSymbolEdit*>(editor);
  QString symbol= symedit->symbol();
  model->setData(index, symbol);
}

// fix my geometry
void VioSymbolDelegate::updateEditorGeometry(QWidget *editor,
  const QStyleOptionViewItem &option, const QModelIndex& index ) const
{
  (void) option; (void) index;
  editor->setGeometry(option.rect);
}


// find my navigation keys and pass to view
// note: this filter gets automatically installed on the editor widget
// note: the editor widget here is the plain widget, not the combobox or lineedit
bool VioSymbolDelegate::eventFilter(QObject *obj, QEvent *event) {

  // inactive
  //return QItemDelegate::eventFilter(obj, event);

  // track parent (should be a view)
  /*
  if(obj==parent())  {
    // close on focus loss
    if(event->type() == QEvent::FocusOut)  {
      FD_DQ("VioSymbolDelegate::eventFilter(...): parent focus");
      foreach(QObject* child,parent()->children()) {
        if(VioSymbolEdit* editor=qobject_cast<VioSymbolEdit*>(child)) {
          FD_DQ("VioSymbolDelegate::eventFilter(...): parent focus: close editor");
          emit commitData(editor);
          emit closeEditor(editor, QAbstractItemDelegate::NoHint);
        }
      }
    }
    // be careful not to filter anything out
    return false;
  }
  */

  // filter only in my editor
  VioSymbolEdit* editor= qobject_cast<VioSymbolEdit*>(obj);
  if(!editor) 
     return QItemDelegate::eventFilter(obj, event);
  // report
  // FD_DQ("VioSymbolDelegate::eventFilter(...): on my editor");
  // filter only keys ...
  if(!(event->type() == QEvent::KeyPress)) 
    return QItemDelegate::eventFilter(obj, event);
  // report
  QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
  FD_DQ("VioSymbolDelegate::eventFilter(...): on my editor, key " << keyEvent->key());
  // plain navigation: commit, filter out, pass on to tableview
  if(
     ((keyEvent->key() == Qt::Key_Return) && (keyEvent->modifiers() & Qt::ShiftModifier)) ||
     (keyEvent->key() == Qt::Key_Up) ||
     (keyEvent->key() == Qt::Key_Down) ||
     (keyEvent->key() == Qt::Key_Left) ||
     (keyEvent->key() == Qt::Key_Right) ) 
  {
    FD_DQ("VioSymbolDelegate::eventFilter(...): navigation");
    emit commitData(editor);
    emit closeEditor(editor);
    event->ignore();
    FD_DQ("VioSymbolDelegate::eventFilter(...): navigation: done");
    return true;
  }
  // return/space: edit next: commit, filter out, dont pass on to tableview
  if((keyEvent->key() == Qt::Key_Return) || (keyEvent->key() == Qt::Key_Space)) {
    FD_DQ("VioSymbolDelegate::eventFilter(...): edit next");
    emit commitData(editor);
    emit closeEditor(editor, QAbstractItemDelegate::EditNextItem);
    event->accept();
    FD_DQ("VioSymbolDelegate::eventFilter(...): edit next: done");
    return true;
  }
  // else: std process
  return QItemDelegate::eventFilter(obj, event);
};


// debug
bool VioSymbolDelegate::event(QEvent* ev) {
  //FD_DQ("VioSymbolDelegate::event()");
  return QItemDelegate::event(ev);
}

/*
*******************************************
*******************************************
*******************************************

implementation of symbol table widget

*******************************************
*******************************************
*******************************************
*/

// constructor
VioSymbolTableWidget::VioSymbolTableWidget(QWidget* parent) : QTableView(parent) {
  FD_DQ("VioSymbolTableWidget::VioSymbolTableWidget()");
  // core members
  setModel(&mModel);
  // set bahavior
  setShowGrid(false);         
  setAlternatingRowColors(true);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setSelectionBehavior(QAbstractItemView::SelectRows);
  setTabKeyNavigation(true);
  verticalHeader()->hide();
  horizontalHeader()->setStretchLastSection(true);
  setSortingEnabled(false);
  // context menu
  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this,SIGNAL(customContextMenuRequested(QPoint)),
      this,SLOT(contextMenuAtPoint(QPoint)));
  // pass on signals
  connect(selectionModel(),SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
	  this,SLOT(EmitCurrentChanged()),Qt::QueuedConnection);
  // default dimension
  mModel.setColumnCount(1);
  mpCompleters.push_back(new VioSymbolCompleter(this));
  mpDelegates.push_back(new VioSymbolDelegate(this));
  pCompleters.push_back(mpCompleters.at(0));
  mpDelegates.at(0)->setCompleter(pCompleters.at(0));
  setItemDelegateForColumn(0,mpDelegates.at(0));
  FD_DQ("VioSymbolTableWidget::VioSymbolTableWidget(): done");
};

// dimensions
void VioSymbolTableWidget::setDimensions(int rows, int cols) {
  FD_DQ("VioSymbolTableWidget::setDimensions(" << rows << ", " << cols << ")");
  // stop sorting
  setSortingEnabled(false);
  // extend columns aka reconfigure
  if(cols!=columnCount()) {
    // clear
    while(mModel.columnCount()>0) mModel.removeColumn(0);
    // adjust width
    mModel.setColumnCount(cols);
    // dispose old completers/delegates
    while(!mpCompleters.isEmpty()) delete mpCompleters.takeLast();
    while(!mpDelegates.isEmpty()) delete mpDelegates.takeLast();
    pCompleters.clear();
    // have new completers/delegates
    while(mpCompleters.size()<cols) mpCompleters.push_back(new VioSymbolCompleter(this));
    while(mpDelegates.size()<cols) mpDelegates.push_back(new VioSymbolDelegate(this));
    // set my completers to be the defaults
    for(int j=0; j<cols; j++) {
      pCompleters.push_back(mpCompleters.at(j));
      mpDelegates.at(j)->setCompleter(pCompleters.at(j));
      setItemDelegateForColumn(j,mpDelegates.at(j));
    }
  }
  // clear model
  if(rows!=rowCount()) {
    // remove all lines
    while(mModel.rowCount()>0) mModel.removeRow(0);
    // set new length
    mModel.setRowCount(rows);
    // fill
    for(int i=0; i< rows; i++) 
    for(int j=0; j< cols; j++) {
      QModelIndex index = mModel.index(i,j);
      mModel.setData(index,QString(""),Qt::EditRole);
    }
  }
  // enable sorting now
  setSortingEnabled(true);
}  

// dimensions (single column convelience)
void VioSymbolTableWidget::setDimension(int rows) {
  setDimensions(rows,1);
}

// get dimensions
int VioSymbolTableWidget::rowCount(void) { return mModel.rowCount(); };
int VioSymbolTableWidget::columnCount(void) { return mModel.columnCount(); };

// set behaviour
void VioSymbolTableWidget::setSymbolMode(int col, int mode) {
  if(col <0 || col >= mModel.columnCount()) return;
  FD_DQ("VioSymbolTableWidget::setSymbolMode(" << col << ", " << mode << ")");
  mpDelegates.at(col)->setSymbolMode((VioSymbol::Mode) mode);
}

// set behaviour (single column convenience)
void VioSymbolTableWidget::setSymbolMode(int mode) {
  setSymbolMode(0,mode);
}

// set base set (must set mode later)
void VioSymbolTableWidget::setSymbolWorld(int col, QCompleter* completer) {
  if(col <0 || col >= mModel.columnCount()) return;
  FD_DQ("VioSymbolTableWidget::setSymbolWorld(" << col << "): by completer");
  pCompleters[col]=completer;
  mpDelegates[col]->setCompleter(completer);
}

// set base set
void VioSymbolTableWidget::setSymbolWorld(int col, const QStringList& rStringList) {
  if(col <0 || col >= mModel.columnCount()) return;
  FD_DQ("VioSymbolTableWidget::setSymbolWorld(): by stringlist");
  mpCompleters[col]->setSymbolWorld(rStringList);
}

// set base set
void VioSymbolTableWidget::setSymbolWorld(int col, const faudes::NameSet& rNameSet) {
  if(col <0 || col >= mModel.columnCount()) return;
  FD_DQ("VioSymbolTableWidget::setSymbolWorld(): by eventset");
  mpCompleters[col]->setSymbolWorld(rNameSet);
}

// set base set
void VioSymbolTableWidget::setSymbolWorld(int col, QAbstractItemModel* pStringModel, int srccol) {
  if(col <0 || col >= mModel.columnCount()) return;
  FD_DQ("VioSymbolTableWidget::setSymbolWorld(): by a model");
  mpCompleters[col]->setSymbolWorld(pStringModel,srccol);
}

// set base set  (single column convenience)
void VioSymbolTableWidget::setSymbolWorld(QCompleter* completer) {
  setSymbolWorld(0,completer);}
// set base set  (single column convenience)
void VioSymbolTableWidget::setSymbolWorld(const QStringList& rStringList) {
  setSymbolWorld(0,rStringList);}
// set base set  (single column convenience)
void VioSymbolTableWidget::setSymbolWorld(const faudes::NameSet& rNameSet) {
  setSymbolWorld(0,rNameSet);}
// set base set  (single column convenience)
void VioSymbolTableWidget::setSymbolWorld(QAbstractItemModel* pStringModel, int srccol) {
  setSymbolWorld(0,pStringModel,srccol);}

 
// set item
void VioSymbolTableWidget::setSymbol(int row, int col, const QString& symbol) {
  if(col <0 || col >= mModel.columnCount()) return;
  if(row <0 || row >= mModel.rowCount()) return;
  FD_DQ("VioSymbolTableWidget::setSymbol("<<row<<","<<col<<","<< VioStyle::StrFromQStr(symbol)<<")");
  QModelIndex index = mModel.index(row,col);
  mModel.setData(index,symbol,Qt::EditRole);
};

// set item (single column)
void VioSymbolTableWidget::setSymbol(int row, const QString& symbol) {
  setSymbol(row,0,symbol);
}

// get item
QString VioSymbolTableWidget::Symbol(int row, int col) const {
  if(col <0 || col >= mModel.columnCount()) return "";
  if(row <0 || row >= mModel.rowCount()) return "";
  QModelIndex index = mModel.index(row,col);
  QString symbol = mModel.data(index,Qt::EditRole).toString();
  return symbol;
};


// set column
void VioSymbolTableWidget::setSymbolColumn(int col, const QStringList& rStringList) {
  if(col <0 || col >= mModel.columnCount()) return;
  FD_DQ("VioSymbolTableWidget::setSymbolColumn(" << col << "): by stringlist");
  // leader col: truncate/extend
  if(col==0) {
    setSortingEnabled(false);
    while(mModel.rowCount()>rStringList.size()) mModel.removeRow(mModel.rowCount()-1);
    mModel.setRowCount(rStringList.size());
    setSortingEnabled(true);
  }
  // copy items
  for(int pos=0; pos < rStringList.size(); pos ++) {
    QModelIndex index = mModel.index(pos,col);
    mModel.setData(index,rStringList[pos],Qt::EditRole);
  }
};

// set column (single column version)
void VioSymbolTableWidget::setSymbolList(const QStringList& rStringList) {
  setSymbolColumn(0,rStringList);
}

// get strings
QStringList VioSymbolTableWidget::symbolColumn(int col) const {
  QStringList symbollist;
  if(col <0 || col >= mModel.columnCount()) return symbollist;
  for(int row=0; row < mModel.rowCount(); row++) {
    QModelIndex index = mModel.index(row,col);
    symbollist.push_back(mModel.data(index,Qt::EditRole).toString());
  }
  return symbollist;
};

// get strings (single column)
QStringList VioSymbolTableWidget::symbolList() const {
  return symbolColumn(0);
}

// get strings as event set
faudes::EventSet VioSymbolTableWidget::eventSetColumn(int col) const {
  faudes::EventSet res;
  columnToNameSet(col,res);
  return res;
};

// get strings as event set (single column)
faudes::EventSet VioSymbolTableWidget::eventSet(void) const {
  return eventSetColumn(0);
}

// copy strings to name set
void VioSymbolTableWidget::columnToNameSet(int col, faudes::EventSet& rNameSet) const {
  rNameSet.Clear();
  if(col <0 || col >= mModel.columnCount()) return;
  for(int row=0; row < mModel.rowCount(); row++) {
    QModelIndex index = mModel.index(row,col);
    std::string symbol=VioStyle::StrFromQStr(mModel.data(index,Qt::EditRole).toString());
    if(faudes::SymbolTable::ValidSymbol(symbol)) rNameSet.Insert(symbol);
  }
};

// scopy strings to name set (single column)
void VioSymbolTableWidget::toNameSet(faudes::EventSet& rNameSet) const {
  columnToNameSet(0,rNameSet);
}

// set strings from event set
void VioSymbolTableWidget::setEventSetColumn(int col, const faudes::EventSet& rEventSet) {
  FD_DQ("VioSymbolTableWidget::setEventSetColumn(): to eventset " << rEventSet.ToString());
  QStringList strings;
  VioStyle::EventsQStrList(strings,&rEventSet);
  setSymbolColumn(col, strings);
}


// set strings from event set (single column)
void VioSymbolTableWidget::setEventSet(const faudes::EventSet& rEventSet) {
  setEventSetColumn(0,rEventSet);
}


// behaviour: header
void VioSymbolTableWidget::setHeader(QStringList headers) {
  if(headers.size()!= mModel.columnCount()) return;
  FD_DQ("VioSymbolTableWidget::setHeader()");
  mModel.setHorizontalHeaderLabels(headers);
};

// behaviour: header (single column)
void VioSymbolTableWidget::setHeader(QString header) {
  setHeader(QStringList() << header);
}

// set current
void VioSymbolTableWidget::setCurrentRow(int row) {
  setCurrentIndex(model()->index(row,0));
  selectionModel()->clearSelection();
  selectionModel()->select(model()->index(row,0),QItemSelectionModel::Select);
}

// get current
int VioSymbolTableWidget::currentRow(void) {
  return currentIndex().row();
}

// pass on navigation
void VioSymbolTableWidget::EmitCurrentChanged(void) {
  FD_DQ("VioSymbolTableWidget::EmitCurrentChanged(...): " << currentRow());
  emit currentRowChanged(currentRow());
}

// select
void VioSymbolTableWidget::userSelect(const QModelIndex& index, bool on) {
  FD_DQ("VioSymbolTableWidget::userSelect()");
  if(!index.isValid()) return;
  if(selectionModel()->isSelected(index) == on) return;
  if(on) selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
  else   selectionModel()->select(index, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
}


// delete selection
void VioSymbolTableWidget::userDelSelection(void) {
  FD_DQ("VioSymbolTableWidget::userDelSelection()");
  QModelIndexList selectedindexes=selectionModel()->selectedIndexes();
  QList<int> selectedrows;
  foreach(QModelIndex index, selectedindexes) {
    if(!index.isValid()) continue;
    if(index.row()<0) continue;
    if(index.row()>=model()->rowCount()) continue;
    if(selectedrows.contains(index.row())) continue;
    selectedrows.append(index.row());
  }
  FD_DQ("VioSymbolTableWidget::userDelSelection(): #" << selectedrows.size());
  if(selectedrows.size()==0) return;
  qSort(selectedrows);
  int off=0;
  QList<int>::iterator rit=selectedrows.begin();
  for(;rit!=selectedrows.end();rit++) {
    model()->removeRow(*rit+off);
    off--;
  }
  emit resizeModel();
}

// clear selection
void VioSymbolTableWidget::userSelectionClear(void) {
  selectionModel()->clearSelection();
};


// user ins element
void VioSymbolTableWidget::userInsertRow(void) {
  FD_DQ("VioSymbolTableWidget::userInsertRow()");
  int row = -1;
  QModelIndex index = currentIndex();
  if(index.isValid())row=index.row();
  if(row<0 || row> model()->rowCount()) row=model()->rowCount();
  mEditing=true;
  mInsertMode=true;
  userSelectionClear();
  model()->insertRow(row);
  emit resizeModel();
  setCurrentIndex(model()->index(row,0));
  QTableView::edit(currentIndex());
}


// contextmenu
void VioSymbolTableWidget::contextMenuAtPoint(QPoint pos) {
  FD_DQ("VioSymbolTableWidget::contextMenuAtPoint("<< pos.x()<<", "<<pos.y() << ")");
  QModelIndex index=indexAt(pos);
  if(QTableView* widget=qobject_cast<QTableView*>(sender())) {
    pos=widget->viewport()->mapToGlobal(pos);
  } 
  //else if(QWidget* widget=qobject_cast<QWidget*>(sender())) {
  //  pos=widget->mapToGlobal(pos);
  //}
  contextMenu(pos,index);
}

// contextmenu
void VioSymbolTableWidget::contextMenu(QPoint pos, const QModelIndex& index) {
  FD_DQ("VioSymbolTableWidget::contextMenuAt("<< index.row() <<", "<<index.column() << ")");
  // have a menu
  QMenu* menu = new QMenu("Insert");
  QAction* deleteaction= menu->addAction("Delete");
  QAction* insaction= menu->addAction("Insert");
  deleteaction->setEnabled(index.isValid());
  // position
  setCurrentIndex(index);
  // run menu
  QAction *selaction = menu->exec(pos);
  if(selaction==deleteaction){
    userDelSelection();
  }
  if(selaction==insaction){
    userInsertRow();
  }
  // done
  delete menu;
  FD_DQ("VioSymbolTableWidget::ContextMenu: done ");

}



// get key events
void VioSymbolTableWidget::keyPressEvent(QKeyEvent *event) {
  FD_DQ("VioSymbolTableWidget::keyPressEvent(...): " << event->key());
  // inactive
  /*
  QTableView::keyPressEvent(event);
  return;
  */

  // figure where
  int row = -1;
  int column = -1;
  QModelIndex index = currentIndex();
  if(index.isValid()) row=index.row();
  column=index.column();
  // switch: ingnore tab navigation
  if(event->key() == Qt::Key_Tab) {
    event->ignore();
    return;
  } 
  // switch: insert mode
  if( (event->key() == Qt::Key_Insert) || 
      ( (event->key() == Qt::Key_Return) && (event->modifiers() & Qt::ShiftModifier ) ) ) {
    FD_DQ("VioSymbolLisWidget::keyPressEvent(...): Insert at row " << row+1);
    mEditing=true;
    mInsertMode=true;
    userSelectionClear();
    model()->insertRow(row+1);
    emit resizeModel();
    setCurrentIndex(model()->index(row+1,0));
    QTableView::edit(currentIndex());
    event->accept();
    return;
  } 
  // switch: edit next row
  if( (event->key() == Qt::Key_Return) ) {
    FD_DQ("VioSymbolTableWidget::keyPressEvent(...): Next Row");
    if(mEditing || mInsertMode) row=row+1;
    if(mInsertMode) {
      if(row>=model()->rowCount()) row=model()->rowCount();
      model()->insertRow(row);
      emit resizeModel();
    }
    QModelIndex next=model()->index(row,column);
    if(next.isValid() && (mModel.flags(next) & Qt::ItemIsEditable) ) {      
      mEditing=true;
      userSelectionClear();
      setCurrentIndex(next);
      QTableView::edit(currentIndex());
    }
    event->accept();
    FD_DQ("VioSymbolTableWidget::keyPressEvent(...): accept");
    return;
  } 
  // switch: delete selection
  if( (event->key() == Qt::Key_Delete)  || (event->key() == Qt::Key_Backspace) ) {
    FD_DQ("VioSymbolTableWidget::keyPressEvent(...): Delete");
    mInsertMode=false;
    mEditing=false;
    int current=currentIndex().row();
    userDelSelection();
    userSelectionClear();
    setCurrentIndex(model()->index(current,0));
    event->accept();
    return;
  } 
  // switch: call base
  QTableView::keyPressEvent(event);
  mInsertMode=false;
  mEditing=false; 
};  

// edit hook
bool VioSymbolTableWidget::edit(const QModelIndex& index, EditTrigger trigger, QEvent* event) {
  FD_DQ("VioSymbolTableWidget::edit(" << index.row() << ", " << index.column() << ")");
  // record for later use
  mEditIndex=index;
  return QTableView::edit(index,trigger,event);
} 



// edit hook
void VioSymbolTableWidget::closeEditor(QWidget* editor, QAbstractItemDelegate::EndEditHint hint) {
  FD_DQ("VioSymbolTableWidget::closeEditor(..): hint " << hint);
  VioSymbolEdit* vioeditor=qobject_cast<VioSymbolEdit*>(editor);
  //bail out on other editors
  if(!vioeditor) return;
  // figure indes
  QModelIndex index= vioeditor->modelIndex();
  int row=index.row();
  int col=index.column();
  FD_DQ("VioSymbolTableWidget::closeEditor(..): at(" << row<< ", " << col << ")");
  // figure data
  QString symbol = mpDelegates.at(index.column())->symbol(editor);
  FD_DQ("VioSymbolTableWidget::closeEditor(..): symbol " << VioStyle::StrFromQStr(symbol) );
  // remove invalid
  if(symbol=="" && col==0)  model()->removeRow(index.row()); 
  // remove doublets
  for(int row=0; row < model()->rowCount(); row++) {
    if(row==index.row()) continue;
    if(model()->data(model()->index(row,0),Qt::EditRole).toString()!= symbol) continue;  
    FD_DQ("VioSymbolTableWidget::closeEditor(..): removing symbol " << VioStyle::StrFromQStr(symbol) << " in " << row );
    model()->removeRow(row);
  }
  emit editingFinished(row,col);
  // pass on (incl open next)
  QTableView::closeEditor(editor,hint);
}

// todo
void VioSymbolTableWidget::Copy(void) {};
void VioSymbolTableWidget::Paste(void) {};
 
