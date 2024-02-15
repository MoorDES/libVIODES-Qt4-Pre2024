/* viogenerator.h  - vio generator model and widget */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2009  Thomas Moor, Ruediger Berndt

*/



#ifndef FAUDES_VIOGENERATOR_H
#define FAUDES_VIOGENERATOR_H

// std includes
#include "libviodes.h"
#include "viogenstyle.h"




/*
 ************************************************
 ************************************************
 
 User Layout Options
 
 ************************************************
 ************************************************
 */

// struct to hold user layout options
class VIOGEN_API VioGeneratorLayout : public QObject {

Q_OBJECT

public:
  // constructor (default values)
  VioGeneratorLayout(QObject* parent=0);

  // assignment operator
  virtual VioGeneratorLayout& operator=(const VioGeneratorLayout& rSrc);

  // members (should protect)
  int mToggleStage;
  QByteArray mSplitterState;
  bool  mPropBuiltIn;
  int   mListWidth;
  int   mGraphWidth;
  int   mPropWidth;
  qreal mGraphScale;
  bool  mGraphGridVisible;

  // load/save layout (no exceptions)
  virtual void Write(faudes::TokenWriter& rTw) const;
  virtual void Read(faudes::TokenReader& rTr);

protected:

  // protected io core
  virtual void WriteCore(faudes::TokenWriter& rTw) const;
  virtual void ReadCore(faudes::TokenReader& rTr);
};



// forward
class VioGeneratorData;
class VioGeneratorAbstractData;
class VioGeneratorModel;
class VioGeneratorAbstractModel;
class VioGeneratorView;
class VioGeneratorAbstractView;
class VioGeneratorPropertyView;
class VioGeneratorGraphView;
class VioGeneratorWidget;
class PioTProp;
class PioSProp;
class PioEProp;
class PioGProp;
class VioGeneratorListModel;
class VioGeneratorGraphModel;


/*
 ************************************************
 ************************************************

 VioGeneratorData holds static data required
 for the widget representations of generators.
 To acomodate for multiple representations (graph, list),
 the VioGenerator family introduces another layer
 of abstract interfacing. Each representation model
 is required to supply its VioGeneratorAbrstractData
 derivate, and the VioGeneratorData is assembled as a
 record of all participating representation model 
 data objects. Since not all representations models
 participate, there is no one to one correspondence
 of the data list and the model list.
 
 ************************************************
 ************************************************
 */


// forward
class GioStateData;
class GioTransData;

class VIOGEN_API VioGeneratorData : public VioData {

Q_OBJECT

  friend class VioGeneratorModel;
  friend class VioGeneratorView;

public:

  // destruct
  virtual ~VioGeneratorData(void);

  // conversion (ret 0 on sucess)
  virtual QMimeData* ToMime(void);
  virtual int FromMime(const QMimeData* pMime);
  virtual int TestMime(const QMimeData* pMime);

  // clear to default/empty faudes object
  virtual void Clear(void);

  // state reindexing
  virtual void ApplyStateIndicees(const QMap<faudes::Idx,faudes::Idx> & rNewIdx) {};

  // public access to static data (pre 0.47)
  QList<VioGeneratorAbstractData*> mDataList; 


  // public data (post 0.47)
  QList<GioStateData*> mGioStateData;
  QList<GioTransData*> mGioTransData;
  QList<VioElement> mTransList;
  QList<VioElement> mStateList;
  QList<VioElement> mEventList;


protected:

  // construct
  VioGeneratorData(QObject* parent=0);

};



/*
 ************************************************
 ************************************************

 VioGeneratorAbstractData is the base for representation
 model static data. In contrast to VioData, it uses 
 token streams for io.

 In order to initialize a derived class from a token
 stream, there is a static pseudo constructor that
 figures the actual type by the first token. You need
 to adapt this function to sense all relevant data
 records.
 
 
 ************************************************
 ************************************************
 */

