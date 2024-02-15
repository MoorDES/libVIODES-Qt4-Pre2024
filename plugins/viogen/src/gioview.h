/* gioview.h  - qgraphicsview for gioscenes */

/*
   Graphical  IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006, 2007  Thomas Moor, Klaus Schmidt, Sebastian Perk

*/


#ifndef FAUDES_GIOVIEW_H
#define FAUDES_GIOVIEW_H

#include <QtGui>

#include "gioscenero.h"
#include "gioscene.h"

/*
 ************************************************
 ************************************************

 A GioView is a QGraphicsView intended to show a 
 GioSceneRo or a GioScene.  It adds zoom via 
 mouse wheel and other features.

 ************************************************
 ************************************************
 */


 
class GioView : public QGraphicsView {
  Q_OBJECT

public:
  // construct from scene
  GioView(QWidget* parent=0);

public slots:

  // set views matrix
  void Reset(void);
  void Scale(qreal sc);
  qreal Scale(void);
  void Fit(void);

signals:

  // report user zoom
  void NotifyZoom(qreal sf);



protected:
  // reimplement mouse events
  void wheelEvent(QWheelEvent *event);
  void mousePressEvent(QMouseEvent * event);
  void mouseReleaseEvent(QMouseEvent * event);
  void mouseDoubleClickEvent(QMouseEvent * event);
  void mouseMoveEvent(QMouseEvent * event);

  // choose tool



};


#endif
