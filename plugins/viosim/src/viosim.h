/* viosim.h  - viodes sim plugin */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2010 Thomas Moor

*/

#ifndef FAUDES_VIOSIM_H
#define FAUDES_VIOSIM_H

// std includes
#include "libviodes.h"

// interface class
class VioSimPlugin : 
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