class VIOGEN_API VioGeneratorAbstractData : public QObject {

Q_OBJECT

public:

  // construct/destruct
  VioGeneratorAbstractData(QObject* parent=0);
  virtual ~VioGeneratorAbstractData(void)=0;

  // conversion (ret 0 on sucess)
  virtual void ToTokenWriter(faudes::TokenWriter& rTw) const =0;
  virtual int  FromTokenReader(faudes::TokenReader& rTr) =0;

  // state reindeing
  virtual void ApplyStateIndicees(const QMap<faudes::Idx,faudes::Idx> & rNewIdx)=0;

  // static token constructor
  static VioGeneratorAbstractData* NewFromTokenReader(faudes::TokenReader& rTr);

  // clear to default/empty faudes object
  virtual void Clear(void) = 0;

  // public access to static data .. in derived classes
};



/*
 ************************************************
 ************************************************

 A VioGeneratorModel holds data for the visual
 representation of a generator. In contrast to other
 derivatives of VioModel, the generator class may hold
 data for multiple representations, including list and
 graph representation. The virtual base for such internal
 models is VioGeneratorAbstractModel, see below.

 Both, for external access and for internal synchronisation
 of the multiple represenations, the VioGeneratorModel
 provides a minimal editing interface and a notifucation
 mechanism. When one representation is edited, it uses the
 minimal interface on the VioGeneratorModel, which in turn
 passes it on to any other representation models.

 ************************************************
 ************************************************
 */



class VIOGEN_API VioGeneratorModel : public VioModel {

Q_OBJECT

public:

  // construct/destruct
  VioGeneratorModel(QObject* parent=0, VioStyle* config=0, bool alloc=true);
  virtual ~VioGeneratorModel(void);

  // reimplement viomodel: construct on heap
  virtual VioGeneratorModel* NewModel(QObject* parent=0) const;
  virtual VioData* NewData(QObject* parent=0) const;
  virtual VioView* NewView(QWidget* parent=0) const;
  virtual VioView* NewPropertyView(QWidget* parent=0) const;
  virtual VioWidget* NewWidget(QWidget* parent=0) const;

  // typed read access to faudes object
  const faudes::vGenerator* Generator(void) const;
  int Size(void) const { if(!Generator()) return 0; return Generator()->Size();};

  // typed access to configuration
  VioGeneratorStyle* GeneratorConfiguration(void) const;

  // access to representation models
  const QList<VioGeneratorAbstractModel*> ModelList(void) { return mModelList; };

  // set/get default layout
  const VioGeneratorLayout& Layout(void) { return *mpUserLayout; };
  void Layout(const VioGeneratorLayout& layout) { *mpUserLayout=layout; };

  // reimplement to clear childs
  virtual void Modified(bool ch);

  // global vio data access
  VioData* Data(void);
  int Data(const VioData* pData);
  int TypeCheckData(const VioData* pData);

  // directed vio data access
  int InsertData(const VioData* pData);
  VioData* SelectionData(void);
  void DeleteSelection(void);



public slots:  

  /*
  void SortAscendingX1(void);
  void SortDescendingX1(void);
  void SortAscendingEv(void);
  void SortDescendingEv(void);
  void SortAscendingX2(void);
  void SortDescendingX2(void);
  */

  // editing slots: some representation view of the application
  // asks the model to perform an action; thus, the model will
  // perform the action uniform to all representations models.

  // editing faudes object: by elementary type
  // (return non-void on success)
  VioElement ElementIns(const VioElement& elem);
  //VioElement ElementIns(const VioElement& elem, int pos);
  VioElement ElementDel(const VioElement& elem);
  //VioElement ElementDel(int pos);
  //VioElement ElementEdit(int pos, const VioElement& delem);
  VioElement ElementEdit(const VioElement& selem, const VioElement& delem);
  //VioElement ElementMove(int from, int to);
  VioElement ElementName(const VioElement& elem, const QString& name);

