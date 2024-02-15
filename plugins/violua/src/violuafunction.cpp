/* violuafunction.cpp  - vio model/view for lua function definitions  */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2010 Thomas Moor

*/


//#define FAUDES_DEBUG_VIO_LUA

#include "violuafunction.h"

/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioLuaFunctionModel

****************************************************************
****************************************************************
****************************************************************
*/

// construct
VioLuaFunctionModel::VioLuaFunctionModel(QObject* parent, VioStyle* config, bool alloc) : 
  VioModel(parent, config, false),
  mpFaudesLuaFunctionDefinition(0), 
  pLuaStyle(0),
  mUserLayout(0)
{
  FD_DQL("VioLuaFunctionModel::VioLuaFunctionModel(): " << VioStyle::StrFromQStr(mFaudesType));
  // have typed style
  pLuaStyle=dynamic_cast<VioLuaStyle*>(pConfig);
  if(!pLuaStyle) {
    FD_WARN("VioLuaFunctionModel::VioLuaFunctionModel(): invalid style, using default.");
    pLuaStyle= new VioLuaStyle(mFaudesType);
  }
  // allocate faudes object and have default rep data
  if(alloc) {
    DoVioAllocate();
    DoVioUpdate();
  }
  // fix modified
  mModified=false;
  FD_DQL("VioLuaFunctionModel::VioLuaFunctionModel(): done");
}

// destruct
VioLuaFunctionModel::~VioLuaFunctionModel(void) {
}


// allocate visual model data
void VioLuaFunctionModel::DoVioAllocate(void) {
  FD_DQN("VioLuaFunctionModel::DoVioAllocate()");
  VioModel::DoVioAllocate();
  // have a layout
  mUserLayout = new VioLuaFunctionLayout(this);
  mUserLayout->mPropBuiltIn=true;
}


// construct on heap
VioLuaFunctionModel* VioLuaFunctionModel::NewModel(QObject* parent) const {
  FD_DQL("VioLuaFunctionModel::NewModel(): type " << VioStyle::StrFromQStr(mFaudesType)); 
  return new VioLuaFunctionModel(parent,pConfig);
}

// construct view on heap
VioView* VioLuaFunctionModel::NewView(QWidget* parent) const {
  FD_DQL("VioLuaFunctionModel::NewView(): " << VioStyle::StrFromQStr(FaudesType()));
  return new VioLuaFunctionView(parent, pConfig);
}

// construct view on heap
VioView* VioLuaFunctionModel::NewPropertyView(QWidget* parent) const {
  FD_DQL("VioLuaFunctionModel::NewPropertyView(): " << VioStyle::StrFromQStr(FaudesType()));
  return new VioLuaFunctionPropertyView(parent, pConfig);
}

// construct widget on heap
VioWidget* VioLuaFunctionModel::NewWidget(QWidget* parent) const {
  FD_DQL("VioLuaFunctionMode::NewWidget(): type " << VioStyle::StrFromQStr(FaudesType()));
  return new VioLuaFunctionWidget(parent, pConfig);
}


// allocate faudes object
void VioLuaFunctionModel::DoFaudesAllocate(void) {
  FD_DQL("VioLuaFunctionModel::DoFaudesAllocate()");
  // delete old faudes object
  mData->FaudesObject(0);
  // allocate my faudes function definition
  mData->FaudesObject(new faudes::LuaFunctionDefinition());  
  // report
  FD_DQL("VioLuaFunctionModel::DoFaudesAllocate(): done ");
}

// test whether we can host this faudes object
int VioLuaFunctionModel::DoTypeCheck(const faudes::Type* fobject) const {  
  // we host anything that casts to to a function def
  if(dynamic_cast<const faudes::LuaFunctionDefinition*>(fobject)) return 0;
  return 1; 
}

// update visual data from (new) faudes object (incl layout change TODO: signals)
void VioLuaFunctionModel::DoVioUpdate(void) {
  FD_DQL("VioLuaFunctionModel::DoVioUpdate()");
  // have my typed reference
  mpFaudesLuaFunctionDefinition= 
    dynamic_cast<faudes::LuaFunctionDefinition*>(mData->FaudesObject());
  // update my features
  mLuaCode = VioStyle::QStrFromStr(mpFaudesLuaFunctionDefinition->LuaCode());
  mVariants.clear();
  for(int i=0; i<   mpFaudesLuaFunctionDefinition->VariantsSize(); i++)  {
    mVariants.append(mpFaudesLuaFunctionDefinition->Variant(i));
  }
  FD_DQL("VioLuaFunctionModel::DoVioUpdate(): #" << mLuaCode.size());
#ifdef FAUDES_DEBUG_VIO_WIDGETS
  // call base (incl set text and notify)
  VioModel::DoVioUpdate();
#else
  // we notify
  emit NotifyAnyChange();
#endif
}


// token io
void VioLuaFunctionModel::DoVioWrite(faudes::TokenWriter& rTw) const {
  // write layout
  mUserLayout->Write(rTw);
}

// token io
void VioLuaFunctionModel::DoVioRead(faudes::TokenReader& rTr) {
 FD_DQL("VioLuaFunctionModel::DoVioRead(): code #" << mpFaudesLuaFunctionDefinition->LuaCode().size());
 // read layout
 mUserLayout->Read(rTr);
 // we have no own data, but we need to set our selfs up from the faudes object
 mLuaCode= VioStyle::QStrFromStr(mpFaudesLuaFunctionDefinition->LuaCode());
 mVariants.clear();
 for(int i=0; i< mpFaudesLuaFunctionDefinition->VariantsSize(); i++)  {
   mVariants.append(mpFaudesLuaFunctionDefinition->Variant(i));
 }
}

