/* viomtc.h  - viodes mtc plugin */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2009 Ruediger Berndt, Thomas Moor;

*/

#ifndef FAUDES_VIOMTC_H
#define FAUDES_VIOMTC_H

// std includes
#include "libviodes.h"
#include "libviogen.h"

// interface class
class VioMtcPlugin : 
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
