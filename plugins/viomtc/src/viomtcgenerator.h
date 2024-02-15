/* viomtcgenerator.h  - mtc generator model and widget */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2009  Thomas Moor, Ruediger Berndt

*/



#ifndef FAUDES_VIOMTCGENERATOR_H
#define FAUDES_VIOMTCGENERATOR_H

// std includes
#include "libviodes.h"
#include "viogenerator.h"
#include "viomtcgenstyle.h"


/*
 ************************************************
 ************************************************
 
 User Layout options, incl colormap
 
 ************************************************
 ************************************************
 */

// struct to hold layout hints
class VioMtcGeneratorLayout : public VioGeneratorLayout {

Q_OBJECT

public:

  // constructor (default values)
  VioMtcGeneratorLayout(QObject* parent=0);

  // assignment operator
  virtual VioMtcGeneratorLayout& operator=(const VioMtcGeneratorLayout& rSrc);
  virtual VioMtcGeneratorLayout& operator=(const VioGeneratorLayout& rSrc);

  // read access map
  const QMap<QString, int>& ColorMap(void) const;
  const QList<QString>& ColorList(void) const;

  // edit set
  void VioColors(const QList<QString>& rColorList, const QMap<QString,int>& rColorMap);

  // load/save layout (no exceptions)
  void Write(faudes::TokenWriter& rTw) const;
  void Read(faudes::TokenReader& rTr);

protected:

  // protected token io
  void WriteCore(faudes::TokenWriter& rTw) const;
  void ReadCore(faudes::TokenReader& rTr);

  // aditional members 
  QMap<QString, int> mColorMap;
  QList<QString> mColorList;
};



// forward
class VioMtcGeneratorModel;
class VioMtcGeneratorView;

/*
 ************************************************
 ************************************************

 A VioMtcGeneratorModel is derived from the VopGenerator
 to add the color mapping as a user layout option.

 ************************************************
 ************************************************
 */



class VioMtcGeneratorModel : public VioGeneratorModel {

Q_OBJECT

public:

  // construct/destruct
  VioMtcGeneratorModel(QObject* parent=0, VioStyle* config=0, bool alloc=true);
  virtual ~VioMtcGeneratorModel(void);

  // reimplement viogeneratormodel: construct on heap
  virtual VioMtcGeneratorModel* NewModel(QObject* parent=0) const;
  virtual VioView* NewView(QWidget* parent=0) const;
  virtual VioView* NewPropertyView(QWidget* parent=0) const;
  virtual VioView* NewConfigView(QWidget* parent=0) const;
  virtual VioWidget* NewWidget(QWidget* parent=0) const;

  // typed access to configuration
  VioMtcGeneratorStyle* GeneratorConfiguration(void) const { return pMtcGeneratorConfig; };

  // set/get default layout (todo: layout signals)
  const VioMtcGeneratorLayout& Layout(void) const { return *pMtcUserLayout; };
  void Layout(const VioMtcGeneratorLayout& layout) { *pMtcUserLayout=layout; };

  // set my layout features (todo: layout signals)
  void VioColors(const QList<QString>& rColorList, const QMap<QString,int>& rColorMap);

protected:

  // typed version of configuration
  VioMtcGeneratorStyle* pMtcGeneratorConfig;

  // typed default layout
  VioMtcGeneratorLayout* pMtcUserLayout;

  // reimplement viomodel: allocate faudes generator
  virtual void DoFaudesAllocate(void);

  // reimplement viomodel: allocate visual model data
  void DoVioAllocate(void);
};



/*
 ************************************************
 ************************************************

 A VioMtcGeneratorView is a widget to represent a
 generator provided as VioMtcGeneratorModel.
 It adds nothing to the VioGenertorView and is
 provided for formal reasons only.

 ************************************************
 ************************************************
 */



class VioMtcGeneratorView : public VioGeneratorView {

Q_OBJECT

public:
  // construct/destruct
  VioMtcGeneratorView(QWidget* parent=0, VioStyle* config=0, bool alloc=true);
  virtual ~VioMtcGeneratorView(void);

  // set model
  virtual int Model(VioModel* model);

protected:

  // update view from new model
  void DoVioUpdate(void);


public slots:

  // save current layout to model
  virtual void SaveUserLayout(void);


protected:

  // update view layout
  virtual void UpdateUserLayout(void);

  // typed representation data
  VioMtcGeneratorStyle* pMtcGeneratorConfig;
  VioMtcGeneratorModel* pMtcGeneratorModel;
  VioMtcGeneratorLayout* pMtcUserLayout;

  // actions
  QAction* mColorAction;

};




/*
 ************************************************
 ************************************************

 A VioMtcGeneratorConfigView is a widget to represent the
 mtc user configuration, ie the color map.

 ************************************************
 ************************************************
 */




class VioMtcGeneratorConfigView : public VioView {

Q_OBJECT

public:
  // construct/destruct
  VioMtcGeneratorConfigView(QWidget* parent=0, VioStyle* config=0, bool alloc=true);
  virtual ~VioMtcGeneratorConfigView(void);

  // reimplement vioattributeview: allocate view data
  virtual void DoVioAllocate(void);


protected:

  // update view from model
  virtual void DoVioUpdate(void);

  // update to model (exception on error)
  virtual void DoModelUpdate(void);

  // typed refs
  VioMtcGeneratorModel* pMtcGeneratorModel;

  // layout items
  VioSymbolTableWidget* mColorColumns;


protected slots:

  // color set changed
  void UpdateFromColorColumns(int row=-1, int col=-1);
};



/*
 ************************************************
 ************************************************

 A VioMtcGeneratorWidget is a widget to represent a
 mtc generator. It adds nothing to the VioGeneratorWidget
 and is provided for convenience only.

 ************************************************
 ************************************************
 */



class VioMtcGeneratorWidget : public VioGeneratorWidget {

Q_OBJECT

public:
  // construct/destruct
  VioMtcGeneratorWidget(QWidget* parent=0, VioStyle* config=0, bool alloc=true);
  virtual ~VioMtcGeneratorWidget(void);

  // reimplement set model
  virtual int Model(VioModel* model); 

protected:

  // fix view refs and connect
  virtual void DoVioAllocate(void);

  // typed representation data
  VioMtcGeneratorView* pMtcGeneratorView;
  VioMtcGeneratorModel* pMtcGeneratorModel;

};


#endif
