/* violuastyle.h  - vionameset configuration */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006, 2009  Thomas Moor, Klaus Schmidt, Sebastian Perk

*/



#ifndef FAUDES_VIOLUASTYLE_H
#define FAUDES_VIOLUASTYLE_H

#include "libviodes.h"

// debugging: all name set
#ifdef FAUDES_DEBUG_VIO_LUA
#define FD_DQL(message) FAUDES_WRITE_CONSOLE("FAUDES_VIO_LUA: " << message)
#else
#define FD_DQL(message)
#endif

// win32 dll symbol export/import
#ifndef VIOLUA_API
#ifdef VIOLUA_BUILD_LIB
#define VIOLUA_API Q_DECL_EXPORT
#else
#define VIOLUA_API Q_DECL_IMPORT
#endif
#endif



/*
 ************************************************
 ************************************************

 VioLuaStyle  is derived from VioStyle to
 configure the epresentation of a VioLuaFunctionModel

 On construction, the actual data will be read from 
 the same file as the global configuration VioStyle 
 or set to default values. 

 Currently, the lua style only provides one flag to
 distinuish plain script and functions.

 ************************************************
 ************************************************
 */


class VIOLUA_API VioLuaStyle : public VioStyle {

public:

  // construct/destruct
  VioLuaStyle(const QString& ftype="NameSet");
  ~VioLuaStyle(void) {};

  // my flag 
  bool mPlainScript;


protected:

  // load hard-coded default
  virtual void Initialise(const QString& ftype);

  // configuration from file (dummy)
  virtual void ReadFile(const QString& filename="");

};



#endif
