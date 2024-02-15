/* giostate.cpp  - graphical representation of one state */


/*
   Graphical  IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006, 2007  Thomas Moor, Klaus Schmidt, Sebastian Perk

*/


#include <QtGui>

#include "giostate.h"
#include "giotrans.h"

// resolve forwards
#include "gioscenero.h"


// convenience macros to acess base points
#define POINT_C 0   // center point: is always zero in local koordinates 
#define POINT_L 1   // diameter as rel. lower right point
#define POINT_A 2   // init arrow A->
#define POINT_B 3   // init arrow ->B
#define POINTS  4

// convenience macros to acess paths
#define PATH_NAME 0   // name
#define PATH_CL   1   // disk (aka path, plus extra paths)
#define PATH_AB   (mDrawElement.size()-1)   // arrow
#define PATHS 3       // minmum number of paths (plus multi marking)


// convenience macros to acess control points
#define CTRL_C1 0   // lower right
#define CTRL_C2 1   // lower left
#define CTRL_C3 2   // upper left
#define CTRL_C4 3   // upper right
#define CTRL_A  4   // arrow A->
#define CTRL_B  5   // arrow ->B
#define CTRLS   6



// default constructor
GioState::GioState(VioGeneratorModel* gmodel) : GioItem(gmodel) {
  FD_DQ("GioState::GioState()");

  // on top
  setZValue(-5);

  // user data
  mIdx=0;

  // points and paths
  while(mBasePoints.size()<POINTS)  mBasePoints.append(QPointF(0,0));
  while(mCtrlPoints.size()<CTRLS)   mCtrlPoints.append(QPointF(0,0));
  while(mCtrlFixed.size()<CTRLS)    mCtrlFixed.append(false);

  // set to reasonable default
  mBasePoints[POINT_C]=QPointF(0.0,0.0);
  mBasePoints[POINT_L]=QPointF(pGeneratorConfig->StateNormalSize()/2,pGeneratorConfig->StateNormalSize()/2);
  mBasePoints[POINT_A]=QPointF(- pGeneratorConfig->StateNormalSize(),0);
  mBasePoints[POINT_B]=QPointF(- pGeneratorConfig->StateNormalSize()/2,0);
  mBasePoints[POINT_L]=VioStyle::GridPoint(mBasePoints[POINT_L]);

  // update from base
  updateData();
}

// get all data
GioState::Data GioState::data(void) {
  Data res;
  res.mIdx = mIdx;
  res.mPosition = pos();
  res.mBasePoints = mBasePoints;
  return res;
}

// set all data
void GioState::setData(const Data& data) {
  mIdx=data.mIdx; 
  if(data.mBasePoints.size() == POINTS) {
    GioItem::setData(data); 
    return;
  }
  FD_DQ("GioState::setData(data): incompatible data");
  hintA(data.mPosition);
}

// data io read
void GioState::Data::read(faudes::TokenReader& tr) {
  FD_DQ("GioState::Data::read");
  tr.ReadBegin("State");
  // faudes index
  mIdx=tr.ReadInteger();
  FD_DQ("GioState::Data::read: " << mIdx);
  // read item
  GioItem::Data::read(tr);
  // done
  tr.ReadEnd("State");  
  // check validity
  if(mBasePoints.size()!=POINTS) {
    std::stringstream errstr;
    errstr << "base points mismatch " << tr.FileLine();
    throw faudes::Exception("GioState::Data::read", errstr.str(), 50);
  }
}


// data io write
void GioState::Data::write(faudes::TokenWriter& tw, const fGenerator* pGen) const {
  faudes::Token token;
  tw.WriteBegin("State");
  // faudes index
  faudes::Idx index = mIdx;
  if(pGen) index=pGen->MinStateIndex(index);
  tw << index;
  // write item
  GioItem::Data::write(tw);
  // done
  tw.WriteEnd("State");
}

// item io reading
void GioState::read(faudes::TokenReader& tr) {
  Data sdata;
  sdata.read(tr);
  setData(sdata);
}

// item io writing
void GioState::write(faudes::TokenWriter& tw) {
  Data sdata;
  sdata=data();
  sdata.write(tw,Generator());
}

