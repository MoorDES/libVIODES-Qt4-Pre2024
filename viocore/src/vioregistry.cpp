/* vioregistry.cpp  - vio type registry */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2009 Ruediger Berndt, Thomas Moor;

*/

#include "vioregistry.h"
#include "vioattribute.h"
#include "vionameset.h"


/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioTypeRegistry

****************************************************************
****************************************************************
****************************************************************
*/

// static members
QStringList VioTypeRegistry::mTypeList;
QStringList VioTypeRegistry::mUserTypeList;
QStringList VioTypeRegistry::mPluginList;
QStringList VioTypeRegistry::mSectionList;
QMap<QString, QStringList> VioTypeRegistry::mSectionToTypes;
QMap<QString,VioModel*> VioTypeRegistry::mTypeToModel;


// initialise from qt plugins
void VioTypeRegistry::Initialise(void) {
  FD_DQT("VioTypeRegistry::Initialise()");

  // abuse to initialise viodes
  qRegisterMetaType<VioElement>();

  // register the universal fallback widget
  FD_DQT("VioTypeRegistry::Initialise(): register universal fallback");
  VioModel* uniproto = new VioModel(0,VioStyle::G()); 
  VioTypeRegistry::Insert(uniproto);

  // built in widgets: attributes incl flags from configfile
  FD_DQT("VioTypeRegistry::Initialise(): register attributes");
  VioRegisterAttributes();
  VioRegisterAttributeFlagsStyles();

  // built in widgets: namesets excl attributes from config file
  FD_DQT("VioTypeRegistry::Initialise(): register namesets");
  VioRegisterNameSets();


  // look for static plugins
  foreach(QObject *plugin, QPluginLoader::staticInstances()) {
    FD_WARN("VioTypeRegistry::Initialise(): found a static plugin");
    VioTypePlugin* vioplugin = qobject_cast<VioTypePlugin*>(plugin);
    if(!vioplugin) continue;
    // register my types
    FD_WARN("VioTypeRegistry::Initialise(): register types");
    mPluginList.append(vioplugin->Name());
    vioplugin->RegisterTypes();
  }

  // set up dir for dynamically linked plugins
  QDir pdir = QDir(qApp->applicationDirPath());
#ifdef Q_OS_MAC
  pdir.cdUp();
  pdir.cd("plugins");
  pdir.cd("viotypes");
#else
  pdir.cd("plugins");
  pdir.cd("viotypes");
#endif
  FD_WARN("VioTypeRegistry::Initialise(): plugin dir " << VioStyle::StrFromQStr(pdir.absoluteFilePath("./")));



  // loop plugin loading to figure order (relevant for windows)
  QStringList filelist = pdir.entryList(QDir::Files);
  QList<VioTypePlugin*> vplugins;
  bool found=true;
  while(found) {

  // look for dynamically linked plugins and register base types
  found=false;
  foreach(QString file, filelist) {
    FD_WARN("VioTypeRegistry::Initialise(): loading file " << VioStyle::StrFromQStr(file));
    // load plugin
    QPluginLoader loader(pdir.absoluteFilePath(file));
    QObject *plugin = loader.instance();
    if(!plugin) {
      FD_WARN("VioTypeRegistry::Initialise(): load error: " << VioStyle::StrFromQStr(loader.errorString()));
      continue;
    }
    // cast for my interface
    FD_WARN("VioTypeRegistry::Initialise(): casting root object");
    VioTypePlugin* vplugin = qobject_cast<VioTypePlugin*>(plugin);
    if(!vplugin) { 
      continue;
    }   
    // register my types
    FD_WARN("VioTypeRegistry::Initialise(): register types");
    mPluginList.append(vplugin->Name());
    vplugin->RegisterTypes();
    vplugins.push_back(vplugin);
    filelist.removeAll(file);
    found=true;
  }

  } // end loop  

  // register name sets configures by file (using attributes from plugin)
  VioRegisterNameSetStyles();

  // register more types from plugins
  foreach(VioTypePlugin* vplugin, vplugins)
    vplugin->FinaliseTypes();

  // loop faudes registry and install default models
  faudes::TypeRegistry::Iterator tit;
  for(tit=faudes::TypeRegistry::G()->Begin(); tit!=faudes::TypeRegistry::G()->End(); tit++) {
    // bail out if this faudes type has no prototype object
    if(!tit->second->Prototype()) continue;
    // get faudes type name
    QString ftype=VioStyle::QStrFromStr(tit->second->Name());
    // bail out if this type has a specialised vio model
    if(Prototype(ftype)) continue; 
    // register the universal fallback model
    VioStyle* unistyle = new VioStyle(ftype);
    VioModel* uniproto = new VioModel(0,unistyle); 
    VioTypeRegistry::Insert(uniproto,ftype);
  }
  FD_DQT("VioTypeRegistry::Initialise(): done");
}

