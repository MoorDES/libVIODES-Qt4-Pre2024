/* gioitem.cpp  - base class for items in gioscene */


/*
   Graphical  IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006, 2007  Thomas Moor, Klaus Schmidt, Sebastian Perk

*/


#include <cmath>
#include <string>
#include <iostream>

// resolve forwards
#include "viogengraph.h"
#include "gioitem.h"
#include "giostate.h"
#include "giotrans.h"
#include "gioscenero.h"



// default constructor
GioItem::GioItem(VioGeneratorModel* gmodel) : 
  QGraphicsItem(0), 
  pGeneratorModel(gmodel), 
  pGeneratorConfig(0), 
  pRenderOptions(0)
{
  FD_DQ("GioItem::GioItem()");


  // style reference
  pGeneratorConfig = pGeneratorModel->GeneratorConfiguration();
  pRenderOptions =  pGeneratorConfig->NewRenderOptions();
  
  // item edit mode
  mCtrlPointEdit=-1;
  mCtrlPointEditing=-1;
  mUpdateFlag=false;
  mHighlite=false;

  // item flags
  setFlag(QGraphicsItem::ItemIsMovable, false);
  setFlag(QGraphicsItem::ItemIsSelectable, true);
  setAcceptDrops(false);
  setAcceptedMouseButtons(Qt::LeftButton);

  // dummy graphic representation for debugging
  mBasePoints.clear();
  mBasePoints.append(QPointF(0,0));
  mBasePoints.append(QPointF(-10,-10));
  mBasePoints.append(QPointF(10,10));


  // do it
  GioItem::updateData();
}


// set all data
void GioItem::setData(const Data& data) {
  // clear item edit mode
  mCtrlPointEdit=-1;
  mCtrlPointEditing=-1;
  mUpdateFlag=false;
  // set data
  mBasePoints=data.mBasePoints;
  setPos(data.mPosition);
  // update other members
  updateData();
}

// get all data
GioItem::Data GioItem::data(void) {
  Data res;
  res.mPosition = pos();
  res.mBasePoints = mBasePoints;
  return res;
}

// data io reading
void GioItem::Data::read(faudes::TokenReader& tr) {
  // item
  tr.ReadBegin("Item");
  // position
  tr.ReadBegin("Position");
  mPosition.rx()=tr.ReadFloat();
  mPosition.ry()=tr.ReadFloat();
  tr.ReadEnd("Position");
  // read base points
  mBasePoints.clear();
  tr.ReadBegin("BasePoints");
  while(!tr.Eos("BasePoints")){
    QPointF point;
    point.rx()=tr.ReadFloat();
    point.ry()=tr.ReadFloat();
    mBasePoints.append(point);
    FD_DQ("GioItem::Data::read: basepoint " << point.rx() << " " << point.ry());
  }
  tr.ReadEnd("BasePoints");
  // done
  tr.ReadEnd("Item");
}

// data io writing
void GioItem::Data::write(faudes::TokenWriter& tw, const fGenerator* pGen) const {
  (void) pGen;
  tw.WriteBegin("Item");
  // position
  tw.WriteBegin("Position");
  tw.WriteFloat(mPosition.x());
  tw.WriteFloat(mPosition.y());
  tw.WriteEnd("Position");
  // base points
  tw.WriteBegin("BasePoints");
  foreach(QPointF point, mBasePoints) { 
    tw.WriteFloat(point.x());
    tw.WriteFloat(point.y());
  }
  tw.WriteEnd("BasePoints");
  // done
  tw.WriteEnd("Item");
}

// data to/from string
QString GioItem::Data::toString(void) {
  faudes::TokenWriter tw(faudes::TokenWriter::String);
  write(tw);
  return VioStyle::QStrFromStr(tw.Str());
}

// data from string
void GioItem::Data::fromString(const QString& str) {
  std::string buf=VioStyle::StrFromQStr(str);
  faudes::TokenReader tr(faudes::TokenReader::String,buf);
  read(tr);
}


// item io reading
void GioItem::read(faudes::TokenReader& tr) {
  Data idata;
  idata.read(tr);
  setData(idata);
}

// item io writing
void GioItem::write(faudes::TokenWriter& tw) {
  Data idata;
  idata=data();
  idata.write(tw);
}



// access generator model
VioGeneratorModel* GioItem::GeneratorModel(void) {
  return pGeneratorModel;
}

// access generator config
VioGeneratorStyle* GioItem::GeneratorConfiguration(void) {
  return pGeneratorConfig;
}

// access generator 
const fGenerator* GioItem::Generator(void) {
  return pGeneratorModel->Generator();
}

// test validity
bool GioItem::valid(void) {
  return false;
}