// token io: faudes write to file
void VioLuaFunctionModel::ExportFaudesFile(const QString& rFilename) const {
  FD_DQT("VioLuaFunctionModel::ExportFaudesFile()");
  // write faudes rti format
  if(!PlainScript()) {
    mData->FaudesObject()->XWrite(VioStyle::LfnFromQStr(rFilename));
  } else {
  // write plain text
    QFile qf(rFilename);
    qf.open(QIODevice::Truncate | QIODevice::Text | QIODevice::WriteOnly);
    QTextStream qts(&qf);
    qts << mLuaCode;
    qf.close();
    if(qf.error()!=QFile::NoError) {
      std::stringstream errstr;
      errstr << "Exception opening/writing file \""<< VioStyle::StrFromQStr(rFilename) << "\"";
      throw faudes::Exception("ExportFaudesFile", errstr.str(), 2);
    }
  }
}


// token io: faudes read from file
void VioLuaFunctionModel::ImportFaudesFile(const QString& rFilename) {
  // read faudes rti format
  if(!PlainScript()) {
    mData->FaudesObject()->Read(VioStyle::LfnFromQStr(rFilename));
  } else {
  // read plain script
    mLuaCode="";
    QFile qf(rFilename);
    qf.open(QIODevice::Text | QIODevice::ReadOnly);
    QTextStream qts(&qf);
    mLuaCode = qts.readAll();
    qf.close();
    if(qf.error()!=QFile::NoError) {
      std::stringstream errstr;
      errstr << "Exception opening/reading file \""<< VioStyle::StrFromQStr(rFilename) << "\"";
      throw faudes::Exception("ExportFaudesFile", errstr.str(), 1);
    }
    mpFaudesLuaFunctionDefinition->LuaCode(VioStyle::StrFromQStr(mLuaCode));
  }
  // update all 
  DoVioUpdate();
}

// (user) edit code
void VioLuaFunctionModel::VioLuaCode(const QString& code) {
  // bail out on trivial
  if(mLuaCode==code) return;
  // doit
  FD_DQL("VioLuaFunctionModel::VioLuaCode(): set #" << code.size());
  mLuaCode=code;
  mpFaudesLuaFunctionDefinition->LuaCode(
    VioStyle::StrFromQStr(mLuaCode));
  // we notify
  emit NotifyCodeChange();
}

// set/get default layout
const VioLuaFunctionLayout& VioLuaFunctionModel::Layout(void) { return *mUserLayout; };
void VioLuaFunctionModel::Layout(const VioLuaFunctionLayout& layout) { *mUserLayout=layout; };


// (user) edit variants
void VioLuaFunctionModel::VioVariants(const QList<faudes::Signature>& variants) {
  // bail out on plain script
  if(pLuaStyle->mPlainScript) return;
  // bail out on trivial
  if(mVariants==variants) return;
  // doit
  FD_DQL("VioLuaFunctionModel::VioVariants(): set #" << variants.size());
  mVariants.clear();
  mpFaudesLuaFunctionDefinition->ClearVariants();
  for(int i=0; i< variants.size(); i++)  {
    const faudes::Signature& sig=variants.at(i);
    if(mpFaudesLuaFunctionDefinition->ExistsVariant(sig.Name())) 
      continue; // ignore doublets
    mpFaudesLuaFunctionDefinition->AppendVariant(sig);
    mVariants.append(variants.at(i));
  }
  // we notify
  emit NotifySignatureChange();  
}    

// (user) edit variant
void VioLuaFunctionModel::VioVariant(int pos, const faudes::Signature& signature) {
  // bail out on plain script
  if(pLuaStyle->mPlainScript) return;
  // bail out on invalid
  if(pos<0 || pos >= mVariants.size()) return;
  // bail out on trivial
  if(mVariants.at(pos)==signature) return;
  // bail out on doublets
  int opos=mpFaudesLuaFunctionDefinition->VariantIndex(signature.Name());
  if(opos!= pos && opos !=-1) return; 
  // doit vio
  FD_DQL("VioLuaFunctionModel::VioVariant(): set #" << pos);
  mVariants[pos]=signature;
  // copy to faudes
  mpFaudesLuaFunctionDefinition->ClearVariants();
  for(int i=0; i< mVariants.size(); i++)  {
    mpFaudesLuaFunctionDefinition->AppendVariant(mVariants.at(i));
  }
  // we notify
  emit NotifySignatureChange();  
}    


// test/provoke errors (todo: fork/process etc)
void VioLuaFunctionModel::TestScript(void) {
  FD_DQL("VioLuaFunctionView::TestScript()");
  // ask al views to commit pending changes
  emit NotifyFlush();
  // syntax check
  VioLuaExecute* exec = new VioLuaExecute(this,true);
  QString err=exec->Execute();
  delete exec;
  // done
  if(err=="") err="syntax check passed";
  emit StatusMessage(err);
  FD_DQL("VioLuaFunctionView::TestScript(): done");
}


// run script (todo: fork/process etc)
void VioLuaFunctionModel::RunScript(void) {
  FD_DQL("VioLuaFunctionView::RunScript()");
  // ask al views to commit pending changes
  emit NotifyFlush();
  // do run
  VioLuaExecute* exec = new VioLuaExecute(this);
  QString err=exec->Execute();
  delete exec;
  // done
  if(err=="") err="evaluation complete";
  emit StatusMessage(err);
  FD_DQL("VioLuaFunctionView::RunScript(): done");
}




// collect and pass on modifications of childs
void VioLuaFunctionModel::ChildModified(bool changed) { 
  FD_DQL("VioLuaFunctionModel::ChildModified("<<changed<<"): model modified " << mModified);
  // ignore negatives
  if(!changed) return;
  // report
  FD_DQL("VioLuaFunctionModel::ChildModified(1): model modified " << mModified);
  Modified(true);
};


