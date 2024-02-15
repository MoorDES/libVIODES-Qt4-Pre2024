/* gioscene.cpp  - faudes generator as qgraphicsscene */


/*
   Graphical  IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006, 2007  Thomas Moor, Klaus Schmidt, Sebastian Perk

*/


//#define FAUDES_DEBUG_VIO_GENERATOR


#include <QtGui>

#include <stdlib.h>
#include <stdio.h>
#include <iostream>

using namespace std;

#include "gioscene.h"

#include "viogenerator.h"
#include "piotseview.h"





// constructor
GioScene::GioScene(VioGeneratorGraphModel* gmodel) : GioSceneRo(gmodel) {
  FD_DQG("GioScene::GioScene(): ok");
  mTool=SelectTool;
  mInsTrans = new GioTrans(pGeneratorModel);
  mMoveSelection=false;
  mMovingSelection=false;
  mGridVisible=false;
};


// destructor
GioScene::~GioScene(void) {
  FD_DQG("GioScene::~GioScene()");
  delete mInsTrans;
}

// clear all
int GioScene::Clear(void) {
  FD_DQG("GioScene::Clear()");
  if(mInsTrans->scene()==this) removeItem(mInsTrans);
  GioSceneRo::Clear();
  clearSelection();
  fixSelection();
  mMoveSelection=false;
  mMovingSelection=false;
  setTool(SelectTool);
  return 0;
}


// set tool/mouse mode
void GioScene::setTool(Tool tool) {
  FD_DQG("GioScene::setTool("<< tool << ")");
  // cancel pending transitions
  if(tool == SelectTool) userInsTransCancel();
  // sense change
  if(mTool!=tool){
    // clear nontrivial selections
    if(selectedItems().size()!=1){
      userSelectNothing();
    }
  }
  // set tool
  mTool=tool;
  // hack: indicate tool to user vio cursor
  // todo: sort out scene-view-widget hierarchy
  foreach(QGraphicsView* gview, views()) {
    if(mTool==SelectTool) 
      gview->setCursor(Qt::ArrowCursor);
    if(mTool==InsertOnceTool || mTool==InsertHoldTool) 
      gview->setCursor(VioStyle::CursorCross());
  }
}

// static trans ins mode
GioTrans::EditMode GioScene::mTransInsMode=GioTrans::Smooth;
void GioScene::TransInsertMode(GioTrans::EditMode mode) {
  mTransInsMode=mode;}
GioTrans::EditMode GioScene::TransInsertMode(void) {
  return mTransInsMode;}


// get selection as data 
/*
int GioScene::GetSelection(GioScene::Data& giodata) {
  FD_DQG("GioScene::GetSelection()");
  giodata.clear();
  fixSelection();
  foreach(GioState* state, mSelectedStates) {
    giodata.mStateItemsData.append(state->data());
  }
  foreach(GioTrans* trans, mSelectedTransFAB) {
    giodata.mTransItemsData.append(trans->data());
  }
  return 0;
}
*/

// insert selection
/*
int GioScene::InsSelection(const Data& gdata) {
  FD_DQG("GioScene::InsSelection() for scene " << this);
  // todo: find proper offset/center of mass
  QPointF offset(20,20);
  offset=VioStyle::GridPoint(offset);
  // insert states
  for(int i=0; i<gdata.mStateItemsData.size(); i++) {
    if(!generator()->ExistsState(gdata.mStateItemsData[i].mIdx)) continue;
    GioState* state = new GioState(generator());
    state->setData(gdata.mStateItemsData[i]);
    state->setPos(state->pos()+offset);
    addGioState(state);
  }
  // insert transitions
  for(int i=0; i<gdata.mTransItemsData.size(); i++) {
    faudes::Transition ftrans;
    ftrans.X1=gdata.mTransItemsData[i].mIdxA;
    ftrans.Ev=generator()->EventIndex(gdata.mTransItemsData[i].mNameEv);
    ftrans.X2=gdata.mTransItemsData[i].mIdxB;
    if(!generator()->ExistsTransition(ftrans)) continue;
    GioTrans* trans = new GioTrans(generator());
    trans->setData(gdata.mTransItemsData[i]);
    trans->setPos(trans->pos()+offset);
    addGioTrans(trans);
  }
  // fix it
  fixSelection();
  setChanged(true);
  int res=testConsistent();
  setConsistent(res==0);
  return res; 
}
*/


// remove state item
void GioScene::removeGioState(GioState* state) {
  FD_DQG("GioScene::removeGioState");
  userSelectNothing();
  GioSceneRo::removeGioState(state);
}

// remove state item
void GioScene::removeGioState(faudes::Idx index) {
  FD_DQG("GioScene::removeGioState");
  userSelectNothing();
  GioSceneRo::removeGioState(index);
}

// remove transition item
void GioScene::removeGioTrans(GioTrans* trans) {
  FD_DQG("GioScene::removeGioTrans");
  userSelectNothing();
  GioSceneRo::removeGioTrans(trans);
}

// remove transition item
void GioScene::removeGioTrans(const faudes::Transition& ftrans) {
  FD_DQG("GioScene::removeGioTrans");
  userSelectNothing();
  GioSceneRo::removeGioTrans(ftrans);
}


// select transition
void GioScene::userSelectTrans(GioTrans* trans, bool on) {
  FD_DQG("GioScene::userSelectTrans(): " << Generator()->TStr(trans->FTrans()));
  pGeneratorModel->Select(VioElement::FromTrans(trans->FTrans()), on);
}

// select state
void GioScene::userSelectState(GioState* state, bool on) {
  FD_DQG("GioScene::userSelectState()");
  pGeneratorModel->Select(VioElement::FromState(state->Idx()),on);
}

// select nothing
void GioScene::userSelectNothing() {
  FD_DQG("GioScene::userSelectNothing()");
  pGeneratorModel->SelectionClear();
}


