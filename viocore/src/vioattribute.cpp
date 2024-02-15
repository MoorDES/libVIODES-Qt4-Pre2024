/* vioattribute.cpp  - vio attribute model and widget */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2009  Thomas Moor, Ruediger Berndt

*/


#include "vioattribute.h"

/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioAttributeModel

****************************************************************
****************************************************************
****************************************************************
*/

// construct
VioAttributeModel::VioAttributeModel(QObject* parent, VioStyle* config, bool alloc) : 
  VioModel(parent, config, false), 
  pContext(0),
  mMerged(false)
{
  FD_DQT("VioAttributModel::VioAttributeModel(): " << VioStyle::StrFromQStr(mFaudesType));
  // have typed style
  pAttributeStyle=dynamic_cast<const VioAttributeStyle*>(pConfig);
  if(!pAttributeStyle) {
    FD_WARN("VioAttributModel::VioAttributeModel(): invalid style, using default.");
    pAttributeStyle= new VioAttributeStyle(mFaudesType);
  }
  pBooleanProperties=&pAttributeStyle->BooleanProperties();
  // allocate faudes object and have default rep data
  if(alloc) {
    DoVioAllocate();
    DoVioUpdate();
  }
  FD_DQT("VioAttributModel::VioAttributeModel(): done");
}

// destruct
VioAttributeModel::~VioAttributeModel(void) {
}

// construct on heap
VioAttributeModel* VioAttributeModel::NewModel(QObject* parent) const {
  FD_DQT("VioAttributeModel::NewModel(): type " << VioStyle::StrFromQStr(mFaudesType));
  return new VioAttributeModel(parent,pConfig);
}

// construct view on heap
VioView* VioAttributeModel::NewView(QWidget* parent) const {
  FD_DQT("VioAttributeModel::NewView(): " << VioStyle::StrFromQStr(FaudesType()));
  return new VioAttributeView(parent, pConfig);
}

// construct on heap
VioWidget* VioAttributeModel::NewWidget(QWidget* parent) const {
  FD_DQT("VioAttributeMode::NewWidget(): type " << VioStyle::StrFromQStr(FaudesType()));
  return new VioAttributeWidget(parent, pConfig);
}


// allocate faudes object
void VioAttributeModel::DoFaudesAllocate(void) {
  FD_DQT("VioAttributeModel::DoFaudesAllocate()");
  // let base handle this
  VioModel::DoFaudesAllocate();
  // impose my requirements
  if(DoTypeCheck(mData->FaudesObject())) {
    FD_DQT("VioAttributeMtcStateModel::DoFaudesAllocate(): fallback ctype");
    mData->FaudesObject(new faudes::AttributeFlags());  
  }
}

// test whether we can host this faudes object
int VioAttributeModel::DoTypeCheck(const faudes::Type* fobject) const {  
  // we host anything that casts to flags Attribute
  if(dynamic_cast<const faudes::AttributeFlags*>(fobject)) return 0;
  return 1; 
}

// allocate visual model data
void VioAttributeModel::DoVioAllocate(void) {
  FD_DQT("VioAttributeModel::DoVioAllocate()");
  // base allocates their stuff
  VioModel::DoVioAllocate();
  // have value vector
  while(mBooleanValues.size()<pBooleanProperties->size())
    mBooleanValues.append(VioBooleanProperty::Void);
}

// update visual data from (new) faudes object
void VioAttributeModel::DoVioUpdate(void) {
  FD_DQT("VioAttributeModel::DoVioUpdate()");
  // have typed reference
  mpFaudesAttribute = dynamic_cast<faudes::AttributeFlags*>(mData->FaudesObject());
  FD_DQ("VioAttributeModel::DoVioUpdate(): " << mpFaudesAttribute->ToString());
  // copy values
  for(int i=0; i< BooleansSize(); i++) {
    if(pBooleanProperties->at(i).Test(mpFaudesAttribute->mFlags))
       mBooleanValues[i]=VioBooleanProperty::True;
     else
       mBooleanValues[i]=VioBooleanProperty::False;
   }
  // this is not merged
  mMerged=false;
#ifdef FAUDES_DEBUG_VIO_WIDGETS
  // call base (incl set text and notify)
  VioModel::DoVioUpdate();
#else
  // we notify
  emit NotifyAnyChange();
#endif
}


// data access: size
int VioAttributeModel::BooleansSize(void) const {
  return pBooleanProperties->size();
}

// data access: get
Qt::CheckState VioAttributeModel::BooleansValue(int pos) {
  FD_DQA("VioAttributeModel::BooleansValue(): at " << pos);
  if(pos < 0 || pos >= BooleansSize()) return Qt::PartiallyChecked;
  return VioBooleanProperty::State(mBooleanValues.at(pos));
}

