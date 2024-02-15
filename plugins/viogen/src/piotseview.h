/* piotseview.h  - widgets for generator property editing  */


/*
   Graphical  IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006, 2007  Thomas Moor, Klaus Schmidt, Sebastian Perk

*/


#ifndef FAUDES_PIOTSEVIEW_H
#define FAUDES_PIOTSEVIEW_H


#include "libviodes.h"

// foreard
class VioGeneratorModel;
class VioGeneratorStyle;

/*
 *****************************************************
 *****************************************************
 The properties widgets are the intended content of a 
 dialog box to edit state/transition properties.

 Here, the term "property" refers to faudes attributes
 such as flags and other faudes data e.g. symbolic names. 
 The property widget is connected meant to reside within
 a VoGeneratorPropertyView and performs user changes 
 synchronously via callbacks to the underlying VioGeneratorModel. 
 Vice versa, it receives signals from the VioGeneratorModel to
 update itself.

 Technically, the PioPView holds a VioAttributeWidget and, hence
 a model, to actually show the property. Wrt the underlying Generator
 it is still a view.

 There are classes PioTProp, PioSProp ans PioEProp for
 transition, states and events. They share a common base
 PioVProp

 *****************************************************
 *****************************************************
 */

/*
 *****************************************************
 virtual base edit widget
 *****************************************************
 */

class PioVProp : public QWidget {

Q_OBJECT

public:
  // construct
  PioVProp(QWidget* parent=0, VioStyle* config=0);
  ~PioVProp(void);

  // configure behaviour
  void SymbolMode(VioSymbol::Mode mode);
  // todo set completer

  // set/get vio generator 
  virtual void GeneratorModel(VioGeneratorModel* genmodel=NULL);
  virtual const VioGeneratorModel* GeneratorModel(void) const;

  // read only access to faudes generator 
  const faudes::vGenerator* Generator(void) const;

  // get faudes element that is currently displyed
  const VioElement& Element(void) const;

  // get name (label/statle name/ event name) that is displayed
  QString Name(void);

public slots:

  // set model from visual representation
  void UpdateModel(void);

  // set view from model
  void UpdateView(void);

  // show interface
  virtual void Show(const VioElement& elem);
  virtual void ShowSelection(void);

signals:

  // notify application on user changes
  void NotifyModified(bool ch);

  // indicate return in symbolic name edit
  void DoneEditing(void);


protected slots:

  // collect changes, callback VioGenerator, pass on uniform signal
  virtual void PropertyChanged(void) {}; 

  // do clear
  virtual void DoClear(void) {};

  // do update
  virtual void DoModelUpdate(void) {};
  virtual void DoVioUpdate(void) {};

protected:

  // configuration
  VioGeneratorStyle* pGeneratorConfig;

  // record connection to model
  VioGeneratorModel* pVioGeneratorModel;

  // element to view
  VioElement mElement;
  
  // access name (no signals)
  void Name(const QString& name);

  // set/get faudes attribute (no signals, takes ownership)
  void Attribute(faudes::AttributeVoid* attr);
  void AttributeFromSelection(void);
  void AttributeToSelection(void);

  // block internal attribute update signal
  bool mBlockModelUpdate;

  // my layout 
  QVBoxLayout* mVbox;
  QLabel* mLabelName;
  VioSymbolEdit* mEditName;
  VioAttributeWidget* mAttribute;
  VioView* mConfigure;

};

/*
 *****************************************************
 transition based edit widget
 *****************************************************
 */

class PioTProp : public PioVProp {

Q_OBJECT

public:
  // construct
  PioTProp(QWidget* parent=0, VioStyle* config=0);
  ~PioTProp(void);

  // get faudes state idx
  faudes::Transition Trans(void) const { return mElement.Trans(); };

protected slots:

  // reimplement
  virtual void DoClear(void);
  virtual void DoModelUpdate(void);
  virtual void DoVioUpdate(void);

};


/*
 *****************************************************
 state base edit widget
 *****************************************************
 */

class PioSProp : public PioVProp {

Q_OBJECT

public:
  // construct
  PioSProp(QWidget* parent=0, VioStyle* config=0);
  ~PioSProp(void);

  // get faudes state idx
  faudes::Idx Idx(void) const { return mElement.State(); };


protected slots:

  // reimplement
  virtual void DoClear(void);
  virtual void DoModelUpdate(void);
  virtual void DoVioUpdate(void);

};



/*
 *****************************************************
 event based edit widget
 *****************************************************
 */

class PioEProp : public PioVProp {

Q_OBJECT

public:
  // construct
  PioEProp(QWidget* parent=0, VioStyle* config=0);
  ~PioEProp(void);

  // get faudes state idx
  faudes::Idx Idx(void) const { return mElement.Event(); };

protected slots:

  // reimplement
  virtual void DoClear(void);
  virtual void DoModelUpdate(void);
  virtual void DoVioUpdate(void);

};


/*
 *****************************************************
 global based edit widget
 *****************************************************
 */

class PioGProp : public PioVProp {

Q_OBJECT

public:
  // construct
  PioGProp(QWidget* parent=0, VioStyle* config=0);
  ~PioGProp(void);


protected slots:

  // reimplement
  virtual void DoClear(void);
  virtual void DoModelUpdate(void);
  virtual void DoVioUpdate(void);

protected:

  // my extra properties 
  QLabel* mNumStates;
  QLabel* mNumEvents;
  QLabel* mNumTrans;

};



 
#endif