// sync any user selection interaction
// note: either transitions or states, prefer transitions
void GioScene::userSelectionUpdate(void) {
  FD_DQG("GioScene::userSelectionUpdate()");
  // bail out on pending transition
  if(userInsTrans()) return;
  // sense "transitions-select-mode" in scene
  bool seltrans=false;
  foreach(QGraphicsItem* item, selectedItems()) 
    if(qgraphicsitem_cast<GioTrans *>(item)) seltrans=true; 
  // have copy of selection
  QList<VioElement> sel;
  if(!seltrans)
    foreach(QGraphicsItem* item, selectedItems()) 
      if(GioState* state=qgraphicsitem_cast<GioState *>(item)) 
        sel.append(VioElement::FromState(state->Idx()));
  if(seltrans) 
    foreach(QGraphicsItem* item, selectedItems()) 
      if(GioTrans* trans=qgraphicsitem_cast<GioTrans *>(item)) 
        sel.append(VioElement::FromTrans(trans->FTrans()));
  // avoid re-select if nothing changed
  qSort(sel);
  if(sel==pGeneratorModel->Selection()) return;
  // clear selection
  pGeneratorModel->SelectionClear();
  // select by copy (uses call back to report to scene)
  foreach(VioElement elem, sel) 
    pGeneratorModel->Select(elem);
  FD_DQG("GioScene::userSelectionUpdate(): done"); 
}


// selection update: nothing
void GioScene::UpdateSelectionClear(void) {
  FD_DQG("GioScene::UpdateSelectionClear");
  clearSelection();
  fixSelection();
}

// selection update: element
void GioScene::UpdateSelectionElement(const VioElement& elem, bool on) {
  FD_DQG("GioScene::UpdateSelectionElement(): " << elem.Str() << " to " << on);
  // switch type
  switch(elem.Type()) {
  case VioElement::ETrans: {
    GioTrans* trans= TransItem(elem.Trans());
    if(!trans) break;
    if(trans->isSelected()==on) break;
    trans->setSelected(on);
    fixSelection();
    break;
  }
  case VioElement::EState: {
    GioState* state= StateItem(elem.State());
    if(!state) break;
    if(state->isSelected()==on) break;
    state->setSelected(on);
    fixSelection();
    break;
  }
  default: break;
  }
}


// selection update: all
void GioScene::UpdateSelectionAny(void) {
  FD_DQG("GioScene::UpdateSelectionAny()");
  // clear internal version
  clearSelection();
  // loop over model selection
  foreach(VioElement elem,pGeneratorModel->Selection()) {
    // switch type
    switch(elem.Type()) {
    case VioElement::ETrans: {
      GioTrans* trans= TransItem(elem.Trans());
      if(!trans) break;
      trans->setSelected(true);
      break;
    }
    case VioElement::EState: {
      GioState* state= StateItem(elem.State());
      if(!state) break;
      state->setSelected(true);
      break;
    }
    default: break;
    }
  }
  // fix internal selection
  fixSelection();
}


// have a private version of what is selected in preparation of an
// an upcomming move selection action. thus, avoid loose ends and
// track transitions that are linked to a selected state; also keep 
// track of current state positions  
void GioScene::fixSelection() {
  FD_DQG("GioScene::fixSelection()");
  // (0) bail out on pensing  transition
  if(userInsTrans()) return;
  // part (a): copy actual selection
  mSelectedStates.clear();
  mSelectedTrans.clear();
  foreach(QGraphicsItem* item, selectedItems()) {
    if(GioState* state=qgraphicsitem_cast<GioState *>(item)) 
      mSelectedStates.append(state);
    if(GioTrans* trans=qgraphicsitem_cast<GioTrans *>(item)) 
      mSelectedTrans.append(trans);
  }
  // part (x): set up states to follow
  mSelectedStatesF.clear();
  foreach(GioTrans* trans, mSelectedTrans){
    if(GioState* state=StateItem(trans->IdxA())) 
      if(!mSelectedStates.contains(state))
        mSelectedStatesF.append(state);
    if(GioState* state=StateItem(trans->IdxB())) 
      if(!mSelectedStates.contains(state))
        mSelectedStatesF.append(state);
  }
  // part (x): set up transitions to follow
  mSelectedTransFAB.clear();
  mSelectedTransFA.clear();
  mSelectedTransFB.clear();
  foreach(GioTrans* trans, mTransItems){
    if(mSelectedTrans.contains(trans)) continue;
    faudes::Idx idxA=trans->IdxA();
    faudes::Idx idxB=trans->IdxB();
    bool selA=false;
    if(mSelectedStates.contains(StateItem(idxA))) 
      selA=true;
    if(mSelectedStatesF.contains(StateItem(idxA))) 
      selA=true;
    bool selB=false;
    if(mSelectedStates.contains(StateItem(idxB))) 
      selB=true;
    if(mSelectedStatesF.contains(StateItem(idxB))) 
      selB=true;
    if(selA && selB) 
      if(!mSelectedTransFAB.contains(trans))
        mSelectedTransFAB.append(trans);
    if(selA && !selB) 
      mSelectedTransFA.append(trans);
    if(!selA && selB) 
      mSelectedTransFB.append(trans);
  }
  FD_DQG("GioScene::fixSelection: #a " << mSelectedTransFA.size() << " #b " 
    << mSelectedTransFB.size() << " #ab " << mSelectedTransFAB.size());
  // part (d) record position
  mMovePosMap.clear();
  //foreach(GioState* state, mStateItems)
  foreach(GioState* state, mSelectedStates) {
    mMovePosMap[state]=VioStyle::GridPoint(state->pos());
  }
  foreach(GioState* state, mSelectedStatesF) {
    mMovePosMap[state]=VioStyle::GridPoint(state->pos());
  }
  // part (e): set zvalue of transitions 
  foreach(GioTrans* trans, mTransItems) {
    if(trans->isSelected())
      trans->setZValue(5);
    else
      trans->setZValue(-10); // fixme: style 
  }
  foreach(GioState* state, mStateItems) {
    if(state->isSelected())
      state->setZValue(0);
    else
      state->setZValue(-5); // fixme: style 
  }
}