// query changes (dont emit signal)
bool VioLuaFunctionModel::Modified(void) const { 
  return mModified;
};


// record changes and emit signal
void VioLuaFunctionModel::Modified(bool ch) { 
  // set
  if(!mModified && ch) {
    mModified=true;
    FD_DQL("VioLuaFunctionModel::Modified(" << this << "): emit set modified notification");
    emit NotifyModified(mModified);
  }
  // clr
  if(mModified && !ch) {
    mModified=false;
    FD_DQL("VioLuaFunctionModel::Modified(" << this << "): emit clr modified notification");
    emit NotifyModified(mModified);
  }
}



/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioLuaFunctionLayout

****************************************************************
****************************************************************
****************************************************************
*/

// defaults
VioLuaFunctionLayout::VioLuaFunctionLayout(QObject* parent) : QObject(parent) {
  mPropBuiltIn=false;
  mPropSize=150;
  mListSize=150;
  mTextSize=-1; // system default
}

// assignment
VioLuaFunctionLayout& VioLuaFunctionLayout::operator=(const VioLuaFunctionLayout& rSrc) {
  mPropBuiltIn=rSrc.mPropBuiltIn;
  mPropSize=rSrc.mPropSize;
  mListSize=rSrc.mListSize;
  mTextSize=rSrc.mTextSize;
  return *this;
}  

// write layout
void VioLuaFunctionLayout::Write(faudes::TokenWriter& rTw) const {
  rTw.WriteBegin("VioLayout");
  rTw.WriteInteger(mPropBuiltIn);
  rTw.WriteInteger(mPropSize);
  rTw.WriteInteger(mListSize);
  rTw.WriteInteger(mTextSize);
  rTw.WriteEnd("VioLayout");
}

// read layout 
void VioLuaFunctionLayout::Read(faudes::TokenReader& rTr) {
  try {
    rTr.ReadBegin("VioLayout");
    mPropBuiltIn= rTr.ReadInteger();
    mPropSize= rTr.ReadInteger();
    mListSize= rTr.ReadInteger();
    // compatible with pre 0.73
    faudes::Token token;
    rTr.Peek(token);
    if(token.IsInteger()) mTextSize=rTr.ReadInteger();
    rTr.ReadEnd("VioLayout");
  } catch(faudes::Exception expection) {
  } 
}


/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioLuaFunctionView

****************************************************************
****************************************************************
****************************************************************
*/

// construct
VioLuaFunctionView::VioLuaFunctionView(QWidget* parent, VioStyle* config, bool alloc) : 
  VioView(parent,config,false),
  pLuaFunctionModel(0),
  mCodeEdit(0),
  mUserLayout(0),
  mFindDialog(0)
{
  FD_DQL("VioLuaFunctionView::VioLuaFunctionView(): " << VioStyle::StrFromQStr(FaudesType()));
  // have typed style
  pLuaStyle=dynamic_cast<VioLuaStyle*>(pConfig);
  if(!pLuaStyle) {
    FD_WARN("VioLuaFunctionView::VioLuaFunctionView(): invalid style, using default.");
    pLuaStyle= new VioLuaStyle(mFaudesType);
  }
  // my alloc
  if(alloc) DoVioAllocate();
  FD_DQL("VioLuaFunctionView::VioLuaFunctionView(): done");
}

// destruct
VioLuaFunctionView::~VioLuaFunctionView(void) {
}

// typed faudes object access
const faudes::LuaFunctionDefinition* VioLuaFunctionView::LuaFunctionDefinition(void) const {
  if(!pLuaFunctionModel) return 0;
  return pLuaFunctionModel->LuaFunctionDefinition();
}

// allocate view data
void VioLuaFunctionView::DoVioAllocate(void) {
  FD_DQL("VioLuaFunctionView::DoVioAllocate()");
  // call base
  VioView::DoVioAllocate();
  // add my features
  mCodeEdit = new VioLuaCodeEditor();
  mCodeEdit->setTextInteractionFlags(Qt::TextSelectableByKeyboard | Qt::TextEditable); 
  // mCodeEdit->installEventFilter(this);
  QObject::connect(mCodeEdit,SIGNAL(modificationChanged(bool)),this,SLOT(ChildModified(bool)));
  // have buil in prop view
  mPropView=0;
  if(!pLuaStyle->mPlainScript)
    mPropView = new VioLuaFunctionPropertyView();
  // add via splitter
  mSplitter = new QSplitter();
  mSplitter->addWidget(mCodeEdit);
  if(mPropView)
    mSplitter->addWidget(mPropView);   
  mVbox->addWidget(mSplitter);
  // my find dialog
  mFindDialog = new VioFindDialog(this);
  mFindDialog->Replace(true);
  mFindPattern="";
  mFindReplace="";
  mFindFlags=0;
  // my actions
  mFindAction = new QAction("Find ...",this);
  mFindAction->setEnabled(true);
  mFindAction->setShortcut(tr("Ctrl+F"));
  mEditActions.append(mFindAction);
  mAgainAction = new QAction("Find Again",this);
  mAgainAction->setEnabled(true);
  mAgainAction->setShortcut(tr("Ctrl+G"));
  mEditActions.append(mAgainAction);
  QAction* sepaction = new QAction(this);
  sepaction->setSeparator(true);
  mEditActions.append(sepaction);
  mTestAction = new QAction("Syntax Check",this);
  mTestAction->setEnabled(false);
  mEditActions.append(mTestAction);
  mRunAction = new QAction("Execute Script",this);
  mRunAction->setEnabled(false);
  mRunAction->setShortcut(tr("Ctrl+Shift+E"));
  mEditActions.append(mRunAction);
  if(pLuaStyle->mPlainScript) 
    mRunAction->setEnabled(true);
  else
    mTestAction->setEnabled(true);
  mZoomInAction = new QAction("Zoom In",this);
  mZoomInAction->setEnabled(true);
  mZoomInAction->setShortcut(tr("Ctrl++"));
  mViewActions.append(mZoomInAction);
  mZoomOutAction = new QAction("Zoom Out",this);
  mZoomOutAction->setEnabled(true);
  mZoomOutAction->setShortcut(tr("Ctrl+-"));
  mViewActions.append(mZoomOutAction);
  // have property action
  mPropAction=0;
  if(mPropView) {
    mPropAction=new QAction("&Properties",this);
    mPropAction->setStatusTip("show/hide property view");
    mPropAction->setCheckable(true);
    mPropAction->setEnabled(true);
    mPropAction->setChecked(!mPropView->isHidden());
    mViewActions.append(mPropAction); 
    connect(mPropAction, SIGNAL(triggered(bool)), this, SLOT(ShowPropertyView(bool)));
  }
  // have a user layout
  mUserLayout= new VioLuaFunctionLayout(this);
  // connect prop changes
  if(mPropView)
    connect(mPropView,SIGNAL(NotifyModified(bool)),this,SLOT(ChildModified(bool)));
  // connections
  QObject::connect(mFindAction,SIGNAL(triggered(bool)),
     this,SLOT(FindDialog(void)));
  QObject::connect(mAgainAction,SIGNAL(triggered(bool)),
     this,SLOT(FindAgain(void)));
  QObject::connect(mZoomInAction,SIGNAL(triggered(bool)),
     this,SLOT(ZoomIn(void)));
  QObject::connect(mZoomOutAction,SIGNAL(triggered(bool)),
     this,SLOT(ZoomOut(void)));
  // done
  FD_DQL("VioLuaFunctionView::DoVioAllocate(): done");
}

