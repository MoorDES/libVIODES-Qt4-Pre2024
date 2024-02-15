/* viosim.cpp  - viodes simulator plugin */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2010 Thomas Moor

*/

#include "viosim.h"
#include "viosimcondattr.h"

// advertise my plugin
Q_EXPORT_PLUGIN2(viosim, VioSimPlugin)

// tell name of this plugin
QString VioSimPlugin::Name(void) {
  return QString("VioSim-")+QString(VIODES_VERSION);
}

// register my types
void VioSimPlugin::RegisterTypes(void) {
  FD_DQT("VioSimPlugin::RegisterTypes(): sim condition attribute ");
  
  // sim cond attribute: manually register with libfaudes
  faudes::TypeRegistry::G()->Insert<faudes::AttributeSimCondition>("AttributeSimCondition"); 
  // sim cond attribute: create style
  VioAttributeStyle* pstyle = new VioAttributeStyle("AttributeSimCondition");
  // sim cond attribute: have some flags, too
  pstyle->InsertBooleanProperty(VioBooleanProperty(
    "Enabled", "E", true, 0x01,0x01));
  pstyle->InsertBooleanProperty(VioBooleanProperty(
    "Breakpoint", "B", true, 0x02,0x02));
  // cond attribute: create prototype
  VioAttributeSimCondModel* pproto= new VioAttributeSimCondModel(0,pstyle); 
  // sim state attribute: register with libviodes
  VioTypeRegistry::Insert(pproto);
  // sim condition set:  manually register with libfaudes
  faudes::TypeRegistry::G()->Insert<faudes::SimConditionSet>("SimConditionSet"); 
  // note: the actual vio type is configures as a styled nameset from the config file
  FD_DQT("VioSimPlugin::RegisterTypes(): done ");
}


// register my types
void VioSimPlugin::FinaliseTypes(void) {
  FD_DQT("VioSimPlugin::FinaliseTypes()");
  // destool version of sim conditions ... 
  // 1. have simconditionset under new name with faudes rego
  faudes::Type* fproto=faudes::TypeRegistry::G()->NewObject("SimConditionSet");
  faudes::TypeRegistry::G()->Insert(fproto,"DEST_SimConditionSet");
  // 2. create style
  VioNameSetStyle* nstyle = new VioNameSetStyle("DEST_SimConditionSet");
  nstyle->mLayoutFlags |= VioNameSetStyle::Decorate;
  nstyle->mLayoutFlags &= ~VioNameSetStyle::Properties;
  nstyle->mLayoutFlags |= VioNameSetStyle::PropV;
  nstyle->mHeader = "Conditions";
  nstyle->mDefSymbol = "cond";
  nstyle->mAttribute = qobject_cast<VioAttributeModel*>(VioTypeRegistry::NewModel("AttributeSimCondition"));
  if(!nstyle->mAttribute) {
    FD_DQT("VioSimPlugin::FinaliseTypes(): fatal error on SimCondition");
    return;
  }
  // 3. create and register vio prototype
  VioNameSetModel* nproto = new VioNameSetModel(0,nstyle);
  VioTypeRegistry::Insert(nproto);
}
