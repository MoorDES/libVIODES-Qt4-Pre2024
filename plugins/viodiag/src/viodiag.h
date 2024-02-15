/* viodiag.h  - viodes diagnosis plugin */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2010 Thomas Moor

*/

#ifndef FAUDES_VIODIAG_H
#define FAUDES_VIODIAG_H

// std includes
#include "libviodes.h"
#include "libviogen.h"

// interface class
class VioDiagnosisPlugin : 
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