  bool ElementExists(const VioElement& elem) const;  
  //VioElement Element(VioElement::EType, int pos) const;
  //int ElementIndex(const VioElement& elem) const;
  std::string ElementStr(const VioElement& elem) const;

  // editing faudes object: attributes
  const VioAttributeStyle* AttributeConfiguration(VioElement::EType etype) const;
  const QList<VioBooleanProperty>& ElementBooleanProperties(VioElement::EType etype) const;
  bool ElementBooleanProperty(const VioElement& lem, int prop) const;
  bool ElementBooleanProperty(const VioElement& elem, int prop, bool val);
  VioElement ElementAttr(const VioElement& elem, const faudes::AttributeVoid& attr);
  bool ElementAttrTest(const VioElement& elem, const faudes::AttributeVoid& attr) const;
  faudes::AttributeFlags* ElementAttr(const VioElement& elem) const; // caller owns results (!!)

  // selection: write access (uniform type)
  void Select(const VioElement& elem, bool on=true);
  void SelectAllStates(void);
  void SelectAllTransitions(void);
  

signals:

  // editing signals: the model did perform changes and sends 
  // this signal to all virtual models.

  // editing faudes object: by elementary type
  void NotifyElementIns(const VioElement& elem);
  void NotifyElementDel(const VioElement& elem);
  void NotifyElementEdit(const VioElement& selem, const VioElement& delem);
  void NotifyElementProp(const VioElement& elem);

  // editing: user layout change
  void NotifyLayout(void);

  // notification of faudes object changes: global
  void NotifyTrimElements(void);  
  void NotifyAnyAttr(void); 

  // other changes (internal, gets mapped to NotifyAnyChange)
  void NotifyChange(void);

protected:

  // typed version of faudes object
  faudes::vGenerator* mpFaudesGenerator;

  // typed version of configuration
  VioGeneratorStyle* pGeneratorConfig;
  int mLayoutFlags;


  // representation data (post 0.47)
  VioGeneratorData* mpGeneratorData;

  // representation models (pre 0.47)
  QList<VioGeneratorAbstractModel*> mModelList;
  VioGeneratorListModel*  mTransList;
  VioGeneratorListModel*  mStateList;
  VioGeneratorListModel*  mEventList;
  VioGeneratorGraphModel* mGraph;

  // default layout
  VioGeneratorLayout* mpUserLayout;

  // reimplement viomodel: allocate faudes generator and visual model data
  virtual void DoFaudesAllocate(void);
  virtual void DoVioAllocate(void);

  // reimplement assignment: do best for generators
  VioGeneratorModel& DoAssign(const VioModel& rSrc);

  // reimplement viomodel: test whether we can host this faudes object
  virtual int DoTypeCheck(const faudes::Type* fobject) const;

  // reimplement viomodel: token io, implementation 
  virtual void DoVioWrite(faudes::TokenWriter& rTw) const;
  virtual void DoVioRead(faudes::TokenReader& rTr);

  // reimplement viomodel: update visual data from (new) faudes object
  virtual void DoVioUpdate(void);

  // connect another representation model (we take ownership)
  void InsertRepresentationModel(VioGeneratorAbstractModel* repmodel);

  // data access: merge data
  virtual int DoMergeData(const VioData* pData);

};


/*
 ************************************************
 ************************************************

 A VioGeneratorAbstractModel is the virtual base of all
 viodes models for the representation of a generator,
 eg list and graph representation. 
 
 The VioGeneratorAbstractModel provides a minimal 
 editing interface to receive notifications of 
 generator changes. Derive classes are meant to
 hold representation data and relevant access methods.
 Technically, VioGeneratorAbstractModel is not virtual, 
 you can actually instantuate objects. However, they dont 
 hold any representation data and thus are good for
 debugging/development only.

 ************************************************
 ************************************************
 */


class VIOGEN_API VioGeneratorAbstractModel : public QObject {

Q_OBJECT

public:

