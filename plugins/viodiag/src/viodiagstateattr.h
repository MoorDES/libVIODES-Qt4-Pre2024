/** viodiagstateattr.h  - vio attribute for faudes diagnoser label  */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2010 Thomas Moor

*/



#ifndef FAUDES_VIODIAGSTATE_H
#define FAUDES_VIODIAGSTATE_H

// std includes
#include "libviodes.h"


// debugging: diag plugin
#ifdef FAUDES_DEBUG_VIO_DIAG
#define FD_DQD(message) FAUDES_WRITE_CONSOLE("FAUDES_VIO_DIAG: " << message)
#else
#define FD_DQD(message) { };
#endif



/*
 ************************************************
 ************************************************

 A VioAttributeDiagStateModel is derived from the 
 std boolean property VioAttributeModel. It provides
 read-only access to the diagnoser label.

 ************************************************
 ************************************************
 */

// forward
class VioAttributeDiagStateView;
class VioAttributeDiagStateWidget;


class VioAttributeDiagStateModel : public VioAttributeModel {

Q_OBJECT

public:

  // construct/destruct
  VioAttributeDiagStateModel(QObject* parent=0, VioStyle* config=0, bool alloc=true);
  virtual ~VioAttributeDiagStateModel(void);

  // reimplement viomodel: construct on heap
  virtual VioAttributeDiagStateModel* NewModel(QObject* parent=0) const;
  virtual VioView* NewView(QWidget* parent=0) const;
  virtual VioWidget* NewWidget(QWidget* parent=0) const;

  // todo: merge with other faudes attributes for selection edit
  // return non-zero on error
  virtual int MergeClear(void) { return 1;};
  virtual void MergeAttribute(const faudes::AttributeFlags* fattr) { (void) fattr; };

  // typed faudes object access 
  const faudes::AttributeDiagnoserState* AttributeDiagnoserState(void) const 
    { return mpFaudesDiagnoserState; };



protected:

  // typed version of faudes object
  faudes::AttributeDiagnoserState*  mpFaudesDiagnoserState;

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

 A VioAttributeDiagStateView is a widget to represent an
 attribute via an VioAttributeModel.

 ************************************************
 ************************************************
 */



class VioAttributeDiagStateView : public VioAttributeView {

Q_OBJECT

public:
  // construct/destruct
  VioAttributeDiagStateView(QWidget* parent=0, VioStyle* config=0, bool alloc=true);
  virtual ~VioAttributeDiagStateView(void);

  // reimplement vioattributeview: allocate view data
  virtual void DoVioAllocate(void);

  // typed faudes object access
  const faudes::AttributeFlags* Attribute(void) const;
  const faudes::AttributeDiagnoserState* AttributeDiagnoserState(void) const; 

protected:

  // update view from model
  virtual void DoVioUpdate(void);

  // update to model (exception on error)
  virtual void DoModelUpdate(void);

  // typed refs
  VioAttributeDiagStateModel* pAttributeDiagStateModel;

  // layout items
  QLineEdit* mStateLabel;


protected slots:

};


/*
 ************************************************
 ************************************************

 A VioAttributeDiagStateWidget is a widget to represent an
 attribute by an VioAttributeDiagStateView. Internally, it 
 uses a VioAttributeModel to hold defining data.

 ************************************************
 ************************************************
 */



class VioAttributeDiagStateWidget : public VioAttributeWidget {

Q_OBJECT

public:
  // construct/destruct
  VioAttributeDiagStateWidget(QWidget* parent=0, VioStyle* config=0, bool alloc=true);
  virtual ~VioAttributeDiagStateWidget(void);

  // set model
  int Model(VioModel* model);

protected:

  // reimplement viomodel: allocate vio model
  virtual void DoVioAllocate(void);

  // typed representation data
  VioAttributeDiagStateModel* pAttributeDiagStateModel;

};




#endif
