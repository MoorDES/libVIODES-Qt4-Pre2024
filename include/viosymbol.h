/* viosymbol.h  - symbol line editors and friends  */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2008 Thomas Moor, Klaus Schmidt, Sebastian Perk

*/


#ifndef VIOSYMBOL_H
#define VIOSYMBOL_H


#include "viostyle.h"

// global enum for behaviour config
class VioSymbol {
  public:
  typedef enum { DefaultMode=0x0, ValidSymbols=0x0, FakeSymbols=0x1, AnyString=0x2, KnownSymbolsOnly=0x4, 
	   ComboBox=0x10, ReadOnly=0x20} Mode;
};


/*
 *****************************************************
 *****************************************************

 A VioSymbolValidator is a validator for faudes symbols.
 It accepts proper symbols as well as optional fake
 symbole (e.g. "#67" for an element with index 67
 and no symbolic name).

 *****************************************************
 *****************************************************
 */


class VIODES_API VioSymbolValidator : public QValidator {

Q_OBJECT

public:
  // construct
  VioSymbolValidator(QObject* parent);
  ~VioSymbolValidator(void);

  // configure behaviour
  void setSymbolMode(VioSymbol::Mode mode);
  void setSymbolMode(int mode) { setSymbolMode((VioSymbol::Mode) mode);};
  int symbolMode(void) {return mSymbolMode;};

  // set completer to indicate known symbols
  void setCompleter(QCompleter* completer);

  // reimplement validation
  State validate(QString & input, int & pos) const;
    
protected:
  // fake indicator
  VioSymbol::Mode mSymbolMode;

  // completer to indicate known symbols
  QCompleter* pCompleter;

};



/*
 *****************************************************
 *****************************************************

 A VioSymbolCompleter is a completer that completes
 symboly wrt a given symboltable or faudes set

 *****************************************************
 *****************************************************
 */


class VIODES_API VioSymbolCompleter : public QCompleter {

Q_OBJECT

public:

  // construct/destruct
  VioSymbolCompleter(QObject *parent = 0);
  ~VioSymbolCompleter(void);

  // alternative setting for source
  void setSymbolWorld(const QStringList& rStringList);
  void setSymbolWorld(const faudes::EventSet& rEventSet);
  void setSymbolWorld(QAbstractItemModel* pStringModel, int col=0);
  QStringListModel* symbolWorld(void) { return mSymbolWorld; };

  // copy source model to own model
  void Update(void);

  // release mem
  void clrSymbolWorld(void);

protected:

  // my known symbols
  QStringListModel* mSymbolWorld; 

  // source model
  QAbstractItemModel* pSymbolSource;
  int mSymbolSourceColumn;
};

/* 
 ******************************************
 ******************************************
 ******************************************

 A VioSymbolEdit object apears as a QLineEdit 
 or QComcoBox to edit or choose faudes symbolic 
 names. It provides convenient access to the 
 symbol name and its faudes index as well as 
 validation and completion.

 ******************************************
 ******************************************
 ******************************************
 */



class VIODES_API VioSymbolEdit : public QWidget {

  Q_OBJECT

public:

  // construct
  VioSymbolEdit(QWidget *parent = 0);
  ~VioSymbolEdit(void);

  // set/get user data
  void setText(QString symbol);
  void setSymbol(QString symbol);
  void setSymbol(const std::string& symbol);
  void setIndex(faudes::Idx idx);
  void setModelIndex(const QModelIndex& index);
  QString symbol(void);
  QString text(void);
  faudes::Idx index(void);
  QModelIndex modelIndex(void);

  // select all text for editing
  void selectAll(void);

  // set symbol table
  void setSymbolTable(faudes::SymbolTable* symtab);

  // set symbolmode
  void setSymbolMode(VioSymbol::Mode mode);
  void setSymbolMode(int mode) { setSymbolMode((VioSymbol::Mode) mode);};
  int symbolMode(void) {return mSymbolMode;};

  // set completer
  void setCompleter(QCompleter* completer);

  // provide event filter
  bool eventFilter(QObject *obj, QEvent *event);

signals:

  // symbol changed
  void textChanged(const QString& text);
  void returnPressed(void);

public slots:

  // check validit (incl color)  
  bool validate(void);

protected:
  
  // reimplement
  void focusInEvent(QFocusEvent* event); 
  void keyPressEvent(QKeyEvent* event);
  bool event(QEvent* ev);


  // my edit widgets
  QLineEdit* mLineEdit;
  QComboBox* mComboBox;

  // my layout
  QVBoxLayout* mVbox;
  QStackedWidget* mStack;

  // ref to symboltable (or 0)
  faudes::SymbolTable* pSymbolTable;

  // symbol mode
  VioSymbol::Mode mSymbolMode;

  // my validator
  VioSymbolValidator* mValidator;

  // completer to use
  QCompleter* pCompleter;

  // record std foreground
  QPalette mPalette;

  // record recent modelindex
  QModelIndex mModelIndex;

};

/* 
 ******************************************
 ******************************************
 ******************************************

 A VioSymbolDelegate is a delegate for 
 models that consit of faudes symbols.
 It uses the VioSymbolEdit widget as editor
 and sets a suitable completer.

 ******************************************
 ******************************************
 ******************************************
 */