// data access: set
void VioAttributeModel::BooleansValue(int pos, Qt::CheckState  val) {
  FD_DQA("VioAttributeModel::BooleansValue(): at " << pos);
  VioBooleanProperty::ValueType vval=VioBooleanProperty::Value(val);
  if(pos < 0 || pos >= BooleansSize()) return;
  if(mBooleanValues.at(pos)==vval) return;
  // do assign
  mBooleanValues[pos]=vval;
  if(val==Qt::Checked)
    pBooleanProperties->at(pos).Set(mpFaudesAttribute->mFlags);
  if(val==Qt::Unchecked)
    pBooleanProperties->at(pos).Clr(mpFaudesAttribute->mFlags);
  // notify
  emit NotifyAnyChange();
}


// merge: tell
bool VioAttributeModel::Merged(void) {
  return mMerged;
}

// merge: clear
int VioAttributeModel::MergeClear(void) {
  mMerged=true;
  for(int i=0; i< BooleansSize(); i++) 
    mBooleanValues[i]=VioBooleanProperty::Void;
  return 0;
}

// merge: insert
void VioAttributeModel::MergeInsert(const faudes::AttributeFlags* fattr) {
  for(int i=0; i< BooleansSize(); i++) {
    bool bval= pBooleanProperties->at(i).Test(fattr->mFlags);
    VioBooleanProperty::ValueType vval= mBooleanValues.at(i);
    if(vval==VioBooleanProperty::Void) {
      if(bval) mBooleanValues[i]=VioBooleanProperty::True;
      if(!bval) mBooleanValues[i]=VioBooleanProperty::False;
      continue;
    }
    if(vval==VioBooleanProperty::Partial) 
      continue;
    if(vval==VioBooleanProperty::True) 
      if(!bval) mBooleanValues[i]=VioBooleanProperty::Partial;
    if(vval==VioBooleanProperty::False) 
      if(bval) mBooleanValues[i]=VioBooleanProperty::Partial;
  }
}
 
// merge: done
void VioAttributeModel::MergeDone(void) {
  emit NotifyAnyChange();
}


// partial assign
void VioAttributeModel::MergeAssign(faudes::AttributeFlags* fattr) {
  for(int i=0; i< BooleansSize(); i++) {
    VioBooleanProperty::ValueType vval= mBooleanValues.at(i);
    if(vval==VioBooleanProperty::False) 
      pBooleanProperties->at(i).Clr(fattr->mFlags);
  }
  for(int i=0; i< BooleansSize(); i++) {
    VioBooleanProperty::ValueType vval= mBooleanValues.at(i);
    if(vval==VioBooleanProperty::True) 
      pBooleanProperties->at(i).Set(fattr->mFlags);
  }
}


/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioAttributeView

****************************************************************
****************************************************************
****************************************************************
*/

// construct
VioAttributeView::VioAttributeView(QWidget* parent, VioStyle* config, bool alloc) : 
  VioView(parent,config,false),
  pAttributeModel(0),
  mCheckBoxes(0)
{
  FD_DQT("VioAttributeView::VioAttributeView(): " << VioStyle::StrFromQStr(FaudesType()));
  // have typed style
  pAttributeStyle=dynamic_cast<const VioAttributeStyle*>(pConfig);
  if(!pAttributeStyle) {
    FD_WARN("VioAttributView::VioAttributeModel(): invalid style, using default.");
    pAttributeStyle= new VioAttributeStyle(mFaudesType);
  }
  // my alloc
  if(alloc) DoVioAllocate();
  FD_DQT("VioAttributeView::VioAttributeView(): done");
}

// destruct
VioAttributeView::~VioAttributeView(void) {
}

// typed faudes object access
const faudes::AttributeFlags* VioAttributeView::Attribute(void) const {
  if(!pAttributeModel) return 0;
  return pAttributeModel->Attribute();
}

// allocate my data
void VioAttributeView::DoVioAllocate(void) {
  FD_DQT("VioAttributeView::DoVioAllocate(): create layout");
  // allocate base
  VioView::DoVioAllocate();
  // debugging widget
#ifdef FAUDES_DEBUG_VIO_WIDGETS
  mTextInfo->setText(QString("Attribute \"%1\"").arg(FaudesType()));
  mTextEdit->setReadOnly(true);
  mApplyButton->hide();
#endif
  // my checkboxes
  mCheckBoxes = new QVBoxLayout();
  mCheckBoxes->setMargin(0);
  mCheckBoxes->setSpacing(2);
  mVbox->addLayout(mCheckBoxes);
  mCheckBoxList.clear();
  // populate my checkboxes
  for(int i=0; i< pAttributeStyle->BooleanProperties().size(); i++) {
    FD_DQT("VioAttributeView::DoVioAllocate(): boolean prop " << VioStyle::StrFromQStr( 
                pAttributeStyle->BooleanProperties().at(i).mName));
    // have a hbox layout 
    QHBoxLayout* hbox = new QHBoxLayout();
    VioCheckBox* checkbox = new VioCheckBox();
    checkbox->setText(pAttributeStyle->BooleanProperties().at(i).mName); 
    checkbox->setEnabled(pAttributeStyle->BooleanProperties().at(i).mEditable); 
    checkbox->setTristate(true);
    checkbox->setProperty("FAUDES_cbid",QVariant(i));
    hbox->addWidget(checkbox); 
    hbox->addStretch(1);  
    mCheckBoxes->addLayout(hbox); 
    // record and connect checkbox
    mCheckBoxList.append(checkbox);
    QObject::connect(checkbox,SIGNAL(clicked(void)),this,SLOT(UpdateSingleCheckbox(void)));
  }
  FD_DQT("VioAttributeView::DoVioAllocate(): done");
}

