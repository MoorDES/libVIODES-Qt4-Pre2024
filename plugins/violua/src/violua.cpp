/* violua.cpp  - viodes luaulator plugin */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2010 Thomas Moor

*/

#include "violua.h"
#include "violuastyle.h"
#include "violuafunction.h"

// advertise my plugin
Q_EXPORT_PLUGIN2(violua, VioLuaPlugin)

// tell name of this plugin
QString VioLuaPlugin::Name(void) {
  return QString("VioLua-")+QString(VIODES_VERSION);
}

// register my types
void VioLuaPlugin::RegisterTypes(void) {
  FD_DQL("VioLuaPlugin::RegisterTypes(): lua function definition");  
  // lua definition: manually register with libfaudes
  faudes::TypeRegistry::G()->Insert<faudes::LuaFunctionDefinition>("LuaFunctionDefinition"); 
  // lua definition: create style
  VioLuaStyle* fstyle = new VioLuaStyle("LuaFunctionDefinition");
  // lua definition: create prototype
  VioLuaFunctionModel* fproto= new VioLuaFunctionModel(0,fstyle); 
  // lua definition: register with libviodes
  VioTypeRegistry::Insert(fproto);

  // lua script: manually register with libfaudes
  faudes::TypeRegistry::G()->Insert<faudes::LuaFunctionDefinition>("LuaPlainScript"); 
  // lua script: create style
  VioLuaStyle* sstyle = new VioLuaStyle("LuaPlainScript");
  // lua script: create prototype
  VioLuaFunctionModel* sproto= new VioLuaFunctionModel(0,sstyle); 
  // lua script: register with libviodes
  VioTypeRegistry::Insert(sproto);


  FD_DQL("VioLuaPlugin::RegisterTypes(): done ");
}


// register my types
void VioLuaPlugin::FinaliseTypes(void) {
  FD_DQL("VioLuaPlugin::FinaliseTypes()");
}
