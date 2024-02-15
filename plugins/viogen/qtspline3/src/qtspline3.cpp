/** qt wrapper for alglib spline3 implementation, Thomas Moor, 2008 */

#include "qtspline3.h"
#include <iostream>

namespace qtap {

// arguments: points 0,3,6, ... aka knots
// return:    pionts 0,1,2,3,4, ... aka knots (unchanged) and controls
  void SplineInterpolation(QList<QPointF>& BezierControls, int Offset) {
 // figure number of knots
 int n= ((BezierControls.size()-Offset-1)/3)+1;
 // report y component for debugging
 /*
 std::cout << "spline3 report, segments " << n-1 << " offset " << Offset << std::endl;
 */
 // prepare spline3 input data
 ap::real_1d_array xknots;
 ap::real_1d_array yknots;
 ap::real_1d_array tknots;
 tknots.setbounds(0,n-1);
 xknots.setbounds(0,n-1);
 yknots.setbounds(0,n-1);
 // pick knots from input control
 for(int i=0; i<n; i++) {
   tknots(i)=i;
   xknots(i)=BezierControls[3*i+Offset].x();
   yknots(i)=BezierControls[3*i+Offset].y();
 }
 // pick iboundary conditions
 QPointF dir0 = 3*(BezierControls[Offset+1] - BezierControls[Offset]);
 QPointF dirn = 3*(BezierControls[3*(n-1)+Offset]- BezierControls[3*(n-1)+Offset-1]);
 // prepare spline3 output data
 ap::real_1d_array xspline;
 ap::real_1d_array yspline;
 // run spline3
 buildcubicspline(tknots,xknots,n,1,dir0.x(),1,dirn.x(),xspline);
 buildcubicspline(tknots,yknots,n,1,dir0.y(),1,dirn.y(),yspline);
 // unpack result
 ap::real_2d_array xcoeff;
 ap::real_2d_array ycoeff;
 splineunpack(xspline, n, xcoeff);
 splineunpack(yspline, n, ycoeff);
 // convert to bezier controls
 for(int i=0; i<n-1; i++){
   QPointF pointC0= QPointF(xcoeff(i,2),ycoeff(i,2));
   QPointF pointC1= QPointF(xcoeff(i,3),ycoeff(i,3));
   QPointF pointC2= QPointF(xcoeff(i,4),ycoeff(i,4));
   QPointF pointC3= QPointF(xcoeff(i,5),ycoeff(i,5));
   QPointF pointA = pointC0;
   QPointF pointCA= (1/3.) * pointC1 + pointA;
   QPointF pointCB= (1/3.) * pointC2 + 2*pointCA - pointA;
   QPointF pointB = pointC3 + pointA -3*pointCA + 3*pointCB;
   BezierControls[3*i+Offset]=pointA;
   BezierControls[3*i+Offset+1]=pointCA;
   BezierControls[3*i+Offset+2]=pointCB;
   BezierControls[3*i+Offset+3]=pointB;
   // report y component for debugging
   /*
   std::cout << "spline3 report, segment " << i << std::endl;
   std::cout << "ti, ti+1: " <<  ycoeff(i,0) << " " << ycoeff(i,1) << std::endl;
   std::cout << "c0-3: " << pointC0.y() << "  " << pointC1.y() << "  " << pointC2.y() << "  " << pointC3.y() << std::endl;
   std::cout << "a-b: " << pointA.y() << "  " << pointCA.y() << "  " << pointCB.y() << "  " << pointB.y() << std::endl;
   */
 }
};


}; // namespace