// access scene
GioSceneRo* GioItem::Scene(void) {
  return dynamic_cast<GioSceneRo*>(scene());
}

// access some state
GioState* GioItem::StateItem(faudes::Idx index) {
  GioSceneRo* scene=Scene();
  if(!scene) return NULL;
  return scene->StateItem(index);
}

// access some trans
GioTrans* GioItem::TransItem(const faudes::Transition&  ftrans) {
  GioSceneRo* scene=Scene();
  if(!scene) return NULL;
  return scene->TransItem(ftrans);
}

// access some trans by target
QList<GioTrans*> GioItem::TransItemsByTarget(faudes::Idx idxB) {
  GioSceneRo* scene=Scene();
  if(!scene) { QList<GioTrans*> res; return res;}
  return scene->TransItemsByTarget(idxB);
}

// report user changes to parent
void GioItem::Modified(void) {
  GioSceneRo* scene=Scene();
  if(!scene) return;
  scene->ChildModified(true);
}

// position by hints
void GioItem::hintA(QPointF posA) {
  FD_DQ("GioIte,::hintA");
  setPos(posA);
}


// draw: controls
void GioItem::paintCtrls(
  QPainter *painter, 
  const QStyleOptionGraphicsItem *option,
  QWidget *widget) 
{
  (void) option; // avoid warning
  (void) widget; // avoid warning
  if(!isSelected()) return;
  if(mCtrlFixed.size()!=mCtrlPoints.size()) {
    FD_WARN("gioItem: internal error: ctrl mismatch");
    return;
  }
  /*
  FD_WARN("gioItem: scale x " << painter->worldTransform().m11());
  FD_WARN("gioItem: scale y " << painter->worldTransform().m22());
  qreal sx=10.0 / painter->worldTransform().m11();
  qreal sy=10.0 / painter->worldTransform().m22();
  */
  qreal sx=4.0;
  qreal sy=4.0;
  for(int i=mCtrlPoints.size()-1; i>=0; i--)
    if(mCtrlFixed[i])
      painter->fillRect(QRectF(mCtrlPoints[i].x()-sx, mCtrlPoints[i].y()-sy, 2*sx,2*sy), Qt::blue);
  for(int i=mCtrlPoints.size()-1; i>=0; i--)
    if(!mCtrlFixed[i])
      painter->fillRect(QRectF(mCtrlPoints[i].x()-sx, mCtrlPoints[i].y()-sy, 2*sx,2*sy), Qt::red);
}

// draw: highlite
void GioItem::paintHighlite(
  QPainter *painter, 
  const QStyleOptionGraphicsItem *option,
  QWidget *widget) 
{
  (void) option; // avoid warning
  (void) widget; // avoid warning
  if(mHighlite) { 
    painter->setBrush(Qt::red);
    painter->setPen(Qt::NoPen); // todo: use highlight pen for state
    painter->drawPath(mCoreShape);
  }  
}

// draw paths, controls etc
void GioItem::paint(
  QPainter *painter, 
  const QStyleOptionGraphicsItem *option,
  QWidget *widget) {
  (void) painter;
  (void) option;
  (void) widget;
  //FD_DQ("GioItem::paint"); 
  // paintHighlite(painter,option,widget);
  // paintCtrls(painter,option,widget);
#ifdef FAUDES_DEBUG_VIO
  // debug shape
  /*
  painter->setPen(Qt::green);
  painter->setBrush(Qt::green);
  painter->drawPath(shape());
  painter->setPen(Qt::red);
  painter->setBrush(Qt::NoBrush);
  painter->drawRect(boundingRect());
  */
#endif
}


// tell view shape for scene select etc
QPainterPath GioItem::shape() const {
  if(isSelected()) return mShape;
  return mCoreShape;
}


// tell view our bounary for scene select etc
QRectF GioItem::boundingRect() const {
  return mOuterRect;
};


// construct draw paths from updated base points
void GioItem::updatePaths(void) {
  FD_DQ("GioItem::updatePaths()");

  // set default ctrl path
  mCtrlPath = QPainterPath(); 
  if(mCtrlPoints.size()>0){
    mCtrlPath.moveTo(mCtrlPoints[0]);
    for(int i=1; i<mCtrlPoints.size(); i++) { 
      mCtrlPath.lineTo(mCtrlPoints[i]);
    }
    for(int i=mCtrlPoints.size()-2; i>=0; i--) { 
      mCtrlPath.lineTo(mCtrlPoints[i]);
    }
  }
  // set shape and outer rect for selection an co
  mOuterRect=mCtrlPath.controlPointRect(); 
  mOuterRect.adjust(-4,-4,4,4); //todo: style
  mShape=QPainterPath();
  mShape.addRect(mOuterRect);
  mCoreShape=mShape;
}


