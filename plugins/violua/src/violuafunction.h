/* violuafunction.h  - vio model/view for lua function definitions */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2010 Thomas Moor

*/



#ifndef FAUDES_VIOLUAFUNCTION_H
#define FAUDES_VIOLUAFUNCTION_H

// std includes
#include "libviodes.h"

// need code editor and styles
#include "violuastyle.h"
#include "violuacode.h"


/*
 ************************************************
 ************************************************

 A VioLuaFunctionModel is derived from the plain VioModel.
 We try to keep this really simple, in that we
 dont have an extra style class.

 ************************************************
 ************************************************
 */

// forward
class VioLuaFunctionLayout;
class VioLuaFunctionView;
class VioLuaFunctionPropertyView;
class VioLuaFunctionWidget;


class VioLuaFunctionModel : public VioModel {

Q_OBJECT

public:

  // construct/destruct
  VioLuaFunctionModel(QObject* parent=0, VioStyle* config=0, bool alloc=true);
  virtual ~VioLuaFunctionModel(void);

  // reimplement viomodel: construct on heap
  virtual VioLuaFunctionModel* NewModel(QObject* parent=0) const;
  virtual VioView* NewView(QWidget* parent=0) const;
  virtual VioView* NewPropertyView(QWidget* parent=0) const;
  virtual VioWidget* NewWidget(QWidget* parent=0) const;

  // typed faudes object access 
  const faudes::LuaFunctionDefinition* LuaFunctionDefinition(void) const 
    { return mpFaudesLuaFunctionDefinition; };

  // reimplement file io for plain lua
  virtual void ExportFaudesFile(const QString& rFilename) const;
  virtual void ImportFaudesFile(const QString& rFilename);

  // edit set
  void VioLuaCode(const QString& code);
  void VioVariants(const QList<faudes::Signature>& variants);
  void VioVariant(int pos,const faudes::Signature& signature);
  
  // edit get
  const QString& VioLuaCode(void) const 
    { return mLuaCode;};
  const QList<faudes::Signature>& VioVariants(void) const
    { return mVariants; };

  // set/get default layout
  const VioLuaFunctionLayout& Layout(void); 
  void Layout(const VioLuaFunctionLayout& layout); 

  // get style flag
  bool PlainScript(void) const { return pLuaStyle->mPlainScript;};

  // record/query user changes 
  bool Modified(void) const;
  virtual void Modified(bool ch);

public slots:

  // test/provoke errors
  void TestScript(void);
  void RunScript(void);

  // collect changes
  void ChildModified(bool changed);

signals:

  // changes notification
  void NotifySignatureChange(void);  
  void NotifyCodeChange(void);  

protected:

  // typed version of faudes object
  faudes::LuaFunctionDefinition* mpFaudesLuaFunctionDefinition;

  // typed style
  VioLuaStyle* pLuaStyle;

  // reimplement viomodel: allocate faudes object 
  virtual void DoFaudesAllocate(void);

  // reimplement viomodel: allocate visual model data
  virtual void DoVioAllocate(void);

  // reimplement viomodel: test whether we can host this faudes object
  virtual int DoTypeCheck(const faudes::Type* fobject) const;

  // reimplement viomodel: token io, implementation 
  virtual void DoVioWrite(faudes::TokenWriter& rTw) const;
  virtual void DoVioRead(faudes::TokenReader& rTr);

  // reimplement viomodel: update visual data from (new) faudes object
  virtual void DoVioUpdate(void);

  // local copy of my data
  QString mLuaCode;
  QList<faudes::Signature> mVariants;

  // default layout
  VioLuaFunctionLayout* mUserLayout;

};


/*
 ************************************************
 ************************************************
 
 User Layout Options
 
 ************************************************
 ************************************************
 */

// struct to hold user layout options
class VioLuaFunctionLayout : public QObject {

Q_OBJECT

public:
  // constructor (default values)
  VioLuaFunctionLayout(QObject* parent=0);

  // assignment operator
  virtual VioLuaFunctionLayout& operator=(const VioLuaFunctionLayout& rSrc);

  // members (should protect)
  bool mPropBuiltIn;
  int mPropSize;
  int mListSize;
  int mTextSize;

  // load/save layout (no exceptions)
  virtual void Write(faudes::TokenWriter& rTw) const;
  virtual void Read(faudes::TokenReader& rTr);
};


/*
 ************************************************
 ************************************************

 A VioLuaFunctionView is a widget to represent the
 function's lua code

 ************************************************
 ************************************************
 */



class VioLuaFunctionView : public VioView {

Q_OBJECT

public:
  // construct/destruct
  VioLuaFunctionView(QWidget* parent=0, VioStyle* config=0, bool alloc=true);
  virtual ~VioLuaFunctionView(void);

  // reimplement vioview: allocate view data
  virtual void DoVioAllocate(void);

  // typed faudes object access
  const faudes::LuaFunctionDefinition* LuaFunctionDefinition(void) const; 