// fix moving transition
bool GioScene::fixMove(const QPointF& diff) {
  QPointF gdiff=VioStyle::GridPoint(diff);
  if(gdiff==QPointF(0,0)) return false;
  // track relevant rectangle: start within scene
  QRectF rect=sceneRect();
  rect.adjust(+0.2*rect.width(),+0.2*rect.height(),
    -0.2*rect.width(),-0.2*rect.height());
  // move states
  FD_DQG("GioScene::fixMove: states on gridpoints");
  foreach(GioState* state, mSelectedStates) {
    state->setPos(VioStyle::GridPoint(mMovePosMap[state]+gdiff));
    rect |= state->sceneBoundingRect();
  }
  foreach(GioState* state, mSelectedStatesF) {
    state->setPos(VioStyle::GridPoint(mMovePosMap[state]+gdiff));
    rect |= state->sceneBoundingRect();
  }
  // move transitions
  FD_DQG("GioScene::fixMove: related trans");
  foreach(GioTrans* trans, mSelectedTrans) {
    trans->setPos(mStateMap[trans->IdxA()]->pos());
    trans->moveA(mStateMap[trans->IdxA()]->pos());
    trans->moveB(mStateMap[trans->IdxB()]->pos());
    rect |= trans->sceneBoundingRect();
  }
  foreach(GioTrans* trans, mSelectedTransFAB) {
    trans->setPos(mStateMap[trans->IdxA()]->pos());
    trans->moveA(mStateMap[trans->IdxA()]->pos());
    trans->moveB(mStateMap[trans->IdxB()]->pos());
    rect |= trans->sceneBoundingRect();
  }
  foreach(GioTrans* trans, mSelectedTransFA) {
    trans->moveA(mStateMap[trans->IdxA()]->pos());
    rect |= trans->sceneBoundingRect();
  }
  foreach(GioTrans* trans, mSelectedTransFB) {
    trans->moveB(mStateMap[trans->IdxB()]->pos());
    rect |= trans->sceneBoundingRect();
  }
  // enlarge scene to 1% extra
  rect.adjust(-0.01*rect.width(),-0.01*rect.height(),
    0.01*rect.width(),0.01*rect.height());
  if(!sceneRect().contains(rect)) {
    FD_DQG("GioScene::fixMove: enlarge scene");
    rect |= sceneRect();
    setSceneRect(rect);
  }
  return true;
}


// handle my events: mouse press
// note: qt selects on release, we prefer select on press
void GioScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
  FD_DQG("GioScene::mousePress(...): #" << selectedItems().size()); 

  // emit api signal by base class
  GioSceneRo::mousePressEvent(event);
  event->ignore();
  
  // where are we
  QGraphicsItem* item = itemAt(event->scenePos());
  int selsize=selectedItems().size();

  // fix: dont interpret clicks on pending transition
  if(item==mInsTrans) {
    mInsTrans->setZValue(-10);
    item = itemAt(event->scenePos());
    mInsTrans->setZValue(5);
  }
 
  // ********************* distinguish my tools
  switch(mTool) {

  // ********************* select and move items
  case SelectTool:

  FD_DQG("GioScene::mousePress(...): SelectTool"); 

  // care about my selection
  mMoveSelection=false;
  mMovingSelection=false;
  // hit item that is selected: upcomming state edit
  if(item) 
  // if(selsize==1)  // comment out to alow for multi-edit
  if(item->isSelected()) 
  if(GioState* state=qgraphicsitem_cast<GioState *>(item)) 
  if(state->isCtrl(event->scenePos())) {
    FD_DQG("GioScene::mousePressEvent(...): pass to scene for state edit"); 
    QGraphicsScene::mousePressEvent(event);
    event->accept();
    break;
  }
  // hit item that is selected: upcomming trans edit, A
  if(item) 
  if(selsize==1)
  if(item->isSelected()) 
  if(GioTrans* trans=qgraphicsitem_cast<GioTrans *>(item)) 
  if(trans->isCtrl(event->scenePos())) {
    FD_DQG("GioScene::mousePressEvent(...): pass to scene for trans edit"); 
    QGraphicsScene::mousePressEvent(event);
    event->accept();
    break;
  }
  // hit item that is selected: upcomming trans edit, B
  if(item) 
  if(selsize==1)
  if(item->isSelected()) 
  if(event->button() == Qt::LeftButton)
  if(event->modifiers() & Qt::ShiftModifier) 
  if(qgraphicsitem_cast<GioTrans *>(item)) { 
    FD_DQG("GioScene::mousePressEvent(...): pass to scene for trans edit"); 
    QGraphicsScene::mousePressEvent(event);
    event->accept();
    break;
  }
  // hit item that is selected: upcomming item move
  if(item) 
  if(item->isSelected()) 
  if(event->button() == Qt::LeftButton)
  if(!(event->modifiers() & Qt::ControlModifier)) {
    mMoveSelection=true;
    fixSelection();
    event->accept();
    break;
  }
  // hit item that is not selected: select
  if(item) 
  if(!item->isSelected()) 
  if(event->button() == Qt::LeftButton) {
    if(!(event->modifiers() & Qt::ControlModifier))
      userSelectNothing();
    if(GioTrans* trans=qgraphicsitem_cast<GioTrans *>(item)) 
      userSelectTrans(trans);
    if(GioState* state=qgraphicsitem_cast<GioState *>(item)) 
      userSelectState(state);
    mMoveSelection=true;
    event->accept();
    break;
  }
  // hit item that is selected with ctrl: deselect
  if(item) 
  if(item->isSelected()) 
  if(event->button() == Qt::LeftButton)
  if(event->modifiers() & Qt::ControlModifier) {
    if(GioTrans* trans=qgraphicsitem_cast<GioTrans *>(item)) 
      userSelectTrans(trans,false);
    if(GioState* state=qgraphicsitem_cast<GioState *>(item)) 
      userSelectState(state,false);
    fixSelection();
    event->accept();
    break;
  }
  // no item: select nothing and start drag
  if(!item) 
  if(event->button() == Qt::LeftButton) {
    userSelectNothing();
    //event->accept(); // note: drag cannot have accept here
    break;
  }
  // no item with ctrl mod: ignore
  if(!item) 
  if(event->button() == Qt::LeftButton)
  if(event->modifiers() & Qt::ControlModifier) {
    event->accept();
    break;
  }
  // right button: done by context menu, no accept here
  if(event->button() == Qt::RightButton) {  
    break;
  }
  // anything else: pass on to scene
  QGraphicsScene::mousePressEvent(event);
  fixSelection();
  break;

  // ********************* insert items
  case InsertOnceTool:
  case InsertHoldTool:

  FD_DQG("GioScene::mousePress(...): InsertOnceTool/InsertHoldTool with \"pening trans\"=" << userInsTrans()); 

  // no item, no selected state, no pending transition: insert state 
  if(!item) 
  if(!userInsTrans()) {
    userInsState(event->scenePos());
    userSelectNothing();
    if(event->modifiers() & Qt::ShiftModifier) 
      setTool(InsertHoldTool);
    else
      setTool(SelectTool);
    event->accept();
    break;
  }
  // no item, one selected state, no pending transition: insert state and transition
  /*
  if(!item) 
  if(!userInsTrans())  
  if(selsize==1)
  if(GioState* stateA = qgraphicsitem_cast<GioState *>(selectedItems()[0])) {
    GioState* stateB= userInsState(event->scenePos());
    if(stateA && stateB) userInsTrans(stateA->Idx(),stateB->Idx());
    if(! (event->modifiers() & Qt::ShiftModifier))     
      setTool(SelectTool);
    event->accept();
    break;
  }
  */
  // no state item, but pending transition: insert control
  if(!qgraphicsitem_cast<GioState *>(item))  
  if(userInsTrans()) { 
    userInsTransCtrl(event->scenePos());
    if(event->modifiers() & Qt::ShiftModifier) 
      setTool(InsertHoldTool);
    else
      setTool(InsertOnceTool);
    event->accept();
    break;
  }
  // state item, and pending transition: commit
  if(userInsTrans()) 
  if(GioState* state = qgraphicsitem_cast<GioState *>(item))  {
    userInsTransEnd(state,event->scenePos());
    userSelectNothing();
    if(event->modifiers() & Qt::ShiftModifier) 
      setTool(InsertHoldTool);
    else
      setTool(SelectTool);
    event->accept();
    break;
  }
  // no item, some selection: re-select 
  if(!item) 
  if(selsize>0) {
    userSelectNothing();
    event->accept();
    break;
  }
  // hit first state: have pending trasition
  if(item) 
  if(GioState* state=qgraphicsitem_cast<GioState *>(item)) {
    userSelectNothing();
    userInsTransBegin(state,event->scenePos());
    if(event->modifiers() & Qt::ShiftModifier) 
      setTool(InsertHoldTool);
    else
      setTool(InsertOnceTool);
    event->accept();
    break;
  }
  // hit second state: transition insert
  /*
  if(item) 
  if(selsize==1)
  if(GioState*  stateA = qgraphicsitem_cast<GioState *>(selectedItems()[0])) 
  if(GioState* stateB=qgraphicsitem_cast<GioState *>(item)) { 
    userInsTrans(stateA->Idx(),stateB->Idx());
    userSelectNothing();
    if(! (event->modifiers() & Qt::ShiftModifier))     
      setTool(SelectTool);
    event->accept();
    break;
  }
  */
  // hit a not yet selected  transition: allow for selection (convenience for delete)
  if(item) 
  if(!item->isSelected())
  if(GioTrans* trans = qgraphicsitem_cast<GioTrans *>(item)) {
    userSelectNothing();
    userSelectTrans(trans);
    event->accept();
    break;
  }

  // hit a selected transition: upcomming trans edit, type B, end insert tool
  if(item) 
  if(selsize==1)
  if(item->isSelected()) 
  if(event->button() == Qt::LeftButton)
  if(qgraphicsitem_cast<GioTrans *>(item)) { 
    Qt::KeyboardModifiers orig=event->modifiers(); 
    FD_DQG("GioScene::mousePressEvent(...): pass to scene for trans edit"); 
    event->setModifiers(Qt::ShiftModifier);  
    QGraphicsScene::mousePressEvent(event);
    event->accept();
    event->setModifiers(orig);  
    if(orig & Qt::ShiftModifier) 
      setTool(InsertHoldTool);
    else
      setTool(InsertOnceTool);
    break;
  }

  // formally accept to effectively ignore
  event->accept();
  break;


  // ********************* no such tool 
  default:
  // formally accept to effectively ignore
  event->accept();
  break;
  };

  FD_DQG("GioScene::mousePress(...): done"); 
}

