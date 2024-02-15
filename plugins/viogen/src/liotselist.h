/* liotselist.h  - faudes lists as item models */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006, 2007  Thomas Moor, Klaus Schmidt, Sebastian Perk

*/


#ifndef FAUDES_LIOTSELIST_H
#define FAUDES_LIOTSELIST_H


#include "libviodes.h"

// forward
class VioGeneratorListModel;
class VioGeneratorModel;
class VioGeneratorStyle;

/*
 ************************************************
 ************************************************

 A Lio?list is a QAbstractItemModel that manages
 list of vio elements. It is meant to operate on data 
 within a VioGeneratorListModel. Ie, it converts the
 vio style model to a qt item style modle. The widget 
 representation can be any table view, however the 
 VioGeneratorListView uses the Lio?View which provides 
 specialised functionality.

 There is a LioTList for transitions, as LioSList for
 states and a LioEList for events. There is also a
 LioVList as a common base.

 ************************************************
 ************************************************
 */


class LioVList : public QAbstractTableModel { 
 
Q_OBJECT

public:
  // constructor, destructor
  LioVList(VioGeneratorListModel* genlist);
  virtual ~LioVList(void);

  // configure faudes flag checkboxes
  void InsertFlags(const QList<VioBooleanProperty>& boolprops);

  // convenience access to model hierarcy
  const faudes::vGenerator* Generator(void) const;
  VioGeneratorModel* GeneratorModel(void);
  VioGeneratorListModel* GeneratorListModel(void);

  // convert faudes items to/from model indicees
  bool IsFaudesElement(const QModelIndex& index);
  VioElement FaudesElement(const QModelIndex& index);
  QModelIndex ModelIndex(const VioElement& elem);

  // reimplement qabstracttable 
  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role) const;
  bool setData(const QModelIndex &index, const QVariant & value, int role = Qt::EditRole);
  Qt::ItemFlags flags(const QModelIndex &index) const;
  bool insertRows(int row, int count, const QModelIndex& parent=QModelIndex());
  bool removeRows(int row, int count, const QModelIndex& parent=QModelIndex());
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

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

signals:

  // notify user modifications
  void NotifyModified(bool ch);

protected:

  // ref to underlying vio models
  VioGeneratorListModel* pVioGeneratorListModel;
  VioGeneratorModel* pVioGeneratorModel;
  VioGeneratorStyle* pVioGeneratorConfig;

  // faudes flags
  QList<QString> mFlagNames;
  QList<int> mFlagAddresses;
  int mDataColumns;
  int mFlagColumns;

  // drag/drop hack
  QByteArray mDragData;
  QModelIndexList mDragIndexes;

  // drag and drop;
  virtual void UserInternalMove(const QList<int>& sourcelist, int dest);

  // update/resize hook
  bool mUpdateChanged;
  int mUpdateOldRows;

  // track user changes
  bool mModified;


};



/*
 ************************************************
 ************************************************

 A LioTList is a LioVList for transitions 

 ************************************************
 ************************************************
 */

 
class LioTList : public LioVList { 
 
  Q_OBJECT

public:
  // constructor, destructor
  LioTList(VioGeneratorListModel* genlist);
  virtual ~LioTList(void);

  // reimplement qabstracttable 
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  QVariant data(const QModelIndex &index, int role) const;
  bool setData(const QModelIndex &index, const QVariant & value, int role = Qt::EditRole);
  void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

public slots:

protected:
};



/*
 ************************************************
 ************************************************

 A LioSList is a LioVList for states

 ************************************************
 ************************************************
 */

 
class LioSList : public LioVList { 
 
  Q_OBJECT

public:
  // constructor, destructor
  LioSList(VioGeneratorListModel* genlist);
  virtual ~LioSList(void);

  // reimplement qabstracttable 
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  QVariant data(const QModelIndex &index, int role) const;
  bool setData(const QModelIndex &index, const QVariant & value, int role = Qt::EditRole);
  void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

  // provide completers based on myself as  models
  //QCompleter* StateCompleter(void) { return mCompleter; };

protected:

  // completer with this model as source
  VioSymbolCompleter* mCompleter;

};



/*
 ************************************************
 ************************************************

 A LioEList is a LioVList for events

 ************************************************
 ************************************************
 */

 
class LioEList : public LioVList { 
 
  Q_OBJECT

public:
  // constructor, destructor
  LioEList(VioGeneratorListModel* genlist);
  virtual ~LioEList(void);

  // reimplement qabstracttable 
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  QVariant data(const QModelIndex &index, int role) const;
  bool setData(const QModelIndex &index, const QVariant & value, int role = Qt::EditRole);
  void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

  // provide completers based on myself as  models
  //QCompleter* EventCompleter(void) { return mCompleter; };

protected:

  // completer with this model as source
  VioSymbolCompleter* mCompleter;

};


#endif
