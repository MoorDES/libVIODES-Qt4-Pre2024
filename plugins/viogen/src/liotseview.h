/* liotseview.h  - view on liotselist item model */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006, 2007  Thomas Moor, Klaus Schmidt, Sebastian Perk

*/


#ifndef FAUDES_LIOTSEVIEW_H
#define FAUDES_LIOTSEVIEW_H


#include "liotselist.h"
#include "viosymbol.h"


// forward
class VioGeneratorModel;
class VioGeneratorListModel;

/*
 ************************************************
 ************************************************

 A Lio?View is a QTableView is pecialised to show
 a Lio?list. It uses the VioSymbolDelegate for
 editing entries.

 There is a LioTView for transitions, as LioSView
 for states and a LioEView for events. There is 
 also a LioVView as a common base.

 ************************************************
 ************************************************
 */

 
class LioVView : public QTableView { 
 
  Q_OBJECT

public:
  // constructor, destructor
  LioVView(QWidget* parent=0);
  virtual ~LioVView(void);

  // reimplement: set model
  virtual void setModel(LioVList* liolist);

  // be more compatible to treeview
  QHeaderView* header(void) { return horizontalHeader(); };

  // convenience access to model hierarcy
  const faudes::vGenerator* Generator(void) const;
  VioGeneratorModel* GeneratorModel(void);
  VioGeneratorListModel* GeneratorListModel(void);

  // set completers to use (not necssarily the same as provided)
  void SetStateCompleter(QCompleter* completer); 
  void SetEventCompleter(QCompleter* completer); 

public slots:

  // editing model: selection
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

  // perform selection
  void SelectionUpdate(const QItemSelection& selected, const QItemSelection& deselected); 
  

protected:

  // refs to model hierarchy
  LioVList* pTableModel;
  VioGeneratorModel* pVioGeneratorModel;

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
  void UserInsElement(void);
  void UserSelect(const QModelIndex& index, bool on=true);

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
 ************************************************
 ************************************************

 A LioTView is a LioVView for transitions 

 ************************************************
 ************************************************
 */

 
class LioTView : public LioVView { 
 
  Q_OBJECT

public:

  // constructor, destructor
  LioTView(QWidget* parent=0);
  virtual ~LioTView(void);

  // reimplement: set model
  virtual void setModel(LioVList* liolist);

protected:
  // have a context menu
  void ContextMenu(QPoint pos, const QModelIndex& index);
};



/*
 ************************************************
 ************************************************

 A LioSView is a LioVView for states

 ************************************************
 ************************************************
 */


class LioSView : public LioVView { 
 
  Q_OBJECT

public:

  // constructor, destructor
  LioSView(QWidget* parent=0);
  virtual ~LioSView(void);

  // reimplement: set model
  virtual void setModel(LioVList* liolist);

protected:
  // have a context menu
  void ContextMenu(QPoint pos, const QModelIndex& index);
};

/*
 ************************************************
 ************************************************

 A LioEView is a LioVView for events

 ************************************************
 ************************************************
 */


class LioEView : public LioVView { 
 
  Q_OBJECT

public:

  // constructor, destructor
  LioEView(QWidget* parent=0);
  virtual ~LioEView(void);

  // reimplement: set model
  virtual void setModel(LioVList* liolist);

protected:
  // have a context menu
  void ContextMenu(QPoint pos, const QModelIndex& index);
};


#endif