  // construct/destruct
  VioGeneratorAbstractModel(VioGeneratorModel* parent);
  virtual ~VioGeneratorAbstractModel(void) {}; // todod: tell my view?

  // create new view for this representationmodel
  virtual VioGeneratorAbstractView* NewView(VioGeneratorView* parent);

  // clear to default/empty representation
  void Clear(void) {};

  // convenience: access faudes generator, typed model parent, and typed configuration
  const faudes::vGenerator* Generator(void) const;
  VioGeneratorModel* GeneratorModel(void);
  VioGeneratorStyle* GeneratorConfiguration(void) const;

  // token io (faudes exceptions on errors)
  void Write(faudes::TokenWriter& rTw) const;
  void Write(const QString& rFilename) const;
  void Read(faudes::TokenReader& rTr);
  void Read(const QString& rFilename);

  // global vio data access
  virtual VioGeneratorAbstractData* Data(void) {return 0;};
  virtual VioGeneratorAbstractData* SelectionData(void) { return Data();};
  virtual int Data(const VioGeneratorAbstractData* pData) {(void) pData; return 1;};
  virtual int TypeCheckData(const VioGeneratorAbstractData* pData) {(void) pData; return 1;};

  // record/query changes 
  bool Modified(void) const;
  virtual void Modified(bool ch);

public slots:

  // collect and pass on modifications of childs
  void ChildModified(bool ch);

  // editing slots: the generator model notifies us on changes
  // performed on the faudes generator

  // editing faudes object: by elementary type, defaults to update all
  virtual void UpdateElementIns(const VioElement& elem);
  virtual void UpdateElementDel(const VioElement& elem);
  virtual void UpdateElementEdit(const VioElement& selem, const VioElement& delem);
  virtual void UpdateElementProp(const VioElement& elem);

  // editing faudes object: global, defaults to update all
  virtual void UpdateTrimElements(void);
  virtual void UpdateAnyAttr(void);
  virtual void UpdateAnyChange(void);
  virtual void UpdateNewModel(void);

  // editing model: selection
  virtual void UpdateSelectionElement(const VioElement& elem, bool on=true);
  virtual void UpdateSelectionClear(void);
  virtual void UpdateSelectionAny(void);

signals:

  // notify vio generator widget on changees
  void NotifyModified(bool ch);

  // editing signals: we signal our views about changes in the faudes generator
  // by passing on update requests in the above slots

  // notification of faudes object changes: elements
  void NotifyElementIns(const VioElement& elem);
  void NotifyElementDel(const VioElement& elem);
  void NotifyElementEdit(const VioElement& selem, const VioElement& delem);
  void NotifyElementProp(const VioElement& elem);

  // notification of faudes object changes: global
  void NotifyTrimElements(void);  
  void NotifyAnyAttr(void); 
  void NotifyAnyChange(void);  


protected:

  // update visual data from (new) faudes object
  virtual void DoVioUpdate(void);

  // token io, should reimplement 
  virtual void DoVioWrite(faudes::TokenWriter& rTw) const { (void) rTw;};
  virtual void DoVioRead(faudes::TokenReader& rTr) { (void) rTr;};

  // typed ref to parent
  VioGeneratorModel* pVioGeneratorModel;

  // record changes
  bool mModified;

};


/*
 ************************************************
 ************************************************

 A VioGeneratorView is a widget to represent a
 generator provided as VioGeneratorModel.
 Each VioGeneratorAbstractModel from the VioGeneratoModel
 is represented by a  VioGeneratorAbstractView. Together, 
 they form the content of a split view.

 ************************************************
 ************************************************
 */



class VIOGEN_API VioGeneratorView : public VioView {

Q_OBJECT

public:
  // construct/destruct
  VioGeneratorView(QWidget* parent=0, VioStyle* config=0, bool alloc=true);
  virtual ~VioGeneratorView(void);

  // set/get model
  virtual int Model(VioModel* model);
  virtual VioGeneratorModel* Model(void);

