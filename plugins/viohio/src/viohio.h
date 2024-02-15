/* viohio.h  - viodes hiosys plugin */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2009 Thomas Moor;

*/

#ifndef FAUDES_VIOHIO_H
#define FAUDES_VIOHIO_H

// std includes
#include "libviodes.h"
#include "libviogen.h"

// interface class
class VioHiosysPlugin : 
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