// update view from (new) model
void VioLuaFunctionView::DoVioUpdate(void) {
  FD_DQL("VioLuaFunctionView::DoVioUpdate(): type " << VioStyle::StrFromQStr(FaudesType()));
  // call base
  VioView::DoVioUpdate();
  // have typed model reference (incl set to 0)
  pLuaFunctionModel=qobject_cast<VioLuaFunctionModel*>(pModel);
  // bail out on no model
  if(!pLuaFunctionModel) return;
  // report
  FD_DQL("VioLuaFunctionView::DoVioUpdate: #" <<  pLuaFunctionModel->VioLuaCode().size());
  // set text
  mCodeEdit->setPlainText(pLuaFunctionModel->VioLuaCode());
  mCodeEdit->document()->setModified(false);
  // pass on to propview
  if(mPropView) mPropView->Model(pModel);
  // fix my action
  QObject::disconnect(mTestAction,0,0,0);
  QObject::disconnect(mRunAction,0,0,0);
  QObject::connect(mTestAction,SIGNAL(triggered(bool)),
     pLuaFunctionModel,SLOT(TestScript(void)));
  QObject::connect(mRunAction,SIGNAL(triggered(bool)),
     pLuaFunctionModel,SLOT(RunScript(void)));
  // reconnect modification not
  QObject::disconnect(this,SIGNAL(NotifyModified(bool)),0,0);
  QObject::connect(this,SIGNAL(NotifyModified(bool)),pLuaFunctionModel,
    SLOT(ChildModified(bool)));
  // get and set user layout
  *mUserLayout =pLuaFunctionModel->Layout();
  UpdateUserLayout(); 
  FD_DQL("VioLuaFunctionView::DoVioUpdate(): done (modified " << mModified <<")");
}

void VioLuaFunctionView::UpdateFromCodeEdit(void) {
  // bail out if widget is not ready or model not there
  if(!mCodeEdit || !pLuaFunctionModel) return;
  FD_DQL("VioLuaFunctionView::UpdateFromCodeEdit()");
  // set changed (the view is now saved)
  Modified(false);
  // doit  
  pLuaFunctionModel->VioLuaCode(mCodeEdit->toPlainText());
}



// update model from view (we are a lazy view and thus really need this)
void VioLuaFunctionView::DoModelUpdate(void) {
  FD_DQL("VioLuaFunctionView::DoModelUpdate(): type " << VioStyle::StrFromQStr(FaudesType()));
  // bail out if widget is not ready or model not there
  if(!mCodeEdit || !pModel) return;
  // update my features
  UpdateFromCodeEdit();
  SaveUserLayout();
}


// query changes (dont emit signal)
bool VioLuaFunctionView::Modified(void) const { 
  return mModified;
};


// record changes and emit signal
void VioLuaFunctionView::Modified(bool ch) { 
  // set
  if(!mModified && ch) {
    mModified=true;
    FD_DQL("VioLuaFunctionView::Modified(" << this << "): emit set modified notification");
    emit NotifyModified(mModified);
  }
  // clr
  if(mModified && !ch) {
    mModified=false;
    FD_DQL("VioLuaFunctionView::Modified(" << this << "): emit clr modified notification");
    emit NotifyModified(mModified);
  }
  // clr my widgets 
  if(!ch && mCodeEdit) 
    mCodeEdit->document()->setModified(false);
  if(mPropView) mPropView->Modified(false);
}

// collect and pass on modifications of childs
void VioLuaFunctionView::ChildModified(bool changed) { 
  // ignore negatives
  if(!changed) return;
  // report
  FD_DQL("VioLuaFunctionView::ChildModified(1): model modified " << mModified);
  Modified(true);
};


// show a line
void VioLuaFunctionView::Show(const VioElement& elem) {
  FD_DQL("VioLuaFunctionView::Show(): " << elem.Str());
  if(!elem.IsLine()) return;
  mCodeEdit->ShowLine(elem.Line());
}

