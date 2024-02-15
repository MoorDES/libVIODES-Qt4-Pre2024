/* giotrans.cpp  - graphical representation of one transition */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006, 2007  Thomas Moor, Klaus Schmidt, Sebastian Perk

*/



#include <QtGui>

#include "giotrans.h"
#include "giostate.h"

#ifdef USING_SPLINE
#include "qtspline3.h"
#endif

// convenience macros to access base/ctrl points
#define POINT_N 0                            // name tag
#define POINT_A 1                            // begin of arc (allways 0)
#define POINT_R 2                            // automatic root of arc 
#define POINT_I (mBasePoints.size()-2)       // automatic arrow tip
#define POINT_B (mBasePoints.size()-1)       // end of arc
#define MINPOINTS 7                          // N,A,R, spline 2* control, I, B
 
// same for sontrols
#define CTRL_N 0                            // name tag
#define CTRL_A 1                            // begin of arc (allways 0)
#define CTRL_R 2                            // automatic root of arc 
#define CTRL_I (mCtrlPoints.size()-2)       // automatic arrow tip
#define CTRL_B (mCtrlPoints.size()-1)       // end of arc

// points between POINT_R and POINT_I (incl) are segments of b-splines, 3 points each
#define SEGMENTS ((mBasePoints.size()-3)/3) // number of segments 0,1,2 ..
#define SEGMENT_A(i)  (3*(i)+POINT_R)       // begin of b-spline
#define SEGMENT_CA(i) (3*(i)+POINT_R+1)     // first control point of b-spline
#define SEGMENT_CB(i) (3*(i)+POINT_R+2)     // second control point of b-spline
#define SEGMENT_B(i)  (3*(i)+POINT_R+3)     // end point (=begin point of next segment)
#define POINT_T SEGMENT_A((SEGMENTS/2))     // begin of spline with tick

// pick segment (tricky for last segment point b)
#define SEGMENT(ctrl) ( ctrl < POINT_I ? (((ctrl)-POINT_R)/3) : SEGMENTS-1)  
                                            



// convenience macros to acess paths
#define PATH_ARC    0   // arc
#define PATH_ARR    1   // arrow
#define PATH_TCK    2   // tick
#define PATH_NAME00 3   // label, relative to POINT_N
#define PATHS 4         // minimum number of paths


// default constructor
GioTrans::GioTrans(VioGeneratorModel* gmodel) : GioItem(gmodel) {
  FD_DQ("GioTrans::GioTrans()");

  // on bottom
  setZValue(-10);

  // transitions are moved by states
  setFlag(QGraphicsItem::ItemIsMovable, false);

  // my user data
  mIdxA=0;
  mIdxEv=0;
  mIdxB=0;

  // points and paths
  while(mBasePoints.size()<MINPOINTS) mBasePoints.append(QPointF(0,0));
  while(mDrawPaths.size()<PATHS) mDrawPaths.append(QPainterPath());

  // clear edit mode
  mEditMode=Free;

  // set to reasonable default
  hintAB(QPointF(-10.0,-10.0), QPointF(10.0,10.0));

  // update everything
  updateData();
}

// get all data
GioTrans::Data GioTrans::data(void) {
  Data res;
  res.mIdxA =  mIdxA;
  res.mNameEv = FNameEv();
  res.mIdxB =  mIdxB;
  res.mEditMode = mEditMode;
  res.mPosition = pos();
  res.mBasePoints = mBasePoints;
  return res;
}

// set all data
void GioTrans::setData(const Data& data) {
  mIdxA=data.mIdxA; 
  mIdxEv=Generator()->EventIndex(data.mNameEv); 
  mIdxB=data.mIdxB; 
  mEditMode=data.mEditMode;
  if((data.mBasePoints.size() >= MINPOINTS)  &&   
    ((data.mBasePoints.size() % 3) == (MINPOINTS %3))) {
    GioItem::setData(data);
    return;
  } 
  FD_DQ("GioTrans::setData(data): incompatible data");
  // hintAB
  setPos(data.mPosition);
  updateData();
}

// data io read
void GioTrans::Data::read(faudes::TokenReader& tr) {
  FD_DQ("GioTrans::Data::read");
  tr.ReadBegin("Trans");
  // core data: faudes name and index
  mIdxA=tr.ReadInteger();
  mNameEv=tr.ReadString();
  mIdxB=tr.ReadInteger();
  // optional edit mode
  mEditMode=Free;
  faudes::Token token;
  tr.Peek(token); 
  if(token.Type()==faudes::Token::Option) {
    std::string option;
    option=tr.ReadOption();
    if(option=="Free")    mEditMode=Free;
    if(option=="Polygon") mEditMode=Polygon;
    if(option=="Line")    mEditMode=Line;
    if(option=="Spline")  mEditMode=Spline;
    if(option=="Smooth")  mEditMode=Smooth;
    if(option=="Mute")    mEditMode=Mute;
  };
  FD_DQ("GioTrans::Data::read: " << mIdxA << " " << mNameEv << " " << mIdxB);
  // read item, incl. position
  GioItem::Data::read(tr);
  // done
  tr.ReadEnd("Trans");  
  // check validity (relax, need poits N, A and B)
  if(mBasePoints.size()<3) {
    std::stringstream errstr;
    errstr << "base points mismatch " << tr.FileLine();
    throw faudes::Exception("GioState::Data::read", errstr.str(), 50);
  }
  // use end points
  if(mBasePoints.size()<MINPOINTS) {
    FD_DQ("GioTrans::Data::read: not enough points");
    QPointF pointa=mBasePoints[POINT_A];
    QPointF pointb=mBasePoints[POINT_B];
    QPointF dir=(pointb-pointa);
    mBasePoints.clear();
    while(mBasePoints.size()<MINPOINTS) mBasePoints.append(QPointF(0,0));
    mBasePoints[POINT_A]=pointa;
    mBasePoints[SEGMENT_A(0)]=pointa+0.5*dir; // gets fixed
    mBasePoints[SEGMENT_CA(0)]=pointa+0.5*dir;
    mBasePoints[SEGMENT_CB(0)]=pointa+0.5*dir;
    mBasePoints[SEGMENT_B(0)]=pointa+0.5*dir; // gets fixed
    mBasePoints[POINT_B]=pointb;
    mBasePoints[POINT_N]=pointa+0.5*dir;
  }
  // ignore others
  while( (mBasePoints.size() % 3) != (MINPOINTS % 3)) {
    FD_DQ("GioTrans::Data::read: spare points");
    mBasePoints.removeAt(SEGMENT_CA(0));
  }
}