// insert new type representation
void VioTypeRegistry::Insert(VioModel* pPrototype, const QString& rTypeName) {
  // report
  FD_DQT("VioTypeRegistry::Insert(): faudes type \"" << VioStyle::StrFromQStr(pPrototype->FaudesType()));
  // set up type name
  QString ftype = rTypeName;
  if(ftype=="") ftype=pPrototype->FaudesType();
  // report
  FD_DQT("VioTypeRegistry::Insert(): using type name \"" << VioStyle::StrFromQStr(ftype)<<"\"");
  // is it a user type
  bool utype=true;
  if(!faudes::TypeRegistry::G()->Exists(VioStyle::StrFromQStr(pPrototype->FaudesType())))
    utype=false; 
  if(faudes::TypeRegistry::G()->Exists(VioStyle::StrFromQStr(pPrototype->FaudesType()))) 
    if(faudes::TypeRegistry::G()->Definition(VioStyle::StrFromQStr(pPrototype->FaudesType())).HtmlDoc().empty()) 
      utype=false;
  if(faudes::TypeRegistry::G()->Exists(VioStyle::StrFromQStr(pPrototype->FaudesType()))) 
    if(faudes::TypeRegistry::G()->Definition(VioStyle::StrFromQStr(pPrototype->FaudesType())).TextDoc().empty()) 
      utype=false;
  if(typeid(*pPrototype)==typeid(VioModel))
    utype=false;
  // allow for overwrite
  mTypeList.removeAll(ftype);
  // register (incl take ownership)
  pPrototype->setParent(0); // should be "this"
  VioTypeRegistry::mTypeToModel.insert(ftype,pPrototype);
  // record in flat list
  mTypeList.append(ftype);
  // record in flat list
  if(utype) 
  if(!mUserTypeList.contains(ftype)) 
    mUserTypeList.append(ftype);
  // get faudes section
  QString fsect;
  if(faudes::TypeRegistry::G()->Exists(VioStyle::StrFromQStr(pPrototype->FaudesType()))) {
    const faudes::TypeDefinition& fdef=faudes::TypeRegistry::G()->Definition(VioStyle::StrFromQStr(pPrototype->FaudesType()));
    fsect = VioStyle::QStrFromStr(fdef.KeywordAt(0));
  }
  if(fsect=="") fsect="Other";
  // new section
  InsertSection(fsect); 
  // record in section list
  mSectionToTypes.find(fsect).value().append(ftype);
  // done
  FD_DQT("VioTypeRegistry::Insert(): done (sect \"" << VioStyle::StrFromQStr(fsect) << "\", #" << mSectionToTypes.find(fsect).value().size() << ")");
}

// collect sections
void  VioTypeRegistry::InsertSection(const QString& rSection) {
  if(mSectionList.contains(rSection)) return; 
  mSectionList.append(rSection);
  mSectionList.sort();
  if(!mSectionToTypes.contains(rSection)) 
    mSectionToTypes.insert(rSection, QStringList());
}

// return list of types
const QStringList& VioTypeRegistry::Types(void) {
  return mTypeList;  
}

// return list of user types (i.e. types with docu)
const QStringList& VioTypeRegistry::UserTypes(void) {
  return mUserTypeList;  
}

// return types by section
const QStringList& VioTypeRegistry::Types(const QString& rSection) {
  return mSectionToTypes.constFind(rSection).value();
}

// return list of plugins
const QStringList& VioTypeRegistry::Plugins(void) {
  return mPluginList;  
}

// return list of sections
const QStringList& VioTypeRegistry::Sections(void) {
   return mSectionList;
}

// query documentation
QString VioTypeRegistry::DocumentationHtml(const QString& rTypeName) {
  std::string ftype=VioStyle::StrFromQStr(rTypeName);
  if(!faudes::TypeRegistry::G()->Exists(ftype)) return "";
  std::string html = faudes::TypeRegistry::G()->Definition(ftype).HtmlDoc();
  FD_DQT("VioTypeRegistry::DocumentationHtml("<<ftype<<"): " << html);
  return  VioStyle::QStrFromStr(html);
}

// query documentation
QString VioTypeRegistry::DocumentationPlain(const QString& rTypeName) {
  std::string ftype=VioStyle::StrFromQStr(rTypeName);
  if(!faudes::TypeRegistry::G()->Exists(ftype)) return "";
  std::string plain = faudes::TypeRegistry::G()->Definition(ftype).TextDoc();
  FD_DQT("VioTypeRegistry::DocumentationPlain("<<ftype<<"): " << plain);
  return  VioStyle::QStrFromStr(plain);
}


