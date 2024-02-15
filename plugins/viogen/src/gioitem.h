/* gioitem.h  - base class for items in gioscene */


/*
   Graphical  IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006, 2007  Thomas Moor, Klaus Schmidt, Sebastian Perk

*/


#ifndef FAUDES_GIOITEM_H
#define FAUDES_GIOITEM_H

#include "libviodes.h"
#include "viogenstyle.h"

// forward
class VioGeneratorModel;

class GioSceneRo;
class GioState;
class GioTrans;




/*
 ************************************************
 ************************************************

 Editable graphics item, base class for GioState and GioTrans
 
 Graphics construction is organized in the following stages:

 1) base points:  are the basis for graphics construction; 
    define the abstract geometry of the item;  are used for file io; e.g.
    center of and one point on a circle; 

 2) painter paths: actual geometry of the item; paths are constructed from 
    base points via VioStyle functions e.g. VioStyle::State(center, lower right point); 
    construction may depend on user data e.g. state and event flags; paths
    includes outer shape and outer rect for the purpose of item selection

 3) painter paths are painted by painter

 *) for user interaction, control points are derived from base points, 
    edited by the user, and than converted back to base points

 A GioItem may only exist in the context of a generator model. The item 
 itself holds some minimal data to identifiy the corresponding state 
 or transition in the faudes generator. Editing of generator data (e.g. flags)
 is done by the corresponding GioScene. 

 ************************************************
 ************************************************
 */

class GioItem : public QGraphicsItem {

public:
  //******  constructor 

  GioItem(VioGeneratorModel* gmodel);
  ~GioItem(void) { delete pRenderOptions; }; // does this need to be virtual?

  //******  scene/generator data access

  GioSceneRo* Scene(void);
  VioGeneratorModel* GeneratorModel(void);
  VioGeneratorStyle* GeneratorConfiguration(void);
  const faudes::vGenerator* Generator(void);
  GioState* StateItem(faudes::Idx index); 
  GioTrans* TransItem(const faudes::Transition& ftrans); 
  QList<GioTrans*> TransItemsByTarget(faudes::Idx idxB); 
  void Modified(void);

  //****** convenience class: all item data
  class Data {
  public:
    QPointF        mPosition;   
    QList<QPointF> mBasePoints; 
    virtual void write(faudes::TokenWriter& tw, const faudes::vGenerator* pGen=NULL) const;
    virtual void read(faudes::TokenReader& tr);
    virtual QString toString(void);
    virtual void fromString(const QString& str);   
    virtual ~Data(void) {};
  };

  //****** set/get all data
  Data data(void);
  void setData(const Data& data);

  // file io (exception on error)
  virtual void write(faudes::TokenWriter& tw);
  virtual void read(faudes::TokenReader& tr);

  //****** request update from data, incl faudes flags/names etc
  virtual void updateData(void);

  //****** test whether this is a valid faudes item and exists in the generator model
  virtual bool valid(void);

  //******  set appearance
  void highlite(bool on);

  //****** position by hints
  void hintA(QPointF posA);

  //****** is control ?
  bool isCtrl(const QPointF& pos);

  //****** edit controls
  virtual int whichCtrlPoint(const QPointF &point,int a=-1, int b=-1);
  virtual int whichCtrlSegment(const QPointF &point,int a=-1, int b=-1);
  virtual void insCtrl(int pos, const QPointF& ctrlpoint);
  virtual void delCtrl(int ctrl);
  virtual void moveCtrlStart(int ctrl);
  virtual bool moveCtrlPos(const QPointF& ctrlpoint);
  virtual void moveCtrlStop(void);


  //****** graphical info: core shape (without controlpoints)
  const QPainterPath& coreShape(void) const {return mCoreShape;};

  //******  (re) implement QGraphicsItem functions

  // bounding rect and shape
  virtual QRectF boundingRect() const;
  virtual QPainterPath shape() const;

  // paint myself
  void paint(QPainter*, const QStyleOptionGraphicsItem* ,QWidget*);
  virtual void paintCtrls(QPainter*, const QStyleOptionGraphicsItem* o=0 ,QWidget* w=0);
  virtual void paintHighlite(QPainter*, const QStyleOptionGraphicsItem* o=0 ,QWidget* w=0);

  // rtti
  enum { Type = UserType+1};
  virtual int type(void) const { return Type;};



 protected:

  //******  reimplement

  // get mouse events
  virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
  virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
  virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
  virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
  virtual void dragEnterEvent(QGraphicsSceneDragDropEvent* event);
  virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent* event);
  virtual void dragMoveEvent(QGraphicsSceneDragDropEvent* event);
  virtual QVariant itemChange(GraphicsItemChange change, const QVariant & value);

  //******  points and paths for actual drawing

  // derive control points from base points (initialisation)
  virtual void updateCtrls(void);

  // derive base points from control points from (user interaction)
  virtual void updateBase(void);

  // derive base points from control points from (prepare drawing)
  virtual void updatePaths(void);

  // derive paths from user data that do not depend on base 
  virtual void updateText(void) {};

  // record the need for an update based on control points
  virtual void updateFlag(void);

  // execute the update based on control points
  virtual void updateDoit(void);

  // base points: abstract definition of item geometry
  QList<QPointF> mBasePoints;

  // control points: user interface to change base points
  QList<QPointF> mCtrlPoints;
  QList<bool>   mCtrlFixed;
  QPainterPath mCtrlPath;
  int mCtrlPointEdit;
  int mCtrlPointEditing;
  QPointF mCtrlPointStartPos;
  QList<QPointF> mBasePointsStartPos;
  
  // actual draw elements
  QList<GioDrawElement> mDrawElements;
  QList<QPainterPath> mDrawPaths;
  bool mHighlite;

  // update flags
  bool mUpdateFlag;

  // shape 
  QRectF mOuterRect;
  QPainterPath mShape;
  QPainterPath mCoreShape;

  // user data
  VioGeneratorModel* pGeneratorModel; 
  VioGeneratorStyle* pGeneratorConfig; 
  GioRenderOptions* pRenderOptions;

private:


};

#endif