// construct base points from updated controls
void GioItem::updateBase(void) {
  mBasePoints=mCtrlPoints;
}

// construct control points from updated base points
void GioItem::updateCtrls(void) {
  FD_DQ("GioItem::updateCtrls()");
  mCtrlPoints=mBasePoints;
  mCtrlFixed.clear();
  for(int i=0; i<mCtrlPoints.size(); i++)
    mCtrlFixed.append(false);
}

// update flag: controls have changed
void GioItem::updateFlag(void) {
  mUpdateFlag=true;
}
 
// update doit: update if flagged
void GioItem::updateDoit(void) {
  if(!mUpdateFlag) return;
  prepareGeometryChange();
  updateBase();
  updatePaths();
  update();
  mUpdateFlag=false;
}

// update user data: redraw from base
void GioItem::updateData(void) {
  FD_DQ("GioItem::updateData()");
  prepareGeometryChange();
  updateText();
  updateCtrls();
  updatePaths();
  update();
}


// set appearance: highlite
void GioItem::highlite(bool on) {
  if(mHighlite!=on) {
    mHighlite=on;
    updatePaths();
    update();
  }  
};


// is the (scene-) point a control?
bool GioItem::isCtrl(const QPointF& pos) {
  return whichCtrlPoint(mapFromScene(pos))>=0;
};


// closeset qualified control (or -1 if no such)
int GioItem::whichCtrlPoint(const QPointF &point, int a, int b) {
  int mindis=10000; 
  int minidx=-1;
  if(!isSelected()) return(minidx);
  if(a<0) a=0;
  if(b<0) b=mCtrlPoints.size()-1;
  if(a>=mCtrlPoints.size()) a=mCtrlPoints.size()-1;
  if(b>=mCtrlPoints.size()) b=mCtrlPoints.size()-1;
  if(a<0 || b <0) return -1;
  int dist;
  for(int i=a; i<=b; i++) {
     QPoint diff=point.toPoint()-mCtrlPoints[i].toPoint();
     dist=diff.manhattanLength();
     if(mCtrlFixed[i]) dist+= (int) pGeneratorConfig->CtrlTolerance()/2;
     if(dist<mindis) {mindis=dist; minidx=i;};
  }
  if(mindis> pGeneratorConfig->CtrlTolerance()) minidx =-1;
  return(minidx);
}


// closest qualified control segment (or -1 if no such)
int GioItem::whichCtrlSegment(const QPointF &point, int a, int b) {
  QRectF whererect= QRectF(
     point.x()-pGeneratorConfig->CtrlTolerance(),
     point.y()-pGeneratorConfig->CtrlTolerance(),
     pGeneratorConfig->CtrlTolerance() *2,
     pGeneratorConfig->CtrlTolerance() *2);
  //if(!mCtrlPath.intersects(whererect))  return -1;
  if(a<0) a=0;
  if(b<0) b=mCtrlPoints.size()-1;
  if(a>=mCtrlPoints.size()) a=mCtrlPoints.size()-1;
  if(b>=mCtrlPoints.size()) b=mCtrlPoints.size()-1;
  for(int i=a; i< b-1; i++) {
    QPainterPath segpath;
    segpath.moveTo(mCtrlPoints[i]+QPointF(0,1));  // tweak: make this work with exactly horizontal lines
    segpath.lineTo(mCtrlPoints[i+1]+QPointF(1,0));
    segpath.moveTo(mCtrlPoints[i]+QPointF(0,1));
    if(segpath.intersects(whererect))  return i;
  }
  return -1;
}


// insert a control
void GioItem::insCtrl(int ctrl, const QPointF& ctrlpoint) {
  FD_DQ("GioItem::insCtrl" << ctrl); 
  if(ctrl<0 || ctrl > mCtrlPoints.size() ) return;
  mCtrlPoints.insert(ctrl,ctrlpoint);
  mCtrlFixed.insert(ctrl,false);
  Modified();
  updateFlag();
}

// delete a control
void GioItem::delCtrl(int ctrl) {
  FD_DQ("GioItem::delCtrl" << ctrl); 
  if(ctrl<0 || ctrl >= mCtrlPoints.size() ) return;
  mCtrlPoints.removeAt(ctrl);
  mCtrlFixed.removeAt(ctrl);  
  Modified();
  updateFlag();
}