// find prototype in rego
const VioModel* VioTypeRegistry::Prototype(const QString& rTypeName) {
  FD_DQT("VioTypeRegistry::Prototype(): type " << VioStyle::StrFromQStr(rTypeName) << 
    " out of #" << mTypeToModel.size());
  // lookup in rego
  QMap<QString,VioModel*>::iterator rit;
  rit=mTypeToModel.find(rTypeName);
  // if not in rego, complain
  if(rit==mTypeToModel.end()) {
    FD_DQT("VioTypeRegistry::Prototype(): not in viodes registry");
    return 0;
  }
  return *rit;
}

// test existence
bool VioTypeRegistry::Exists(const QString& rTypeName) {
  return Prototype(rTypeName)!=0;
}


// instantiate new widget
VioWidget* VioTypeRegistry::NewWidget(const QString& rTypeName) {
  FD_DQT("VioTypeRegistry::NewWidget(): type " << VioStyle::StrFromQStr(rTypeName));
  // get prototype
  const VioModel* proto=Prototype(rTypeName);
  if(!proto) {
    FD_DQT("VioTypeRegistry::NewWidget(): type not found");
    return 0;
  }
  // done
  return proto->NewWidget();    
}

// instantiate new widget
VioModel* VioTypeRegistry::NewModel(const QString& rTypeName) {
  FD_DQT("VioTypeRegistry::NewModel(): type " << VioStyle::StrFromQStr(rTypeName));
  // get prototype
  const VioModel* proto=Prototype(rTypeName);
  if(!proto) {
    FD_DQT("VioTypeRegistry::NewModel(): type not found");
    return 0;
  }
  // done
  return proto->NewModel();    
}

// instantiate new widget
VioView* VioTypeRegistry::NewView(const QString& rTypeName) {
  FD_DQT("VioTypeRegistry::NewView(): type " << VioStyle::StrFromQStr(rTypeName));
  // get prototype
  const VioModel* proto=Prototype(rTypeName);
  if(!proto) {
    FD_DQT("VioTypeRegistry::NewView(): type not found");
    return 0;
  }
  // done
  return proto->NewView();    
}



// instantiate new representation 
VioWidget* VioTypeRegistry::FromFile(const QString& rFileName) {
  FD_DQT("VioTypeRegistry::FromFile(): " << VioStyle::StrFromQStr(rFileName));
  // get token reader
  faudes::TokenReader tr(VioStyle::StrFromQStr(rFileName));
  // inspect type (alt: search for begin tags starting with Vio)
  faudes::Token token;
  tr.Peek(token);
  QString ftype=VioStyle::QStrFromStr(token.StringValue());
  if(ftype.startsWith("Vio")) ftype.remove(0,3);
  FD_DQT("VioTypeRegistry::FromFile(): found type " << VioStyle::StrFromQStr(ftype));
  // have new widget
  VioWidget* res=NewWidget(ftype);
  // throw on type error
  if(!res) {
    std::stringstream errstr;
    errstr << "Unknown type \"" << VioStyle::StrFromQStr(ftype) << "\" in " << tr.FileLine();
    throw faudes::Exception("VioModel::FromFile", errstr.str(), 1000); 
  }; 
  // actually read
  res->Read(tr);
  res->Modified(false);
  // done
  FD_DQT("ViodesTypeRegistry::FromFile(): done " << VioStyle::StrFromQStr(res->FaudesType()));
  return res;
}

  
// instantiate new representation 
VioWidget* VioTypeRegistry::FromFaudesObject(faudes::Type* pFaudesObject) {
  FD_DQT("VioTypeRegistry::FromFaudes(): looking up type");
  if(!faudes::TypeRegistry::G()->Exists(*pFaudesObject)) return 0;
  const faudes::TypeDefinition& fdef= faudes::TypeRegistry::G()->Definition(*pFaudesObject);
  QString ftype= VioStyle::QStrFromStr(fdef.Name());
  FD_DQT("VioTypeRegistry::FromFaudes(): with type " << VioStyle::StrFromQStr(ftype));
  // instantiate
  VioWidget* res = NewWidget(ftype);
  if(!res) return 0;
  // take over contents
  res->InsertFaudesObject(pFaudesObject);
  // done
  return res;
}

// instantiate new representation 
VioWidget* VioTypeRegistry::FromFaudesFile(const QString& rTypeName, const QString& rFileName) {
  FD_DQT("VioTypeRegistry::FromFaudesFile("<< VioStyle::StrFromQStr(rTypeName) << "," 
    << VioStyle::StrFromQStr(rFileName));
  // instatiate
  VioWidget* res = NewWidget(rTypeName);
  if(!res) return 0;
  // read faudes file
  res->ImportFaudesFile(rFileName);
  // done
  return res;
} 