// handle my events: mouse release
void GioScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
  FD_DQG("GioScene::mouseReleaseEvent(...)"); 
  // move selection: set on grid
  if(mMoveSelection) {
    FD_DQG("GioScene::mouseReleaseEvent(...): done moving"); 
    QPointF diff = event->scenePos()-event->buttonDownScenePos(Qt::LeftButton);
    bool moved= fixMove(diff);
    mMoveSelection=false;
    mMovingSelection=false;
    pGeneratorModel->UndoEditStop();
    if(moved) Modified(true);
    AdjustScene();
  }
  // anything else: trust in scene to dispatch
  FD_DQG("GioScene::mouseResleaseEvent(...): pass to scene"); 
  QGraphicsScene::mouseReleaseEvent(event);
  // might have been a select on release (on dragging?)
  fixSelection();
  FD_DQG("GioScene::mouseResleaseEvent(...): done"); 
  // if it was a insert (except pending transition control), release changes tool
  if(mTool==InsertOnceTool) 
  if(!userInsTrans()) 
    setTool(SelectTool);
}

// handle my events: mouse move
void GioScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
  //FD_DQG("GioScene::mouseMoveEvent(...)"); 
  // track moving selection
  if(mMoveSelection && !mMovingSelection) {
    FD_DQG("GioScene::mouseMoveEvent(...): start moving selection"); 
    Modified(true);
    pGeneratorModel->UndoEditStart();
    mMovingSelection=true;
  }
  if(mMoveSelection) {
    FD_DQG("GioScene::mouseMoveEvent(...): now moving selection"); 
    QPointF diff = event->scenePos()-event->buttonDownScenePos(Qt::LeftButton);
    fixMove(diff);
    event->accept();
    return;
  }
  // track insert transition
  if(userInsTrans()) {
    userInsTransUpdate(event->scenePos());
    //return;
  }
  // anything else: trust in scene to dispatch to items
  QGraphicsScene::mouseMoveEvent(event);
  //FD_DQG("GioScene::mouseMoveEvent(...): done"); 
}