// data io write
void GioTrans::Data::write(faudes::TokenWriter& tw, const faudes::vGenerator* pGen) const {
  faudes::Token token;
  tw.WriteBegin("Trans");
  // faudes transition: state A
  faudes::Idx indexA = mIdxA;
  if(pGen) indexA=pGen->MinStateIndex(indexA);
  tw << indexA;
  // faudes transition: event
  tw << mNameEv;
  // faudes transition: state B  
  faudes::Idx indexB = mIdxB;
  if(pGen) indexB=pGen->MinStateIndex(indexB);
  tw << indexB;
  FD_DQ("GioTrans::write: " << indexA << " " << mNameEv << " " << indexB);
  // optional: editmode
  if(mEditMode==Free)  tw.WriteOption("Free");
  if(mEditMode==Spline)  tw.WriteOption("Spline");
  if(mEditMode==Smooth)  tw.WriteOption("Smooth");
  if(mEditMode==Mute)     tw.WriteOption("Mute");
  if(mEditMode==Polygon)  tw.WriteOption("Polygon");
  if(mEditMode==Line)     tw.WriteOption("Line");
  // item
  GioItem::Data::write(tw);
  // done
  tw.WriteEnd("Trans");
}

// item io reading
void GioTrans::read(faudes::TokenReader& tr) {
  Data tdata;
  tdata.read(tr);
  setData(tdata);
}

// item io writing
void GioTrans::write(faudes::TokenWriter& tw) {
  Data tdata;
  tdata=data();
  tdata.write(tw,Generator());
}


// create geometry by point B
void GioTrans::hintB(const QPointF& posB) {
  FD_DQ("GioTrans::hintB");
  // prepare base
  mBasePoints.clear();
  while(mBasePoints.size()<MINPOINTS) mBasePoints.append(QPointF(0,0));
  // two cases: std or selfloop
  if(IdxA()!=IdxB()) {
    // std case: straight line
    QPointF pointa = QPointF(0,0);
    QPointF pointb = mapFromScene(posB);
    QPointF pointc = 0.5*pointb;
    QPointF pointo = VioStyle::NormalF(pointb);
    mBasePoints[POINT_N]=pointc+ 10*pointo;  // style
    mBasePoints[POINT_A]=QPointF(0,0);
    mBasePoints[SEGMENT_A(0)]=pointc;        // gets fixed anyway
    mBasePoints[SEGMENT_CA(0)]=pointb;       // gets fixed below
    mBasePoints[SEGMENT_CB(0)]=pointa;       // gets fixed below
    mBasePoints[SEGMENT_B(0)]=pointc;        // gets fixed anyway
    mBasePoints[POINT_B]=pointb;
    fixRootAndTip();
    QPointF dir = mBasePoints[SEGMENT_B(0)]-mBasePoints[SEGMENT_A(0)];
    mBasePoints[SEGMENT_CA(0)]=mBasePoints[SEGMENT_A(0)]+0.33 *dir; 
    mBasePoints[SEGMENT_CB(0)]=mBasePoints[SEGMENT_A(0)]+0.66 *dir; 
    mEditMode=Line;
  } else {
   // selfloop:  spline
    mBasePoints[POINT_A]       =QPointF(0,0);
    mBasePoints[SEGMENT_A(0)]  =QPointF(35,-70);  // gets fixed anyway
    mBasePoints[SEGMENT_CA(0)] =QPointF(35,-70);  // gets fixed below
    mBasePoints[SEGMENT_CB(0)] =QPointF(-35,-70); // gets fixed below
    mBasePoints[SEGMENT_B(0)]  =QPointF(-35,-70); // gets fixed anyway
    mBasePoints[POINT_B]       =QPointF(0,0);
    mBasePoints[POINT_N]       =QPointF(0,-70);   // gets fixed below    
    fixRootAndTip();
    QPointF dira = mBasePoints[SEGMENT_A(0)]-mBasePoints[POINT_A];
    mBasePoints[SEGMENT_CA(0)] =  mBasePoints[SEGMENT_A(0)] + 2*dira;
    QPointF dirb = mBasePoints[SEGMENT_B(0)]-mBasePoints[POINT_B];
    mBasePoints[SEGMENT_CB(0)] =  mBasePoints[SEGMENT_B(0)] + 2*dirb;
    mBasePoints[POINT_N]       = 0.5 * (mBasePoints[SEGMENT_CA(0)]+mBasePoints[SEGMENT_CB(0)]);
    mEditMode=Spline;
  } 
  updateCtrls();
  updateFlag();
  updateDoit();
}


// create geometry by points A and B
void GioTrans::hintAB(const QPointF& posA, const QPointF& posB) {
  FD_DQ("GioTrans::hintAB");
  setPos(posA);
  hintB(posB);
}

// change geometry by setting label position
void GioTrans::moveN(const QPointF& labelpos) { 
  mBasePoints[POINT_N]=mapFromScene(labelpos);
  updateCtrls();
  updateFlag();
  updateDoit();
}


// change geometry by setting spline ctrl points
void GioTrans::moveC(const QList<QPointF>& ctrlpoints) { 
  // must be one segment
  if(ctrlpoints.size()<4) return;
  if( (ctrlpoints.size()-1) % 3 != 0) return;
  // adjust spline
  while(POINT_I - POINT_R + 1 < ctrlpoints.size())  mBasePoints.insert(POINT_B,QPointF(0,0)); 
  while(POINT_I - POINT_R + 1 > ctrlpoints.size())  mBasePoints.removeAt(POINT_I);
  // set points
  for(int i=0; i < ctrlpoints.size(); i++) {
    FD_DQ("GioTrans::moveC(): " << i << " " << ctrlpoints[i].x() << " " << ctrlpoints[i].y());
    mBasePoints[POINT_R+i]=mapFromScene(ctrlpoints[i]);
  }
  // fix drawing
  mEditMode=Free;
  updateCtrls();
  updateFlag();
  updateDoit();
}

