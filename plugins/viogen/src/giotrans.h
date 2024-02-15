/* giotrans.h  - graphical representation of one transition */

/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006, 2007  Thomas Moor, Klaus Schmidt, Sebastian Perk

*/


#ifndef FAUDES_GIOTRANS_H
#define FAUDES_GIOTRANS_H

#include "gioitem.h"

/*
 ************************************************
 ************************************************

 Graphic representation of a transition; see GioItem
  
 A GioTrans is a GioItem with additional members mIdxA,
 mIdxEv, mIdxB to identify the corresponding faudes transition 
 in pGenerator. Such transition may or may not exist. GioTrans also 
 adds semantics to mBasePoints (name tag, begin and end point, 
 arrow tip, tick). The actual geometry is implemented via 
 VioStyle methods and may depend on faudes event flags e.g.
 controllability.

 ************************************************
 ************************************************
 */


class GioTrans : public GioItem {

public:
  //******  constructor 
  GioTrans(VioGeneratorModel* gmodel);

  // set faudes data
  void IdxA(faudes::Idx idxa)  {mIdxA=idxa;};
  void IdxEv(faudes::Idx idxe) {mIdxEv=idxe;};
  void IdxB(faudes::Idx idxb)  {mIdxB=idxb;};
  void FTrans(const faudes::Transition& ftrans)  {
    mIdxA=ftrans.X1; mIdxEv=ftrans.Ev; mIdxB=ftrans.X2;};

  // validity
  bool valid(void) { return Generator()->ExistsTransition(FTrans());};

  // generator data access
  faudes::Idx IdxA(void) {return mIdxA;};
  faudes::Idx IdxEv(void){return mIdxEv;};
  faudes::Idx IdxB(void) {return mIdxB;};
  faudes::Transition FTrans(void) {return faudes::Transition(mIdxA,mIdxEv,mIdxB); }
  const std::string FNameA(void) {return Generator()->StateName(mIdxA);};
  const std::string FNameEv(void) {return Generator()->EventName(mIdxEv);};
  const std::string FNameB(void) {return Generator()->StateName(mIdxB);};
  const QString NameA(void) {return VioStyle::QStrFromStr(FNameA());};
  const QString NameEv(void) {return VioStyle::QStrFromStr(FNameEv());};
  const QString NameB(void) {return VioStyle::QStrFromStr(FNameB());};

  //****** query lable pos
  QPointF PointN(void) { return mapToScene(mBasePoints[3]);}; // 3 aka POINT_N

  //****** edit mode
  typedef enum {Free, Line, Polygon, Smooth, Spline, Mute} EditMode;
  void setEditMode(EditMode mode);
  EditMode editMode(void) const;

  //****** transform spline
  void convertToFree(void);
  void convertToSpline(void);
  void convertToLine(void);
  void convertToPolygon(void);
  void convertToSmooth(void);
  void convertToMute(void);

  //****** convenience class: all data
  class Data : public GioItem::Data {
  public:
    // transition user data
    faudes::Idx    mIdxA;   
    std::string    mNameEv; 
    faudes::Idx    mIdxB;    
    EditMode       mEditMode; 
    // override io
    virtual void write(faudes::TokenWriter& tw, const fGenerator* pGen=NULL) const;
    virtual void read(faudes::TokenReader& tr);
    virtual ~Data(void) {};
  };

  //****** set/get all data
  Data data(void);
  void setData(const Data& data);

  // file io (exception on error)
  void write(faudes::TokenWriter& tw);
  void read(faudes::TokenReader& tr);

  //****** positioning/shaping by hints
  void hintB(const QPointF& posB);
  void hintAB(const QPointF& posA, const QPointF& posB);
  void moveA(const QPointF& posA);
  void moveB(const QPointF& posB);
  void moveX(const QPointF& posX);
  void moveT(const QPointF& posT);
  void moveN(const QPointF& labelpos);
  void moveC(const QList<QPointF>& ctrlpoints);
  void moveS(const Data& data);

  //******  (re) implement QGraphicsItem functions

  // paint myself
  void paint(
    QPainter *painter, 
    const QStyleOptionGraphicsItem *option,
    QWidget *widget);

  // rtti
  enum { Type = UserType+3};
  virtual int type(void) const { return Type;};

  // get mouse events
  void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
  void mousePressEvent(QGraphicsSceneMouseEvent *event);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

  //******  (re) implement GioItem functions


protected:
  //******  points and paths for actual drawing (overload from gioitem)

  void updateCtrls(void);
  void updatePaths(void);
  void updateText(void);
  void updateBase(void);

  //******  edit controls 
  void insCtrl(int pos, const QPointF& ctrlpoint);
  void delCtrl(int ctrl);
  void moveCtrlStart(int ctrl);
  bool moveCtrlPos(const QPointF& ctrlpoint);
  void moveCtrlStop(void);

  //******* spline/trans specific
  int whichCtrlPoint(const QPointF &point,int a=-1, int b=-1);
  int whichSplineSegment(const QPointF &point);
  void splitSplineSegment(int segment, const QPointF& point);
  void mergeSplineSegment(int segment);
  void fixRootAndTip(void);

private:
  // transition user data
  faudes::Idx mIdxA, mIdxEv, mIdxB;

  // helper vars for drawing
  QRectF mNameRect00;

  // edit mode;
  EditMode mEditMode;

};

#endif