// handle my events: key
void GioScene::keyPressEvent(QKeyEvent *keyEvent) {
  FD_DQG("GioScene::keyPressEvent(...)"); 
  // delete selection
  if(keyEvent->key()==Qt::Key_Backspace || keyEvent->key()==Qt::Key_Delete) {
    userDelSelection();
    keyEvent->accept();
  }
  // testing features
  if(keyEvent->key()==Qt::Key_A) {
    FD_DQG("GioScene::keyPressEvent: Testing Features a"); 
    //MarkUndoPoint();
    FD_DQG("GioScene::keyPressEvent: Testing Features done"); 
  }  
  // tool select: std insert
  if(keyEvent->key()==Qt::Key_Shift) 
  if(!(keyEvent->modifiers() & Qt::ControlModifier)) {
    if(mTool==SelectTool) setTool(InsertOnceTool);
    else setTool(SelectTool);
  }
  // tool select: std quit insert
  if(keyEvent->key()==Qt::Key_Escape) {
    setTool(SelectTool);
  }
  // copy shape
  if(keyEvent->key()==Qt::Key_C) 
  if(keyEvent->modifiers()== (Qt::ShiftModifier | Qt::ControlModifier)) {
    userCopyTransShape();
    userCopyStateShape();
  }
  // paste shape
  if(keyEvent->key()==Qt::Key_V) 
  if(keyEvent->modifiers()== (Qt::ShiftModifier | Qt::ControlModifier)) {
    if(testPasteTrans()) userPasteTransShape();
    if(testPasteState()) userPasteStateShape();
  }
  // pass on
  QGraphicsScene::keyPressEvent(keyEvent);
}


// handle my events: key release
void GioScene::keyReleaseEvent(QKeyEvent *keyEvent) {
  FD_DQG("GioWidget::keyReleaseEvent(...)"); 
  // tool select
  if(keyEvent->key()==Qt::Key_Shift) {
    if(mTool==InsertHoldTool) setTool(SelectTool);
  }
  // pass on to parent
  QGraphicsScene::keyReleaseEvent(keyEvent);
}
   


// handle my events: contex menu
void GioScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
  QGraphicsItem* item = itemAt(event->scenePos());
  // scene context
  if(!item) {
    userSelectNothing();
    sceneContextMenu(event->screenPos());
    return;
  }
  // state contex
  if(GioState* state= qgraphicsitem_cast<GioState *>(item)) {
    if(!item->isSelected())
      userSelectNothing();
    stateContextMenu(event->screenPos(), state);
    return;
  }
  // trans contex
  if(GioTrans* trans= qgraphicsitem_cast<GioTrans *>(item)) {
    if(!item->isSelected())
      userSelectNothing();
    transContextMenu(event->screenPos(), trans);
    return;
  }
}

// edit transition data via context menu
void GioScene::transContextMenu(const QPoint& screenpos, GioTrans* trans) {
  FD_DQG("GioScene::transContextMenu");
  QMenu* menu = new QMenu("Transition");
  // fix selection
  userSelectTrans(trans);
  // setup property widget
  PioTProp* transwidget = new PioTProp(menu,pGeneratorConfig);
  transwidget->GeneratorModel(pGeneratorModel);
  transwidget->ShowSelection();
  QWidgetAction* transaction= new QWidgetAction(menu);
  transaction->setDefaultWidget(transwidget);
  menu->addAction(transaction);
  // add my buttons: curve
  menu->addSeparator();
  QAction* freeaction= menu->addAction("Free Bezier Segments");
  freeaction->setCheckable(true);
  freeaction->setChecked(trans->editMode()==GioTrans::Free);
  QAction* smoothaction= menu->addAction("Smooth Bezier Curve");
  smoothaction->setCheckable(true);
  smoothaction->setChecked(trans->editMode()==GioTrans::Smooth);
#ifdef USING_SPLINE
  QAction* splineaction= menu->addAction("Spline Interpolation");
  splineaction->setCheckable(true);
  splineaction->setChecked(trans->editMode()==GioTrans::Spline);
#endif 
  QAction* polygonaction= menu->addAction("Polygon");
  polygonaction->setCheckable(true);
  polygonaction->setChecked(trans->editMode()==GioTrans::Polygon);
  QAction* lineaction= menu->addAction("Straight Line");
  lineaction->setCheckable(true);
  lineaction->setChecked(trans->editMode()==GioTrans::Line);
  QAction* muteaction= menu->addAction("Invisible");
  muteaction->setCheckable(true);
  muteaction->setChecked(trans->editMode()==GioTrans::Mute);
  // add my buttons: copy apply
  menu->addSeparator();
  QAction* copyaction= menu->addAction("Copy Curve");
  copyaction->setEnabled(mSelectedTrans.size()==1);
  copyaction->setShortcut(tr("Ctrl+Shift+C"));
  QAction* pasteaction= menu->addAction("Paste Curve");
  pasteaction->setEnabled(testPasteTrans());
  pasteaction->setShortcut(tr("Ctrl+Shift+V"));
  // add my buttons: delete
  menu->addSeparator();
  QAction* deleteaction;
  if(mSelectedTrans.size()==1)
    deleteaction=menu->addAction("Delete Transition");
  else
    deleteaction=menu->addAction("Delete Transitions");
  // run menu
  FD_DQG("GioScene::transContextMenu: run");
  QAction *selaction = menu->exec(screenpos);
  FD_DQG("GioScene::transContextMenu: evaluate");
  // delete
  if(selaction==deleteaction){
    userDelSelection();
    trans=NULL;
  }
  // copy/paste
  if(selaction==copyaction){
    userCopyTransShape();
  }
  if(selaction==pasteaction){
    userPasteTransShape();
  }
  // converters
  if(selaction==lineaction)
  if(selaction->isChecked()) {
    pGeneratorModel->UndoEditStart();
    Modified(true);
    foreach(GioTrans* trans, mSelectedTrans) 
     trans->setEditMode(GioTrans::Line);
    pGeneratorModel->UndoEditStop();
  }
  if(selaction==polygonaction)
  if(selaction->isChecked()) {
    pGeneratorModel->UndoEditStart();
    Modified(true);
    foreach(GioTrans* trans, mSelectedTrans) 
      trans->setEditMode(GioTrans::Polygon);
    pGeneratorModel->UndoEditStop();
  }
  if(selaction==smoothaction)
  if(selaction->isChecked()) {
    pGeneratorModel->UndoEditStart();
    Modified(true);
    foreach(GioTrans* trans, mSelectedTrans) 
      trans->setEditMode(GioTrans::Smooth);
    pGeneratorModel->UndoEditStop();
  }
#ifdef USING_SPLINE
  if(selaction==splineaction)
  if(selaction->isChecked()) {
    pGeneratorModel->UndoEditStart();
    Modified(true);
    foreach(GioTrans* trans, mSelectedTrans) 
      trans->setEditMode(GioTrans::Spline);
    pGeneratorModel->UndoEditStop();
  }
#endif
  if(selaction==freeaction)
  if(selaction->isChecked()) {
    pGeneratorModel->UndoEditStart();
    Modified(true);
    foreach(GioTrans* trans, mSelectedTrans) 
      trans->setEditMode(GioTrans::Free);
    pGeneratorModel->UndoEditStop();
  }
  if(selaction==muteaction)
  if(selaction->isChecked()) {
    pGeneratorModel->UndoEditStart();
    Modified(true);
    foreach(GioTrans* trans, mSelectedTrans) 
      trans->setEditMode(GioTrans::Mute);
    pGeneratorModel->UndoEditStop();
  }
  // done
  delete menu;
  FD_DQG("GioScene::transContextMenu: done ");
}