// show/hide views from layout
void VioLuaFunctionView::UpdateUserLayout(void) {
  FD_DQL("VioLuaFunctionView::UpdateUserLayout()");
  // deal with size
  mCodeEdit->FontSize(mUserLayout->mTextSize);
  // bail out on no splitter
  if(!mSplitter) return;
  if(mSplitter->count()!=2) return;
  // doit
  if(mPropView) {
    mSplitter->widget(1)->setVisible(mUserLayout->mPropBuiltIn);
    mPropAction->setChecked(mUserLayout->mPropBuiltIn);
  }
  // propview size
  int w;
  if(mPropView) {
    QList<int> sz = mSplitter->sizes();
    sz[0] = mUserLayout->mListSize;
    sz[1] = mUserLayout->mPropSize;
    w = sz[0]+sz[1];
    mSplitter->setSizes(sz);
  }
  // done
  FD_DQL("VioLuaFunctionView::UpdateUserLayout(): done " << w);
}

// retrieve layout
void VioLuaFunctionView::SaveUserLayout(void) {
  FD_DQL("VioLuaFunctionView::SaveUserLayout()");
  // do splitter
  if(mSplitter) 
  if(mSplitter->count()==2) {
    QList<int> sz = mSplitter->sizes();
    // show/hide 
    if(mPropView) {
      mUserLayout->mPropBuiltIn=mSplitter->widget(1)->isVisible();
      if(mUserLayout->mPropBuiltIn) {
        mUserLayout->mPropSize=sz[1];
        mUserLayout->mListSize=sz[0];
      }
    }
  }
  // save to model
  if(pLuaFunctionModel) pLuaFunctionModel->Layout(*mUserLayout);
  FD_DQL("VioLuaFunctionView::SaveUserLayout():");
}

// show/hide  abstract view
void VioLuaFunctionView::ShowPropertyView(bool on) {
  // bail out on invalid
  if(!mPropView) return;
  if(mSplitter->count()!=2) return;
  // get data
  QList<int> sizes = mSplitter->sizes();
  FD_DQL("VioLuaFunctionView::ShowPropertyView(): " << on);
  if(!on && mSplitter->widget(1)->isVisible()) {
    mUserLayout->mListSize=sizes[0];
    mUserLayout->mPropSize=sizes[1];
    mSplitter->widget(1)->setVisible(false);
  } 
  if(on && !mSplitter->widget(1)->isVisible()) {
    QList<int> sizes = mSplitter->sizes();
    sizes[0]= mUserLayout->mListSize;
    sizes[1]= mUserLayout->mPropSize;
    if(sizes[1]==0) sizes[1]=150;
    mSplitter->widget(1)->setVisible(true);
    mSplitter->setSizes(sizes);
  }
  mPropAction->setChecked(on);
  // record layout
  SaveUserLayout();
}
 
// reimplemengt std edit from viotype
void VioLuaFunctionView::Cut(void) {
  mCodeEdit->cut();
  UpdateModel();
}

// reimplemengt std edit from viotype
void VioLuaFunctionView::Copy(void) {
  mCodeEdit->copy();
}

// reimplemengt std edit from viotype
void VioLuaFunctionView::Paste(void) {
  mCodeEdit->paste();
  UpdateModel();
}

// reimplemengt std edit from viotype
void VioLuaFunctionView::Undo(void) {
  mCodeEdit->undo();
  UpdateModel();
}

// reimplemengt std edit from viotype
void VioLuaFunctionView::Redo(void) {
  mCodeEdit->redo();
  UpdateModel();
}

// find string
void VioLuaFunctionView::Find(const QString& pattern, const QString& replace, QTextDocument::FindFlags flags) {
  FD_DQH("VioLuaFunctionView::Find(" << VioStyle::StrFromQStr(pattern) << ")");
  mFindPattern=pattern;
  mFindReplace=replace;
  mFindFlags=flags;
  // reset position
  if(mFindFlags & 0x1000) {
    if(mFindFlags & QTextDocument::FindBackward)
      mCodeEdit->moveCursor(QTextCursor::End);
    else
      mCodeEdit->moveCursor(QTextCursor::Start);
  }
  // doit find
  bool res=mCodeEdit->find(mFindPattern, mFindFlags & (~0xf000) );
  if(res) mCodeEdit->ensureCursorVisible();
  // doit replace
  if(res && (mFindFlags & 0x2000)) {
    mCodeEdit->textCursor().removeSelectedText();
    mCodeEdit->textCursor().insertText(mFindReplace);
  } 

}

// find string
void VioLuaFunctionView::FindAgain(void) {
  Find(mFindPattern, mFindReplace, mFindFlags & ((QTextDocument::FindFlag) (~ 0x1000)));
}

// find string
void VioLuaFunctionView::FindDialog(void) {
  FD_DQH("VioLuaFunctionView::FindDialog()");
  if(mFindDialog->exec()==QDialog::Accepted) 
    Find(mFindDialog->Pattern(), mFindDialog->Replace(), mFindDialog->Flags());
  FD_DQH("VioLuaFunctionView::FindDialog(): done");
}

// zoom
void VioLuaFunctionView::ZoomIn(void) {
  FD_DQH("VioLuaFunctionView::ZoomIn()");
  int sz = (mCodeEdit->FontSize() * 1.1) + 1;
  mCodeEdit->FontSize(sz);
  mUserLayout->mTextSize=mCodeEdit->FontSize();
}

// zoom
void VioLuaFunctionView::ZoomOut(void) {
  FD_DQH("VioLuaFunctionView::ZoomOut()");
  int sz = (mCodeEdit->FontSize() * 0.9) - 1;
  mCodeEdit->FontSize(sz);
  mUserLayout->mTextSize=mCodeEdit->FontSize();
}

