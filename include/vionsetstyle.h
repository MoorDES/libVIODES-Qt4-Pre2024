/* vionsetstyle.h  - vionameset configuration */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006, 2009  Thomas Moor, Klaus Schmidt, Sebastian Perk

*/



#ifndef FAUDES_VIONSETSTYLE_H
#define FAUDES_VIONSETSTYLE_H

#include "viostyle.h"
#include "vioattrstyle.h"
#include "vioattribute.h"

// debugging: all name set
#ifdef FAUDES_DEBUG_VIO_NAMESET
#define FD_DQN(message) FAUDES_WRITE_CONSOLE("FAUDES_VIO_NAMESET: " << message)
#else
#define FD_DQN(message)
#endif




/*
 ************************************************
 ************************************************

 VioNameSetStyle is derived from VioStyle to
 configure the epresentation of a VioNameSetModel

 On construction, the actual data will be read from 
 the same file as the global configuration VioStyle 
 or set to default values. 

 ************************************************
 ************************************************
 */


class VIODES_API VioNameSetStyle : public VioStyle {

public:

  // construct/destruct
  VioNameSetStyle(const QString& ftype="NameSet");
  ~VioNameSetStyle(void) {};

  // attribute prototype 
  VioAttributeModel* mAttribute;

  // nameset view layout options
  typedef enum { 
    LayoutMask=0x00ff,   // exclusive base layouts
    NameSet=1,           // std  layout
    OptionMask=0xff00,   // options   
    Properties=0x0300,   // have my own property view
    PropH=     0x0100,   // split horizontally
    PropV=     0x0200,   // split vertically
    Decorate=  0x1000    // have things in group boxes
  } Layout;

  // actual layout
  int mLayoutFlags;
  QString mHeader;
  QString mDefSymbol;


protected:

  // load hard-coded default
  virtual void Initialise(const QString& ftype);

  // configuration from file
  virtual void ReadFile(const QString& filename="");

};


/*
 ************************************************
 ************************************************

 Register all built in name sets

 EventSet:   no attributes
 Alphabet:   AttributeCFlags


 ************************************************
 ************************************************
 */

void VIODES_API VioRegisterNameSets(void);

/*
 ************************************************
 ************************************************

 Register all faudes::NameSet derivates
 found in a config file

 ************************************************
 ************************************************
 */


void VIODES_API VioRegisterNameSetStyles(const QString& filename="");

#endif