// get/set grid
bool GioScene::GridVisible(void) const {
  return mGridVisible;
}
void GioScene::GridVisible(bool on) {
  if(mGridVisible==on) return;
  mGridVisible=on;
  invalidate(QRectF(),QGraphicsScene::BackgroundLayer);
}


// reimplement background
void GioScene::drawBackground( QPainter * painter, const QRectF & rect) {
  // give base a try
  QGraphicsScene::drawBackground(painter,rect);
  if(!mGridVisible) return;
  // add by grid
  painter->save();
  qreal gw=pGeneratorConfig->GridWidth();
  QPointF xgw=  QPointF(gw,0);
  QPointF ygw=  QPointF(0,gw);
  // vertical, thin
  QPointF it1 = rect.topLeft() - xgw - ygw;
  QPointF it2 = rect.bottomLeft() - xgw + ygw;
  painter->setPen(pGeneratorConfig->GridNPen());
  while(it1.x()<= rect.right()) {
    it1= pGeneratorConfig->GridPoint(it1);
    it2= pGeneratorConfig->GridPoint(it2);
    painter->drawLine(it1,it2);
    it1+=xgw;
    it2+=xgw;
  }
  // horizontal, thin
  it1 = rect.topLeft()  - xgw - ygw;
  it2 = rect.topRight() + xgw - ygw;
  painter->setPen(pGeneratorConfig->GridNPen());
  while(it1.y()<= rect.bottom()) {
    it1= pGeneratorConfig->GridPoint(it1);
    it2= pGeneratorConfig->GridPoint(it2);
    painter->drawLine(it1,it2);
    it1+=ygw;
    it2+=ygw;
  }
  // vertical, bold
  it1 = rect.topLeft() - xgw - ygw;
  it2 = rect.bottomLeft() - xgw + ygw;
  painter->setPen(pGeneratorConfig->GridBPen());
  while(it1.x()<= rect.right()) {
    it1= pGeneratorConfig->GridPoint(it1);
    it2= pGeneratorConfig->GridPoint(it2);
    if(qAbs ( qRound(it1.x() / (6*gw) ) * (6*gw) - it1.x()) < 0.5*gw ) {
      painter->drawLine(it1,it2);
      it1+=6*xgw;
      it2+=6*xgw;
    } else {
      it1+= xgw;
      it2+= xgw;
    }
  }
  // horizontal, bold
  it1 = rect.topLeft()  - xgw - ygw;
  it2 = rect.topRight() + xgw - ygw;
  painter->setPen(pGeneratorConfig->GridBPen());
  while(it1.y()<= rect.bottom()) {
    it1= pGeneratorConfig->GridPoint(it1);
    it2= pGeneratorConfig->GridPoint(it2);
    if(  qAbs ( qRound(it1.y() / (6*gw) ) * (6*gw) - it1.y()) < 0.5*gw ) {
      painter->drawLine(it1,it2);
      it1+=6*ygw;
      it2+=6*ygw;
    } else {
      it1+=ygw;
      it2+=ygw;
    }
  }
  painter->restore();
};


// reimplement forground
void GioScene::drawForeground(QPainter * painter, const QRectF & rect) {
  FD_DQG("GioScene::drawForeground: 1 ");
  // let base do the job
  if(!mGridVisible || !mCtrlEditing) {
    QGraphicsScene::drawForeground(painter,rect);
    return;
  }
  FD_DQG("GioScene::drawForeground: at  " << mCtrlPosition.x() << " " << mCtrlPosition.y());
  // draw the lines
  painter->save();
  painter->setPen(pGeneratorConfig->GridXPen());
  QPointF p1,p2,cp;
  cp=pGeneratorConfig->GridPoint(mCtrlPosition);
  p1.setX(cp.x());
  p2.setX(cp.x());
  p1.setY(rect.top()-1);
  p2.setY(rect.bottom()+1);
  if(rect.contains(0.5*(p1+p2)))
    painter->drawLine(p1,p2);
  p1.setY(cp.y());
  p2.setY(cp.y());
  p1.setX(rect.left()-1);
  p2.setX(rect.right()+2);
  if(rect.contains(0.5*(p1+p2)))
    painter->drawLine(p1,p2);
  painter->restore();

};