/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioLuaFunctionPropertyView

****************************************************************
****************************************************************
****************************************************************
*/

// construct
VioLuaFunctionPropertyView::VioLuaFunctionPropertyView(QWidget* parent, VioStyle* config, bool alloc) : 
  VioView(parent,config,false),
  mSignatureList(0),
  mParameterTable(0)
{
  FD_DQL("VioLuaFunctionPropertyView::VioLuaFunctionPropertyView(): " << VioStyle::StrFromQStr(FaudesType()));
  // fix type since we dont use config style
  mFaudesType="LuaFunctionDefinition";
  // my alloc
  if(alloc) DoVioAllocate();
  FD_DQL("VioLuaFunctionPropertyView::VioLuaFunctionPropertyView(): done");
}

// destruct
VioLuaFunctionPropertyView::~VioLuaFunctionPropertyView(void) {
}

// typed faudes object access
const faudes::LuaFunctionDefinition* VioLuaFunctionPropertyView::LuaFunctionDefinition(void) const {
  if(!pLuaFunctionModel) return 0;
  return pLuaFunctionModel->LuaFunctionDefinition();
}

// allocate view data
void VioLuaFunctionPropertyView::DoVioAllocate(void) {
  FD_DQL("VioLuaFunctionPropertyView::DoVioAllocate()");
  // call base
  VioView::DoVioAllocate();
  // add my features
  mSignatureList = new VioSymbolTableWidget();
  mSignatureList->setHeader("Variants");
  mSignatureList->setSymbolMode(VioSymbol::AnyString);
  mParameterTable = new VioSymbolTableWidget();
  mParameterTable->setDimensions(0,3);
  mParameterTable->setHeader(QStringList() << "Parameter" << "Type" << "Attrib");
  mParameterTable->setSymbolMode(0,VioSymbol::AnyString);
  mParameterTable->setSymbolMode(1,VioSymbol::KnownSymbolsOnly);
  mParameterTable->setSymbolMode(2,VioSymbol::KnownSymbolsOnly);
  mParameterTable->setSymbolWorld(1,VioTypeRegistry::UserTypes());
  mParameterTable->setSymbolWorld(2,QStringList() << "In" << "Out" << "InOut");
  mParameterTable->setEnabled(false);
  //mVbox->addWidget(mSignatureList);
  //mVbox->addWidget(mParameterTable);
  QGroupBox* gb1 = new QGroupBox("Variant");
  QVBoxLayout* vb1 = new QVBoxLayout(gb1);
  vb1->setMargin(0);
  vb1->addWidget(mSignatureList);
  QGroupBox* gb2 = new QGroupBox("Signature");
  QVBoxLayout* vb2 = new QVBoxLayout(gb2);
  vb2->setMargin(0);
  vb2->addWidget(mParameterTable);
  mVbox->addSpacing(10); // why??
  mVbox->addWidget(gb1);
  mVbox->addWidget(gb2);
  // ionitialise
  mCurrentVariant=0;
  // connections: navigation
  connect(mSignatureList,SIGNAL(currentRowChanged(int)),this,SLOT(ShowVariant(int)));
  connect(mSignatureList,SIGNAL(editingFinished(int,int)),this,SLOT(UpdateCurrentNameFromWidgets()));
  connect(mSignatureList,SIGNAL(resizeModel()),this,SLOT(UpdateAllFromWidgets()));
  connect(mParameterTable,SIGNAL(editingFinished(int,int)),this,SLOT(UpdateCurrentSignatureFromWidgets()));
  connect(mParameterTable,SIGNAL(resizeModel()),this,SLOT(UpdateCurrentSignatureFromWidgets()));
  FD_DQL("VioLuaFunctionPropertyView::DoVioAllocate(): done");
}

// update view from (new) model
void VioLuaFunctionPropertyView::DoVioUpdate(void) {
  FD_DQL("VioLuaFunctionPropertyView::DoVioUpdate()");
  // call base
  VioView::DoVioUpdate();
  // have typed model reference (incl set to 0)
  pLuaFunctionModel=qobject_cast<VioLuaFunctionModel*>(pModel);
  // bail out on no model
  if(!pLuaFunctionModel) return;
  // report
  FD_DQL("VioLuaFunctionPropertyView::DoVioUupdate: #" <<  pLuaFunctionModel->VioLuaCode().size());
  // set contents: variants
  QStringList siglist;
  for(int i=0; i<pLuaFunctionModel->VioVariants().size(); i++)
    siglist << VioStyle::QStrFromStr(pLuaFunctionModel->VioVariants().at(i).Name());
  if(mSignatureList->symbolList()!=siglist) 
    mSignatureList->setSymbolList(siglist);
  // set contents: current signature
  ShowVariant(mCurrentVariant);
  // reset changes
  Modified(false);
  FD_DQL("VioLuaFunctionPropertyView::DoVioUpdate(): done");
}

