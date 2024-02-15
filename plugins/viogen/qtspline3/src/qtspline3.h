/*********************************************************
 
 qt wrapper for alglib spline3 implementation

 purpose: given a QList of QPointF, pass it to alglib to find 
 the interpolating cubic spline and return the result as a QList 
 of bezier control points ready for QPainterPath

 Copyright Thomas Moor 2008



 *********************************************************/

#ifndef QTSPLINE3
#define QTSPLINE3

#include <QtCore>
#include "spline3.h"

namespace qtap {

// optimize spline
  void SplineInterpolation(QList<QPointF>& BezierControls, int Offset=0);

} // namespace
#endif