// change geometry by moving point B
void GioTrans::moveB(const QPointF& posB) {
  FD_DQ("GioTrans::moveB");
  // ail out on invalid
  if(SEGMENTS <1) return;
  // record old relative name position
  QPointF nam= mBasePoints[POINT_N] - 0.5*(
      mBasePoints[POINT_A]+mBasePoints[POINT_B]); 
  // shift point b and arrow
  QPointF diff = mapFromScene(posB)- mBasePoints[POINT_B];
  mBasePoints[POINT_B]+=diff;
  mBasePoints[POINT_I]+=diff;
  mBasePoints[POINT_I-1]+=diff;
  // if we have only one segment, restore name pos
  if(SEGMENTS==1) 
    mBasePoints[POINT_N] = nam + 0.5*(
      mBasePoints[POINT_A]+mBasePoints[POINT_B]); 
  // update others
  setEditMode(mEditMode);
}

// change geometry by moving pointA
void GioTrans::moveA(const QPointF& posA) {
  FD_DQ("GioTrans::moveA");
  updateFlag();
  // record old relative name position
  QPointF nam= mBasePoints[POINT_N] - 0.5*(
      mBasePoints[POINT_A]+mBasePoints[POINT_B]); 
  // since point A is (0,0) by def, a new point A means to shift 
  // the entire transition and then set its pos to new A 
  QPointF dir= mapFromScene(posA);
  for(int i=POINT_R+2; i<mBasePoints.size(); i++)
    mBasePoints[i]-=dir;
  mBasePoints[POINT_N]-=dir;
  mBasePoints[POINT_A]=QPointF(0,0);
  setPos(posA);
  // if we have only one segment, restore name pos
  if(SEGMENTS==1) 
    mBasePoints[POINT_N] = nam + 0.5*(
      mBasePoints[POINT_A]+mBasePoints[POINT_B]); 
  // update others
  setEditMode(mEditMode);
}

// change geometry by moving tip (pending transition)
void GioTrans::moveT(const QPointF& posB) {
  FD_DQ("GioTrans::moveT");
  // bail out on invalid
  if(SEGMENTS <1) return;
  // record old relative name position
  QPointF nam= mBasePoints[POINT_N] - 0.5*(
      mBasePoints[POINT_A]+mBasePoints[POINT_B]); 
  // shift point b 
  QPointF newb=mapFromScene(pGeneratorConfig->GridPoint(posB));
  mBasePoints[POINT_B]=newb;
  mBasePoints[POINT_I]=newb;
  // if we have a destination state, fix root and tip 
  if(GioState* stateB=StateItem(mIdxB)){
    fixRootAndTip();  
    mBasePoints[POINT_B]=mapFromScene(stateB->pos());
  }
  // currently, only support spline/smooth/polygon with fallback smooth
  if(SEGMENTS>1) 
  if(mEditMode!=Polygon)
  if(mEditMode!=Smooth)
  if(mEditMode!=Spline)
    mEditMode=Smooth;
  // fix last segment
  if(mEditMode==Polygon)
  if(SEGMENTS>=1) {
    FD_DQ("GioTrans::moveT one segment");
    int seg=SEGMENTS-1;
    QPointF dir= 0.33*(mBasePoints[SEGMENT_B(seg)]-mBasePoints[SEGMENT_A(seg)]);
    mBasePoints[SEGMENT_CA(seg)]=mBasePoints[SEGMENT_A(seg)]+dir;
    mBasePoints[SEGMENT_CB(seg)]=mBasePoints[SEGMENT_B(seg)]-dir;
  }
  // fix last segment (originally for smooth, also used for interpolation)
  if(mEditMode==Smooth || mEditMode==Spline)
  if(SEGMENTS>1) {
    FD_DQ("GioTrans::moveT more than segment");
    /* version 1: strategic direction averaging */
    /* note: version 1 catches ingularities of version 2 */ 
    int segm1=SEGMENTS-2;
    int seg=SEGMENTS-1;
    QPointF dirm1= mBasePoints[SEGMENT_B(segm1)]-mBasePoints[SEGMENT_CB(segm1)];
    dirm1=VioStyle::NormalizeF(dirm1); 
    QPointF dir= mBasePoints[SEGMENT_B(seg)]-mBasePoints[SEGMENT_A(seg)];
    qreal dist=VioStyle::NormF(dir);
    dir=VioStyle::NormalizeF(dir); 
    qreal m = (.5 - sqrt(.5)) / ( 1- sqrt(.5));
    qreal b = sqrt(0.5)*(1-m);
    dirm1 = dist* (b + m* VioStyle::ScalarF(dir,dirm1) ) * dirm1; 
    QPointF vert = mBasePoints[SEGMENT_A(seg)]+ dirm1;
    mBasePoints[SEGMENT_CA(seg)]=0.5*(mBasePoints[SEGMENT_A(seg)]+vert);
    mBasePoints[SEGMENT_CB(seg)]=0.5*(mBasePoints[SEGMENT_B(seg)]+vert);
  }
  // fix last segment (for smooth, improoved version)
  if(mEditMode==Smooth)
  if(SEGMENTS>1) {
    /* version 2: circle construct */
    int segm1=SEGMENTS-2;
    int seg=SEGMENTS-1;
    QPointF A = mBasePoints[SEGMENT_A(seg)];
    QPointF B = mBasePoints[SEGMENT_B(seg)];
    QPointF BmA = B-A;
    QPointF a=  mBasePoints[SEGMENT_B(segm1)]-mBasePoints[SEGMENT_CB(segm1)];
    QPointF oa=VioStyle::NormalF(a); 
    a = VioStyle::NormalizeF(a);
    qreal alpha_nom = pow(BmA.x(),2) + pow(BmA.y(),2);
    qreal alpha_den = 2*(oa.x()* BmA.x()) + 2 *(oa.y()*BmA.y());
    if(((alpha_nom* alpha_nom) < 0.01) || (alpha_den * alpha_den < 0.0001) ) {
    } else {
      qreal alpha= alpha_nom/alpha_den;
      QPointF M = A +   alpha * oa;
      qreal rotcos = VioStyle::ScalarF(A-M,B-M) / (alpha *alpha);
      if( VioStyle::ScalarF(B-M,a) < 0 ) rotcos = -2 - rotcos; // hack: fix > 180deg
      if( VioStyle::ScalarF(BmA,oa) < 0 ) a = -a; // hack: fix left turn sign
      qreal l = alpha * (1-rotcos) * 0.6;
      QPointF b =VioStyle::NormalF(B-M); 
      FD_DQ("Circle: rotcos " << rotcos << " dir " << b.x() << "," << b.y());
      mBasePoints[SEGMENT_CA(seg)]=A + l * a;
      mBasePoints[SEGMENT_CB(seg)]=B - l * b;
    }
  }
  // fix all for smooth spline
  if(mEditMode==Spline)
  if(SEGMENTS>1) 
    convertToSpline();
  // if we have only one segment, restore name pos
  if(SEGMENTS==1) 
    mBasePoints[POINT_N] = nam + 0.5*(
      mBasePoints[POINT_A]+mBasePoints[POINT_B]); 
  // if we have only one segment, run convert to have a nice line
  if(SEGMENTS==1) 
    convertToLine();
  // fix drawing
  updateCtrls();
  updateFlag();
  updateDoit();
}


