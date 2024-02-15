/* vionsetstyle.cpp  - vionset configuration */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006, 2009  Thomas Moor, Klaus Schmidt, Sebastian Perk

*/


#include "vioregistry.h"
#include "vionsetstyle.h"
#include "vionameset.h"


/*
 ************************************************
 ************************************************
 ************************************************

 Implementation: VioNameSetStyle 

 ************************************************
 ************************************************
 ************************************************
 */


// constructor
VioNameSetStyle::VioNameSetStyle(const QString& ftype) : VioStyle()
{
  FD_DQN("VioNameSetStyle::VioNameSetStyle(): ftype \"" << VioStyle::StrFromQStr(ftype) << "\"");
  // configure 
  Initialise(ftype);
  ReadFile();
  FD_DQN("VioNameSetStyle::VioNameSetStyle(): done");
};

// set defaults
void VioNameSetStyle::Initialise(const QString& ftype){
  // record type
  mFaudesType=ftype;
  // attribute prototype
  mAttribute=qobject_cast<VioAttributeModel*>(VioTypeRegistry::NewModel("AttributeEventFlags"));  
  if(!mAttribute) {
    std::stringstream errstr;
    errstr << "VioDes RTI lookup error when defining " << VioStyle::StrFromQStr(ftype) << " nameset style";
    throw faudes::Exception("VioNameSetStyle::Initialise", errstr.str(), 48);
  }
  // layout
  mLayoutFlags= NameSet | PropH;
  mHeader="Symbol";
  mDefSymbol="sy";
};

// load from file
void VioNameSetStyle::ReadFile(const QString& filename){
  FD_DQN("VioNameSetStyle::ReadFile()");
  // figure my section and get token reader
  if(mFaudesType=="") return;
  faudes::TokenReader* trp=NewStyleReader(mFaudesType+"Style", filename);
  std::string fsection=VioStyle::StrFromQStr(mFaudesType+"Style");
  if(!trp) return;
  // loop my section
  while(!trp->Eos(fsection)){
    faudes::Token token;
    trp->Peek(token);
    if(token.Type()!=faudes::Token::Begin) { trp->Get(token); continue; }
    // read my subsections: attributes
    if(token.StringValue()=="Attribute") {
      FD_DQN("VioNameSetStyle::Config: setting up attribute prototypes ");
      trp->ReadBegin("Attribute");
      if(mAttribute) delete mAttribute;
      mAttribute=qobject_cast<VioAttributeModel*>(VioTypeRegistry::NewModel(VioStyle::QStrFromStr(trp->ReadString())));  
      if(!mAttribute) {
        std::stringstream errstr;
        errstr << "VioDes RTI lookup error when defining " << VioStyle::StrFromQStr(mFaudesType) << " nameset style";
        throw faudes::Exception("VioNameSetStyle::ReadFile", errstr.str(), 48);
      }     
      trp->ReadEnd("Attribute");
      continue;
    }
    // read my subsections: edit functions
    if(token.StringValue()=="EditFunctions") {
      trp->ReadBegin("EditFunctions");
      VioEditFunction vfnct;
      while(vfnct.Read(*trp)) InsEditFunction(vfnct);
      trp->ReadEnd("EditFunctions");
      continue;
    }
    // read my subsections: header
    if(token.StringValue()=="Header") {
      trp->ReadBegin("Header");
      mHeader=VioStyle::QStrFromStr(trp->ReadString());
      trp->ReadEnd("Header");
      continue;
    }
    // read my subsections: def symbol
    if(token.StringValue()=="DefSymbol") {
      trp->ReadBegin("DefSymbol");
      mDefSymbol=VioStyle::QStrFromStr(trp->ReadString());
      trp->ReadEnd("DefSymbol");
      continue;
    }
    // skip unkown subsection
    trp->ReadBegin(token.StringValue());
    trp->ReadEnd(token.StringValue());
  }
  // done
  trp->ReadEnd(fsection);
  delete trp;
  FD_DQN("VioNameSetStyle::ReadFile(): done");
}




// register built in name stes
void VioRegisterNameSets(void) {
  FD_DQN("VioRegisterNameSets(): eventset and alphabet");
  VioNameSetStyle* stdsetstyle;
  VioNameSetModel* setproto;
  stdsetstyle= new VioNameSetStyle("EventSet");
  setproto= new VioNameSetModel(0,stdsetstyle); 
  VioTypeRegistry::Insert(setproto);
  stdsetstyle= new VioNameSetStyle("Alphabet");
  setproto= new VioNameSetModel(0,stdsetstyle); 
  VioTypeRegistry::Insert(setproto);
  FD_DQN("VioRegisterNameSets(): eventset and alphabet: done");
}

// register all nameset styles from config file
void VioRegisterNameSetStyles(const QString& filename) {
  // figure my section and get token reader
  faudes::TokenReader* trp=VioStyle::NewStyleReader("NameSetStyles", filename);
  if(!trp) return;
  std::string fsection=VioStyle::StrFromQStr("NameSetStyles");
  // make sure, at least Generator and EventSet are faudes registerd
  if(!faudes::TypeRegistry::G()->Exists("Generator")) 
    faudes::TypeRegistry::G()->Insert<faudes::Generator>("Generator");
  if(!faudes::TypeRegistry::G()->Exists("EventSet")) 
    faudes::TypeRegistry::G()->Insert<faudes::EventSet>("EventSet");
  // loop my section
  while(!trp->Eos(fsection)){
    // name of style
    QString ftype=VioStyle::QStrFromStr(trp->ReadString());
    FD_DQN("VioRegisterNameSetStyle(): register " << VioStyle::StrFromQStr(ftype)); 
    // get base faudes type (defaults to name of style)
    QString rftype=ftype;
    faudes::Token token;
    trp->Peek(token);
    if(token.Type()==faudes::Token::Option) {
      rftype=VioStyle::QStrFromStr(trp->ReadOption());
      // remove name space faudes
      if(rftype.startsWith("faudes::")) rftype.remove(0,QString("faudes::").length());
    }
    // register style as faudes type
    bool err=false;
    if(!faudes::TypeRegistry::G()->Exists(VioStyle::StrFromQStr(ftype))) {
      try{
        faudes::Type* fproto=faudes::TypeRegistry::G()->NewObject(
          VioStyle::StrFromQStr(rftype));        // exception on non ex
        faudes::TypeRegistry::G()->Insert(
          fproto, VioStyle::StrFromQStr(ftype)); // exception on doublets
      } catch(faudes::Exception& fexception) {
        std::stringstream errstr;
        errstr << "RTI lookup error when defining " << VioStyle::StrFromQStr(ftype) << " from " << VioStyle::StrFromQStr(rftype);
        FD_WARN(errstr.str());
        // throw faudes::Exception("VioRegisterNameSetStyle", errstr.str(), 48);
        err=true;
      }
    }
    // skip on error
    if(err) continue;
    // construct nameset style (incl read actual configuration)
    VioNameSetStyle* astyle = new VioNameSetStyle(ftype);
    // construct vio prototype
    VioNameSetModel* aproto = new VioNameSetModel(0,astyle); 
    // register
    VioTypeRegistry::Insert(aproto);
    FD_DQA("VioRegisterNameSetStyle(): " << VioStyle::StrFromQStr(ftype) << ": done"); 
  }
  // done
  trp->ReadEnd(fsection);
  delete trp;
}