// move control
bool GioItem::moveCtrlPos(const QPointF& ctrlpoint) {
  FD_DQ("GioItem::moveCtrlPos " << mCtrlPointEdit << " flag " << mCtrlPointEditing); 
  if(mCtrlPointEdit <0 || mCtrlPointEdit >= mCtrlPoints.size() ) return false;
  if(mCtrlFixed[mCtrlPointEdit]) return false;
  QPointF sctrlpoint = pGeneratorConfig->GridPoint(mapToScene(ctrlpoint));
  QPointF newpos= mapFromScene(sctrlpoint);
  if(mCtrlPoints[mCtrlPointEdit] == newpos) return false;  
  mCtrlPoints[mCtrlPointEdit] = newpos;
  if(mCtrlPointEditing!=1) 
  if(mCtrlPoints[mCtrlPointEdit]!=mCtrlPointStartPos) {
    if(mCtrlPointEditing<0) GeneratorModel()->UndoEditStart();
    mCtrlPointEditing=1;
  }
  Scene()->CtrlEditing(sctrlpoint);
  updateFlag();
  return true;
} 

// record ctrl to move (eg via mouse-move)
void GioItem::moveCtrlStart(int ctrl) {
  FD_DQ("GioItem::moveCtrlStart " << ctrl); 
  if(ctrl<0 || ctrl >= mCtrlPoints.size() ) return;
  if(mCtrlFixed[ctrl]) return;
  mCtrlPointStartPos=mCtrlPoints[ctrl];
  mBasePointsStartPos=mBasePoints;
  mCtrlPointEdit=ctrl;
} 

// end moving 
void GioItem::moveCtrlStop(void) {
  FD_DQ("GioItem::moveCtrlStop " << mCtrlPointEdit); 
  if(mCtrlPointEdit>=0 && mCtrlPointEdit< mCtrlPoints.size()) 
  if(mCtrlPointStartPos != mCtrlPoints[mCtrlPointEdit]) Modified();
  mCtrlPointEdit=-1;
  mCtrlPointEditing=0;
  Scene()->CtrlEditing();
  mBasePointsStartPos.clear();
}


// track selection state
QVariant GioItem::itemChange(GraphicsItemChange change, const QVariant & value) {
  if(change == QGraphicsItem::ItemSelectedHasChanged) {
    if(value.toBool()==false) 
    if(mCtrlPointEditing>=0) { 
      mCtrlPointEditing=-1;
      GeneratorModel()->UndoEditStop();
      Scene()->CtrlEditing();
    }
    if(value.toBool()==true) 
      mCtrlPointEditing=-1;
  }
  return QGraphicsItem::itemChange(change,value);
}

// handle my events: mouse press
void GioItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
  FD_DQ("GioItem::mousePress"); 
  if(!isSelected()) return;
  if(event->button() != Qt::LeftButton) return;
  // figure where we are: control point
  const QPointF where=event->pos();
  int ctrlpoint=whichCtrlPoint(where);
  // edditing control point?
  if(ctrlpoint>=0) {  
    if(!mCtrlFixed[ctrlpoint]) {
      moveCtrlStart(ctrlpoint);
      updateDoit();
      event->accept();
    }
  }
  // pass on event to qt
  QGraphicsItem::mousePressEvent(event);
}

// handle my events: mouse release
void GioItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
  FD_DQ("GioItem::mouseRelease " << mCtrlPointEdit); 
  // end editing
  if(mCtrlPointEdit>=0) {
    moveCtrlStop();
    updateDoit();
    event->accept();
    return;
  }
  // pass on event to qt
  QGraphicsItem::mouseReleaseEvent(event);
}

// handle my events: mouse move
void GioItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
  FD_DQ("GioItem::mouseMove: at " << mCtrlPointEdit); 
  // move ctrl
  if(mCtrlPointEdit>=0) {
    moveCtrlPos(event->pos());
    updateDoit();
    event->accept();
    return;
  }
  FD_DQ("GioItem::mouseMove: pass to qitem "); 
  // pass on event to qt (will move the entire item on scene)
  QGraphicsItem::mouseMoveEvent(event);
}

// debug double clicks
void GioItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event){
   (void) event;
   FD_DQ("GioItem::mouseDoubleClick()");
}



// handle my events: drag enter
void GioItem::dragEnterEvent(QGraphicsSceneDragDropEvent* event) {
   (void) event;
   FD_DQ("GioItem::dragEnterEvent()");
}

// handle my events: drag leave
void GioItem::dragLeaveEvent(QGraphicsSceneDragDropEvent* event) {
   (void) event;
   FD_DQ("GioItem::dragLeaveEvent()");
}

// handle my events: drag enter
void GioItem::dragMoveEvent(QGraphicsSceneDragDropEvent* event) {
   (void) event;
   FD_DQ("GioItem::dragMoveEvent()");
}