// edit state data via context menu
void GioScene::stateContextMenu(const QPoint& screenpos, GioState* state) {
  FD_DQG("GioScene::stateContextMen(): #" << selectedItems().size() << " with state " << state->Idx());
  QMenu* menu = new QMenu("State");
  // fix selection
  userSelectState(state);
  // setup property widget
  PioSProp* statewidget = new PioSProp(menu,pGeneratorConfig);
  statewidget->GeneratorModel(pGeneratorModel);
  statewidget->ShowSelection();
  QWidgetAction* stateaction= new QWidgetAction(menu);
  stateaction->setDefaultWidget(statewidget);
  menu->addAction(stateaction);
  // add my buttons: copy apply
  menu->addSeparator();
  QAction* copyaction= menu->addAction("Copy Shape");
  copyaction->setEnabled(mSelectedStates.size()==1);
  copyaction->setShortcut(tr("Ctrl+Shift+C"));
  QAction* pasteaction= menu->addAction("Paste Shape");
  pasteaction->setEnabled(testPasteState());
  pasteaction->setShortcut(tr("Ctrl+Shift+V"));
  // add my buttons: delete state
  menu->addSeparator();
  QAction* deleteaction;
  if(mSelectedStates.size()==1)
    deleteaction=menu->addAction("Delete State");
  else
    deleteaction=menu->addAction("Delete States");
  // run menu
  QAction *selaction = menu->exec(screenpos);
  // copy/paste
  if(selaction==copyaction){
    userCopyStateShape();
  }
  if(selaction==pasteaction){
    userPasteStateShape();
  }
  // delete
  if(selaction==deleteaction){
    userDelSelection();
    state=NULL;
  }
  // done
  delete menu;
  FD_DQG("GioScene::stateContextMenu: done ");
}

// edit scene via context menu
void GioScene::sceneContextMenu(const QPoint& screenpos) {
  FD_DQG("GioScene::sceneContextMenu");
  userSelectNothing();
  // insert state
  if(selectedItems().size()==0) { 
    QMenu* menu = new QMenu("Generator");
    menu->addAction("Generator \"" + VioStyle::QStrFromStr(Generator()->Name()) + "\"");
    menu->addSeparator();
    QAction *insertToolAction = menu->addAction("Insert Transition/State");
    insertToolAction->setShortcut(Qt::ShiftModifier);
    menu->addSeparator();
    QAction *dogridAction = menu->addAction("Re-arrange Graph via Grid");
    QAction *dodotAction =  menu->addAction("Re-arrange Graph via Dot");
    QAction *dodottransAction =  menu->addAction("Re-arrange Transitions via Dot");
    menu->addSeparator();
    QAction *modesmoothAction =  menu->addAction("Ins. Mode: Smooth Curve");
    modesmoothAction->setCheckable(true);
    modesmoothAction->setChecked(TransInsertMode()==GioTrans::Smooth);
#ifdef USING_SPLINE
    QAction *modesplineAction =  menu->addAction("Ins. Mode: Spline Interpolation");
    modesplineAction->setCheckable(true);
    modesplineAction->setChecked(TransInsertMode()==GioTrans::Spline);
#endif
    QAction *modepolygonAction =  menu->addAction("Ins. Mode: Polygon");
    modepolygonAction->setCheckable(true);
    modepolygonAction->setChecked(TransInsertMode()==GioTrans::Polygon);

    QAction *selectedAction = menu->exec(screenpos);

    if(selectedAction==insertToolAction) {
       setTool(InsertOnceTool);
    }
    if(selectedAction==dodottransAction) {
       pGeneratorModel->UndoEditStart();
       Modified(true);
       DotConstruct(true);
       pGeneratorModel->UndoEditStop();
    }
    if(selectedAction==dodotAction) {
       pGeneratorModel->UndoEditStart();
       Modified(true);
       DotConstruct();
       pGeneratorModel->UndoEditStop();
    }
    if(selectedAction==dogridAction) {
       pGeneratorModel->UndoEditStart();
       Modified(true);
       GridConstruct();
       pGeneratorModel->UndoEditStop();
    }
    if(selectedAction==modesmoothAction)
    if(selectedAction->isChecked()) {
      TransInsertMode(GioTrans::Smooth);
    }
    if(selectedAction==modesplineAction)
    if(selectedAction->isChecked()) {
      TransInsertMode(GioTrans::Spline);
    }
    if(selectedAction==modepolygonAction)
    if(selectedAction->isChecked()) {
      TransInsertMode(GioTrans::Polygon);
    }



    delete menu;
    FD_DQG("GioScene::sceneContextMenu: done");
    return;
  }
}


// user insert state
GioState* GioScene::userInsState(const QPointF& pos) {
  FD_DQG("GioScene::userInsState(pos)");
  pGeneratorModel->UndoEditStart();
  Modified(true);
  // create a new state in generator (introduces giostate by notification!!)
  faudes::Idx index = (pGeneratorModel->ElementIns(VioElement::FromState(0))).State(); 
  // figure state
  GioState* state=StateItem(index);
  if(!state) {   
    pGeneratorModel->UndoEditCancel();
    return NULL;
  }
  // move item
  addGioState(index,pGeneratorConfig->GridPoint(pos)); 
  AdjustScene();
  userSelectNothing();
  pGeneratorModel->UndoEditStop();
  return state;
}

// user insert transition, do so
GioTrans* GioScene::userInsTrans(faudes::Idx idxA, faudes::Idx idxB) {
  FD_DQG("GioScene::userInsTrans(idxA=" << idxA << ",idxB=" << idxB <<")");
  pGeneratorModel->UndoEditStart();
  Modified(true);
  GioTrans* trans= addGioTrans(idxA,"", idxB);
  if(!trans) {
    pGeneratorModel->UndoEditCancel();
    return NULL;
  }
  AdjustScene();
  // pass on to other models (we ourselfs will ignore the callback)
  pGeneratorModel->ElementIns(VioElement::FromTrans(trans->FTrans()));
  userSelectNothing();
  userSelectState(StateItem(idxB));
  pGeneratorModel->UndoEditStop();
  return trans;
} 

// user insert transition, begin
void GioScene::userInsTransBegin(GioState* state, QPointF bpos) {
  FD_DQG("GioScene::userInsTransBegin()");
  pGeneratorModel->UndoEditStart();
  // configure pending trasition on scene
  mInsTrans->FTrans(faudes::Transition(state->Idx(),0,0));
  mInsTrans->hintAB(state->pos(),bpos);
  mInsTrans->setSelected(true);
  mInsTrans->setZValue(5);
  mInsTrans->updateData();
  mInsTrans->setEditMode(TransInsertMode());
  // activate pending transition
  if(!userInsTrans()) addItem(mInsTrans);
  // adjust
  AdjustScene();
}

// user insert transition, test
bool GioScene::userInsTrans(void) {
  return mInsTrans->scene()==this;
}

