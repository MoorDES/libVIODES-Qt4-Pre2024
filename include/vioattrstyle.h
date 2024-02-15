/* vioattrstyle.h  - vioattr configuration */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006, 2007  Thomas Moor, Klaus Schmidt, Sebastian Perk

*/



#ifndef FAUDES_VIOATTRSTYLE_H
#define FAUDES_VIOATTRSTYLE_H

#include "viostyle.h"
#include "viotypes.h"



/*
 ************************************************
 ************************************************

 A VioBooleanProperty is a boolean component of
 a faudes attribute. Typically, it is a bit in the
 faudes flag word. The class provides parameter data
 to map the property to/from a bit in the vio flag
 word. Advanced attributes may implement a more
 sophisticated mapping.

 The faudes type defaults to "AttributeFlags".

 ************************************************
 ************************************************
 */

class VIODES_API VioBooleanProperty {

public:

  // construct 
  VioBooleanProperty(void);

  // construct from values
  VioBooleanProperty(
    const QString& name,
    const QString& shortname,
    bool editable,
    faudes::fType fmask, 
    faudes::fType fvalue);

  // construct from values (with clear mask/value)
  VioBooleanProperty(
    const QString& name,
    const QString& shortname,
    bool editable,
    faudes::fType fsmask, 
    faudes::fType fsvalue, 
    faudes::fType fcmask, 
    faudes::fType fcvalue);

  // read from tokenreader (not implemented)
  bool Read(faudes::TokenReader& rTr);

  // name of property
  QString mName;
  QString mShortName;

  // user editable
  bool mEditable;

  // query from faudes flag
  bool Test(const faudes::fType& fflags) const;  

  // set/clr in faudes flag
  void Set(faudes::fType& fflags) const;  
  void Clr(faudes::fType& fflags) const;  

  // value type 
  typedef enum { True, False, Partial, Void } ValueType;

  // convert to Qt type
  static Qt::CheckState State(ValueType val);
  static ValueType Value(Qt::CheckState val);

protected:

  // faudes mask that indicates property (or 0/f)
  faudes::fType mFSetMask;
  faudes::fType mFSetValue;

  // faudes mask that indicates absence of property (or 0/f)
  faudes::fType mFClrMask;
  faudes::fType mFClrValue;

};


/*
 ************************************************
 ************************************************

 VioAttributeStyle provides configuration for the
 vioattr plugin of libviodes. The class is derived from
 VioStyle and adds a registry of named boolean properties
 mapped to/from faudes flags.

 On construction, the actual data will be read from 
 the same file as the global configuration VioStyle 
 or set to default values. However, there may exist 
 multiple VioAttributeStyle instnances that differ 
 programmatically. 

 ************************************************
 ************************************************
 */


class VIODES_API VioAttributeStyle : public VioStyle {

public:

  // constructor (optionally reads section from config file)
  VioAttributeStyle(const QString& ftype="AttributeFlags");
  virtual ~VioAttributeStyle(void) {};

  // register named boolean properties
  void InsertBooleanProperty(const VioBooleanProperty& boolprop);

  // clear properies
  void ClearBooleanProperties(void);


  // access property registry
  const QList<VioBooleanProperty>& BooleanProperties(void) const {
    return mBooleanProperties;};

protected:

  // load hard-coded default 
  void Initialise(const QString& ftype);

  // configuration section in config file
  void ReadFile(const QString& filename="");

  // property registry
  QList<VioBooleanProperty> mBooleanProperties;

};

/*
 ************************************************
 ************************************************

 Register all built in attributes, as there are

 AttributeFlags:       none
 AttributeGlobalFlags: none
 AttributeTransFlags:  none
 AttributeStateFlags:  initial/marked
 AttributeEventFlags:  controllable,observable,forcible


 ************************************************
 ************************************************
 */

void VIODES_API VioRegisterAttributes(void);

/*
 ************************************************
 ************************************************

 Register all faudes::AttributeFlags derivates
 found in a config file

 ************************************************
 ************************************************
 */


void VIODES_API VioRegisterAttributeFlagsStyles(const QString& filename="");

#endif
