/* viomtcglobalattr.h  - vio attribute model and widget for mtc color map */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2010  Thomas Moor

*/



#ifndef FAUDES_VIOMTCGLOBAL_H
#define FAUDES_VIOMTCGLOBAL_H

// std includes
#include "libviodes.h"


// debugging: mtv plugin
#ifdef FAUDES_DEBUG_VIO_MTC
#define FD_DQM(message) FAUDES_WRITE_CONSOLE("FAUDES_VIO_MTC: " << message)
#else
#define FD_DQM(message) { };
#endif



/*
 ************************************************
 ************************************************

 A VioAttributeMtcGlobalModel is derived from the 
 std boolean property VioAttributeModel. It provides
 an additional member to relate faudes color labels
 to vio representation colors. Its a fake attribute
 in the sense that actual faudes MtcSystems do not
 have such a map.

 ************************************************
 ************************************************
 */

// forward
class VioAttributeMtcGlobalView;
class VioAttributeMtcGlobalWidget;


class VioAttributeMtcGlobalModel : public VioAttributeModel {

Q_OBJECT

public:

  // construct/destruct
  VioAttributeMtcGlobalModel(QObject* parent=0, VioStyle* config=0, bool alloc=true);
  virtual ~VioAttributeMtcGlobalModel(void);

  // reimplement viomodel: construct on heap
  virtual VioAttributeMtcGlobalModel* NewModel(QObject* parent=0) const;
  virtual VioView* NewView(QWidget* parent=0) const;
  virtual VioWidget* NewWidget(QWidget* parent=0) const;

  // todo: merge with other faudes attributes for selection edit
  // return non-zero on error
  virtual int MergeClear(void) { return 1;};
  virtual void MergeAttribute(const faudes::AttributeFlags* fattr) { (void) fattr; };


  // edit get
  const QMap<QString,int>& VioColorMap(void) const { return mColorMap; }
  const QList<QString>& VioColorList(void) const { return mColorList; }


protected:

  // reimplement viomodel: token io, implementation 
  // note: only when used as a global attribute, vio io is functional
  virtual void DoVioWrite(faudes::TokenWriter& rTw) const;
  virtual void DoVioRead(faudes::TokenReader& rTr);

  // reimplement viomodel: update visual data from (new) faudes object
  virtual void DoVioUpdate(void);
  virtual void DoVioAllocate(void);


};


/*
 ************************************************
 ************************************************

 A VioAttributeMtcGlobalView is a widget to represent an
 attribute via an VioAttributeView.

 ************************************************
 ************************************************
 */
/*
 ************************************************
 ************************************************

 A VioAttributeMtcGlobalWidget is a widget to represent an
 attribute by an VioAttributeMtcGlobalView. Internally, it 
 uses a VioAttributeModel to hold defining data.

 ************************************************
 ************************************************
 */



class VioAttributeMtcGlobalWidget : public VioAttributeWidget {

Q_OBJECT

public:
  // construct/destruct
  VioAttributeMtcGlobalWidget(QWidget* parent=0, VioStyle* config=0, bool alloc=true);
  virtual ~VioAttributeMtcGlobalWidget(void);

  // set model
  int Model(VioModel* model);

protected:

  // reimplement viomodel: allocate vio model
  virtual void DoVioAllocate(void);

  // typed representation data
  VioAttributeMtcGlobalModel* pAttributeMtcGlobalModel;

};




#endif
