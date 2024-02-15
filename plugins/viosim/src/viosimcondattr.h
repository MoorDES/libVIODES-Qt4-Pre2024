/* viosimcondattr.h  - vio attribute for faudes sim condition  */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2010 Thomas Moor

*/



#ifndef FAUDES_VIOSIMCOND_H
#define FAUDES_VIOSIMCOND_H

// std includes
#include "libviodes.h"


// debugging: sim plugin
#ifdef FAUDES_DEBUG_VIO_SIM
#define FD_DQS(message) FAUDES_WRITE_CONSOLE("FAUDES_VIO_SIM: " << message)
#else
#define FD_DQS(message) { };
#endif



/*
 ************************************************
 ************************************************

 A VioAttributeSimCondModel is derived from the 
 std boolean property VioAttributeModel. It provides
 additional access to the libfaudes mtc plugin 
 color set.

 We try to keep this really simple, in that we
 dont have an extra style class.

 ************************************************
 ************************************************
 */

// forward
class VioAttributeSimCondView;
class VioAttributeSimCondWidget;


class VioAttributeSimCondModel : public VioAttributeModel {

Q_OBJECT

public:

  // construct/destruct
  VioAttributeSimCondModel(QObject* parent=0, VioStyle* config=0, bool alloc=true);
  virtual ~VioAttributeSimCondModel(void);

  // reimplement viomodel: construct on heap
  virtual VioAttributeSimCondModel* NewModel(QObject* parent=0) const;
  virtual VioView* NewView(QWidget* parent=0) const;
  virtual VioWidget* NewWidget(QWidget* parent=0) const;

  // multi element vie (return non-zero on error/not implemented)
  virtual int MergeClear(void);
  virtual void MergeInsert(const faudes::AttributeFlags* fattr);
  virtual void MergeAssign(faudes::AttributeFlags* fattr);


  // typed faudes object access 
  const faudes::AttributeSimCondition* AttributeSimCondition(void) const 
    { return mpFaudesSimCondition; };

  // edit attribute
  void  VioCondType(int type); // 0<> event; 1<> states, conj; 2<> states, disj
  void VioStartEvents(const QList<QString>& rStartEvs);
  void VioStopEvents(const QList<QString>& rStopEvs);

  // edit get
  const QList<QString>& VioStartEvents(void) const { return mStartEvents;}
  const QList<QString>& VioStopEvents(void) const { return mStopEvents;}


protected:

  // typed version of faudes object
  faudes::AttributeSimCondition* mpFaudesSimCondition;

  // reimplement viomodel: allocate faudes object and visual model data
  virtual void DoFaudesAllocate(void);

  // reimplement viomodel: test whether we can host this faudes object
  virtual int DoTypeCheck(const faudes::Type* fobject) const;

  // reimplement viomodel: token io, implementation 
  virtual void DoVioWrite(faudes::TokenWriter& rTw) const { (void) rTw; };
  virtual void DoVioRead(faudes::TokenReader& rTr) {(void) rTr; };

  // reimplement viomodel: update visual data from (new) faudes object
  virtual void DoVioUpdate(void);

  // local copy of attribute data
  QList<QString> mStartEvents;
  QList<QString> mStopEvents;


};


/*
 ************************************************
 ************************************************

 A VioAttributeSimCondView is a widget to represent an
 attribute via an VioAttributeModel.

 ************************************************
 ************************************************
 */



class VioAttributeSimCondView : public VioAttributeView {

Q_OBJECT

public:
  // construct/destruct
  VioAttributeSimCondView(QWidget* parent=0, VioStyle* config=0, bool alloc=true);
  virtual ~VioAttributeSimCondView(void);

  // reimplement vioattributeview: allocate view data
  virtual void DoVioAllocate(void);

  // typed faudes object access
  const faudes::AttributeFlags* Attribute(void) const;
  const faudes::AttributeSimCondition* AttributeSimCondition(void) const; 

protected:

  // update view from model
  virtual void DoVioUpdate(void);

  // update to model (exception on error)
  virtual void DoModelUpdate(void);

  // typed refs
  VioAttributeSimCondModel* pAttributeSimCondModel;

  // layout items
  QComboBox* mTypeCombo;
  VioSymbolTableWidget* mStartWidget;
  VioSymbolTableWidget* mStopWidget;


protected slots:

  // update from view widgets (row -1 <==> all)
  void UpdateFromTypeCombo(int type=-1);
  void UpdateFromStopWidget();
  void UpdateFromStartWidget();
};


/*
 ************************************************
 ************************************************

 A VioAttributeSimCondWidget is a widget to represent an
 attribute by an VioAttributeSimCondView. Internally, it 
 uses a VioAttributeModel to hold defining data.

 ************************************************
 ************************************************
 */



class VioAttributeSimCondWidget : public VioAttributeWidget {

Q_OBJECT

public:
  // construct/destruct
  VioAttributeSimCondWidget(QWidget* parent=0, VioStyle* config=0, bool alloc=true);
  virtual ~VioAttributeSimCondWidget(void);

  // set model
  int Model(VioModel* model);

protected:

  // reimplement viomodel: allocate vio model
  virtual void DoVioAllocate(void);

  // typed representation data
  VioAttributeSimCondModel* pAttributeSimCondModel;

};




#endif