// change geometry by adding a segment
void GioTrans::moveX(const QPointF& posB) {
  FD_DQ("GioTrans::moveX");
  // allocate new set of controls
  QPointF newb = mapFromScene(posB);
  // fix last segment
  /*
  if(mEditMode==Smooth || mEditMode==Polygon)
  if(SEGMENTS >= 1) {
    int seg=SEGMENTS-1;
    mBasePoints[SEGMENT_B(seg)] = newb;
    QPointF dir= 0.33*(mBasePoints[SEGMENT_B(seg)]-mBasePoints[SEGMENT_A(seg)]);
    mBasePoints[SEGMENT_CA(seg)]=   mBasePoints[SEGMENT_A(seg)]+dir;
    mBasePoints[SEGMENT_CB(seg)]=  mBasePoints[SEGMENT_B(seg)]-dir;
  }
  */
  // fix before last segment,  (for smooth interpolation)
  /*
  if(mEditMode==Smooth || mEditMode==Polygon)
  if(SEGMENTS>1) {
    FD_DQ("GioTrans::moveX second segment");
    int seg=SEGMENTS-2;
    QPointF dir= 0.33*(mBasePoints[SEGMENT_B(seg)]-mBasePoints[SEGMENT_A(seg)]);
    mBasePoints[SEGMENT_CA(seg)]=mBasePoints[SEGMENT_A(seg)]+dir;
    mBasePoints[SEGMENT_CB(seg)]=mBasePoints[SEGMENT_B(seg)]-dir;
  }
  */
  // allocate new segment, aka 3 new points
  mBasePoints.insert(POINT_B,newb); 
  mBasePoints.insert(POINT_B,newb); 
  mBasePoints.insert(POINT_B,newb); 
  // for spline interpolation, set recent direction to previous segment
  /*
  if(SEGMENTS>1) {
    int seg=SEGMENTS-2;
    QPointF dir= 0.1*0.33*(mBasePoints[SEGMENT_B(seg)]-mBasePoints[SEGMENT_A(seg)]);
    mBasePoints[SEGMENT_CB(seg+1)]= mBasePoints[SEGMENT_B(seg+1)]-dir;
    mBasePoints[SEGMENT_CA(seg+1)]= mBasePoints[SEGMENT_A(seg+1)]+dir;
  }
  */
  // set point b to tip
  mBasePoints[POINT_B]=newb; 
  // fix drawing
  updateCtrls();
  updateFlag();
  updateDoit();
  FD_DQG("GioTrans::moveX: #" << mCtrlPoints.size()); 
}

// change geometry by retrieving shape from other transition
void GioTrans::moveS(const Data& data) {
  FD_DQG("GioTrans::moveS(): src #" << data.mBasePoints.size());
  // bail out on invalid base points
  if(data.mBasePoints.size()<MINPOINTS) return;
  // figure non-matching self-loops
  qreal nsrc=VioStyle::NormF(data.mBasePoints.back());  // data's pointB
  qreal ndst=VioStyle::NormF(mBasePoints[POINT_B]);
  if(nsrc==0 && ndst !=0) return;
  if(nsrc!=0 && ndst ==0) return;
  // get coordinate systems
  QPointF xsrc = VioStyle::NormalizeF(data.mBasePoints.back()); // data's pointB
  QPointF ysrc = VioStyle::NormalF(data.mBasePoints.back());    // data's pointB
  QPointF xdst = VioStyle::NormalizeF(mBasePoints[POINT_B]);
  QPointF ydst = VioStyle::NormalF(mBasePoints[POINT_B]);
  // start of with straight copy
  mBasePoints=data.mBasePoints;
  // do transformation for non-self-loop
  if(nsrc!=0 && ndst !=0) {
    // figure transformation 
    qreal scale = ndst/nsrc;
    QTransform qtr(
      scale*(xsrc.x()*xdst.x()+ysrc.x()*ydst.x()), /* m11 */
      scale*(xsrc.x()*xdst.y()+ysrc.x()*ydst.y()), /* m12 */
      scale*(xsrc.y()*xdst.x()+ysrc.y()*ydst.x()), /* m21 */
      scale*(xsrc.y()*xdst.y()+ysrc.y()*ydst.y()), /* m22 */
      0, 0); /* dx dy */
    FD_DQG("GioTrans::moveS(): m11 " << qtr.m11() << " m12 " << qtr.m12() <<
  	 " m21 " << qtr.m21() << " m22 " << qtr.m22());
    // apply transformation to base
    for(int i=POINT_A+1; i<= POINT_B; i++) {
      mBasePoints[i]= qtr.map(mBasePoints[i]);
    }
    mBasePoints[POINT_N]= qtr.map(mBasePoints[POINT_N]);
    FD_DQG("GioTrans::moveS(): dst #" << mBasePoints.size());
  }
  // copy to ctrls and fix drawing
  updateCtrls();
  fixRootAndTip();
  // on mode change, do a complete fix
  if(mEditMode!=data.mEditMode) {
    setEditMode(data.mEditMode);
    FD_DQG("GioTrans::moveS(): done (full update)");
    return;
  }
  // do just the paths
  updateFlag();
  updateDoit();
  FD_DQG("GioTrans::moveS(): done");
}