// update view from (new) model
void VioAttributeView::DoVioUpdate(void) {
  FD_DQT("VioAttributeView::DoVioUpdate(): type " << VioStyle::StrFromQStr(FaudesType()));
  // have typed model reference (incl set to 0)
  pAttributeModel=qobject_cast<VioAttributeModel*>(pModel);
  // bail out on no model
  if(!pAttributeModel) return;
  // update check boxes
  for(int i=0; i< pAttributeModel->BooleanProperties()->size(); i++) {
    if(i>=mCheckBoxList.size()) continue; // cannot happen
    QCheckBox* checkbox = mCheckBoxList.at(i);
    checkbox->setCheckState(pAttributeModel->BooleansValue(i));
  }
#ifdef FAUDES_DEBUG_VIO_WIDGETS
  mTextEdit->setPlainText(VioStyle::QStrFromStr(pModel->FaudesObject()->ToText()));
#endif
  FD_DQT("VioAttributeView::DoVioUpdate(): done");
}

// one checkbox has been triggered
void VioAttributeView::UpdateSingleCheckbox(void) {
  FD_DQT("VioAtributeView::UpdateCheckbox(): make model/checkboxes consistent");
  // bail out if widget is not ready or model not there
  if(!mCheckBoxes || !pModel) return;
  // bail out if it was not trigglered by a checkbox
  QCheckBox* checkbox = qobject_cast<QCheckBox*>(sender()) ;
  if(!checkbox) return;
  // get index
  int i = checkbox->property("FAUDES_cbid").toInt();
  FD_DQT("VioAtributeView::UpdateCheckbox(): triggered by " << i);
  // bail out invalid index (cannot happen)
  if(i<0 || i>= pAttributeModel->BooleanProperties()->size()) return;
  // update faudes flags in model
  pAttributeModel->BooleansValue(i, checkbox->checkState());
  // set changed
  Modified(true);
}
// update model from view
void VioAttributeView::DoModelUpdate(void) {
  FD_DQT("VioAttributeView::DoModelUpdate(): type " << VioStyle::StrFromQStr(FaudesType()));
  // bail out if widget is not ready or model not there
  if(!mCheckBoxes || !pModel) return;
  // update flags from check boxes
  for(int i=0; i< pAttributeModel->BooleanProperties()->size(); i++) {
    if(i>=mCheckBoxList.size()) continue; // cannot happen
    QCheckBox* checkbox = mCheckBoxList.at(i);
    pAttributeModel->BooleansValue(i,checkbox->checkState());
  }
}


/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioAttributeWidget

****************************************************************
****************************************************************
****************************************************************
*/

// construct
VioAttributeWidget::VioAttributeWidget(QWidget* parent, VioStyle* config, bool alloc) : 
  VioWidget(parent,config,false),
  pAttributeModel(0) 
{
  FD_DQT("VioAttributeWidget::VioAttributeWidget(): " << VioStyle::StrFromQStr(pConfig->ConfigName()));
  // allocate model and view
  if(alloc) {
    // have view
    mView= new VioAttributeView(this,pConfig);
    mVbox->addWidget(mView);
    // fix typed refs and connect to widget
    DoVioAllocate();
    // have a model and set
    Model(new VioAttributeModel(this,pConfig));
  }
  // fix modified flag
  mModified=false;
  FD_DQT("VioAttributeWidget::VioAttributeWidget(): done");
}

// destruct
VioAttributeWidget::~VioAttributeWidget(void) {
}

// fix view
void VioAttributeWidget::DoVioAllocate(void) {
  // connect view
  QObject::connect(mView,SIGNAL(NotifyModified(bool)),this,SLOT(ChildModified(bool)));
  QObject::connect(mView,SIGNAL(MouseClick(const VioElement&)),this,SIGNAL(MouseClick(const VioElement&)));
  QObject::connect(mView,SIGNAL(MouseDoubleClick(const VioElement&)),this,SIGNAL(MouseDoubleClick(const VioElement&)));
}

// set by vio model
int VioAttributeWidget::Model(VioModel* model) {
  FD_DQT("VioAttribute::Model(" << model << "): type " << VioStyle::StrFromQStr(FaudesType()));
  // bail out on identity
  if(model==mModel) return 0;
  // call base to set view and connects
  int res=VioWidget::Model(model);
  // update typed ref
  pAttributeModel=qobject_cast<VioAttributeModel*>(mModel);
  FD_DQT("VioAttribute::Model(" << model << "): done");  
  return res;
}


// typed faudes object access
const faudes::AttributeFlags* VioAttributeWidget::Attribute(void) const {
  return pAttributeModel->Attribute();
}