// show a signature
void VioLuaFunctionPropertyView::ShowVariant(int pos) {
  // bail out on no model
  if(!pLuaFunctionModel) return;
  // show void
  if(pos<0 || pos>= mSignatureList->rowCount()) {
    FD_DQL("VioLuaFunctionPropertyView::ShowVariant(" << pos <<"): out of range");
    mParameterTable->setDimensions(0,3);
  }
  // bail out on no signatures
  if(pLuaFunctionModel->VioVariants().size()==0) return;
  // doit
  if(pos>=0 && pos< mSignatureList->rowCount()) {
    std::string signame = VioStyle::StrFromQStr(mSignatureList->Symbol(pos));
    FD_DQL("VioLuaFunctionPropertyView::ShowVariant(" << pos <<"): " << signame);
    faudes::Signature signature;
    signature.Name(signame);
    if(pLuaFunctionModel->LuaFunctionDefinition()->ExistsVariant(signame))
      signature=pLuaFunctionModel->LuaFunctionDefinition()->Variant(signame);
    mParameterTable->setDimensions(signature.Size(),3);
    for(int j=0; j<signature.Size(); j++) {
      mParameterTable->setSymbol(j,0,VioStyle::QStrFromStr(signature.At(j).Name()));     
      mParameterTable->setSymbol(j,1,VioStyle::QStrFromStr(signature.At(j).Type()));     
      mParameterTable->setSymbol(j,2,VioStyle::QStrFromStr(
       faudes::Parameter::AStr(signature.At(j).Attribute())));     
    }
   mParameterTable->setEnabled(true);
  }
  // disable
  else {
    mParameterTable->setEnabled(false);
  }
  // record
  mCurrentVariant=pos;
  // select
  mSignatureList->setCurrentRow(pos);
}

// get data from view
void VioLuaFunctionPropertyView::UpdateAllFromWidgets() {
  // bail out if widget is not ready or model not there
  if(!mSignatureList || !pLuaFunctionModel) return;
  FD_DQL("VioLuaFunctionPropertyView::UpdateAllFromWidgets()");
  // collect variants
  QList<faudes::Signature> variants;
  // doit: names from widget, signatures from faudes object
  for(int i=0; i<mSignatureList->rowCount(); i++) {
    std::string signame = VioStyle::StrFromQStr(mSignatureList->Symbol(i));
    FD_DQL("VioLuaFunctionPropertyView::UpdateAllFromWidgets(): " << signame);
    faudes::Signature signature;
    signature.Name(signame);
    if(pLuaFunctionModel->LuaFunctionDefinition()->ExistsVariant(signame))
      signature=pLuaFunctionModel->LuaFunctionDefinition()->Variant(signame);
    variants.append(signature);
  }
  // doit: current signature from widget
  /*
  if(mCurrentVariant>=0 && mCurrentVariant < variants.size()) {
    faudes::Signature signature;
    for(int j=0; j<mParameterTable->rowCount(); j++) {
      faudes::Parameter param;
      param.Name(VioStyle::StrFromQStr(mParameterTable->Symbol(j,0)));
      param.Type(VioStyle::StrFromQStr(mParameterTable->Symbol(j,1)));
      param.Attribute(VioStyle::StrFromQStr(mParameterTable->Symbol(j,2)));
      signature.Append(param);
    }
    signature.Name(variants.at(mCurrentVariant).Name());
    variants[mCurrentVariant]=signature;
  }
  */
  // doit: set
  pLuaFunctionModel->VioVariants(variants);
  // set changed
  Modified(true);
}

// get data from view
void VioLuaFunctionPropertyView::UpdateCurrentNameFromWidgets() {
  // bail out if widget is not ready or model not there
  if(!mSignatureList || !pLuaFunctionModel) return;
  // bail out on invalid current
  if(mCurrentVariant<0 || mCurrentVariant >= mSignatureList->rowCount()) 
    return;
  if(mCurrentVariant<0 || mCurrentVariant >= pLuaFunctionModel->VioVariants().size()) 
    return;
  FD_DQL("VioLuaFunctionPropertyView::UpdateCurrentNameFromWidgets()");
  std::string signame = VioStyle::StrFromQStr(mSignatureList->Symbol(mCurrentVariant));
  faudes::Signature signature =
    pLuaFunctionModel->LuaFunctionDefinition()->Variant(mCurrentVariant);
  signature.Name(signame);
  pLuaFunctionModel->VioVariant(mCurrentVariant,signature);
  // set changed
  Modified(true);
}

// get data from view
void VioLuaFunctionPropertyView::UpdateCurrentSignatureFromWidgets() {
  // bail out if widget is not ready or model not there
  if(!mSignatureList || !pLuaFunctionModel) return;
  // bail out on invalid current
  if(mCurrentVariant<0 || mCurrentVariant >= pLuaFunctionModel->VioVariants().size()) 
    return;
  // retrieve signature
  FD_DQL("VioLuaFunctionPropertyView::UpdateCurrentSignatureFromWidgets(): " << mCurrentVariant);
  std::string signame = pLuaFunctionModel->VioVariants().at(mCurrentVariant).Name();
  FD_DQL("VioLuaFunctionPropertyView::UpdateCurrentFromWidgets(): " << signame);
  faudes::Signature signature;
  signature.Name(signame);
  for(int j=0; j<mParameterTable->rowCount(); j++) {
    faudes::Parameter param;
    param.Name(VioStyle::StrFromQStr(mParameterTable->Symbol(j,0)));
    param.Type(VioStyle::StrFromQStr(mParameterTable->Symbol(j,1)));
    param.Attribute(VioStyle::StrFromQStr(mParameterTable->Symbol(j,2)));
    signature.Append(param);
  }
  // bail out on trivial
  if(signature==pLuaFunctionModel->VioVariants().at(mCurrentVariant)) return;
  // set
  FD_DQL("VioLuaFunctionPropertyView::UpdateCurrentFromWidgets(): set");
  pLuaFunctionModel->VioVariant(mCurrentVariant,signature);
  Modified(true);
}


// update model from view (we are a lazy view and thus really need this)
void VioLuaFunctionPropertyView::DoModelUpdate(void) {
  FD_DQL("VioLuaFunctionPropertyView::DoModelUpdate(): type " << VioStyle::StrFromQStr(FaudesType()));
  // bail out if widget is not ready or model not there
  if(!mSignatureList || !pModel) return;
  // update my features
  UpdateAllFromWidgets();
}