// convert to spline (reset all, incl fixed ctrls)
void GioTrans::convertToFree(void) {
  FD_DQ("GioTrans::free()"); 
  updateCtrls();
  updateFlag();
  updateDoit();
}

// delete all controls
void GioTrans::convertToLine(void) {
  FD_DQ("GioTrans::straight()"); 
  QPointF pointa=mapToScene(mBasePoints[POINT_A]);
  QPointF pointb=mapToScene(mBasePoints[POINT_B]);
  EditMode oldmode=mEditMode;
  hintAB(pointa,pointb);
  mEditMode=oldmode;
}

// convert to polygon
void GioTrans::convertToPolygon(void) {
  FD_DQ("GioTrans::polygon()"); 
  for(int seg = 0; seg < SEGMENTS; seg++){
    QPointF dir= 0.33*(mBasePoints[SEGMENT_B(seg)]-mBasePoints[SEGMENT_A(seg)]);
    mBasePoints[SEGMENT_CA(seg)]=   mBasePoints[SEGMENT_A(seg)]+dir;
    mBasePoints[SEGMENT_CB(seg)]=  mBasePoints[SEGMENT_B(seg)]-dir;
  }
  updateCtrls();
  updateFlag();
  updateDoit();
}

// smoothen curve
void GioTrans::convertToSmooth(void) {
  FD_DQ("GioTrans::smooth()"); 
  // middle controls
  for(int seg = 0; seg < SEGMENTS-1; seg++){
    QPointF dirb= -1*(mBasePoints[SEGMENT_CB(seg)]-mBasePoints[SEGMENT_B(seg)]);
    QPointF dira= mBasePoints[SEGMENT_CA(seg+1)]-mBasePoints[SEGMENT_A(seg+1)];
    QPointF dir= VioStyle::NormalizeF(dirb+dira);
    qreal lena = VioStyle::NormF(dira);
    qreal lenb = VioStyle::NormF(dirb);
    QPointF mdira= lena* dir;
    QPointF mdirb= lenb* dir;
    mBasePoints[SEGMENT_CB(seg)]=   mBasePoints[SEGMENT_B(seg)]-mdirb;
    mBasePoints[SEGMENT_CA(seg+1)]=  mBasePoints[SEGMENT_B(seg)]+mdira;
  }
  // root and tip
  mBasePoints[POINT_R]= mBasePoints[SEGMENT_CA(0)];
  mBasePoints[POINT_I]= mBasePoints[SEGMENT_CB(SEGMENTS-1)];
  // doit
  updateCtrls();
  updateFlag();
  updateDoit();
}

// spline interpolation
void GioTrans::convertToSpline(void) {
  FD_DQ("GioTrans::spline(): A"); 
#ifdef USING_SPLINE
  qtap::SplineInterpolation(mBasePoints,POINT_R);
#endif
  FD_DQ("GioTrans::spline(): B"); 
  // doit
  updateCtrls();
  updateFlag();
  updateDoit();
}


// mute
void GioTrans::convertToMute(void) {
  FD_DQ("GioTrans::mute()"); 
  mEditMode=Mute;
  updateData();
}


// set edit mode, incl re-convert
void GioTrans::setEditMode(EditMode mode) {
  // sense unmute
  if(mEditMode==Mute && mode!=Mute) {
    mEditMode=mode;
    updateData();
  }
  // needed for updateCtrls to indicate fixed ctrls
  mEditMode=mode; 
  // doit
  switch(mode) {
  case Free: convertToFree(); break;
  case Line: convertToLine(); break;
  case Polygon: convertToPolygon(); break;
  case Smooth: convertToSmooth(); break;
  case Spline: convertToSpline(); break;
  case Mute: convertToMute();     break; 
  }
  // needed since converters may overwrite mode
  mEditMode=mode; 
  if(mode==Line) mode=Smooth;
}

// retrieve edit mode
GioTrans::EditMode GioTrans::editMode(void) const {return mEditMode;};


// draw paths, controls etc
void GioTrans::paint(
  QPainter *painter, 
  const QStyleOptionGraphicsItem *option,
  QWidget *widget) {
#ifdef FAUDES_DEBUG_VIO
  GioItem::paint(painter,option,widget);
#endif
  // highlite
  if(mHighlite) {
    // cosmetic pen hack .. should move viostyle
    QPen pen=VioStyle::HighlitePen();
    pen.setWidth((int) (0.7*pen.width()));
    pen.setJoinStyle(Qt::SvgMiterJoin);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(mDrawPaths[PATH_ARC]);
    painter->drawPath(mDrawPaths[PATH_ARR]);
  }
  // line segments
  painter->setPen(pRenderOptions->mLinePen);
  painter->setBrush(Qt::NoBrush);
  painter->drawPath(mDrawPaths[PATH_ARC]);
  painter->setPen(Qt::red);
  // arrow 
  painter->setPen(Qt::NoPen); //pRenderOptions->mLinePen);
  painter->setBrush(pRenderOptions->mLineBrush);
  painter->drawPath(mDrawPaths[PATH_ARR]);
  // tick
  painter->setPen(Qt::NoPen);
  painter->setBrush(pRenderOptions->mBodyBrush);
  painter->drawPath(mDrawPaths[PATH_TCK]);
  // tangents
  if(isSelected()) 
  if(mEditMode==Free || mEditMode==Smooth || mEditMode==Spline) {
    painter->setPen(Qt::blue);  
    for(int seg=0; seg<SEGMENTS; seg++) {
      painter->drawLine(mCtrlPoints[SEGMENT_A(seg)],mCtrlPoints[SEGMENT_CA(seg)]);
      painter->drawLine(mCtrlPoints[SEGMENT_CB(seg)],mCtrlPoints[SEGMENT_B(seg)]);
    }
  }
  // label 
  painter->setBrush(pRenderOptions->mLineBrush);
  painter->setPen(Qt::NoPen);
  if(!valid()) painter->setBrush(VioStyle::WarningBrush());
  painter->save();
  painter->translate(mBasePoints[POINT_N]);
  painter->drawPath(mDrawPaths[PATH_NAME00]);
  painter->restore();
  // controls 
  if(isSelected()) painter->drawPath(mCtrlPath); // show invisible items
  GioItem::paintCtrls(painter,option,widget);
}