// user insert transition, track tip
void GioScene::userInsTransUpdate(QPointF bpos) {
  FD_DQG("GioScene::userInsTransUpdate()");
  // update view
  mInsTrans->moveT(bpos);
  mInsTrans->updateData();
  //AdjustScene();
}

// user insert transition, add extra control
void GioScene::userInsTransCtrl(QPointF bpos) {
  FD_DQG("GioScene::userInsTransCtrl()");
  // update view
  mInsTrans->moveX(bpos);
  mInsTrans->updateData();
  AdjustScene();
}

// user insert transition, cancel
void GioScene::userInsTransCancel(void) {
  FD_DQG("GioScene::userInsTransCancel)");
  if(mInsTrans->scene()==this) removeItem(mInsTrans);
  pGeneratorModel->UndoEditCancel();
}

// user insert transition, commit
void GioScene::userInsTransEnd(GioState* state, QPointF pos) {
  if(mInsTrans->scene()!=this) return;
  FD_DQG("GioScene::userInsTransEnd()");
  // track modified
  Modified(true);
  // set destination 
  mInsTrans->IdxB(state->Idx());
  // test validity
  GioState* stateA = StateItem(mInsTrans->IdxA());
  GioState* stateB = StateItem(mInsTrans->IdxB());
  if(!stateA) { userInsTransCancel(); return; }
  if(!stateB) { userInsTransCancel(); return; }
  // polish pending transitions tip
  mInsTrans->moveT(pos);
  // have the new transition
  GioTrans* trans= addGioTrans(mInsTrans->FTrans());
  // failure: already exits
  if(!trans) { userInsTransCancel(); return; }
  // put into shape
  trans->setData(mInsTrans->data());
  trans->updateData();
  // take pending transition from scene
  removeItem(mInsTrans);
  // adjust scene
  AdjustScene();
  // pass on to other models (we ourselfs will ignore the callback)
  pGeneratorModel->ElementIns(VioElement::FromTrans(trans->FTrans()));
  pGeneratorModel->UndoEditStop();
}

// user delete selection
void GioScene::userDelSelection(void) {
  FD_DQG("GioScene::userDelSelection()"); 
  pGeneratorModel->UndoEditStart();
  Modified(true);
  // do remove trans: (works only because its a copy)
  foreach(VioElement elem, pGeneratorModel->Selection()) {
    if(elem.Type()!=VioElement::ETrans) continue;
    pGeneratorModel->ElementDel(elem);
    removeGioTrans(elem.Trans()); // could have been an invalid trans
  }
  // do remove states: (works only because its a copy)
  FD_DQG("GioScene::userDelSelection(): 2"); 
  foreach(VioElement elem, pGeneratorModel->Selection()) {
    if(elem.Type()!=VioElement::EState) continue;
    pGeneratorModel->ElementDel(elem);
  }
  // test invalid transitions
  FD_DQG("GioScene::userDelSelection(): 3"); 
  foreach(QGraphicsItem* item, items()) {
    GioTrans* trans=qgraphicsitem_cast<GioTrans *>(item);
    if(!trans) continue;
    if(trans->valid()) continue;
    if(StateItem(trans->FTrans().X1))
      if(StateItem(trans->FTrans().X2))
       continue;
    removeGioTrans(trans); 
  }
  AdjustScene();
  pGeneratorModel->UndoEditStop();
  FD_DQG("GioScene::userDelSelection(): done"); 
}



// user interaction: copy shape
void GioScene::userCopyTransShape(void) {
  if(mSelectedTrans.size()!=1) return;
  GioTrans* trans=mSelectedTrans.at(0);
  GioTrans::Data data=trans->data();
  QMimeData* mdat= new QMimeData();
  mdat->setText(data.toString());
  QApplication::clipboard()->setMimeData(mdat);
};

// user interaction: paste shape
void GioScene::userPasteTransShape(void) {
  // retrieve
  const QMimeData* mdat=  QApplication::clipboard()->mimeData();
  QString str=mdat->text();
  GioTrans::Data data;
  try {
    data.fromString(str);
  } catch(faudes::Exception& ex) {
    return;
  }
  // apply
  pGeneratorModel->UndoEditStart();
  foreach(GioTrans* trans,mSelectedTrans) {
    trans->moveS(data);
  }
  pGeneratorModel->UndoEditStop();
};

// user interaction: test for paste
bool GioScene::testPasteTrans(void) {
  const QMimeData* mdat=  QApplication::clipboard()->mimeData();
  QString str=mdat->text();
  GioTrans::Data data;
  bool om=faudes::ConsoleOut::G()->Mute(); 
  try {
    faudes::ConsoleOut::G()->Mute(true);
    data.fromString(str);
    faudes::ConsoleOut::G()->Mute(om);
  } catch(faudes::Exception& ex) {
    return false;
  }
  return true;
};



// user interaction: copy shape
void GioScene::userCopyStateShape(void) {
  if(mSelectedStates.size()!=1) return;
  GioState* state=mSelectedStates.at(0);
  GioState::Data data=state->data();
  QMimeData* mdat= new QMimeData();
  mdat->setText(data.toString());
  QApplication::clipboard()->setMimeData(mdat);
};

// user interaction: paste shape
void GioScene::userPasteStateShape(void) {
  // retrieve
  const QMimeData* mdat=  QApplication::clipboard()->mimeData();
  QString str=mdat->text();
  GioState::Data data;
  try {
    data.fromString(str);
  } catch(faudes::Exception& ex) {
    return;
  }
  // apply
  pGeneratorModel->UndoEditStart();
  foreach(GioState* state,mSelectedStates) {
    state->moveS(data);
  } 
  pGeneratorModel->UndoEditStop();
};

// user interaction: test for paste
bool GioScene::testPasteState(void) {
  const QMimeData* mdat=  QApplication::clipboard()->mimeData();
  QString str=mdat->text();
  GioState::Data data;
  bool om=faudes::ConsoleOut::G()->Mute(); 
  try {
    faudes::ConsoleOut::G()->Mute(true);
    data.fromString(str);
    faudes::ConsoleOut::G()->Mute(om);
  } catch(faudes::Exception& ex) {
    return false;
  }
  return true;
};