// query changes (dont emit signal)
bool VioLuaFunctionPropertyView::Modified(void) const { 
  return mModified;
};


// record changes and emit signal
void VioLuaFunctionPropertyView::Modified(bool ch) { 
  FD_DQL("VioLuaFunctionPropertyView::Modified(" << this << ")");
  // set
  if(!mModified && ch) {
    mModified=true;
    emit NotifyModified(mModified);
  }
  // clr
  if(mModified && !ch) {
    mModified=false;
    FD_DQL("VioLuaFunctionPropertyView::Modified(" << this << "): emit modified notification");
    emit NotifyModified(mModified);
  }
  // clr my widgets 
  if(!ch && mSignatureList) {
    //mSignatureList->Modified(false);
    //mParameterTable->Modified(false);
  }
}

// collect and pass on modifications of childs
void VioLuaFunctionPropertyView::ChildModified(bool changed) { 
  // ignore negatives
  if(!changed) return;
  // report
  FD_DQL("VioLuaFunctionPropertyView::ChildModified(1): model modified " << mModified);
  Modified(true);
};

/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioLuaFunctionWidget

****************************************************************
****************************************************************
****************************************************************
*/

// construct
VioLuaFunctionWidget::VioLuaFunctionWidget(QWidget* parent, VioStyle* config, bool alloc) : 
  VioWidget(parent,config,false) 
{
  FD_DQL("VioLuaFunctionWidget::VioLuaFunctionWidget()");
  // allocate model and view
  if(alloc) {
    // have view
    mView= new VioLuaFunctionView(this,pConfig);
    mVbox->addWidget(mView);
    // fix typed refs and connect to widget
    DoVioAllocate();
    // have a model and set
    Model(new VioLuaFunctionModel(this,pConfig));
  }
  // fix modified flag
  mModified=false;
  FD_DQL("VioLuaFunctionWidget::VioLuaFunctionWidget(): done");
}

// destruct
VioLuaFunctionWidget::~VioLuaFunctionWidget(void) {
}

// fix view
void VioLuaFunctionWidget::DoVioAllocate(void) {
  // call base to connect
  VioWidget::DoVioAllocate();
}

// set by vio model
int VioLuaFunctionWidget::Model(VioModel* model) {
  FD_DQL("VioLuaFunctionWidget::Model(" << model << "): config type " << VioStyle::StrFromQStr(FaudesType()));
  // bail out on identity
  if(model==mModel) return 0;
  // call base to set view and connects (incl callback diovioupdate)
  int res=VioWidget::Model(model);
  // update typed ref
  pLuaFunctionModel=qobject_cast<VioLuaFunctionModel*>(mModel);
  // done
  FD_DQL("VioLuaFunctionWidget::Model(" << model << "): done");  
  return res;
}



/*
 ************************************************
 ************************************************

 implementation of VioLuaExecute 

 ************************************************
 ************************************************
 */


// construct/destruct
VioLuaExecute::VioLuaExecute(VioLuaFunctionModel* lfnct, bool test) : 
  QThread(lfnct) 
{
  pLfnct=lfnct;
  mTest=test;
};

// destruct  
VioLuaExecute::~VioLuaExecute(void) {
};


// run (this is the thread itself, called by start()
void VioLuaExecute::run(void) {
  FD_DQL("VioLuaExecute::run()");
  if(mTest)
    mErrString=VioStyle::QStrFromStr(pLFfnct->SyntaxCheck());
  else
    mErrString=VioStyle::QStrFromStr(pLFfnct->Evaluate());
  // done
  FD_DQL("VioLuaExecute::run(): done");
};


// api: this is callers thread
QString VioLuaExecute::Execute(void) {
  // prepare
  mErrString="";
  const faudes::LuaFunctionDefinition* lfnct = pLfnct->LuaFunctionDefinition();
  if(!lfnct) { 
    mErrString="could not access lua code";
    return mErrString;
  } 
  pLFfnct = const_cast<faudes::LuaFunctionDefinition*>(lfnct);
  // doit
  QApplication::processEvents();
  QApplication::flush();
  // have a progress dialog
  QProgressDialog progress("Evaluating: "+pLfnct->FaudesName(), "Cancel", 0, 0, 0); 
  progress.setWindowModality(Qt::ApplicationModal);
  progress.setValue(0);
  QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
  QApplication::flush();
  // clear global break flag
  VioStyle::FaudesBreakClr(); 
  // run extra thread
  start();
  // process applicationevent loop while running
  FD_DS("WspExecute():: thread started");
  long int i=100; 
  while(isRunning() && (--i)>0) {
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents,10);
    QApplication::flush();
    wait(10);
  }
  FD_DS("WspExecute():: progress on");
  bool canceled=false;
  while(isRunning()) {
    QApplication::processEvents(QEventLoop::AllEvents,10);
    QApplication::flush();
    if(progress.wasCanceled() && !canceled) {
       VioStyle::FaudesBreakSet(); 
       FD_DS("WspExecute():: canceled");
       canceled=true;
    }
    progress.setValue(1); 
    wait(10);
  }
  // clear global break flag
  VioStyle::FaudesBreakClr(); 
  // parse exception message for line number
  if(mErrString.startsWith("error in Lua script:")) {
    int c1 = mErrString.indexOf(": line ");
    if(c1<0) c1=0;
    else c1+=QString(": line").length();     
    int c2 = mErrString.indexOf(':',c1+1);
    if(c2<0) c2=1;
    int line = mErrString.mid(c1+1,c2-c1-1).toInt();  
    if(c2>1) {
      FD_DQL("VioLuaFunctionView::TestScript(): show line " << line);
      mErrString=QString("line %1: %2").arg(line).arg(mErrString.mid(c2+2));
    }
  }
  // done
  return mErrString;
}


