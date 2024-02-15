/* vioelement.h  - elementary type  */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2009  Thomas Moor, Ruediger Berndt

*/

#ifndef FAUDES_VIOELEMENT_H
#define FAUDES_VIOELEMENT_H

// std includes
#include "viostyle.h"



/*
 ************************************************
 ************************************************

 A VioElement is a union style data storage
 that takes one value from an elementary faudes
 type, ie state, event or transition. It is used
 to keep signal/slot interface among VioWidgets
 and VioModels consise.

 It is intentionally lightwight, eg no QObject, since
 it is used in lists and maps for internal qt style
 representation of faudes sets.

 ************************************************
 ************************************************
 */

class VIODES_API VioElement {


public:

  // elementary type enum
  typedef enum { EVoid, ETrans, EState, EEvent, ELine} EType;

  // construct/destruct
  VioElement(void);
  VioElement(const VioElement& src);
  const VioElement& operator=(const VioElement& src);
  ~VioElement(void);

  // static constructors
  static VioElement FromTrans(const faudes::Transition& ftrans=faudes::Transition(0,0,0)); 
  static VioElement FromState(const faudes::Idx& fstate=0); 
  static VioElement FromEvent(const faudes::Idx& fevent=0); 
  static VioElement FromLine(const int& line=0); 
  static VioElement FromType(const VioElement::EType etype=EVoid); 
  
  // get element type
  EType Type(void) const;

  // set value
  void Void(void);
  void Trans(const faudes::Transition& ftrans); 
  void State(const faudes::Idx& fstate); 
  void Event(const faudes::Idx& fevent); 
  void Line(const int& fevent); 

  // get value (todo: have const ref version)
  faudes::Transition Trans(void) const;
  faudes::Idx State(void) const;
  faudes::Idx Event(void) const;
  int Line(void) const;

  // untyped access
  faudes::Idx X1(void) const;
  faudes::Idx Ev(void) const;
  faudes::Idx X2(void) const;
 
  // tell faudes elementary type
  bool IsVoid(void) const;
  bool IsTrans(void) const;
  bool IsState(void) const;
  bool IsEvent(void) const;
  bool IsLine(void) const;

  // valid aka nonzero
  bool IsValid(void) const;

  // sort operator  
  bool operator<(const VioElement& rhs) const;

  // eauality test
  bool operator==(const VioElement& rhs) const;

  // serialize elements: raw indexes
  QDataStream& DoWrite(QDataStream& out) const;
  QDataStream& DoRead(QDataStream& in);

  // serialize elements: wrt generator for file io
  QDataStream& DoWrite(QDataStream& out, const faudes::vGenerator* gen) const;
  QDataStream& DoRead(QDataStream& in, const faudes::vGenerator* gen);

  // faudes style debug string
  std::string Str(const faudes::vGenerator* gen=0) const;

  // faudes style debug string for etype
  std::string static TypeStr(EType etype);

protected:
  
  // my type
  EType mEType;

  // my value
  faudes::Transition mFTrans;

};

// use vioelement in qt signal/slot connection
// (registration is done in VioTypeRegistry::Initialise)
Q_DECLARE_METATYPE(VioElement)

// serialize lists etc
QDataStream& operator<<(QDataStream& out, const VioElement& elem);
QDataStream& operator>>(QDataStream& in , VioElement& elem);



#endif