  // record/query user changes 
  bool Modified(void) const;
  virtual void Modified(bool ch);

  // my actions
  QAction* TestAction(void) const { return mTestAction; };
  QAction* RunAction(void) const { return mRunAction; };

public slots:

  // reimplemengt std edit from viotype
  void Cut(void);
  void Copy(void);
  void Paste(void);
  void Undo(void);
  void Redo(void);

  // find 
  void FindDialog(void);
  void FindAgain(void);
  void Find(const QString& pattern, const QString& replace, QTextDocument::FindFlags flags=0);

  // zoom
  void ZoomIn(void);
  void ZoomOut(void);

  // collect and pass on modifications of childs
  void ChildModified(bool ch=true);

  // process show requests
  void Show(const VioElement& elem);

  // show/hide property view
  void ShowPropertyView(bool on);

protected:

  // update view from model
  virtual void DoVioUpdate(void);

  // update to model (exception on error)
  virtual void DoModelUpdate(void);

  // apply/retrieve user layout
  virtual void UpdateUserLayout(void);
  virtual void SaveUserLayout(void);

  // typed refs
  VioLuaFunctionModel* pLuaFunctionModel;

  // typed style
  VioLuaStyle* pLuaStyle;

  // layout items
  QSplitter* mSplitter;
  VioLuaCodeEditor* mCodeEdit;
  VioLuaFunctionPropertyView* mPropView;

  // my actions
  QAction* mTestAction;
  QAction* mRunAction;
  QAction* mFindAction;
  QAction* mAgainAction;
  QAction* mZoomInAction;
  QAction* mZoomOutAction;

  // action
  QAction* mPropAction;

  // user layout
  VioLuaFunctionLayout* mUserLayout;

  // my serach pattern
  QString mFindPattern;
  QString mFindReplace;
  QTextDocument::FindFlags mFindFlags;
  VioFindDialog* mFindDialog;


protected slots:

  // update from view widgets (row -1 <==> all)
  void UpdateFromCodeEdit();

};


/*
 ************************************************
 ************************************************

 A VioLuaFunctionPropertyView is a widget to represent 
 the functions signature.

 ************************************************
 ************************************************
 */



class VioLuaFunctionPropertyView : public VioView {

Q_OBJECT

public:
  // construct/destruct
  VioLuaFunctionPropertyView(QWidget* parent=0, VioStyle* config=0, bool alloc=true);
  virtual ~VioLuaFunctionPropertyView(void);

  // reimplement vioview: allocate view data
  virtual void DoVioAllocate(void);

  // typed faudes object access
  const faudes::LuaFunctionDefinition* LuaFunctionDefinition(void) const; 

  // record/query user changes 
  bool Modified(void) const;
  virtual void Modified(bool ch);

public slots:

  // show a signature
  void ShowVariant(int pos);

  // collect and pass on modifications of childs
  void ChildModified(bool ch=true);

protected:

  // update view from model
  virtual void DoVioUpdate(void);

  // update to model (exception on error)
  virtual void DoModelUpdate(void);

  // typed refs
  VioLuaFunctionModel* pLuaFunctionModel;

  // layout items
  VioSymbolTableWidget* mSignatureList;
  VioSymbolTableWidget* mParameterTable;
  int mCurrentVariant;

protected slots:

  // update from view widgets 
  void UpdateAllFromWidgets();
  void UpdateCurrentNameFromWidgets();
  void UpdateCurrentSignatureFromWidgets();
};


/*
 ************************************************
 ************************************************

 A VioLuaFunctionWidget is a widget to represent a
 VioLuaFunctionView. Internally, it 
 uses a VioLuaFunctionModel to hold defining data.

 ************************************************
 ************************************************
 */



class VioLuaFunctionWidget : public VioWidget {

Q_OBJECT

public:
  // construct/destruct
  VioLuaFunctionWidget(QWidget* parent=0, VioStyle* config=0, bool alloc=true);
  virtual ~VioLuaFunctionWidget(void);

  // set model
  int Model(VioModel* model);

protected:

  // reimplement viomodel: allocate vio model
  virtual void DoVioAllocate(void);

  // typed representation data
  VioLuaFunctionModel* pLuaFunctionModel;

};


/*
 ************************************************
 ************************************************

 A VioLuaExecute is a helper to evaluate
 the lua code in a seperate task

 ************************************************
 ************************************************
 */

// class definition
class VioLuaExecute : public QThread  {

  Q_OBJECT

public:

  // construct/destruct
  VioLuaExecute(VioLuaFunctionModel* lfnct, bool test=false);
  ~VioLuaExecute(void);


public slots:
  // execute, call from application
  QString Execute(void);

  
private:

  // start() thread calls run
  void run(void);

  // operation reference
  VioLuaFunctionModel* pLfnct;  
  faudes::LuaFunctionDefinition* pLFfnct;
  bool mTest;
  QString mErrString;
};


#endif
