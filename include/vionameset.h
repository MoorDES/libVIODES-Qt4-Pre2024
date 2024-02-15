/* vionameset.h  - vio nameset model and widget */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2010  Thomas Moor

*/



#ifndef FAUDES_VIONAMESET_H
#define FAUDES_VIONAMESET_H

// std includes
#include "viotypes.h"
#include "vionsetstyle.h"




/*
 ************************************************
 ************************************************

 VioNameSetData holds static data required
 for the widget representations of a name set, i.e.
 a qlist of names.
 
 ************************************************
 ************************************************
 */

class VIODES_API VioNameSetData : public VioData {

Q_OBJECT

  friend class VioNameSetModel;
  friend class VioNameSetView;

public:

  // destruct
  virtual ~VioNameSetData(void);

  // clear to default/empty faudes object
  virtual void Clear(void);

  // mime conversion (ret 0 on sucess)
  virtual QMimeData* ToMime(void);
  virtual int FromMime(const QMimeData* pMime);
  virtual int TestMime(const QMimeData* pMime);

  // public data
  QList<QString> mList;

protected:

  // construct
  VioNameSetData(QObject* parent=0);

  // serialisation, virtual hooks
  virtual void DoWriteCore(faudes::TokenWriter& rTw, const QString& ftype="") const;
  virtual void DoWrite(faudes::TokenWriter& rTw, const QString& ftype="") const;
  virtual void DoReadCore(faudes::TokenReader& rTr, const QString& ftype="");
  virtual void DoRead(faudes::TokenReader& rTr, const QString& ftype="");

};


/*
 ************************************************
 ************************************************

 A VioNameSetModel models the representation of 
 a faudes NameSet as an editable list.

 ************************************************
 ************************************************
 */

// forward
class VioNameSetView;
class VioNameSetLayout;
class VioNameSetPropertyView;
class VioNameSetWidget;
class LioNameSetModel;
class LioNameSetView;

class VIODES_API VioNameSetModel : public VioModel {

Q_OBJECT

public:

  // construct/destruct
  VioNameSetModel(QObject* parent=0, VioStyle* config=0, bool alloc=true);
  virtual ~VioNameSetModel(void);

  // typed acces to configuration
  VioNameSetStyle* NameSetConfiguration(void) const { return pNameSetStyle;};

  // reimplement viomodel: construct on heap
  virtual VioNameSetModel* NewModel(QObject* parent=0) const;
  virtual VioData* NewData(QObject* parent=0) const;
  virtual VioView* NewView(QWidget* parent=0) const;
  virtual VioView* NewPropertyView(QWidget* parent=0) const;
  virtual VioWidget* NewWidget(QWidget* parent=0) const;

  // typed faudes object access
  const faudes::NameSet* NameSet(void) const { return mpFaudesNameSet; };

  // edit on representation data (emit notifications)
  void Clear(void);
  int Size(void) const;
  faudes::Idx Index(const QString& name) const;
  VioElement Element(const QString& name) const;
  QString SymbolicName(faudes::Idx idx) const;
  const QString& At(int pos) const;
  bool At(int pos, const QString& name);
  bool ReName(const QString& oldname, const QString& newname);
  bool Append(const QString& name);
  bool Insert(int pos, const QString& name);
  bool Remove(const QString& name);
  bool  RemoveAt(int pos);
  bool Move(int from, int to);
  int IndexOf(const QString& name) const;
  bool Exists(const QString& name) const;
  QString UniqueSymbol(const QString& name="");
  void SortAscending(void);
  void SortDescending(void);
  bool Attribute(const QString& name, const faudes::AttributeVoid& attr);
  bool AttributeTest(const QString& name, const faudes::AttributeVoid& attr);
  const faudes::AttributeFlags& Attribute(const QString& name) const;
  const QList<VioBooleanProperty>& BooleanProperties(void) const;
  bool BooleanProperty(const QString& name,int prop) const;
  void BooleanProperty(const QString& name, int prop, bool val);


  // set/get default layout
  const VioNameSetLayout& Layout(void); 
  void Layout(const VioNameSetLayout& layout); 


  // global vio data access
  VioData* Data(void);
  int Data(const VioData* pData);
  int TypeCheckData(const VioData* pData);

  // directed vio data access
  int InsertData(const VioData* pData);
  VioData* SelectionData(void);
  void DeleteSelection(void);

public slots:

  // selection: write access (existing only)
  void Select(const VioElement& elem, bool on=true);

signals:

  // edit notification
  void NotifySymbolChange(QString name);  
  void NotifyChange(void);

protected:

  // typed version of faudes object
  faudes::NameSet* mpFaudesNameSet;

  // typed style acces
  VioNameSetStyle* pNameSetStyle;

  // reimplement viomodel: allocate faudes object and visual model data
  virtual void DoFaudesAllocate(void);
  virtual void DoVioAllocate(void);

  // reimplement viomodel: test whether we can host this faudes object
  virtual int DoTypeCheck(const faudes::Type* fobject) const;

