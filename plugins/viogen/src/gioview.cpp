/* gioview.cpp  - qgraphicsview for gioscenes */

/*
   Graphical  IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006, 2007  Thomas Moor, Klaus Schmidt, Sebastian Perk

*/


#include "gioview.h"

// construct
GioView::GioView(QWidget* parent) :  QGraphicsView(parent) {
  FD_DQ("GioView::GioView(" << parent << ")");
  setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
  setDragMode(RubberBandDrag);
  setMouseTracking(true);
}

// Scale
void GioView::Scale(qreal sc) {  
  setMatrix(QMatrix(sc,0,0,sc,0,0));
}

// Scale
qreal GioView::Scale(void) {  
  return matrix().m11();
}


// Reset Matrix
void GioView::Reset(void) {
  setMatrix(QMatrix());
}

// fit scene
void GioView::Fit(void){
  GioSceneRo* gs = qobject_cast<GioSceneRo*>(scene());
  if(!gs) return;
  // set scene rectangle
  QRectF rect=gs->itemsBoundingRect();
  rect.adjust(-0.01*gs->width(),-0.01*gs->height(),
    0.01*gs->width(),0.01*gs->height());
  gs->setSceneRect(rect);
  // default: fit entire scene
  QRectF target = gs->sceneRect();
  // alt: fit selection
  if(gs->selectedItems().size() >= 1) {
    target= QRect();
    foreach(QGraphicsItem* item, gs->selectedItems()) {
      QPolygonF itempoly =item->mapToScene(item->boundingRect());
      target |=itempoly.boundingRect();
    }
  }
  // doit
  fitInView(target, Qt::KeepAspectRatio); 
}




// wheel event
void GioView::wheelEvent(QWheelEvent *event) {
  FD_DQ("GioView::wheelEvent(..) at (" << event->pos().x() << ", " << event->pos().y()
	<< ") with value " << event->delta());
  // scale factor
  qreal degree= ((qreal) event->delta()) / 8.0; 
  if(degree < -30) degree = -30.0;
  if(degree >  30) degree = 30.0;
  qreal sc = 1.0 + ( 5.0 * degree / 3000.0 ); // 5% steps per 30 degree
  // where we are before scalung
  QPointF opos=mapToScene(event->pos());
  FD_DQ("GioView::wheelEvent(..) at scenepos (" << opos.x() << ", " << opos.y() << ")");
  // do the scale
  setResizeAnchor(AnchorUnderMouse);
  setTransformationAnchor(AnchorUnderMouse);
  scale(sc,sc);
  setTransformationAnchor(NoAnchor);
  // translate
  QPointF npos=mapToScene(event->pos());
  QPointF diff=npos-opos;
  QPointF vdiff=mapFromScene(diff);
  translate(diff.x(),diff.y());
  // done
  event->accept();
  // notify
  emit NotifyZoom(Scale());
};


// handle my events: mouse press
// note: when rubber band drag ends, press void deselects but is not passed to scene
// note: the rubber band also gets irritated when the scene has a context menu
void GioView::mousePressEvent(QMouseEvent * event) {
   FD_DQ("GioView::mousePress()");
   if(event->button() == Qt::RightButton) return;
   QGraphicsView::mousePressEvent(event);
   if(event->isAccepted())
     if(GioScene* sc=qobject_cast<GioScene*>(scene())) sc->fixSelection();
}


// handle my events: mouse release
// note: when rubber band drag ends, release is not passed to scene
void GioView::mouseReleaseEvent(QMouseEvent * event) {
   FD_DQ("GioView::mouseRelease()");
   QGraphicsView::mouseReleaseEvent(event);
   if(event->isAccepted())
   if(GioScene* sc=qobject_cast<GioScene*>(scene())) {
      sc->userSelectionUpdate(); 
   }
   if(event->isAccepted())
   if(GioScene* sc=qobject_cast<GioScene*>(scene())) {
     if(sc->mMoveSelection) event->ignore();
   }
}


// handle my events: mouse double click
void GioView::mouseDoubleClickEvent(QMouseEvent * event) {
   //FD_DQ("GioView::mouseDoubleClickEvent()");
   // pass on
   QGraphicsView::mouseDoubleClickEvent(event);
}

// handle my events: mouse move
void GioView::mouseMoveEvent(QMouseEvent * event) {
  //FD_DQG("GioView::mouseMove()");
   QGraphicsView::mouseMoveEvent(event);
}