// fix root and tip controls to touch states 
void GioTrans::fixRootAndTip(void) {
  FD_DQ("GioTrans::fixRootAndTipCtrls:  " << Generator()->TStr(FTrans()) );
  // bail out on invalid points 
  if(mBasePoints.size()<MINPOINTS) return;
  if(mCtrlPoints.size()<MINPOINTS) return;
  // fix root with state a 
  if(GioState* stateA=StateItem(mIdxA)){
    FD_DQ("GioTrans::fixRootAndTipCtrls: fix root at " << stateA->Idx() );
    QPointF pointa=mapToItem(stateA,mBasePoints[POINT_R]);
    QPointF pointb=mapToItem(stateA,mBasePoints[POINT_A]);
    VioStyle::FixArrow(pointa,pointb,stateA->coreShape());
    mBasePoints[POINT_R]=mapFromItem(stateA,pointa);
  }
  // fix tip with state b
  if(GioState* stateB=StateItem(mIdxB)){
    FD_DQ("GioTrans::fixRootAndTipCtrls: fix tip to " << stateB->Idx() );
    /*
    // std procedure
    QPointF pointa=mapToItem(stateB,mBasePoints[POINT_I]);
    QPointF pointb=mapToItem(stateB,mBasePoints[POINT_B]);
    VioStyle::FixArrow(pointa,pointb,stateB->coreShape());
    mBasePoints[POINT_I]=mapFromItem(stateB,pointa);
    */
    // advanced version
    QPointF pointIM1=mapToItem(stateB,mBasePoints[POINT_I-1]);
    QPointF pointI=mapToItem(stateB,mBasePoints[POINT_I]);
    QPointF pointB=mapToItem(stateB,mBasePoints[POINT_B]);
    // 1. make sure, IMP is outside the shape
    if(stateB->coreShape().contains(pointIM1)) {
      VioStyle::FixArrow(pointIM1,pointB,stateB->coreShape());
      pointIM1 = pointB + 1.1* (pointIM1-pointB);
    }
    // 2. if I is to close to B, use IM1 instead
    if(VioStyle::NormF(pointI-pointB)<1) 
      pointI=pointIM1;
    // 3. do the fix
    VioStyle::FixArrow(pointI,pointB,stateB->coreShape());
    mBasePoints[POINT_I]=mapFromItem(stateB,pointI);
    mBasePoints[POINT_I-1]=mapFromItem(stateB,pointIM1);
  }
  // fix tip of pending transition
  if(!StateItem(mIdxB)){
    FD_DQ("GioTrans::fixRootAndTipCtrls: fix tip of pending");
    mBasePoints[POINT_I]=mBasePoints[POINT_B];
  }
  // fix ctrls if there are any
  if(mCtrlPoints.size()>=MINPOINTS) {
    mCtrlPoints[CTRL_R]=mBasePoints[POINT_R];
    mCtrlPoints[CTRL_I]=mBasePoints[POINT_I];
    mCtrlPoints[CTRL_I-1]=mBasePoints[POINT_I-1];
  }
}

// construct draw paths from base points
void GioTrans::updatePaths(void) {
  FD_DQG("GioTrans::updatePaths(): " << FNameEv());
  // fix them tips
  fixRootAndTip();
  // update bezier segments, arc and tick
  pGeneratorConfig->TransArcPath(mDrawPaths[PATH_ARC],mBasePoints.mid(POINT_A),pRenderOptions);
  pGeneratorConfig->TransArrPath(mDrawPaths[PATH_ARR],mBasePoints.mid(POINT_R,POINT_I-POINT_R+1),pRenderOptions);
  pGeneratorConfig->TransTickPath(mDrawPaths[PATH_TCK],
    mBasePoints[POINT_T],mBasePoints[POINT_T+1],mBasePoints[POINT_T+2],mBasePoints[POINT_T+3],pRenderOptions);
  // update control path
  mCtrlPath = QPainterPath(); 
  for(int i=0; i < SEGMENTS; i++) {
    mCtrlPath.moveTo(mBasePoints[SEGMENT_A(i)]);
    mCtrlPath.lineTo(mBasePoints[SEGMENT_CA(i)]);
    mCtrlPath.moveTo(mBasePoints[SEGMENT_B(i)]);
    mCtrlPath.lineTo(mBasePoints[SEGMENT_CB(i)]);
  }
  // update shape
  QPainterPathStroker outline;
  outline.setWidth(pGeneratorConfig->CtrlTolerance() *2);
  mCoreShape= outline.createStroke(mDrawPaths[PATH_ARC]);
  QRectF textrect=mNameRect00.translated(mBasePoints[POINT_N]);
  mCoreShape.addRect(textrect);
  mShape=mCoreShape;
  for(int i=CTRL_A; i<CTRL_B; i++) {
    mShape.addRect(mCtrlPoints[i].x()-3,mCtrlPoints[i].y()-3,6,6);
  }
  mOuterRect=mShape.boundingRect();
}

// construct text related draw paths that dont depend on base
void GioTrans::updateText(void) {
  FD_DQG("GioTrans::updateText()");
  // get flags
  pGeneratorConfig->MapElementOptions(GeneratorModel(), VioElement::FromTrans(FTrans()), pRenderOptions);
  // overwrite invisible
  if(mEditMode==Mute) {
    pRenderOptions->mBodyStyle=GioInvisible;
  }
  //prepare text path with pos 0 
  mDrawPaths[PATH_NAME00]=QPainterPath();
  VioStyle::TextCP(mDrawPaths[PATH_NAME00],QPointF(0,0),
		   pGeneratorConfig->DispEventName(Generator(),mIdxEv));
  mNameRect00= mDrawPaths[PATH_NAME00].boundingRect();
  mNameRect00.adjust(-10,-10,10,10);
}

