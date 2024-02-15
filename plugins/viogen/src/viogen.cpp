/* viogen.cpp  - viodes generator widget plugin */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2009 Ruediger Berndt, Thomas Moor;

*/

#include "viogen.h"
#include "viogenstyle.h"
#include "vionsetstyle.h"
#include "viogenerator.h"

// advertise my plugin
Q_EXPORT_PLUGIN2(viogen, VioGeneratorPlugin)

// tell name of this plugin
QString VioGeneratorPlugin::Name(void) {
  return QString("VioGenerator-")+QString(VIODES_VERSION);
}

// register my types
void VioGeneratorPlugin::RegisterTypes(void) {
  FD_DQT("VioGeneratorPlugin::RegisterTypes()");
  // built in (gets overwritten from config file)
  VioGeneratorStyle* stdgenstyle;
  VioGeneratorModel* genproto;
  stdgenstyle= new VioGeneratorStyle("Generator");
  genproto= new VioGeneratorModel(0,stdgenstyle); 
  VioTypeRegistry::Insert(genproto);
  stdgenstyle= new VioGeneratorStyle("System");
  genproto= new VioGeneratorModel(0,stdgenstyle); 
  VioTypeRegistry::Insert(genproto);
}


// more from config file
void VioGeneratorPlugin::FinaliseTypes(void) {
  FD_DQT("VioGeneratorPlugin::FinaliseTypes()");
  // read config file
  VioRegisterGeneratorStyles();
}
