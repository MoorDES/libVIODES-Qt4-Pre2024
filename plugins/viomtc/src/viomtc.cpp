/* viomtc.cpp  - viodes mtc plugin */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2009 Ruediger Berndt, Thomas Moor;

*/

#include "viomtc.h"
#include "viomtcstateattr.h"
#include "viomtcgenstyle.h"
#include "viomtcgenerator.h"

// advertise my plugin
Q_EXPORT_PLUGIN2(viomtc, VioMtcPlugin)

// tell name of this plugin
QString VioMtcPlugin::Name(void) {
  return QString("VioMtc-")+QString(VIODES_VERSION);
}

// register my base types
void VioMtcPlugin::RegisterTypes(void) {
  FD_DQT("VioMtcPlugin::RegisterTypes(): mtc attribute ");
  
  // mtc state attribute: manually register with libfaudes
  faudes::TypeRegistry::G()->Insert<faudes::AttributeColoredState>("AttributeColoredState"); 
  // mtc state attribute: create style
  VioAttributeStyle* pstyle = new VioAttributeStyle("AttributeColoredState");
  // mtc state attribute: have some flags, too
  pstyle->InsertBooleanProperty(VioBooleanProperty(
    "Initial", "I", true, 0x80000000,0x80000000));
  pstyle->InsertBooleanProperty(VioBooleanProperty(
    "Marked", "M", true, 0x40000000,0x40000000));
  // mtc state attribute: create prototype
  VioAttributeMtcStateModel* pproto= new VioAttributeMtcStateModel(0,pstyle); 
  // mtc state attribute: register with libviodes
  VioTypeRegistry::Insert(pproto);

  // mtc generator: have a style
  VioGeneratorStyle* mstyle= new VioMtcGeneratorStyle("MtcSystem");
  // mtc generator: construct prototype
  VioMtcGeneratorModel* mproto= new VioMtcGeneratorModel(0,mstyle); 
  // mtc generator: register with libviodes
  VioTypeRegistry::Insert(mproto);
}


// register more types
void VioMtcPlugin::FinaliseTypes(void) {
  FD_DQT("VioMtcPlugin::FinaliseTypes()");
}
