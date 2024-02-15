/* violua.h  - viodes lua plugin */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2010 Thomas Moor

*/

#ifndef FAUDES_VIOLUA_H
#define FAUDES_VIOLUA_H

// std includes
#include "libviodes.h"

// interface class
class VioLuaPlugin : 
  public QObject,
  public VioTypePlugin {

  Q_OBJECT
  Q_INTERFACES(VioTypePlugin) 

public:

  // tell name of this plugin
  QString Name(void);

  // register aditional types  
  void RegisterTypes(void);

  // register more types 
  void FinaliseTypes(void);
};

#endif
