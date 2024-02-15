/* viohio.cpp  - viodes hiosys plugin */


/*
   Graphical IO for FAU Discrete Event Systems Library (libFAUDES)

   Copyright (C) 2009 Thomas Moor;

*/

#include "viohio.h"

// advertise my plugin
Q_EXPORT_PLUGIN2(viohio, VioHiosysPlugin)

// tell name of this plugin
QString VioHiosysPlugin::Name(void) {
  return QString("VioHiosys-")+QString(VIODES_VERSION);
}

// register base types: all from config file
void VioHiosysPlugin::RegisterTypes(void) {
  FD_DQT("VioHiosysPlugin::RegisterTypes()");
}


// register my types: all from config file
void VioHiosysPlugin::FinaliseTypes(void) {
  FD_DQT("VioHiosysPlugin::FinaliseTypes()");
}