/*
****************************************************************
****************************************************************
****************************************************************

Implementation: VioFunctionRegistry

****************************************************************
****************************************************************
****************************************************************
*/


// static members
QStringList VioFunctionRegistry::mFuncList;
QMap<QString, QStringList> VioFunctionRegistry::mSectionToFunctions;

// initialise from faudes registry
void VioFunctionRegistry::Initialise(void) {
  FD_DQT("VioFunctionRegistry::Initialise()");

  // scan faudes registry
  faudes::FunctionRegistry::Iterator fit;
  for(fit=faudes::FunctionRegistry::G()->Begin(); fit!=faudes::FunctionRegistry::G()->End(); fit++) {
    // bail out if this faudes function has no prototype object
    if(!fit->second->Prototype()) {
      FD_DQT("VioFunctionRegistry::Initialise(): skipping " << fit->second->Name());
      continue;
    }
    // get faudes function name
    QString ffunction = VioStyle::QStrFromStr(fit->second->Name());
    // insert
    VioFunctionRegistry::Insert(ffunction);
  }
  FD_DQT("VioFunctionRegistry::Initialise(): done #" << mFuncList.size());
}

// test existence
bool VioFunctionRegistry::Exists(const QString& rFuncName) {
  return mFuncList.contains(rFuncName);
}

// record name
void VioFunctionRegistry::Insert(const QString& rFuncName) {
  FD_DQT("VioFunctionRegistry::Insert(\"" << VioStyle::StrFromQStr(rFuncName) << "\")");
  // avoid doublets
  if(mFuncList.contains(rFuncName)) return;
  // avoid emptystring
  if(rFuncName.isEmpty()) return;
  // ensure exitence
  if(!faudes::FunctionRegistry::G()->Exists(VioStyle::StrFromQStr(rFuncName))) return;
  // get faudes section
  const faudes::FunctionDefinition& fdef=faudes::FunctionRegistry::G()->Definition(VioStyle::StrFromQStr(rFuncName));
  QString fsect = VioStyle::QStrFromStr(fdef.KeywordAt(0));
  if(fsect=="") return;
  // new section
  if(!mSectionToFunctions.contains(fsect)) 
      mSectionToFunctions.insert(fsect, QStringList());
  // new section (friend type rego holds all)
  VioTypeRegistry::InsertSection(fsect);
  // record in section list
  mSectionToFunctions.find(fsect).value().append(rFuncName);
  // record in flat list
  mFuncList.push_back(rFuncName);
  // done
  FD_DQH("VioFunctionRegistry::Insert(): done (sect \"" << VioStyle::StrFromQStr(fsect) << "\", #" << mSectionToFunctions.find(fsect).value().size() << ")");
}


// get definiion
const faudes::FunctionDefinition* VioFunctionRegistry::Definition(const QString& rFuncName) {
  if(!faudes::FunctionRegistry::G()->Exists(VioStyle::StrFromQStr(rFuncName))) return 0;
  return &faudes::FunctionRegistry::G()->Definition(VioStyle::StrFromQStr(rFuncName));
};

// access static member
const QStringList& VioFunctionRegistry::Functions(void) {
  return(mFuncList);
}

// access static member
const QStringList& VioFunctionRegistry::Functions(const QString& rSection) {
  FD_DQT("VioFunctionRegistry::Functions(): section " << VioStyle::StrFromQStr(rSection));
  static const QStringList empty;
  if(mSectionToFunctions.contains(rSection))
    return mSectionToFunctions.constFind(rSection).value();
  return empty;
}


// query documentation
QString VioFunctionRegistry::DocumentationHtml(const QString& rFunctionName) {
  std::string ftype=VioStyle::StrFromQStr(rFunctionName);
  if(!faudes::FunctionRegistry::G()->Exists(ftype)) return "";
  std::string html = faudes::FunctionRegistry::G()->Definition(ftype).HtmlDoc();
  FD_DQT("VioFunctionRegistry::DocumentationHtml("<<ftype<<"): " << html);
  return  VioStyle::QStrFromStr(html);
}

// query documentation
QString VioFunctionRegistry::DocumentationPlain(const QString& rFunctionName) {
  std::string ftype=VioStyle::StrFromQStr(rFunctionName);
  if(!faudes::FunctionRegistry::G()->Exists(ftype)) return "";
  std::string plain = faudes::FunctionRegistry::G()->Definition(ftype).TextDoc();
  FD_DQT("VioFunctionRegistry::DocumentationPlain("<<ftype<<"): " << plain);
  return  VioStyle::QStrFromStr(plain);
}