  // directed vio data access
  // for generators, these are implemented in the model; we throughpass
  // the request to have a uniform interface
  int InsertData(const VioData* pData);
  VioData* SelectionData(void);
  void DeleteSelection(void);

  // write to image file
  int ExportSvg(const QString& filename);
  int ExportPdf(const QString& filename);
  int ExportEps(const QString& filename);
  int ExportJpg(const QString& filename);
  int ExportPng(const QString& filename);

public slots:

  // user adjust layout (ignore on error)
  void ShowPropertyView(bool on=true);
  void ToggleViews(void);
  void ZoomIn(qreal sf=1.5);
  void ZoomOut(qreal sf=0.66);
  void ZoomFit(void);
  void GridVisible(bool);

  // save current layput to model
  virtual void SaveUserLayout(void);

  // highlite/show request (pass on to indivudual views)
  virtual void Highlite(const VioElement& elem, bool on=true);
  virtual void HighliteClear(void);
  virtual void Show(const VioElement& elem);

  // user exports svg file
  virtual void UserExport(void);


signals:

  // forward show request (this view -> model -> other views)
  void NotifyShow(const VioElement& elem);
  void NotifyHighlite(const VioElement& elem, bool on);
  void NotifyHighliteClear(void);

protected slots:

  // debugging splitter signals
  void TrackSplitter(void);

  // update text version
  void DoPlainText(void);

  // update view layout
  virtual void UpdateUserLayout(void);


protected:


  // typed version of configuration
  VioGeneratorStyle* pGeneratorConfig;
  int mLayoutFlags;

  // typed representation data
  VioGeneratorModel* pGeneratorModel;

  // allocate visual view data
  virtual void DoVioAllocate(void);

  // update view from (new) model
  virtual void DoVioUpdate(void);

  // update view to model (exception on error)
  virtual void DoModelUpdate(void);

  // widgets for abstract views
  QList<VioGeneratorAbstractView*> mViewList;

  // layout state
  VioGeneratorLayout* mUserLayout;

  // layout items
  QSplitter* mSplitter;
  QTabWidget* mTabbed;

  // views
  QWidget* pListView;
  VioGeneratorGraphView* pGraphView;
  VioGeneratorPropertyView* mPropView;

  // actions
  QAction* mListAction;
  QAction* mGraphAction;
  QAction* mToggleAction;
  QAction* mPropAction;
  QAction* mZoomInAction;
  QAction* mZoomOutAction;
  QAction* mZoomFitAction;
  QAction* mGridAction;
  QAction* mExportAction;
  QAction* mSelectAllStatesAction;
  QAction* mSelectAllTransitionsAction;

};



/*
 ************************************************
 ************************************************

 A VioGeneratorAbstractView is a widget to represent 
 a generator wrt a VioGeneratorAbstractModel.
 Both virtual classes need matching derivates for 
 this to wort out, eg a model specialised for
 a graph representation and a widget that actually
 performs the representation.

 The VioGeneratorAbstractView directly uses callbacks
 on the VioGeneratorAbstractModel to pass on user requests
 for changes in the vio generator. Vice versa, we expect 
 notifications on changes in the generator widget by our
 VioGeneratorAbstractModel, ie when another 
 representation was edited by the user. 

 Technically, VioGeneratorAbstractView is not virtual, 
 you can actually instantiate objects. However, they will
 show debugging information and thus are good for 
 debugging/development only.


 ************************************************
 ************************************************
 */



class VIOGEN_API VioGeneratorAbstractView : public QWidget {

Q_OBJECT

public:

  // construct/destruct
  VioGeneratorAbstractView(VioGeneratorView* parent=0);
  virtual ~VioGeneratorAbstractView(void) {}; // todo: tell my model that im gone?

  // set/get model
  // note: the view does *NOT* become the owner of the model
  // note: return non-zero if we cannot represnt this model
  // note: setting the model updates the ref to the VioGeneratorModel
  // note: setting the model will connect its notification signals to my slots
  virtual int Model(VioGeneratorAbstractModel* model);
  const VioGeneratorAbstractModel* Model(void) const;

