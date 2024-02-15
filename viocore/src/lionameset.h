/* lionameset.h  - qt versions of symbol lists */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2010  Thomas Moor

*/



#ifndef FAUDES_LIONAMESET_H
#define FAUDES_LIONAMESET_H

// std includes
#include "viotypes.h"
#include "viosymbol.h"
#include "vionsetstyle.h"
#include "vionameset.h"


/*
 ************************************************
 ************************************************

 A LioNameSetModel is a QAbstractItemModel that interprets
 a VioNameSetModel in order to provide the std Qt
 model interface. It can be viewed e.g. by a QTableView,
 or, by the customized LioNameSetView.

 ************************************************
 ************************************************
 */


class LioNameSetModel : public QAbstractTableModel { 
 
Q_OBJECT

public:
  // constructor, destructor
  LioNameSetModel(VioNameSetModel* nameset);
  virtual ~LioNameSetModel(void);

  // configure faudes flag checkboxes
  void InsertFlags(const QList<VioBooleanProperty>& boolprops);

  // convenience access to model hierarcy
  const faudes::NameSet* NameSet(void) const;
  VioNameSetModel* VioModel(void);

  // access faudes items by model index
  bool IsSymbol(const QModelIndex& index);
  const QString& Symbol(const QModelIndex& index);
  QModelIndex ModelIndex(const QString& name);

  // reimplement qabstracttable 
  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role) const;
  bool setData(const QModelIndex &index, const QVariant & value, int role = Qt::EditRole);
  Qt::ItemFlags flags(const QModelIndex &index) const;
  bool insertRows(int row, int count, const QModelIndex& parent=QModelIndex());
  bool removeRows(int row, int count, const QModelIndex& parent=QModelIndex());
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

  // enable drag and drop
  virtual Qt::DropActions supportedDropActions() const;
  virtual QStringList mimeTypes () const;
  virtual QMimeData* mimeData (const QModelIndexList & indexes) const;
  virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, 
  	int row, int column, const QModelIndex& parent);

  // reimplement :we are a table rather than a tree
  bool hasChildren (const QModelIndex& parent = QModelIndex()) const {
    return parent== QModelIndex();};

  // trigger view update
  void PrepareResize(void);
  void UpdateResize(void);
  void UpdateAll(void);
  void UpdateRow(int row);
  void UpdateReset(void);


  // record/query changes 
  bool Modified(void) const;
  virtual void Modified(bool ch);

public slots:

  // collect and pass on modifications of childs
  void ChildModified(bool ch);

  // receive edit notification from viomodel
  void UpdateAnyChange();
  void UpdateSymbolChange(QString name);

signals:

  // notify user modifications
  void NotifyModified(bool ch);

protected:

  // ref to underlying vio models
  VioNameSetModel* pVioNameSetModel;
  const VioNameSetStyle* pVioNameSetConfig;

  // faudes flags
  QList<QString> mFlagNames;
  QList<int> mFlagAddresses;
  int mDataColumns;
  int mFlagColumns;

  // drag/drop hack
  QByteArray mDragData;
  QModelIndexList mDragIndexes;

  // drag and drop;
  void UserInternalMove(const QList<int>& sourcelist, int dest);

  // update/resize hook
  bool mUpdateChanged;
  int mUpdateOldRows;

  // track user changes
  bool mModified;

  // completer with this model as source
  VioSymbolCompleter* mCompleter;


};





/*
 ************************************************
 ************************************************

 A LioNameSetView is a QTableView specialised to show
 a LioNameSetModel. It uses the VioSymbolDelegate for
 editing entries.

 ************************************************
 ************************************************
 */

 
class LioNameSetView : public QTableView { 
 
  Q_OBJECT

public:
  // constructor, destructor
  LioNameSetView(QWidget* parent=0);
  virtual ~LioNameSetView(void);

  // reimplement: set model
  virtual void setModel(LioNameSetModel* model);

  // be more compatible to treeview
  QHeaderView* header(void) { return horizontalHeader(); };

  // convenience access to model hierarcy
  const faudes::NameSet* NameSet(void) const;
  VioNameSetModel* VioModel(void);

  // set completers to use (not necssarily the same as provided
  void SetStateCompleter(QCompleter* completer); 
  void SetEventCompleter(QCompleter* completer); 

public slots:

