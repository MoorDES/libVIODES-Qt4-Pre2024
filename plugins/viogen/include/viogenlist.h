/* liotsemodel.h  - list representation of generators */


/*
   Visual IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2009  Thomas Moor, Ruediger Berndt

*/

#ifndef FAUDES_GENLIST_H
#define FAUDES_GENLIST_H

// std includes
#include "viogenerator.h"

// forward
class VioGeneratorListView;
class LioVList;
class LioVView;

/*
 ************************************************
 ************************************************

 A VioGeneratorListModel holds data for the
 list representation of generators, ie a
 list of transitions, states, or events
 It is derived from VioGeneratorVirtualModel
 and, hence, is meant to be used in the container
 model VioGeneratorModel. Actual representation
 is done by the view VioGeneratorListView, see
 below.

 The representation data access is limited to
 get/set all of it. Detailed editing is managed 
 the VioGeneratorListView wich uses a qt table
 model internally.

 ************************************************
 ************************************************
 */


class VioGeneratorListModel : public VioGeneratorAbstractModel {

Q_OBJECT

public:

  // construct/destruct
  VioGeneratorListModel(VioGeneratorModel* parent, VioElement::EType etype);
  virtual ~VioGeneratorListModel(void) {}; // todo: tell my view?

  // get element type
  VioElement::EType ElementType(void) { return mEType; }

  // access as q table model
  LioVList* TabelModel(void);

  // reimplement: create new view for this representationmodel
  virtual VioGeneratorAbstractView* NewView(VioGeneratorView* parent);

  // reimplement: clear to default/empty representation
  void Clear(void);

  // read access to representation data (order of elements)
  QList<VioElement>* ElementList(void) { return &mElementList; }
  QMap<VioElement,int>* ElementRowMap(void) { return &mElementRowMap;}

  // debug console dump
  void  Dump(void);

  // local edit: access to representation data (a list, no signals);
  int Size(void) const;
  const VioElement& At(int pos) const;
  bool At(int pos, const VioElement&); 
  bool Move(int from, int to);
  bool Append(const VioElement& elem);
  bool Insert(int pos, const VioElement& elem);
  bool Remove(const VioElement& elem); 
  bool RemoveAt(int pos); 
  int IndexOf(const VioElement& elem) const;
  bool Contains(const VioElement& elem) const;

  // local edit: sorting
  void SortAscendingX1(void);
  void SortDescendingX1(void);
  void SortAscendingEv(void);
  void SortDescendingEv(void);
  void SortAscendingX2(void);
  void SortDescendingX2(void);

  // reimplement to clr child tablemodel
  virtual void Modified(bool ch);

public slots:

  // editing: the generator model notifies us on changes
  // performed on the faudes generator. In turn, we signal our views 
  // about changes in the faudes generator using signals inherited
  // from VioGeneratorVirtualModel

  // editing faudes object: by elementary type, defaults to update all
  virtual void UpdateElementIns(const VioElement& elem);
  virtual void UpdateElementDel(const VioElement& elem);
  virtual void UpdateElementEdit(const VioElement& selem, const VioElement& delem);
  virtual void UpdateElementProp(const VioElement& elem);

  // editing faudes object: global, defaults to update all
  virtual void UpdateTrimElements(void);
  virtual void UpdateAnyAttr(void);
  virtual void UpdateAnyChange(void);
  virtual void UpdateNewModel(void);



protected:

  // typed ref to parent
  VioGeneratorModel* pVioGeneratorModel;

  // reimplement: token io
  virtual void DoVioWrite(faudes::TokenWriter& rTw) const;
  virtual void DoVioRead(faudes::TokenReader& rTr);

  // reimplement: update visual data from (new) faudes object
  virtual void DoVioUpdate(void);

  // representation data: order of elements
  VioElement::EType mEType;
  QList<VioElement> mElementList;
  QMap<VioElement,int> mElementRowMap;

  // representation data: interpret as  q table model
  LioVList* mTableModel;

  // fix internal reverse map
  void DoFixRowMap(void);

};


/*
 ************************************************
 ************************************************

 A VioGeneratorListView is a view for objects of type
 VioGeneratorListModel. It is derived from VioGeneratorVirtualView
 and hence integrates in the VioGeneratorWidget.

 ************************************************
 ************************************************
 */



class VioGeneratorListView : public VioGeneratorAbstractView {

Q_OBJECT

public:

  // construct/destruct
  VioGeneratorListView(VioGeneratorView* parent=0);
  virtual ~VioGeneratorListView(void) {}; // todo: tell my model that im gone?

  // set model
  // note: the view does *NOT* become the owner of the model
  // note: return non-zero if we cannot represnt this model
  // note: setting the model updates the ref to the VioGeneratorModel
  // note: setting the model will connect its notification signals to my slots
  virtual int Model(VioGeneratorAbstractModel* model);
  const VioGeneratorListModel* Model(void) const;

  // get element type
  VioElement::EType  ElementType(void) const { return mEType; }

  // update view from model 
  virtual void DoVioUpdate(void);

public slots:

  // editing slots: my list model notifies me that the faudes
  // generator and the relevant representation data has changed.
  // user editing is passed on by callbacks to the VioGeneratorListModel

  // editing faudes object: by elementary type, defaults to update all
  virtual void UpdateElementIns(const VioElement& elem);
  virtual void UpdateElementDel(const VioElement& elem);
  virtual void UpdateElementEdit(const VioElement& selem, const VioElement& delem);
  virtual void UpdateElementProp(const VioElement& elem);

  // editing faudes object: global, defaults to update all
  virtual void UpdateTrimElements(void);
  virtual void UpdateAnyAttr(void);
  virtual void UpdateAnyChange(void);
  virtual void UpdateNewModel(void);


protected:

  // typed version of model to present
  VioGeneratorListModel* pGeneratorListModel;
  LioVList* pTableModel;

  // element type
  VioElement::EType mEType;

  // main layout
  LioVView* mTableView;

};


#endif
