/* vioattribute.h  - vio attribute model and widget */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2009  Thomas Moor, Ruediger Berndt

*/



#ifndef FAUDES_VIOATTRIBUTE_H
#define FAUDES_VIOATTRIBUTE_H

// std includes
#include "viotypes.h"
#include "vioattrstyle.h"


// debugging: vio attributes
#ifdef FAUDES_DEBUG_VIO_ATTRIBUTE
#define FD_DQA(message) FAUDES_WRITE_CONSOLE("FAUDES_VIO_ATTRIBUTE: " << message)
#else
#define FD_DQA(message)
#endif



/*
 ************************************************
 ************************************************

 A VioAttributeModel provides functions to
 map faudes attributes to boolean vio properties,
 ie visual representations of boolean attributes.
 The base class uses the VioAttributeStyle to map 
 faudes flags a vio flag word and provide a textual 
 description. Derived classes may map other components 
 of advanced faudes attributes.

 Technical note: the class uses internally the
 faudes type  AttributeFlags but will accept any 
 derived faudes classes when setting FaudesObject.
 When used on a attribute that is not derived from
 AttributeFlags, this class will not be functional.

 ************************************************
 ************************************************
 */

// forward
class VioAttributeView;
class VioAttributeWidget;


class VIODES_API VioAttributeModel : public VioModel {

Q_OBJECT

public:

  // construct/destruct
  VioAttributeModel(QObject* parent=0, VioStyle* config=0, bool alloc=true);
  virtual ~VioAttributeModel(void);

  // typed acces to configuration
  const VioAttributeStyle* AttributeConfiguration(void) const { return pAttributeStyle;};

  // set/get context
  virtual void Context(VioModel* context) { pContext=context; emit NotifyContext(); };
  VioModel* Context(void) { return pContext;};

  // reimplement viomodel: construct on heap
  virtual VioAttributeModel* NewModel(QObject* parent=0) const;
  virtual VioView* NewView(QWidget* parent=0) const;
  virtual VioWidget* NewWidget(QWidget* parent=0) const;

  // multi element vie (return non-zero on error/not implemented)
  virtual bool Merged(void);
  virtual int MergeClear(void);
  virtual void MergeInsert(const faudes::AttributeFlags* fattr);
  virtual void MergeDone(void);
  virtual void MergeAssign(faudes::AttributeFlags* fattr);


  // typed faudes object access
  const faudes::AttributeFlags* Attribute(void) const { return mpFaudesAttribute; };

  // typed style access  
  const QList<VioBooleanProperty>* BooleanProperties(void) const { return pBooleanProperties; };

  
  // edit boolean properties: set/get 
  int BooleansSize(void) const;
  Qt::CheckState BooleansValue(int pos);
  void BooleansValue(int pos, Qt::CheckState val);



protected:

  // typed version of faudes object
  faudes::AttributeFlags* mpFaudesAttribute;

  // typed style acces
  const VioAttributeStyle* pAttributeStyle;
  const QList<VioBooleanProperty>* pBooleanProperties;

  // reimplement viomodel: allocate faudes object and visual model data
  virtual void DoFaudesAllocate(void);
  virtual void DoVioAllocate(void);

  // reimplement viomodel: test whether we can host this faudes object
  virtual int DoTypeCheck(const faudes::Type* fobject) const;

  // reimplement viomodel: token io, implementation 
  virtual void DoVioWrite(faudes::TokenWriter& rTw) const { (void) rTw; };
  virtual void DoVioRead(faudes::TokenReader& rTr) {(void) rTr; };

  // reimplement viomodel: update visual data from (new) faudes object
  virtual void DoVioUpdate(void);

  // optional context to which an attribute can refer
  QPointer<VioModel> pContext;

  // representation data
  QList<VioBooleanProperty::ValueType> mBooleanValues;

  // merged flag
  bool mMerged;

signals:

  // notify context change to views and friend
  void NotifyContext(void);

};


/*
 ************************************************
 ************************************************

 A VioAttributeView is a widget to represent an
 attribute via an VioAttributeModel.

 ************************************************
 ************************************************
 */



class VIODES_API VioAttributeView : public VioView {

Q_OBJECT

public:
  // construct/destruct
  VioAttributeView(QWidget* parent=0, VioStyle* config=0, bool alloc=true);
  virtual ~VioAttributeView(void);

  // reimplement vioview: allocate view data
  virtual void DoVioAllocate(void);

  // typed faudes object access
  const faudes::AttributeFlags* Attribute(void) const;

protected:

  // reimplement vioview: update view from model
  virtual void DoVioUpdate(void);

  // reimplement vioview: update to model (exception on error)
  virtual void DoModelUpdate(void);


  // typed model/style
  VioAttributeModel* pAttributeModel;
  const VioAttributeStyle* pAttributeStyle;

  // my widgets
  QVBoxLayout* mCheckBoxes;
  QList<VioCheckBox*>   mCheckBoxList;

protected slots:

  // one checkbox has been triggered
  void UpdateSingleCheckbox(void);
};


/*
 ************************************************
 ************************************************

 A VioAttributeWidget is a widget to represent an
 attribute by an VioAttributeView. Internally, it 
 uses a VioAttributeModel to hold defining data.

 ************************************************
 ************************************************
 */



class VIODES_API VioAttributeWidget : public VioWidget {

Q_OBJECT

public:
  // construct/destruct
  VioAttributeWidget(QWidget* parent=0, VioStyle* config=0, bool alloc=true);
  virtual ~VioAttributeWidget(void);

  // set model
  int Model(VioModel* model);
  VioAttributeModel* Model(void) { return pAttributeModel; };

  // typed faudes object access
  const faudes::AttributeFlags* Attribute(void) const;

protected:

  // reimplement viomodel: allocate vio model
  virtual void DoVioAllocate(void);

  // typed representation data
  VioAttributeModel* pAttributeModel;

};




#endif
