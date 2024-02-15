/* vioattrstyle.cpp  - vioattr configuration */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006, 2007  Thomas Moor, Klaus Schmidt, Sebastian Perk

*/



#include "vioattrstyle.h"
#include "vioattribute.h"
#include "vioregistry.h"

/*
 ************************************************
 ************************************************
 ************************************************

 Implementation: VioBooleanProperty

 ************************************************
 ************************************************
 ************************************************
 */

// construct
VioBooleanProperty::VioBooleanProperty(void) :
  mName(""),
  mShortName(""),
  mEditable(false),
  mFSetMask(0x0),
  mFSetValue(0x0),
  mFClrMask(0x0),
  mFClrValue(0x0) {
}

// construct from values
VioBooleanProperty::VioBooleanProperty(
  const QString& name,
  const QString& shortname,
  bool editable,
  faudes::fType fmask, 
  faudes::fType fvalue) :

  mName(name),
  mShortName(shortname),
  mEditable(editable),
  mFSetMask(fmask),
  mFSetValue(fvalue & fmask),
  mFClrMask(fmask),
  mFClrValue( (~fvalue) & fmask) {
}

// construct from values
VioBooleanProperty::VioBooleanProperty(
  const QString& name,
  const QString& shortname,
  bool editable,
  faudes::fType fsmask, 
  faudes::fType fsvalue, 
  faudes::fType fcmask, 
  faudes::fType fcvalue) :

  mName(name),
  mShortName(shortname),
  mEditable(editable),
  mFSetMask(fsmask),
  mFSetValue(fsvalue & fsmask),
  mFClrMask(fcmask),
  mFClrValue(fcvalue & fcmask) {
}

// attribute style relaxed reading, 
bool VioBooleanProperty::Read(faudes::TokenReader& rTr) {
  faudes::Token token;
  rTr.Peek(token);
  if(token.Type()!=faudes::Token::Begin) return false;
  if(token.StringValue()!="BooleanProperty") return false;
  // get data ...
  rTr.ReadBegin("BooleanProperty");
  // name
  mName=VioStyle::QStrFromStr(rTr.ReadString());
  // short name
  mShortName="";
  rTr.Peek(token);
  if(token.Type()==faudes::Token::String) {
    rTr.Get(token);
    mShortName=VioStyle::QStrFromStr(token.StringValue());
  }
  // editable flag
  mEditable=false;
  rTr.Peek(token);
  if(token.Type()==faudes::Token::Option) 
  if(token.StringValue()=="Editable") {
    rTr.Get(token);
    mEditable=true;
  }
  // masks
  mFSetMask=rTr.ReadInteger();
  mFSetValue= (rTr.ReadInteger() & mFSetMask);
  // optional clear mask
  mFClrMask=mFSetMask;
  mFClrValue= (~mFSetValue) & mFSetMask;
  rTr.Peek(token);
  if(token.Type()==faudes::Token::Integer || token.Type()==faudes::Token::Integer16)  {
    mFClrMask=rTr.ReadInteger();
    mFClrValue= (rTr.ReadInteger() & mFClrMask);
  }
  // done
  rTr.ReadEnd("BooleanProperty");
  FD_DQT("VioBoolenaProperty::Read(rTr): " << VioStyle::StrFromQStr(mName) << " masks "
	 << faudes::ToStringInteger16(mFSetMask) << " " 
	 << faudes::ToStringInteger16(mFSetValue) << " " 
	 << faudes::ToStringInteger16(mFClrMask) << " " 
	 << faudes::ToStringInteger16(mFClrValue))
  return true;
}

// query from flags
bool VioBooleanProperty::Test(const faudes::fType& fflags) const { 
  return (fflags & mFSetMask) == mFSetValue; 
};

// set
void VioBooleanProperty::Set(faudes::fType& fflags) const {
  fflags &= ~mFSetMask;
  fflags |=  mFSetValue;
}

// clear
void VioBooleanProperty::Clr(faudes::fType& fflags) const {
  fflags &= ~mFClrMask;
  fflags |=  mFClrValue;
}

// convert
Qt::CheckState VioBooleanProperty::State(ValueType val) {
  Qt::CheckState res=Qt::PartiallyChecked;
  if(val==True) res=Qt::Checked;
  if(val==False) res=Qt::Unchecked;
  return res;
}

// convert
VioBooleanProperty::ValueType VioBooleanProperty::Value(Qt::CheckState val) {
  ValueType res=Partial;
  if(val==Qt::Checked) res=True;
  if(val==Qt::Unchecked) res=False;
  return res;
}


/*
 ************************************************
 ************************************************
 ************************************************

 Implementation: VioAttributeStyle 

 ************************************************
 ************************************************
 ************************************************
 */


// constructor
VioAttributeStyle::VioAttributeStyle(const QString& ftype) : VioStyle() {
  FD_DQT("VioAttributeStyle::VioAttributeStyle(): ftype \"" << VioStyle::StrFromQStr(ftype) << "\"");
  Initialise(ftype);
  ReadFile();
  FD_DQT("VioAttributeStyle::VioAttributeStyle(): done type \"" << VioStyle::StrFromQStr(mFaudesType) << "\"");
}


// set defaults
void VioAttributeStyle::Initialise(const QString& ftype){
  // record type
  mFaudesType=ftype;
  // default: no flags at all
}