  // reimplement viomodel: token io
  virtual void DoVioWrite(faudes::TokenWriter& rTw) const;
  virtual void DoVioRead(faudes::TokenReader& rTr);

  // reimplement viomodel: update visual data from (new) faudes object
  virtual void DoVioUpdate(void);

  // data access: merge data (return 1 on changes, 0 else)
  virtual int DoMergeData(const VioData* pData);

  // my representation data
  VioNameSetData* mpNameSetData;  
  QMap<QString,int> mRowMap;
  void  DoFixRowMap(void);
  void  DoFixList(void);

  // default layout
  VioNameSetLayout* mUserLayout;


protected slots:

  // update faudes set from internal list model
  void DoFaudesUpdate(void);
};



/*
 ************************************************
 ************************************************
 
 User Layout Options
 
 ************************************************
 ************************************************
 */

// struct to hold user layout options
class VIODES_API VioNameSetLayout : public QObject {

Q_OBJECT

public:
  // constructor (default values)
  VioNameSetLayout(QObject* parent=0);

  // assignment operator
  virtual VioNameSetLayout& operator=(const VioNameSetLayout& rSrc);

  // members (should protect)
  bool mPropBuiltIn;
  int mPropSize;
  int mListSize;

  // load/save layout (no exceptions)
  virtual void Write(faudes::TokenWriter& rTw) const;
  virtual void Read(faudes::TokenReader& rTr);
};


/*
 ************************************************
 ************************************************

 A VioNameSetView is a widget to represent an
 attribute via an VioNameSetModel.

 ************************************************
 ************************************************
 */



class VIODES_API VioNameSetView : public VioView {

Q_OBJECT

public:
  // construct/destruct
  VioNameSetView(QWidget* parent=0, VioStyle* config=0, bool alloc=true);
  virtual ~VioNameSetView(void);

  // reimplement vioview: allocate view data
  virtual void DoVioAllocate(void);

  // typed faudes object access
  const faudes::NameSet* NameSet(void) const;

  // directed vio data access
  // for namesets, these are implemented in the model; we throughpass
  // the request to have a uniform interface
  int InsertData(const VioData* pData);
  VioData* SelectionData(void);
  void DeleteSelection(void);

public slots:

  // show/hile property view
  void ShowPropertyView(bool on);


protected:

  // reimplement vioview: update view from model
  virtual void DoVioUpdate(void);

  // reimplement vioview: update to model (exception on error)
  virtual void DoModelUpdate(void);

  // apply/retrieve user layout
  virtual void UpdateUserLayout(void);
  virtual void SaveUserLayout(void);

  // typed model/style
  VioNameSetModel* pNameSetModel;
  VioNameSetStyle* pNameSetStyle;

  // user layout
  VioNameSetLayout* mUserLayout;

  // my item model
  LioNameSetModel* mListModel;

  // my widgets
  QSplitter* mSplitter;
  LioNameSetView* mListView;
  VioNameSetPropertyView* mPropView;

  // action
  QAction* mPropAction;

protected slots:

};


/*
 ************************************************
 ************************************************

 A VioNameSetWidget is a widget to represent a
 faudes set by an VioNameSetView. Internally, it 
 uses a VioNameSetModel to hold defining data.

 ************************************************
 ************************************************
 */



class VIODES_API VioNameSetWidget : public VioWidget {

Q_OBJECT

public:
  // construct/destruct
  VioNameSetWidget(QWidget* parent=0, VioStyle* config=0, bool alloc=true);
  virtual ~VioNameSetWidget(void);

  // set model
  int Model(VioModel* model);
  VioNameSetModel* Model(void) { return pNameSetModel; };

  // typed faudes object access
  const faudes::NameSet* NameSet(void) const;

protected:

  // reimplement viomodel: allocate vio model
  virtual void DoVioAllocate(void);

  // typed representation data
  VioNameSetModel* pNameSetModel;

};


/*
 ************************************************
 ************************************************

 A VioNameSetPropertyView is a widget to represent 
 properties of name set symbols, ie the symbol and
 its associated attribute.

 ************************************************
 ************************************************
 */


// forward
class PioNameSetView;

class VIODES_API VioNameSetPropertyView : public VioView {

Q_OBJECT

public:
  // construct/destruct
  VioNameSetPropertyView(QWidget* parent=0, VioStyle* config=0, bool alloc=true);
  virtual ~VioNameSetPropertyView(void);

public slots:

  // reimplement show element/selection
  void Show(const VioElement& elem);
  void UpdateSelectionChange(void); 

protected:

  // allocate visual view data
  virtual void DoVioAllocate(void);

  // update view from (new) model
  virtual void DoVioUpdate(void);

  // update view to model (exception on error, not implemented)
  virtual void DoModelUpdate(void);

  // typed representation data
  VioNameSetModel* pNameSetModel;

  // typed version of configuration
  VioNameSetStyle* pNameSetConfig;

  // layout items: property widget
  PioNameSetView* mProperties;

};



#endif