class VIODES_API VioSymbolDelegate : public QItemDelegate {
  Q_OBJECT

public:

  // construct
  VioSymbolDelegate(QObject *parent = 0);


  // get my symbol
  QString symbol(QWidget* editor);
  faudes::Idx index(QWidget* editor);
  QModelIndex modelIndex(QWidget* editor);

  // set symbolmode
  void setSymbolMode(VioSymbol::Mode mode);

  // set edit helpers
  void setValidator(QValidator* validator);
  void setCompleter(QCompleter* completer);


  // reimplement item delegate
  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
    const QModelIndex &index) const;
  void setEditorData(QWidget *editor, const QModelIndex &index) const;
  void setModelData(QWidget *editor, QAbstractItemModel *model,
    const QModelIndex &index) const;
  void updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &index) const;

protected:

  // reimplment std eventfilter
  bool eventFilter(QObject *obj, QEvent *event); 

  // reimplement for report
  bool event(QEvent* ev);

  // my data
  QCompleter* pCompleter;
  VioSymbol::Mode mSymbolMode;


};



/*
*******************************************
*******************************************
*******************************************

Symbol Table widget: represent 
lists of faudes symbols, have sensible
completer and a simple interface. The class
is intended for quick and dirty visualisation
with small rowcounts.

Note: if setDimensions changes the column
count, it will clear the model and reset any 
delegate configuration eg completers. If it
changes the row count, it wil clear the contents.

THIS CLASS NEEDS A REDESIGN
- use model/view
- extend/track completers

*******************************************
*******************************************
*******************************************
*/

class VIODES_API VioSymbolTableWidget : public QTableView {

  Q_OBJECT

public:
   
  // construct destruct
  VioSymbolTableWidget(QWidget* parent=0);
  ~VioSymbolTableWidget(void) {};

  // get/set dimensions
  void setDimensions(int rows, int cols);
  void setDimension(int rows);
  int rowCount(void);
  int columnCount(void);

  // set base set (for completer and validator)
  void setSymbolWorld(int col, QCompleter* completer);
  void setSymbolWorld(int col, const QStringList& rStringList=QStringList());
  void setSymbolWorld(int col, const faudes::NameSet& rNameSet);
  void setSymbolWorld(int col, QAbstractItemModel* pStringModel, int srccol);

  // set base set (single clolumn convenience interface)
  void setSymbolWorld(QCompleter* completer);
  void setSymbolWorld(const QStringList& rStringList=QStringList());
  void setSymbolWorld(const faudes::NameSet& rNameSet);
  void setSymbolWorld(QAbstractItemModel* pStringModel, int srccol);

  // behaviour/apearance (implicitely set #cols)
  void setHeader(QStringList headers);
  void setSymbolMode(int col, int mode);

  // behaviour/apearance (single column convenience interace, sets cols=1)
  void setHeader(QString header);
  void setSymbolMode(int mode);

  // get/set content items
  QString Symbol(int row, int col=0) const;
  void setSymbol(int row, int col, const QString& symbol);
  void setSymbol(int row, const QString& symbol);

  // set/get content columns
  void setSymbolColumn(int col, const QStringList& stringlist=QStringList());
  QStringList symbolColumn(int col) const;
  faudes::EventSet eventSetColumn(int col) const;
  void setEventSetColumn(int col, const faudes::EventSet& rEventSet);
  void columnToNameSet(int col, faudes::NameSet& rNameSet) const; // for non-std symboltabel

  // set/get content columns (for single comlumn case)
  void setSymbolList(const QStringList& stringlist=QStringList());
  QStringList symbolList(void) const;
  void setEventSet(const faudes::EventSet& rEventSet);
  faudes::EventSet eventSet() const;
  void toNameSet(faudes::NameSet& rNameSet) const; // for non-std symboltabel

 
  // set/get current
  void setCurrentRow(int);
  int currentRow(void);


public slots:

  // user interaction
  void userInsertRow(void);
  void userDelSelection(void);
  void userSelectionClear(void);
  void userSelect(const QModelIndex& index, bool on=true);
  void Copy(void);
  void Paste(void);

signals:

  // entry was user changed
  void editingFinished(int row, int col);
  void resizeModel(void);
  void currentRowChanged(int);

protected slots:

  // pass on navigation
  void EmitCurrentChanged(void);

protected:
  
  // members
  QStandardItemModel mModel;
  QStringList  mHeaderLabels;
  QList<VioSymbolCompleter*>  mpCompleters;
  QList<QCompleter*>  pCompleters;
  QList<VioSymbolDelegate*> mpDelegates;

  // edit state
  bool mEditing;
  bool mInsertMode;

  // have my hooks
  void keyPressEvent(QKeyEvent *event);
  bool edit(const QModelIndex& index, EditTrigger trigger, QEvent* event);

  // temp data for edit hook
  QModelIndex mEditIndex;

protected slots:

  // have my hooks
  void closeEditor(QWidget* editor, QAbstractItemDelegate::EndEditHint hint);

  // context menu
  void contextMenu(QPoint pos, const QModelIndex& index);
  void contextMenuAtPoint(QPoint pos);

};




#endif
