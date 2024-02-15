/* viogengraph.h  - graph representation of generators */


/*

   Visual IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2009  Thomas Moor, Ruediger Berndt

*/

#ifndef FAUDES_GENGRAPH_H
#define FAUDES_GENGRAPH_H

// std includes
#include "viogenerator.h"

// item data includes
#include "giostate.h"
#include "giotrans.h"


// forward
class VioGeneratorGraphView;
class GioScene;
class GioView;

/*
 ************************************************
 ************************************************

 VioGeneratorGraphData holds the static data
 for the graph representation, i.e. location of
 nodes and control points of edges.

 The representation data access is limited to
 get/set all of it. Detailed editing is managed 
 by an internal qt graphics scene.

 ************************************************
 ************************************************
 */


class VioGeneratorGraphData : public VioGeneratorAbstractData {

Q_OBJECT

public:

  // construct/destruct
  VioGeneratorGraphData(QObject* parent=0);
  virtual ~VioGeneratorGraphData(void);

  // conversion (ret 0 on sucess)
  virtual void ToTokenWriter(faudes::TokenWriter& rTw) const;
  virtual int  FromTokenReader(faudes::TokenReader& rTr);

  // state reindexing
  virtual void ApplyStateIndicees(const QMap<faudes::Idx,faudes::Idx> & rNewIdx);

  // clear to default (empty)
  virtual void Clear(void);

  // all data container
  QList<GioTrans::Data> mTransItemsData;
  QList<GioState::Data> mStateItemsData;

};


/*
 ************************************************
 ************************************************

 A VioGeneratorGraphModel holds data for the
 graph representation of generators. It is derived 
 from VioGeneratorVirtualModel and, hence, is meant 
 to be used in the container model VioGeneratorModel. 
 Actual representation is done by the view 
 VioGeneratorGraphView, see below.

 The representation data access is limited to
 get/set all of it. Detailed editing is managed 
 by an internal qt graphics scene.

 ************************************************
 ************************************************
 */


class VioGeneratorGraphModel : public VioGeneratorAbstractModel {

Q_OBJECT

public:

  // construct/destruct
  VioGeneratorGraphModel(VioGeneratorModel* parent);
  virtual ~VioGeneratorGraphModel(void) {}; 

  // access as qt graphics scene
  GioScene* GraphScene(void);

  // reimplement: create new view for this representationmodel
  virtual VioGeneratorAbstractView* NewView(VioGeneratorView* parent);

  // reimplement: clear to default/empty representation
  void Clear(void);
  int Size(void) const;

  // re-implement data access
  virtual VioGeneratorAbstractData* Data(void);
  virtual VioGeneratorAbstractData* SelectionData(void);
  virtual int Data(const VioGeneratorAbstractData* pData);
  virtual int TypeCheckData(const VioGeneratorAbstractData* pData) {(void) pData; return 1;};

  // write graphics (ret 0<> OK)
  virtual int WritePdf(const QString& filename) const;
  virtual int WriteEps(const QString& filename) const;
  virtual int WriteSvg(const QString& filename) const;
  virtual int WriteJpg(const QString& filename) const;
  virtual int WritePng(const QString& filename) const;

  // reimplement to clr child 
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

  // editing model: selection
  virtual void UpdateSelectionElement(const VioElement& elem, bool on=true);
  virtual void UpdateSelectionClear(void);
  virtual void UpdateSelectionAny(void);


protected:

  // typed ref to parent 
  VioGeneratorModel* pVioGeneratorModel;

  // reimplement: token io
  virtual void DoVioWrite(faudes::TokenWriter& rTw) const;
  virtual void DoVioRead(faudes::TokenReader& rTr);

  // reimplement: update visual data from (new) faudes object
  virtual void DoVioUpdate(void);

  // representation data: specialzed qt scene
  GioScene* mGraphScene;



};


/*
 ************************************************
 ************************************************

 A VioGeneratorGraphView is a view for objects of type
 VioGeneratorGraphModel. It is derived from 
 VioGeneratorVirtualView and hence integrates in 
 the VioGeneratorWidget.

 ************************************************
 ************************************************
 */



class VioGeneratorGraphView : public VioGeneratorAbstractView {

Q_OBJECT

public:

  // construct/destruct
  VioGeneratorGraphView(VioGeneratorView* parent=0);
  virtual ~VioGeneratorGraphView(void) {}; 

  // set model
  // note: the view does *NOT* become the owner of the model
  // note: return non-zero if we cannot represent this model
  // note: setting the model updates the ref to the VioGeneratorModel
  // note: setting the model will connect its notification signals to my slots
  virtual int Model(VioGeneratorAbstractModel* model);
  const VioGeneratorGraphModel* Model(void) const;

  // update view from model 
  virtual void DoVioUpdate(void);

public slots:

  // editing slots: my graph model notifies me that the faudes
  // generator and the relevant representation data has changed.
  // user editing is passed on by callbacks to the VioGeneratorGraphModel
  virtual void UpdateAnyChange(void);
  virtual void UpdateNewModel(void);

  // test consistency and switch to/from fake widget 
  virtual void UpdateConsistent(bool);

  // user interaction slots
  void  Scale(qreal sf);
  qreal Scale(void);
  void Fit(void);
  void DotConstruct(void);
  void GridConstruct(void);
  void GridVisible(bool on);


  // highlite/show request
  void Highlite(const VioElement& elem, bool on=true);
  void HighliteClear(void);
  void Show(const VioElement& elem);



protected:

  // typed version of model to present
  VioGeneratorGraphModel* pGeneratorGraphModel;
  GioScene* pGraphScene;

  // main layout
  GioView* mGraphView;
  QWidget* mFakeView;
  QStackedWidget* mStack;
  QPushButton* mGridButton;
  QPushButton* mDotButton;

};


#endif