// construct controls points from base points, incl fixed
void GioTrans::updateCtrls(void) {
  FD_DQG("GioTrans::updateCtrls()");
  mCtrlPoints=mBasePoints;
  mCtrlFixed.clear();
  while(mCtrlFixed.size()<mCtrlPoints.size()) mCtrlFixed.append(false);
  mCtrlFixed[POINT_A]=true;
  mCtrlFixed[POINT_B]=true;
  // fix bezier controls
  if(mEditMode == Polygon || mEditMode == Line || mEditMode == Spline) {
    for(int segment = 0; segment<SEGMENTS; segment++) {
      mCtrlFixed[SEGMENT_CA(segment)]=true;
      mCtrlFixed[SEGMENT_CB(segment)]=true;
    }
  }
  // revers end point editing
  if(mEditMode==Smooth || mEditMode==Spline || mEditMode == Line) {
    mCtrlFixed[CTRL_R]=true;
    mCtrlFixed[CTRL_I]=true;
    mCtrlFixed[CTRL_R+1]=false;
    mCtrlFixed[CTRL_I-1]=false;
  }
}

// construct base points from updated controls
void GioTrans::updateBase(void) {
  mBasePoints=mCtrlPoints;
}



// move one control
bool GioTrans::moveCtrlPos(const QPointF &point) {
  // bail out
  if(mCtrlPointEdit<0 || mCtrlPointEdit>= mCtrlPoints.size()) return false;
  // call base to move this control
  if(!GioItem::moveCtrlPos(point)) return false;
  // fixroot and arrowtip
  updateBase();
  fixRootAndTip();
  // figure segment
  int segment = SEGMENT(mCtrlPointEdit);
  // if its a spline begin or end, spline controls must follow
  if(SEGMENT_A(segment) == mCtrlPointEdit) {
    if(segment>0 && segment<SEGMENTS)
      mCtrlPoints[SEGMENT_CA(segment)] =
          mBasePointsStartPos[SEGMENT_CA(segment)] 
        + mCtrlPoints[SEGMENT_A(segment)] 
        - mBasePointsStartPos[SEGMENT_A(segment)];
    if(segment>0 && segment<SEGMENTS)
      mCtrlPoints[SEGMENT_CB(segment-1)] =
        mBasePointsStartPos[SEGMENT_CB(segment-1)]
        + mCtrlPoints[SEGMENT_B(segment-1)] 
        - mBasePointsStartPos[SEGMENT_B(segment-1)];
  }
  // mode switch: line becomes smooth
  if(mEditMode==Line) mEditMode=Smooth;
  // if it is a smooth curve, fix the other control
  if(mEditMode==Smooth) {
    int iAB=-1, iCThis=-1, iCOther=-1;
    if(SEGMENT_CA(segment) == mCtrlPointEdit && segment>0) {
      iCThis=mCtrlPointEdit;
      iAB=mCtrlPointEdit-1;
      iCOther=mCtrlPointEdit-2;
    }
    if(SEGMENT_CB(segment) == mCtrlPointEdit && segment+1<SEGMENTS) {
      iCThis=mCtrlPointEdit;
      iAB=mCtrlPointEdit+1;
      iCOther=mCtrlPointEdit+2;
    }
    if(iCThis>=0) {
      QPointF pointAB=mBasePointsStartPos[iAB];
      QPointF pointCThis=mBasePointsStartPos[iCThis];
      QPointF pointCOther=mBasePointsStartPos[iCOther];
      QPointF dirThis = mCtrlPoints[iCThis] - mCtrlPoints[iAB];
      qreal lenThis= VioStyle::NormF(dirThis);
      dirThis=VioStyle::NormalizeF(dirThis);
      QPointF dirOther = mCtrlPoints[iAB]-mCtrlPoints[iCOther];
      qreal lenOther= VioStyle::NormF(dirOther);
      if(lenThis>1.0)
        mCtrlPoints[iCOther]=  mCtrlPoints[iAB] + (-1.0) * dirThis * lenOther;
    }
  }
  // if it is a smooth curve or a spline, fix root and tip
  if(mEditMode==Smooth || mEditMode==Spline) {
    if(SEGMENT_CA(0) == mCtrlPointEdit) {
      mBasePoints[POINT_R]= mCtrlPoints[SEGMENT_CA(0)];
      fixRootAndTip();
    } 
    if(SEGMENT_CB(SEGMENTS-1) == mCtrlPointEdit) {
      mBasePoints[POINT_I]= mCtrlPoints[SEGMENT_CB(SEGMENTS-1)];
      fixRootAndTip();
    } 
  }
  // if it is a polygon, adjust splinecontrols 
  if(mEditMode==Polygon || mEditMode==Line) {
    for(int seg = 0; seg < SEGMENTS; seg++){
      QPointF dir= 0.33*(mCtrlPoints[SEGMENT_B(seg)]-mCtrlPoints[SEGMENT_A(seg)]);
      mCtrlPoints[SEGMENT_CA(seg)]=  mCtrlPoints[SEGMENT_A(seg)]+dir;
      mCtrlPoints[SEGMENT_CB(seg)]=  mCtrlPoints[SEGMENT_B(seg)]-dir;
    }
  }
  // if it is a spline, adjust controls 
#ifdef USING_SPLINE
  if(mEditMode==Spline) {
    qtap::SplineInterpolation(mCtrlPoints,CTRL_R);
  }
#endif
  return true;
}

// record ctrl to move (eg via mouse-move)
void GioTrans::moveCtrlStart(int ctrl) {
  GioItem::moveCtrlStart(ctrl);
} 

// end moving (incl delete if need be)
void GioTrans::moveCtrlStop(void) {
  int ctrl=mCtrlPointEdit;
  GioItem::moveCtrlStop();
  if(ctrl<0 || ctrl >= mCtrlPoints.size() ) 
    return;
  if(mCtrlFixed[ctrl]) return;
  /*
  if(ctrl>0 && ctrl<mCtrlPoints.size()-1) { 
    QPointF where=mCtrlPoints[ctrl];
    QRectF whererect= QRectF(where.x()-GIO_TOLERANCE,where.y()-GIO_TOLERANCE,GIO_TOLERANCE*2,GIO_TOLERANCE*2);
    QPainterPath segpath;
    segpath.moveTo(mCtrlPoints[ctrl-1]);
    segpath.lineTo(mCtrlPoints[ctrl+1]);
    segpath.moveTo(mCtrlPoints[ctrl-1]);
    if(segpath.intersects(whererect))  
      delCtrl(ctrl);
    updateFlag();
  }
  */
}


