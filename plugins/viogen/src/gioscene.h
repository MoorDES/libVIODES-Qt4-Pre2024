/* gioscene.h  - faudes generator as qgraphicsscene */

/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006, 2007  Thomas Moor, Klaus Schmidt, Sebastian Perk

*/



#ifndef FAUDES_GIOSCENE_H
#define FAUDES_GIOSCENE_H


#include "libviodes.h"
#include "gioscenero.h"
//#include "piotseprop.h"



/*
 ************************************************
 ************************************************

 A GioScene is a QGraphicsSene that holds GioItems and 
 a reference to a VioGenerator to provide the 
 context for GioItems. A GioScene will never change the
 faudes generator itself, but it use callbacks to
 request such change. Conversely, it receives signals
 form the generator that indicate that it was changed.

 While the Items themselfs are responsible for graphic representation 
 incl. user interaction regarding graphic representation, the GioScene 
 cares about generator data (aka state and transition names and flags)
 via conext menus from VioProperties and passes on requests to
 generator callbacks. There is also selection management
 incl cut and paste.

 This class is derived from the read-only version GioSceneRO.

 ************************************************
 ************************************************
 */


 
class GioScene : public GioSceneRo {

  Q_OBJECT

public:
  // allow items/view to access private data
  friend class GioItem;
  friend class GioView;

  // constructor, destructor
  GioScene(VioGeneratorGraphModel* pGen);
  virtual ~GioScene(void);

  // set/get transition insert mode
  static void TransInsertMode(GioTrans::EditMode mode);
  static GioTrans::EditMode TransInsertMode(void);
  
public slots:

  // reset myself
  virtual int Clear(void);

  // select items (cannot select event, cannot deselect)
  void UpdateSelectionElement(const VioElement& elem, bool on=true);
  void UpdateSelectionClear(void);
  void UpdateSelectionAny(void);

  //int  InsSelection(const Data& giodata);
  //int  GetSelection(Data& giodata);

  // set grid
  void GridVisible(bool on);

public:

  // insert and delete items: override for add ons (selection)
  virtual void removeGioState(GioState* state);
  virtual void removeGioState(faudes::Idx idx);
  virtual void removeGioTrans(const faudes::Transition& ftrans);
  virtual void removeGioTrans(GioTrans* trans);

  // access visual selection
  const QList<GioState*>& SelectedStates(void) const { return mSelectedStates; }
  const QList<GioTrans*>& SelectedTransitions(void) const { return mSelectedTrans; }

  // get grid
  bool GridVisible(void) const;

protected:

  // internal selection 
  void fixSelection(void);
  QList<GioState*>  mSelectedStates;
  QList<GioState*>  mSelectedStatesF;
  QList<GioTrans*>  mSelectedTrans;
  QList<GioTrans*>  mSelectedTransFAB;
  QList<GioTrans*>  mSelectedTransFA;
  QList<GioTrans*>  mSelectedTransFB;
  QMap<GioState*, QPointF> mMovePosMap;
  bool mMoveSelection;
  bool mMovingSelection;
  bool fixMove(const QPointF& diff);

  // user interaction on items: edit
  GioState* userInsState(const QPointF& pos);
  GioTrans* userInsTrans(faudes::Idx idxA, faudes::Idx idxB);
  bool userInsTrans(void);
  void userInsTransBegin(GioState* state, QPointF bpos);
  void userInsTransUpdate(QPointF bpos);
  void userInsTransCtrl(QPointF bpos);
  void userInsTransEnd(GioState* state, QPointF bpos);
  void userInsTransCancel(void);
  GioTrans* mInsTrans;

  // user interaction: copy/paste shape
  void userCopyTransShape(void);
  void userPasteTransShape(void);
  bool testPasteTrans(void);
  void userCopyStateShape(void);
  void userPasteStateShape(void);
  bool testPasteState(void);

  // user interaction on items: selection
  void userDelSelection(void);
  void userSelectTrans(GioTrans* trans, bool on=true);
  void userSelectState(GioState* state, bool on=true);
  void userSelectNothing(void);
  void userSelectionUpdate(void);

  // get ui events
  virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
  virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
  virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent*event);
  virtual void keyPressEvent(QKeyEvent *keyEvent);  
  virtual void keyReleaseEvent(QKeyEvent *keyEvent);  
  virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

  // mouse/tool mode
  typedef enum {SelectTool,InsertHoldTool,InsertOnceTool} Tool;
  Tool mTool;
  void setTool(Tool tool);
  bool mGridVisible;
  static GioTrans::EditMode mTransInsMode;


  // reimplement background/foreground
  virtual void drawBackground( QPainter * painter, const QRectF & rect);
  virtual void drawForeground( QPainter * painter, const QRectF & rect);


private:

  // edit item properties
  void sceneContextMenu(const QPoint& screenpos);
  void stateContextMenu(const QPoint& screenpos, GioState* state);
  void transContextMenu(const QPoint& screenpos, GioTrans* trans);

  /* 
  // state based undo scheme, 1: undo stack
  QUndoStack mUndoStack;

  // state based undo scheme, 2: undo command
  class GioUndoCommand : public QUndoCommand {
  public:
    GioUndoCommand(GioScene* gscene) {mpGscene=gscene; mDataT0=mpGscene->data(); mRedo=false;};
    void undo(void) {mDataT1=mpGscene->data(); mpGscene->setData(mDataT0); mRedo=true;};
    void redo(void) {if(mRedo) mpGscene->setData(mDataT1);}; 
  private:
    GioScene* mpGscene;
    Data mDataT0;
    Data mDataT1;
    bool mRedo;
  };

  // state based undo scheme, 3: set undo point
  void markUndoPoint() {
    GioUndoCommand* undocmd = new GioUndoCommand(this);
    mUndoStack.push(undocmd); 
    todo: set undo limit (need qt 4.3) 
  };
  */  

};






#endif
