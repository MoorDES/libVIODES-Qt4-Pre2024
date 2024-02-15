/* vionsetstyle.cpp  - vionset configuration */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006, 2009  Thomas Moor, Klaus Schmidt, Sebastian Perk

*/


#include "violuastyle.h"


/*
 ************************************************
 ************************************************
 ************************************************

 Implementation: VioLuaStyle 

 ************************************************
 ************************************************
 ************************************************
 */


// constructor
VioLuaStyle::VioLuaStyle(const QString& ftype) : VioStyle()
{
  FD_DQL("VioLuaStyle::VioLuaStyle(): ftype \"" << VioStyle::StrFromQStr(ftype) << "\"");
  // configure 
  Initialise(ftype);
  ReadFile();
  FD_DQL("VioLuaStyle::VioLuaStyle(): done");
};

// set defaults
void VioLuaStyle::Initialise(const QString& ftype){
  // record type
  mFaudesType=ftype;
  // set my flag
  mPlainScript=true;
  if(ftype=="LuaFunctionDefinition") mPlainScript=false;
  else if(ftype=="LuaPlainScript") mPlainScript=true;
  else FD_WARN("VioLuaStyle: unknown style");
};

// load from file (dummy)
void VioLuaStyle::ReadFile(const QString& filename){
}



