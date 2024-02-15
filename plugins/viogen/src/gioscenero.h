/* gioscenero.h  - faudes generator as qgraphicsscene */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006, 2007  Thomas Moor, Klaus Schmidt, Sebastian Perk

*/


#ifndef FAUDES_GIOSCENERO_H
#define FAUDES_GIOSCENERO_H

#include "libviodes.h"
#include "giostate.h"
#include "giotrans.h"


/*
 ************************************************
 ************************************************

 A GioScene is a QGraphicsSene that holds GioItems and 
 a reference to a faudes generator to provide the 
 context for GioItems. A GioScene will never change the
 faudes generator itself, but it uses callbacks to
 request such change. Conversely, it receives signals
 form the generator model that indicate that it was changed.

 In contrast to GioScene, GioSceneRo is read only in the
 sense that it does not support user interaction that
 changes the geometry of items or the corresponding
 faudes generator. However, it is interactive in that 
 there is highliting and selection. 

 ************************************************
 ************************************************
 */

class VioGeneratorGraphModel;
 
class GioSceneRo : public QGraphicsScene {

  Q_OBJECT

public:

  // allow item to access private functions
  friend class GioItem;

  // constructor, destructor
  GioSceneRo(VioGeneratorGraphModel* pGen);
  virtual ~GioSceneRo(void);

  // read only access to faudes generator 
  const fGenerator* Generator(void) const;

  // access to generator model
  VioGeneratorModel* GeneratorModel(void) { return pGeneratorModel; };

  // record modification
  void Modified(bool ch);
  bool Modified(void);
  void ChildModified(bool ch);

  // consistency: gioitems match generator
  bool Consistent(void) { return mConsistent;};
  void Consistent(bool cons);
  int  TestConsistent(void);
 
  // access gio items by faudes id
  GioTrans* TransItem(const faudes::Transition& ftrans); 
  GioState* StateItem(faudes::Idx index); 
  QList<GioTrans*> TransItemsByTarget(faudes::Idx idxB);

  // test for empty
  bool Empty(void) const { return mStateItems.empty() && mTransItems.empty();};

  // read-only interface (should be data based)
  const QList<GioState*> States(void) const { return mStateItems; };
  const QList<GioTrans*> Trans(void)  const { return mTransItems; };

  // all data container
  class Data  {
  public:
    void clear(void);
    void statistics(void) const; 
    void write(faudes::TokenWriter& tw, const fGenerator* pGen) const;
    void read(faudes::TokenReader& tr);
    QList<GioTrans::Data> mTransItemsData;
    QList<GioState::Data> mStateItemsData;
  };

public slots:

  // clear scene and  data
  virtual int Clear(void);

  // extract graph data from various sources; returns number of 
  // missing items, or exception on io error
  int GioRead(faudes::TokenReader& rTr);  
  int GioRead(const Data& giodata);
  int GioConstruct(void);
  int GridConstruct(bool clr=true);
  int DotConstruct(const QString &dotfile);
  int DotConstruct(bool trans_only=false);

  // write graph data
  int GioWrite(faudes::TokenWriter& rTw);
  int GioWrite(Data& giodata);
  int PdfWrite(const QString &pdffile);
  int EpsWrite(const QString &epsfile);
  int PngWrite(const QString &pngfile);
  int JpgWrite(const QString &jpgfile);
  int SvgWrite(const QString &svgfile);
  int DotWrite(const QString &dotfile, bool mute_layout=false);
 
  // highlight items
  void Highlite(const VioElement& elem, bool on);
  void HighliteClear(void);

signals:

  // notify clicks to application
  void MouseClick(const VioElement& elem);
  void MouseDoubleClick(const VioElement& elem);

  // notify user interaction (no such in readonly)
  void NotifyModified(bool);

  // notify data match (incl UpdateNewModel from file io)
  void NotifyConsistent(bool);

public slots:

  // new viodes style update interface: by element
  void UpdateElementIns(const VioElement& elem); 
  void UpdateElementDel(const VioElement& elem);
  void UpdateElementEdit(const VioElement& selem, const VioElement& delem);
  void UpdateElementProp(const VioElement& elem);

  // new viodes style update interface: global
  void UpdateTrimElements(void);
  void UpdateAnyChange(void);
  void UpdateAnyAttr(void); 
  void UpdateNewModel(void);

  // insertion interface
  void InsGioState(GioState* state);
  void InsGioTrans(GioTrans* trans);

protected:

  // ensure the scene to be large enough
  void AdjustScene(void);

  // insert and delete items (no notification/generator call back; only my data)
  virtual GioState* addGioState(GioState* state);
  virtual GioState* addGioState(faudes::Idx idx, QPointF pos);
  virtual GioState* addGioState(faudes::Idx idx);
  virtual void removeGioState(GioState* state);
  virtual void removeGioState(faudes::Idx idx);
  virtual void moveGioState(GioState* state, faudes::Idx nfidx);
  virtual void moveGioState(faudes::Idx ofidx, faudes::Idx nfidx);
  virtual GioState* updateGioState(faudes::Idx idx);
  virtual GioTrans* addGioTrans(GioTrans* trans);
  virtual GioTrans* addGioTrans(const faudes::Transition& ftrans);
  virtual GioTrans* addGioTrans(faudes::Idx idxA, std::string event, faudes::Idx idxB);
  virtual void removeGioTrans(const faudes::Transition& ftrans);
  virtual void removeGioTrans(GioTrans* trans);
  virtual GioTrans* updateGioTrans(const faudes::Transition& ftrans);
  virtual void moveGioTrans(GioTrans* trans, const faudes::Transition& nftrans);
  virtual void moveGioTrans(const faudes::Transition& oftrans, const faudes::Transition& nftrans);

  // get ui events
  virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
  virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
  virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
  virtual void mouseDoubleClickEvent (QGraphicsSceneMouseEvent *event);

  // gio items, linked to faudes id
  QList<GioState*> mStateItems;
  QList<GioTrans*> mTransItems;
  QMap<faudes::Idx,GioState*> mStateMap;
  QMap<faudes::Transition,GioTrans*> mTransMap;

  // ref to VioGeneratorModel
  VioGeneratorModel* pGeneratorModel;
  VioGeneratorStyle* pGeneratorConfig;
  VioGeneratorGraphModel* pGeneratorGraphModel;

  // record changes 
  bool mModified;
  bool mConsistent;

  // allow items to tell us about ctrl editing
  bool mCtrlEditing;
  QPointF mCtrlPosition;
  virtual void CtrlEditing(void);
  virtual void CtrlEditing(const QPointF& pos);

};


#endif
