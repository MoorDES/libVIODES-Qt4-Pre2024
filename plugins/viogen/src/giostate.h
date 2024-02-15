/* giostate.h  - graphical representation of one state */


/*
   Graphical  IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006, 2007  Thomas Moor, Klaus Schmidt, Sebastian Perk

*/


#ifndef FAUDES_GIOSTATE_H
#define FAUDES_GIOSTATE_H

#include "gioitem.h"

/*
 ************************************************
 ************************************************

 Graphic representation of a  state; see GioItem
  
 A GioState is a GioItem with additional member mIdx
 to identify the corresponding faudes state in pGenerator.
 Such state may or may not exist. GioState also adds semantics to 
 mBasePoints (center of shape, lower right point of shape,
 name tag, initial state arrow). The actual geometry is 
 implemented via VioStyle methods and may depend on faudes
 flags.

 ************************************************
 ************************************************
 */

class GioState : public GioItem {

public:

  //****** constructor 
  GioState(VioGeneratorModel* gmodel);
  
  //****** set user data
  void Idx(faudes::Idx idx) {mIdx=idx;};

  //****** generator data access
  faudes::Idx Idx(void) {return mIdx;};
  const std::string FName(void) {return Generator()->StateName(mIdx);};
  const QString Name(void) {return VioStyle::QStrFromStr(FName());};

  //****** test whether this is a valid faudes item and exists in the generator model
  bool valid(void) { return Generator()->ExistsState(mIdx);};

  //****** convenience class: all state item data
  class Data : public GioItem::Data {
  public:
    // state user data
    faudes::Idx    mIdx;        
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

  //****** edit shape
  void moveL(const QPointF& point);
  void moveA(const QPointF& point);
  void moveS(const Data& data);

  //******  (re) implement QGraphicsItem functions

  // paint myself
  void paint(
    QPainter *painter, 
    const QStyleOptionGraphicsItem *option,
    QWidget *widget);

  // rtti
  enum { Type = UserType+2};
  virtual int type(void) const { return Type;};


  //******  (re) implement GioItem functions


protected:
  //******  (re) implement GioItem functions

  //points and paths 
  void updateCtrls(void);
  void updatePaths(void);
  void updateText(void);
  void updateBase(void);

  // edit control
  void moveCtrlStart(int ctrl);
  bool moveCtrlPos(const QPointF& ctrlpoint);

  // helper: figure related transitions
  void setTargetTransitions(void);

private:
  // extra user data
  faudes::Idx mIdx;

  // extra graphics helper
  QList<faudes::Transition> mTargetTransitions;
  QRectF mNameRect;
  bool mPaintName;
};


#endif