// handle my events: mouse press
void GioTrans::mousePressEvent(QGraphicsSceneMouseEvent *event) {
  FD_DQ("GioTrans::mousePress"); 
  if(!isSelected()) return;
  // figure where we are: control point
  const QPointF where=event->pos();
  int ctrlpoint=whichCtrlPoint(where);
  // edditing control point?
  if(ctrlpoint>=0)
  if(!mCtrlFixed[ctrlpoint]) 
  if(event->button() == Qt::LeftButton) 
  if(event->modifiers() == Qt::NoModifier)
  {
    moveCtrlStart(ctrlpoint);
    updateDoit();
    event->accept();
    return;  
  }
  // moving item by fixed control point 
  if(ctrlpoint>=0)
  if(mCtrlFixed[ctrlpoint]) 
  if(event->button() == Qt::LeftButton) 
  if(event->modifiers() == Qt::NoModifier)
  {
    QGraphicsItem::mousePressEvent(event);
    event->accept();
    return;
  }
  // removing a control point
  if(ctrlpoint>=0)
  if(!mCtrlFixed[ctrlpoint]) 
  if(event->button() == Qt::LeftButton) 
  if(event->modifiers() & Qt::ShiftModifier)
  {  
    FD_DQ("GioItem::mousePress: remove control " << ctrlpoint); 
    int segment=SEGMENT(ctrlpoint);
    if(ctrlpoint==SEGMENT_A(segment)) {
      mergeSplineSegment(segment);
    }
    event->accept();
    return;
  }
  // figure where we are: control segment
  int ctrlsegment=whichSplineSegment(where);
  // adding a control point
  if(ctrlpoint<0)
  if(ctrlsegment>=0) 
  if(event->button() == Qt::LeftButton) 
  if(event->modifiers() & Qt::ShiftModifier)
  {
    FD_DQ("GioTrans::mousePress: add segment " << ctrlsegment); 
    splitSplineSegment(ctrlsegment,where);
    moveCtrlStart(SEGMENT_B(ctrlsegment));
    event->accept();
    return;
  }
  // pass on event to qt
  QGraphicsItem::mousePressEvent(event);
}


// handle my events: mouse release
void GioTrans::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
  GioItem::mouseReleaseEvent(event);
}

// handle my events: mouse move
void GioTrans::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
  GioItem::mouseMoveEvent(event);
}


// closest qualified spline segment (or -1 if no such)
int GioTrans::whichSplineSegment(const QPointF &point) {
  for(int i=0; i< SEGMENTS; i++) {
    QPainterPath segpath;
    segpath.moveTo(mBasePoints[SEGMENT_A(i)]);
    segpath.cubicTo(mBasePoints[SEGMENT_CA(i)], mBasePoints[SEGMENT_CB(i)], mBasePoints[SEGMENT_B(i)]);
    QPainterPathStroker outline;
    outline.setWidth(pGeneratorConfig->CtrlTolerance() *2);
    segpath= outline.createStroke(segpath);
    if(segpath.contains(point))  return i;
  }
  return -1;
}

// closeset qualified control (or -1 if no such)
int GioTrans::whichCtrlPoint(const QPointF &point, int a, int b) {
  // use base with order priority
  int res=GioItem::whichCtrlPoint(point,a,b);
  if(res<0) return res;
  // sucessor ctrl candidate
  int next = res+1;
  FD_DQ("GioTrans::whichCtrlPoint(): alt  " << res ); 
  if(next >= mCtrlPoints.size()) return res;   // no successor
  if(SEGMENTS <=1) return res;                 // no more than one segment
  if(next % 3 != (POINT_R +1) % 3) return res; // successor ia not a CA point
  if(mCtrlFixed[next]) return res;             // successor not edidable
  if(a >=0 && next < a) return res;            // out of range
  if(b >=0 && next >b) return res;             // out of range
  // check distance
  FD_DQ("GioTrans::whichCtrlPoint(): alt  " << next ); 
  QPoint diff1=point.toPoint()-mCtrlPoints[res].toPoint();
  int  dist1=diff1.manhattanLength();
  QPoint diff2=point.toPoint()-mCtrlPoints[next].toPoint();
  int  dist2=diff2.manhattanLength();
  // prefer CA point over A point
  if(dist2 <= dist1) res=next;
  return res;
}

// split segment 
void GioTrans::splitSplineSegment(int segment, const QPointF& point) {
  FD_DQ("GioTrans::splitSplineSegment " << segment ); 
  if(segment < 0) return;
  if(segment>= SEGMENTS) return;
  if(mEditMode==Line) mEditMode=Spline;
  QPointF dir = mBasePoints[SEGMENT_B(segment)]- mBasePoints[SEGMENT_A(segment)];
  mBasePoints.insert(SEGMENT_CB(segment),point-0.33*dir);
  mBasePoints.insert(SEGMENT_B(segment),point);
  mBasePoints.insert(SEGMENT_CA(segment+1),point+0.33*dir);
  setEditMode(mEditMode);
  FD_DQ("GioTrans::splitSplineSegment: done"); 
}


// merge segment 
void GioTrans::mergeSplineSegment(int segment) {
  FD_DQ("GioTrans::mergeSplineSegment " << segment ); 
  if(segment < 1) return;
  if(segment>= SEGMENTS) return;
  if(SEGMENTS<=1) return;
  mBasePoints.removeAt(SEGMENT_CA(segment));
  mBasePoints.removeAt(SEGMENT_A(segment));
  mBasePoints.removeAt(SEGMENT_CB(segment-1));
  setEditMode(mEditMode);
  FD_DQ("GioTrans::mergeSplineSegment: done"); 
}


// insert a control 
void GioTrans::insCtrl(int ctrl, const QPointF& point) {
  FD_DQ("GioTrans::insCtrl " << ctrl ); 
  if(ctrl<=CTRL_A) return;
  if(ctrl>CTRL_B) return;
  GioItem::insCtrl(ctrl,point);
}

// delete a control
void GioTrans::delCtrl(int ctrl) {
  FD_DQ("GioTrans::delCtrl " << ctrl ); 
  if(mCtrlPoints.size()<=MINPOINTS) return;
  if(ctrl<=CTRL_A) return;
  if(ctrl>=CTRL_B) return;
  GioItem::delCtrl(ctrl);
}


