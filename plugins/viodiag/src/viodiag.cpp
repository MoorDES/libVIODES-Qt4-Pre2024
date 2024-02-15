/* viodiag.cpp  - viodes diagnoser plugin */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2010 Thomas Moor.

*/

#include "viodiag.h"
#include "viodiagstateattr.h"
#include "viodiaggenstyle.h"

// advertise my plugin
Q_EXPORT_PLUGIN2(viodiag, VioDiagnosisPlugin)

// tell name of this plugin
QString VioDiagnosisPlugin::Name(void) {
  return QString("VioDiag-")+QString(VIODES_VERSION);
}

// register my types
void VioDiagnosisPlugin::RegisterTypes(void) {
  FD_DQT("VioDiagnosisPlugin::RegisterTypes(): diagnoser state attribute ");
  
  // diagnoser state attribute: manually register with libfaudes
  faudes::TypeRegistry::G()->Insert<faudes::AttributeDiagnoserState>("AttributeDiagnoserState");
  // diagnoser state attribute: create style
  VioAttributeStyle* pstyle = new VioAttributeStyle("AttributeDiagnoserState");
  // diagnoser state attribute: have std state flags, too
  pstyle->InsertBooleanProperty(VioBooleanProperty(
    "Initial", "I", false, 0x80000000,0x80000000));
  pstyle->InsertBooleanProperty(VioBooleanProperty(
    "Marked", "M", false, 0x40000000,0x40000000));
  // diagnoser state attribute: create prototype
  VioAttributeDiagStateModel* pproto= new VioAttributeDiagStateModel(0,pstyle); 
  // diagnose state attribute: register with libviodes
  VioTypeRegistry::Insert(pproto);

  // diagnoser: have a style
  VioGeneratorStyle* dstyle= new VioDiagGeneratorStyle("Diagnoser");
  // diagnoser: construct prototype
  VioGeneratorModel* dproto= new VioGeneratorModel(0,dstyle); 
  // mtc generator: register with libviodes
  VioTypeRegistry::Insert(dproto);

  FD_DQT("VioDiagnosisPlugin::RegisterTypes(): done ");
}


// register my types
void VioDiagnosisPlugin::FinaliseTypes(void) {
  FD_DQT("VioDiagnosisPlugin::FinaliseTypes()");
}
