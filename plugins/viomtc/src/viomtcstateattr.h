/* viomtcstateattr.h  - vio attribute model and widget for mtc states */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2009  Thomas Moor, Ruediger Berndt

*/



#ifndef FAUDES_VIOMTCSTATE_H
#define FAUDES_VIOMTCSTATE_H

// std includes
#include "libviodes.h"



/*
 ************************************************
 ************************************************

 A VioAttributeMtcStateModel is derived from the 
 std boolean property VioAttributeModel. It provides
 additional access to the libfaudes mtc plugin 
 color set.

 We try to keep this really simple, in that we
 dont have an extra style class.

 ************************************************
 ************************************************
 */

// forward
class VioAttributeMtcStateView;
class VioAttributeMtcStateWidget;


class VioAttributeMtcStateModel : public VioAttributeModel {

Q_OBJECT

public:

  // construct/destruct
  VioAttributeMtcStateModel(QObject* parent=0, VioStyle* config=0, bool alloc=true);
  virtual ~VioAttributeMtcStateModel(void);

  // reimplement viomodel: construct on heap
  virtual VioAttributeMtcStateModel* NewModel(QObject* parent=0) const;
  virtual VioView* NewView(QWidget* parent=0) const;
  virtual VioWidget* NewWidget(QWidget* parent=0) const;

  // multi element vie (return non-zero on error/not implemented)
  virtual int MergeClear(void);
  virtual void MergeInsert(const faudes::AttributeFlags* fattr);
  virtual void MergeAssign(faudes::AttributeFlags* fattr);


  // typed faudes object access 
  const faudes::AttributeColoredState* AttributeColoredState(void) const 
    { return mpFaudesColoredState; };


  // edit set
  void VioMarking(const faudes::ColorSet& rMarking);

  // edit get
  const faudes::ColorSet& VioMarking(void) const { return mpFaudesColoredState->Colors(); }; 


protected:

  // typed version of faudes object
  faudes::AttributeColoredState* mpFaudesColoredState;

  // reimplement viomodel: allocate faudes object and visual model data
  virtual void DoFaudesAllocate(void);

  // reimplement viomodel: test whether we can host this faudes object
  virtual int DoTypeCheck(const faudes::Type* fobject) const;

  // reimplement viomodel: token io, implementation 
  virtual void DoVioWrite(faudes::TokenWriter& rTw) const { (void) rTw; };
  virtual void DoVioRead(faudes::TokenReader& rTr) {(void) rTr; };

  // reimplement viomodel: update visual data from (new) faudes object
  virtual void DoVioUpdate(void);



};


/*
 ************************************************
 ************************************************

 A VioAttributeMtcStateView is a widget to represent an
 attribute via an VioAttributeModel.

 ************************************************
 ************************************************
 */



class VioAttributeMtcStateView : public VioAttributeView {

Q_OBJECT

public:
  // construct/destruct
  VioAttributeMtcStateView(QWidget* parent=0, VioStyle* config=0, bool alloc=true);
  virtual ~VioAttributeMtcStateView(void);

  // reimplement vioattributeview: allocate view data
  virtual void DoVioAllocate(void);

  // typed faudes object access
  const faudes::AttributeFlags* Attribute(void) const;
  const faudes::AttributeColoredState* AttributeColoredState(void) const; 

protected:

  // update view from model
  virtual void DoVioUpdate(void);

  // update to model (exception on error)
  virtual void DoModelUpdate(void);

  // typed refs
  VioAttributeMtcStateModel* pAttributeMtcStateModel;

  // layout items
  VioSymbolTableWidget* mColorColumns;


protected slots:

  // color set changed
  void UpdateFromColorColumns(int row=-1, int col=-1);
};


/*
 ************************************************
 ************************************************

 A VioAttributeMtcStateWidget is a widget to represent an
 attribute by an VioAttributeMtcStateView. Internally, it 
 uses a VioAttributeModel to hold defining data.

 ************************************************
 ************************************************
 */



class VioAttributeMtcStateWidget : public VioAttributeWidget {

Q_OBJECT

public:
  // construct/destruct
  VioAttributeMtcStateWidget(QWidget* parent=0, VioStyle* config=0, bool alloc=true);
  virtual ~VioAttributeMtcStateWidget(void);

  // set model
  int Model(VioModel* model);

protected:

  // reimplement viomodel: allocate vio model
  virtual void DoVioAllocate(void);

  // typed representation data
  VioAttributeMtcStateModel* pAttributeMtcStateModel;

};




#endif