  // editing model: selection changed in VioNameSetModel
  virtual void UpdateSelectionElement(const VioElement& elem, bool on=true);
  virtual void UpdateSelectionClear(void);
  virtual void UpdateSelectionAny(void);


signals:

  // notify model clicks
  void MouseClick(const VioElement& elem);

private slots:

  // show or hide sort
  void FixSortIndicator(int col);

  // have a context menu by position or index
  virtual void ContextMenuAtPoint(QPoint pos);
  virtual void ContextMenu(QPoint pos, const QModelIndex& index);

  // emit mouse click signal
  void EmitMouseClick(const QModelIndex& index);

  // perform selection edit by user
  void SelectionUpdate(const QItemSelection& selected, const QItemSelection& deselected); 
  

protected:

  // refs to model hierarchy
  LioNameSetModel* pLioModel;
  VioNameSetModel* pVioModel;

  // my delegates
  VioSymbolDelegate* mStateDelegate;
  VioSymbolDelegate* mEventDelegate;

  // # of cols with sorting 
  int mSortEnabled;

  // set # of cols with sorting
  void SetSortEnabled(int col);

  // edit hook
  QModelIndex mEditIndex;

  // user commands
  void UserDelSelection(void);
  void UserInsert(void);

  // get internal version of selection
  QList<int> SelectedRows(void);

  // mute selection update signal
  bool mMuteSelection;

  // behaviour state
  bool mInsertMode;
  bool mEditing;

  // reimplement abstractitemview functions I
  void keyPressEvent(QKeyEvent *event);   
  bool edit(const QModelIndex& index, EditTrigger trigger, QEvent* event);
  void focusInEvent(QFocusEvent* event);
  void focusOutEvent(QFocusEvent* event);


protected slots:

  // reimplement abstractitemview functions II
  void closeEditor(QWidget* editor, QAbstractItemDelegate::EndEditHint hint);


};




/*
 *****************************************************
 *****************************************************
 The properties widgets are the intended content of a 
 dialog box to edit nameset attributes

 Technically, the PioNameSetProperts holds a VioAttributeWidget and, hence
 a model, to actually show the property. 
 W.r.t. the underlying NameSet it is, however, still a view.


 *****************************************************
 *****************************************************
 */


class PioNameSetView : public QWidget {

Q_OBJECT

public:
  // construct
  PioNameSetView(QWidget* parent=0, VioStyle* config=0);
  ~PioNameSetView(void);

  // configure behaviour
  void SymbolMode(VioSymbol::Mode mode);
  // todo set completer

  // set/get vio generator 
  void Model(VioNameSetModel* genmodel=NULL);
  const VioNameSetModel* Model(void) const;

  // read only access to nameset
  const faudes::NameSet* NameSet(void) const;

  // get faudes element that is currently displyed
  faudes::Idx Idx(void) const;
  const QString& Symbol(void) const;
  VioElement Element(void) const;

public slots:

  // set model from visual representation
  void UpdateModel(void);

  // set view from model
  void UpdateView(void);

  // show interface
  virtual void Show(const VioElement& elem);
  virtual void ShowSelection(void);

signals:

  // notify application on user changes
  void NotifyModified(bool ch);

  // indicate return in symbolic name edit
  void DoneEditing(void);


protected slots:

  // do clear
  virtual void DoClear(void);

  // do update
  virtual void DoModelUpdate(void);
  virtual void DoVioUpdate(void);

protected:

  // configuration
  VioNameSetStyle* pNameSetConfig;

  // record connection to model
  VioNameSetModel* pVioModel;

  // element to view
  QString mSymbol;
  
  // access name in editor (no signals)
  void Name(const QString& name);
  QString Name(void);

  // set faudes attribute (no signals, takes ownership)
  void Attribute(const faudes::AttributeVoid& attr);
  void AttributeFromSelection(void);
  void AttributeToSelection(void);

  // block internal attribute update signal
  bool mBlockModelUpdate;

  // my layout 
  QVBoxLayout* mVbox;
  QLabel* mLabelName;
  VioSymbolEdit* mEditName;
  VioAttributeWidget* mAttribute;




};


#endif