// draw paths, controls etc
void GioState::paint(
  QPainter *painter, 
  const QStyleOptionGraphicsItem *option,
  QWidget *widget) {
  //FD_DQ("GioState::paint");
#ifdef FAUDES_DEBUG_VIO
  GioItem::paint(painter,option,widget);
#endif
  // highlite
  if(mHighlite) { 
    QPen pen=VioStyle::HighlitePen();
    pen.setWidth((int) 1.5*pen.width());
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(mCoreShape);
  }
  // draw paths elements in order 
  for(int j=PATH_CL; j < mDrawElements.size(); j++) {
    painter->setPen(*mDrawElements.at(j).pPen);
    painter->setBrush(*mDrawElements.at(j).pBrush);
    painter->drawPath(mDrawElements.at(j).mPath);
  }
  // name tag last
  if(mPaintName) {
    painter->setPen(Qt::NoPen);
    painter->setBrush(*mDrawElements.at(PATH_NAME).pBrush);
    painter->drawPath(mDrawElements.at(PATH_NAME).mPath);
  }
  // controls by base.
  GioItem::paintCtrls(painter,option,widget);
}


// construct draw paths from updated base points
void GioState::setTargetTransitions(void) {
  // figure related transitions
  const QList<GioTrans*> transitions = Scene()->Trans();
  mTargetTransitions.clear();
  for(int i=0; i< transitions.size(); i++) {
    if(transitions.at(i)->IdxB() != mIdx) continue;
    mTargetTransitions.append(transitions.at(i)->FTrans());
  }
}


// construct draw paths from updated base points
void GioState::updatePaths(void) {
  FD_DQ("GioState::updatePaths():" << FName());
  // remove paths incl PATH_CL
  while(mDrawElements.size()>PATH_CL) mDrawElements.pop_back();
  // state shape
  pGeneratorConfig->AddStatePaths(mDrawElements, mBasePoints[POINT_C], mBasePoints[POINT_L], pRenderOptions);
  // core shape
  mCoreShape=mDrawElements.at(PATH_CL).mPath;
  // fix initial tip
  pGeneratorConfig->FixArrow(mBasePoints[POINT_B],mBasePoints[POINT_C],mCoreShape);
  mCtrlPoints[CTRL_B]=mBasePoints[POINT_B];
  // fix transtion related tips  
  foreach(faudes::Transition ftrans, mTargetTransitions) {
    if(GioTrans* trans=TransItem(ftrans))
      trans->updateData();
  }
  // initial tip
  pGeneratorConfig->AddInitPath(mDrawElements, mBasePoints[POINT_A], mBasePoints[POINT_B], pRenderOptions);
  // base class (sets outerRect plus ctrlpoint margin
  GioItem::updatePaths();
  mCoreShape=mDrawElements.at(PATH_CL).mPath;
  mOuterRect.adjust(-10,-10,10,10); //todo: style (init arrow)
  // shall we draw name
  mPaintName = mCoreShape.contains(mNameRect);
}

// construct text and flags related draw paths (that may affect base)
// MUST be flowed by subsequent call to mUpdatePaths
void GioState::updateText(void) {
  FD_DQ("GioState::updateText():" << FName());
  // get flags
  pGeneratorConfig->MapElementOptions(GeneratorModel(), VioElement::FromState(mIdx), pRenderOptions);
  pRenderOptions->mBodyBrush.setColor(
    pRenderOptions->mBodyBrush.color().light(300));
  // clear all
  mDrawElements.clear();
  // draw name and set name rect
  pGeneratorConfig->AddStateName(mDrawElements, mBasePoints[POINT_C], mBasePoints[POINT_L],
   pGeneratorConfig->DispStateName(Generator(),mIdx),pRenderOptions); 
  mNameRect=mDrawElements[PATH_NAME].mPath.controlPointRect();
}

// construct control points from updated base points
void GioState::updateCtrls(void) {
  FD_DQ("GioState::updateCtrls " << FName());
  QPointF diag=mBasePoints[POINT_L]-mBasePoints[POINT_C];
  mCtrlPoints[CTRL_C1]=mBasePoints[POINT_C]+
    QPointF(+diag.x(),+diag.y());
  mCtrlPoints[CTRL_C2]=mBasePoints[POINT_C]+
    QPointF(-diag.x(),+diag.y());
  mCtrlPoints[CTRL_C3]=mBasePoints[POINT_C]+
    QPointF(-diag.x(),-diag.y());
  mCtrlPoints[CTRL_C4]=mBasePoints[POINT_C]+
    QPointF(+diag.x(),-diag.y());
  mCtrlPoints[CTRL_A]=mBasePoints[POINT_A];
  mCtrlPoints[CTRL_B]=mBasePoints[POINT_B];
}