// load from file
void VioAttributeStyle::ReadFile(const QString& filename){
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
    // read my subsections: boolean properties 
    if(token.StringValue()=="BooleanProperty") {
      VioBooleanProperty prop;
      prop.Read(*trp);
      InsertBooleanProperty(prop);
      continue;
    }
    // ignore other sections
    trp->ReadBegin(token.StringValue());
    trp->ReadEnd(token.StringValue());
  }
  // done
  trp->ReadEnd(fsection);
  delete trp;
  FD_DQT("VioAttributeStyle::Config:done ");
}


// register named boolean properties (todo: overwrite on exist)
void  VioAttributeStyle::InsertBooleanProperty(const VioBooleanProperty& boolprop) {
  mBooleanProperties.append(boolprop); 
}

// register named boolean properties
void VioAttributeStyle::ClearBooleanProperties(void) {
  mBooleanProperties.clear();
}


/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioRegisterAttributes

****************************************************************
****************************************************************
****************************************************************
*/

// how to hard code attribute styles (will be overwritten from config file)
void VioRegisterAttributes(void) {
  FD_DQT("VioRegisterAttributes()");
  // vars
  VioAttributeStyle* attrstyle;
  VioModel* attrproto;
  // create a style for default flags
  attrstyle = new VioAttributeStyle("AttributeFlags");
  attrproto = new VioAttributeModel(0,attrstyle); 
  VioTypeRegistry::Insert(attrproto);
  // create a style for global flags 
  attrstyle = new VioAttributeStyle("AttributeGlobalFlags");
  attrproto = new VioAttributeModel(0,attrstyle); 
  VioTypeRegistry::Insert(attrproto);
  // create a style for trans flags 
  attrstyle = new VioAttributeStyle("AttributeTransFlags");
  attrproto = new VioAttributeModel(0,attrstyle); 
  VioTypeRegistry::Insert(attrproto);
  // create a style for state flags 
  attrstyle = new VioAttributeStyle("AttributeStateFlags");
  attrstyle->InsertBooleanProperty(VioBooleanProperty(
    "Initial", "I", true, 0x80000000,0x80000000));
  attrstyle->InsertBooleanProperty(VioBooleanProperty(
    "Marked", "M", true, 0x40000000,0x40000000));
  attrproto = new VioAttributeModel(0,attrstyle); 
  VioTypeRegistry::Insert(attrproto);
  // create a style for event flags 
  attrstyle = new VioAttributeStyle("AttributeEventFlags");
  attrstyle->InsertBooleanProperty(VioBooleanProperty(
    "Controllable", "C", true, 0x0001,0x0001));
  attrstyle->InsertBooleanProperty(VioBooleanProperty(
    "Observable", "O", true, 0x0002,0x0002));
  attrstyle->InsertBooleanProperty(VioBooleanProperty(
    "Forcible", "F", true, 0x0004,0x0004));
  attrproto = new VioAttributeModel(0,attrstyle); 
  VioTypeRegistry::Insert(attrproto);
}


// register all flag attributes from config file
void VioRegisterAttributeFlagsStyles(const QString& filename) {
  // figure my section and get token reader
  faudes::TokenReader* trp=VioStyle::NewStyleReader("AttributeFlagsStyles", filename);
  std::string fsection=VioStyle::StrFromQStr("AttributeFlagsStyles");
  if(!trp) return;
  // make sure, at least AttributeFlags is faudes registerd
  if(!faudes::TypeRegistry::G()->Exists("AttributeFlags")) 
    faudes::TypeRegistry::G()->Insert<faudes::AttributeFlags>("AttributeFlags");
  if(!faudes::TypeRegistry::G()->Exists("AttributeCFlags")) 
    faudes::TypeRegistry::G()->Insert<faudes::AttributeCFlags>("AttributeCFlags");
  // read my section
  while(!trp->Eos("AttributeFlagsStyles")) {
    // name of style
    QString ftype=VioStyle::QStrFromStr(trp->ReadString());
    FD_DQA("VioRegisterAttributeFlagsStyle(): register " << VioStyle::StrFromQStr(ftype)); 
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
          VioStyle::StrFromQStr(rftype));        // exception on non ex faudes base type
        faudes::TypeRegistry::G()->Insert(
          fproto, VioStyle::StrFromQStr(ftype)); // exception on doublets
      } catch(faudes::Exception& fexception) {
        std::stringstream errstr;
        errstr << "RTI lookup error when defining " << VioStyle::StrFromQStr(ftype) << " from " << VioStyle::StrFromQStr(rftype);
        // throw faudes::Exception("VioRegisterAttributeStyle", errstr.str(), 48);
        FD_WARN(errstr.str());
        err=true;
      }
    }
    // skip on error
    if(err) continue;
    // construct attribute style (incl read actual configuration)
    VioAttributeStyle* astyle = new VioAttributeStyle(ftype);
    // construct vio prototype
    VioAttributeModel* aproto = new VioAttributeModel(0,astyle); 
    // register
    VioTypeRegistry::Insert(aproto);
    FD_DQA("VioRegisterAttributeFlagsStyle(): " << VioStyle::StrFromQStr(ftype) << ": done"); 
  }
  // done
  trp->ReadEnd("AttributeFlagsStyles");
  delete trp;
}