  // update model from view (default: allways in sync with view)
  virtual void DoModelUpdate(void) {};

  // update view from model 
  virtual void DoVioUpdate(void);

  // convenience: access faudes generator and vio widget 
  const faudes::vGenerator* Generator(void) const;
  const VioGeneratorModel* GeneratorModel(void) const;
  const VioGeneratorView* GeneratorView(void) const;


public slots:

  // editing slots: my virtual model notifies me that the faudes
  // generator and the relevant representation data has changed.
  // user editing is passed on by callbacks to the VioGeneratorModel

  // editing faudes object: by elementary type, defaults to update all
  virtual void UpdateElementIns(const VioElement& elem);
  virtual void UpdateElementDel(const VioElement& elem);
  virtual void UpdateElementEdit(const VioElement& selem, const VioElement& delem);
  virtual void UpdateElementProp(const VioElement& elem);

  // editing faudes object: global, defaults to update all
  virtual void UpdateTrimElements(void);
  virtual void UpdateAnyAttr(void);
  virtual void UpdateAnyChange(void);
  virtual void UpdateNewModel(void);

  // highlite/show request
  virtual void Highlite(const VioElement& elem, bool on=true);
  virtual void HighliteClear(void);
  virtual void Show(const VioElement& elem);



protected:

  // model to present
  VioGeneratorAbstractModel* pGeneratorAbstractModel;

  // typed parent refs
  VioGeneratorModel* pGeneratorModel;
  VioGeneratorView* pGeneratorView;
  const faudes::Generator* pFaudesGenerator;

  // main layout
  QVBoxLayout* mVbox;
  QLabel* mTextInfo;
  QPlainTextEdit* mTextEdit;

};



/*
 ************************************************
 ************************************************

 A VioGeneratorPropertyView is a widget to represent 
 properties of generator elements, ie the element
 id and the element attributes.

 ************************************************
 ************************************************
 */



class VIOGEN_API VioGeneratorPropertyView : public VioView {

Q_OBJECT

public:
  // construct/destruct
  VioGeneratorPropertyView(QWidget* parent=0, VioStyle* config=0, bool alloc=true);
  virtual ~VioGeneratorPropertyView(void);

public slots:

  // reimplement show 
  void Show(const VioElement& elem);
  void UpdateSelectionChange(void);

protected:

  // allocate visual view data
  virtual void DoVioAllocate(void);

  // update view from (new) model
  virtual void DoVioUpdate(void);

  // update view to model (exception on error, not implemented)
  virtual void DoModelUpdate(void);

  // typed representation data
  VioGeneratorModel* pGeneratorModel;

  // typed version of configuration
  VioGeneratorStyle* pGeneratorConfig;

  // layout items: a stacked vbox
  QVBoxLayout* mPropBox;
  PioTProp* mPropTrans;
  PioSProp* mPropState;
  PioEProp* mPropEvent;
  PioGProp* mPropGlobal;

};


/*
 ************************************************
 ************************************************

 A VioGeneratorWidget is a widget to represent a
 generator. Internally, representation data is
 managed by a number of VioGeneratorAbstractModel
 derivates.Each VioGeneratorAbstractModel is represented
 by a  VioGeneratorAbstractView. Together, they 
 form the content of a split view.

 ************************************************
 ************************************************
 */



class VIOGEN_API VioGeneratorWidget : public VioWidget {

Q_OBJECT

public:
  // construct/destruct
  VioGeneratorWidget(QWidget* parent=0, VioStyle* config=0, bool alloc=true);
  virtual ~VioGeneratorWidget(void);

  // reimplement set model
  virtual int Model(VioModel* model); 

protected:

  // allocate vio model/view
  virtual void DoVioAllocate(void);

  // typed representation data
  VioGeneratorModel* pGeneratorModel;
  VioGeneratorView*  pGeneratorView;

};


#endif
