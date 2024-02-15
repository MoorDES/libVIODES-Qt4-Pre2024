/* vioregistry.h  - vio type registry */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2009  Thomas Moor, Ruediger Berndt

*/

#ifndef FAUDES_VIOREGISTRY_H
#define FAUDES_VIOREGISTRY_H

// std includes
#include "viostyle.h"
#include "viotoken.h" 
#include "viotypes.h"



/*
 ************************************************
 ************************************************

 The VioTypeRegistry maps faudes types to their
 visual representation. It uses the underlying 
 libfaudes type registry to retrieve construct 
 faudes objects by type name and to retrieve documentation.
 By default, it uses the base class VioWidget for
 representation. Tailored representations for
 specific faudes types can be registered to
 override the default behaviour.


 ************************************************
 ************************************************
 */

class VIODES_API VioTypeRegistry {

public:

  // initialise from qt plugins
  static void Initialise(void);

  // insert new type representation
  // note: the registry becomes the owner of the prototype
  static void Insert(VioModel* pPrototype, const QString& rTypeName="");

  // test existence
  static bool Exists(const QString& rTypeName);

  // insert new section
  // note: other help so we get the complete list
  static void InsertSection(const QString& rSection);

  // get prototype by typename
  static const VioModel* Prototype(const QString& rTypeName);

  // query list of known plugins/types
  static const QStringList& Plugins(void);
  static const QStringList& Sections(void);
  static const QStringList& Types(void);
  static const QStringList& UserTypes(void);
  static const QStringList& Types(const QString& rSection);


  // query documentation
  static QString DocumentationHtml(const QString& rTypeName);
  static QString DocumentationPlain(const QString& rTypeName);

  // instantiate new representation
  static VioWidget* NewWidget(const QString& rTypeName);
  static VioView*   NewView(const QString& rTypeName);
  static VioModel*  NewModel(const QString& rTypeName);
  static VioWidget* FromFile(const QString& rFileName);
  static VioWidget* FromFaudesObject(faudes::Type* pFaudesObject);
  static VioWidget* FromFaudesFile(const QString& rTypeName, const QString& rFileName);

private:
  
  // my type map
  static QMap<QString,VioModel*> mTypeToModel;

  // my type/plugin list
  static QStringList mTypeList;
  static QStringList mUserTypeList;
  static QStringList mPluginList;
  static QStringList mSectionList;
  static QMap<QString, QStringList> mSectionToTypes;
};



/*
 ************************************************
 ************************************************

 The VioFunctionRegistry is a simple wrapper on the faudes function 
 registry. It reads the faudes registry and records function names 
 as a QStringList. It cannot do anything else.

 ************************************************
 ************************************************
 */


class VIODES_API VioFunctionRegistry {

public:
  // initialise
  static void Initialise(void);

  // test existence
  static bool Exists(const QString& rFuncName);

  // access functions
  static const QStringList& Functions(void);
  static const QStringList& Functions(const QString& rSection);
  static QString DocumentationHtml(const QString& rFuncName);
  static QString DocumentationPlain(const QString& rFuncName);

  // get function definition
  static const faudes::FunctionDefinition* Definition(const QString& rFuncName);

private:
  static void Insert(const QString& rFuncName);
  static QStringList mFuncList;
  static QMap<QString, QStringList> mSectionToFunctions;
};



/*
 ************************************************
 ************************************************

 The VioTypePlugin is a Qt Plugin interface 
 to alow extensions to libviodes for tailored representaions
 of faudes objects. Plugins are meant to implement
 the function RegisterTypes that registers their 
 additional representations in the VioTypeRegistry.

 Todo: use singleton pattern as suggested by rberndt

 ************************************************
 ************************************************
 */

class VIODES_API VioTypePlugin {

public:
  // formal requirement for gcc
  virtual ~VioTypePlugin() {};

  // tell name of this plugin
  virtual QString Name(void) =0;

  // register base types  
  virtual void RegisterTypes(void) =0;

  // register more types 
  virtual void FinaliseTypes(void) =0;

};

Q_DECLARE_INTERFACE(VioTypePlugin,"de.uni-erlangen.eei.rt.viodes/0.3")



#endif