// construct base points from updated controls
void GioState::updateBase(void) {
  FD_DQ("GioState::updateBase "  << FName());
  // bounding box 
  mBasePoints[POINT_C]=0.5*(mCtrlPoints[CTRL_C1]+mCtrlPoints[CTRL_C3]);
  mBasePoints[POINT_L].rx()=fabs( mCtrlPoints[CTRL_C1].x()-mBasePoints[POINT_C].x() );
  mBasePoints[POINT_L].ry()=fabs( mCtrlPoints[CTRL_C1].y()-mBasePoints[POINT_C].y() );
  mBasePoints[POINT_L]+=mBasePoints[POINT_C];
  // initial arrow  
  mBasePoints[POINT_A]=mCtrlPoints[CTRL_A];
  mBasePoints[POINT_B]=mCtrlPoints[CTRL_B];
}


// start move  
void GioState::moveCtrlStart(int ctrl) {
  FD_DQ("GioState::moveCtrlStart " << ctrl);
  // call base
  GioItem::moveCtrlStart(ctrl);
  // figure related transitions
  setTargetTransitions();
  // multiedit: let others know their transitions, too
  foreach(QGraphicsItem* item, scene()->selectedItems()) {
    GioState* state=dynamic_cast<GioState*>(item);
    if(!state) continue;
    if(state==this) continue;
    state->setTargetTransitions();
  }
}
 
// move one control 
bool GioState::moveCtrlPos(const QPointF& point) {
  FD_DQ("GioState::moveCtrlPos " << mCtrlPointEdit);
  // call base
  if(!GioItem::moveCtrlPos(point)) return false;
  // initial arrow
  if(mCtrlPointEdit==CTRL_A) { 
    mBasePoints[POINT_A]=mCtrlPoints[CTRL_A];
  }
  if(mCtrlPointEdit==CTRL_B) { 
    mBasePoints[POINT_B]=mCtrlPoints[CTRL_B];
  }
  // bounding box 
  if(mCtrlPointEdit>= CTRL_C1 && mCtrlPointEdit <= CTRL_C4) {
    mBasePoints[POINT_L].rx()=fabs( mCtrlPoints[mCtrlPointEdit].x()-mBasePoints[POINT_C].x() );
    mBasePoints[POINT_L].ry()=fabs( mCtrlPoints[mCtrlPointEdit].y()-mBasePoints[POINT_C].y() );
    mBasePoints[POINT_L]+=mBasePoints[POINT_C];
  }
  // other controls follow
  updateCtrls();
  // multi edit for shape
  if(mCtrlPointEdit>= CTRL_C1 && mCtrlPointEdit <= CTRL_C4) {
    foreach(QGraphicsItem* item, scene()->selectedItems()) {
      GioState* state=dynamic_cast<GioState*>(item);
      if(!state) continue;
      if(state==this) continue;
      state->mBasePoints[POINT_L]=mBasePoints[POINT_L];
      state->updateCtrls();
      state->updateFlag();
      state->updateDoit();
    }
  }
  return true;
}


// edit geometry by pointL
void GioState::moveL(const QPointF& point) {
  mBasePoints[POINT_L]=mapFromScene(point);
  updateCtrls();
}

// edit geometry by pointL
void GioState::moveA(const QPointF& point) {
  mBasePoints[POINT_A]=mapFromScene(point);
  updateCtrls();
  updatePaths();
}


// edit by retrieving shape from other state
void GioState::moveS(const Data& data) {
  FD_DQG("GioState::moveS(): src #" << data.mBasePoints.size());
  // bail out
  if(data.mBasePoints.size()!=POINTS) return;
  // apply to base
  mBasePoints=data.mBasePoints;
  // copy to ctrls and fix drawing
  setTargetTransitions();
  updateCtrls();
  updateFlag();
  updateDoit();
  FD_DQG("GioState::moveS(): done");
}





/*

// editing user data, need to call my update*()
void GioState::editdisp(bool confirm) {
  GioItem::editdisp(confirm);
  if(confirm) {
    updateCtrls();
    updatePath();
  }
}

*/








