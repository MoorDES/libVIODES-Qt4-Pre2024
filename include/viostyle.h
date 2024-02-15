/* viostyle.h  - viodes configuration */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006, 2007  Thomas Moor, Klaus Schmidt, Sebastian Perk

*/



#ifndef FAUDES_VIOSTYLE_H
#define FAUDES_VIOSTYLE_H


#include <cmath>
#include <string>
#include <iostream>
#include <bitset>

#include "libfaudes.h"

#include <QtGui>
#include <QPalette>

// win32 dll symbol export/import
#ifndef VIODES_BUILD_LIB
#define VIODES_API Q_DECL_IMPORT
#else
#define VIODES_API Q_DECL_EXPORT
#endif



// set version
#ifndef VIODES_VERSION
#define VIODES_VERSION "0.5x"
#endif

// fix consoleout namespace for libfaudes debug macros (should have our own)
#undef FAUDES_WRITE_CONSOLE
#define FAUDES_WRITE_CONSOLE(message)  \
  { if(!faudes::ConsoleOut::G()->Mute()) {   \
      std::ostringstream cfl_line; cfl_line << message << std::endl; faudes::ConsoleOut::G()->Write(cfl_line.str());} }

// have my own write progress with loopcallback
#define FD_QP(message) {FAUDES_WRITE_CONSOLE("FAUDES_PROGRESS:  " << message); faudes::LoopCallback();}


// debugging: all vio level
#ifdef FAUDES_DEBUG_VIO
#define FD_DQ(message) FAUDES_WRITE_CONSOLE("FAUDES_VIO: " << message)
#else
#define FD_DQ(message) { };
#endif


// debugging: helper (console etc)
#ifdef FAUDES_DEBUG_VIO_HELPER
#define FD_DQH(message) FAUDES_WRITE_CONSOLE("FAUDES_VIOHELPER: " << message)
#else
#define FD_DQH(message) { };
#endif

// debugging: extra widgets
#ifdef FAUDES_DEBUG_VIO_WIDGETS
#define FD_DQW(message) FAUDES_WRITE_CONSOLE("FAUDES_VIOWIDGETS: " << message)
#else
#define FD_DQW(message) { };
#endif


// operator << for qstrings
VIODES_API std::ostream& operator<<(std::ostream& o, const QString& str);


// convenience typedef: faudes generators we support
typedef faudes::vGenerator fGenerator;

// convenience typedef: alphabet we support 
typedef faudes::TaEventSet<faudes::AttributeCFlags> fAlphabet;

// convenience typedef: symbolic built-in colors
typedef enum { 
  VioNoColor, VioBlack, VioWhite, VioGrey, VioLightGrey, 
  VioFirstColor, VioRed=VioFirstColor, VioGreen, VioBlue, 
  VioYellow, VioMagents, VioCyan,
  VioDarkRed, VioDarkGreen, VioDarkBlue, 
} VioColor;

// forward
class VioModel;

/*
 ************************************************
 ************************************************

 A VioEditFunction supports an configurable function
 with intended user access via the edit menu. It consits
 of a name and a faudes function to be executed via the
 faudes registry. 

 ************************************************
 ************************************************
 */

class VIODES_API VioEditFunction {

public:
  // construct 
  VioEditFunction(void);
  VioEditFunction(const QString& name, const QString& ffnct);

  // Apply this function 
  void Apply(VioModel* model) const;

  // read from tokenreader (true on success)
  bool Read(faudes::TokenReader& rTr);

  // name of function
  QString mName;

  // associated faudes function
  QString mFFunction;
};

/*
 ************************************************
 ************************************************

 VioStyle provides global configuration of the
 viodes library by parameters and shared  helper 
 functions: path names, fonts, colors, 
 drawing primitives, license text, faudes symbol 
 sorting operators, etc.

 Style data is organized as static members with static 
 access functions. Thus, the global congiguration is
 global indeed, regardless whether there exists none, one
 or multiple instances of VioStyle and derived classes. 
 
 Before you first access any style data, you must call 
 VioStyle::Initialize() in order to set up hard-coded 
 default values. Alternatively, you may use VioStyle::ReadFile() 
 to initialise VioStyle by a configuration file.

 ************************************************
 ************************************************
 */


class VIODES_API VioStyle {

public:

  // construct/destruct
  VioStyle(const QString& ftype="Type");
  virtual ~VioStyle(void) {};

  // load hard-coded default
  static void Initialise();

  // global configuration  from file
  // notes: when first invoked, the filename is recorded and used for further invokations
  // as a default; thus, derived classes can read the file again to find their additional
  // sections. When first invoked with no filename specified, a platform dependent 
  // default path is used eg "(executablepath)/vioconfig.txt". 
  static void ReadFile(const QString& filename="");

  // helper function (for derived classes)
  static faudes::TokenReader* NewStyleReader(const QString& section, const QString& filename);

  // have a static instance
  // note: this is for compatibility with derived classes
  static VioStyle* G(void);

  // viodes: configuration name and file
  static const QString& ConfigFile(void) { return mConfigFile; };
  static const QString& ConfigName(void) { return mConfigName; };


  // faudes break flag
  static void FaudesBreakSet(void);
  static void FaudesBreakClr(void);

  // viodes: configuration variant for derived classes
  QString FaudesType(void) const;
  bool UserAccess(void) const;
  bool GroupView(void) const;

  // viodes: edit functions for derived classes
  QList<VioEditFunction> EditFunctions(void) { return mEditFunctions; };
  void InsEditFunction(const VioEditFunction& vfnct);
    

  // faudes: default faudes data
  static const std::string& StateSymbol (void) { return mStateSymbol; };
  static const std::string& EventSymbol (void) { return mEventSymbol; };

  // faudes: dot executable
  static const QString& DotExecutable(void) { return mDotExecutable; };

  // license text
  static const QString& LicenseText(void) { return mLicenseText; };

  // viodes plugin info
  static QString PluginsString(void);

  // faudes: convert plain strings
  static std::string StrFromQStr(const QString& qstr);
  static std::string LfnFromQStr(const QString& qstr);
  static QString QStrFromStr(const std::string& str);

  // faudes: convert symbol strings
  static faudes::Idx IdxFromSymbol(const QString& name, const faudes::SymbolTable* pSymbolTable=0);
  static faudes::Idx IdxFromSymbolDq(const QString& name, const faudes::SymbolTable* pSymbolTable=0);
  static faudes::Idx IdxFromSymbol(const std::string& name, const faudes::SymbolTable* pSymbolTable=0);
  static QString SymbolFromIdx(faudes::Idx idx, const faudes::SymbolTable* pSymbolTable=0);
  static bool ValidSymbol(const QString& name);
  static bool ValidSymbol(const std::string& name);
  static bool ValidFakeSymbol(const QString& name);
  static bool ValidFakeSymbol(const std::string& name);

  // faudes: convert ftu strings
  static QString QStrFromFtu(faudes::Time::Type ftu);
  static faudes::Time::Type FtuFromQStr(const QString& str);
  static QString QStrFromFloat(faudes::Float x);
  static faudes::Float FloatFromQStr(const QString& str);

  // faudes: convenience string conversion
  static QString DispStateName(const fGenerator* gen, faudes::Idx idx);
  static QString DispEventName(const fGenerator* gen, faudes::Idx idx);
  static QString DispEventName(const faudes::EventSet* set, faudes::Idx idx);
  static QString SortName(const QString& str);
  static QString SortStateName(const fGenerator* gen, faudes::Idx idx);
  static QString SortEventName(const fGenerator* gen, faudes::Idx idx); 
  static QString SortEventName(const faudes::EventSet* set, faudes::Idx idx); 

  // faudes: setup state/event name qt lists from generator
  static void StatesQStrList(QList<QString>& states, const fGenerator* pGen);
  static void EventsQStrList(QList<QString>& events, const fGenerator* pGen);
  static void EventsQStrList(QList<QString>& events, const faudes::EventSet* pAlph);

  // drawing: default pen/font
  static const QFont& DefaultFont(void) { return *mpDefaultFont; };
  static const QFontMetricsF& DefaultFontMetrics(void) { return *mpDefaultFontMetrics; };

  // drawing pens
  static const QPen& DefaultPen(void) { return *mpDefaultPen; };
  static const QBrush& DefaultBrush(void) { return *mpDefaultBrush; };
  static const QPen& HighlitePen(void) { return *mpHighlitePen; };
  static const QPen& WarningPen(void) { return *mpWarningPen; };
  static const QBrush& WarningBrush(void) { return *mpWarningBrush; };
  static const QPen& DefaultDashed(void) { return *mpDashedPen; };
  static const QPen& DefaultDotted(void) { return *mpDottedPen; };
  static const QPen& GridNPen(void) { return *mpGridNPen; };
  static const QPen& GridBPen(void) { return *mpGridBPen; };
  static const QPen& GridXPen(void) { return *mpGridXPen; };

  // drawing: static parameters
  static const qreal& ArrowRatio(void) { return mArrowRatio;};
  static const qreal& ArrowSize(void) { return mArrowSize;};
  static const qreal& LineShapeSize(void) { return mLineShapeSize;};
  static const qreal& CtrlTolerance(void) { return mCtrlTolerance;};
  static const qreal& GridWidth(void) { return mGridWidth;};
  static const qreal& ExportResolution(void) { return mExportResolution;};
  static const qreal& ExportMaxSize(void) { return mExportMaxSize;};
  
  // drawing: translate color
  static int Colors(void);
  static void Color(const QString& name, const QColor& color);
  static const QColor& Color(int color);
  static const QColor& Color(const QString& color);
  static int ColorIndex(const QString& color);
  static const QString& ColorName(int idx);
  static QList<QString> ColorNames(void);

  // drawing: manipulate colors
  static QColor ColorTaint(const QColor& value, const QColor& huecol, qreal sat);

  // drawing: 2D vectors
  static qreal NormF(const QPointF& point);
  static QPointF NormalizeF(const QPointF& point);
  static QPointF NormalF(const QPointF& point);
  static QPointF LotF(const QPointF& base, const QPointF& point);
  static qreal ScalarF(const QPointF& a, const QPointF& b);

  // drawing: draw elements
  static void ArrowTip(QPainterPath& arrow, const QPointF& pointA, const QPointF& pointB);
  static void FixArrow(QPointF& pointA, const QPointF& pointB, const QPainterPath& pathB);

  // drawing: draw text
  static void TextLL(QPainterPath& path, const QPointF& pos, const QString& text);
  static void TextLL(QRectF& drect, const QPointF& pos, const QString& text);
  static void TextCR(QPainterPath& path, const QRectF& rect, const QString& text);
  static void TextCR(QRectF& drect, const QRectF& rect, const QString& text);
  static void TextCP(QPainterPath& path, const QPointF& center, const QString& text);
  static void TextCP(QRectF& drect, const QPointF& center, const QString& text);


  // drawing: magnetic grid points
  static QPointF GridPoint(const QPointF& point);

  // drawing: cursors
  static const QCursor& CursorCross(void) { return *mpCursorCross; };
  static const QCursor& CursorCrossS(void) { return *mpCursorCrossS; };;
  static const QCursor& CursorCrossC(void) { return *mpCursorCrossC; };;
  static const QCursor& CursorCrossP(void) { return *mpCursorCrossP; };;
 
  
  
private:

  // faudes callback function/flag
  static bool FaudesBreakFnct(void);
  static bool mFaudesBreakFlag;

  // color defs
  static QList<QColor> mColorDefs;
  static QMap<QString,int>  mColorIndexes;
  static QMap<int,QString>  mColorNames;

  // static member declaration
  static QString mConfigFile;
  static QString mConfigName;
  static std::string mStateSymbol;
  static std::string mEventSymbol;
  static QString mDotExecutable;
  static QString mLicenseText;
  static qreal mArrowRatio;
  static qreal mArrowSize;
  static qreal mLineShapeSize;
  static qreal mCtrlTolerance;
  static qreal mGridWidth;
  static qreal mExportResolution;
  static qreal mExportMaxSize;
  static QPen*   mpDefaultPen;
  static QPen* mpDashedPen; 
  static QPen* mpDottedPen;
  static QBrush* mpDefaultBrush;
  static QPen*   mpHighlitePen;
  static QPen*   mpWarningPen;
  static QBrush*   mpWarningBrush;
  static QPen*   mpGridNPen;
  static QPen*   mpGridBPen;
  static QPen*   mpGridXPen;
  static QFontMetricsF* mpDefaultFontMetrics;
  static QFont* mpDefaultFont;
  static QCursor* mpCursorCross;
  static QCursor* mpCursorCrossS;
  static QCursor* mpCursorCrossC;
  static QCursor* mpCursorCrossP;

protected:

  // config variant
  QString mFaudesType;

  // config variant
  bool mUserAccess;

  // config variant
  bool mGroupView;

  // edit functions (for derived styles)
  QList<VioEditFunction> mEditFunctions;

};


/*
 *****************************************************
 *****************************************************
 helper: hash for faudes transitions
 *****************************************************
 *****************************************************
 */

inline uint qHash(const faudes::Transition& ftrans) {
  return qHash(ftrans.X1) ^ qHash(ftrans.X2) ^ qHash(ftrans.Ev);
}

/*
 *****************************************************
 *****************************************************
 A VioAlternateLayout similar to a QStackedWidget
 show one widget at a time. In contrast to the stcked widget 
 it is based on a vbox and hence adjutst its size according
 to the currentr widget.
 *****************************************************
 *****************************************************
 */

class VIODES_API VioAlternateLayout : public QVBoxLayout {

Q_OBJECT

public:
  // constructors and destructors
  VioAlternateLayout(QWidget *parent=0) : QVBoxLayout(parent) {setMargin(0); setSpacing(0);};
  ~VioAlternateLayout() {};

  // track recent widget
  int addWidget (QWidget* widget) {
    QVBoxLayout::addWidget(widget);
    setCurrentWidget(widget);
    return count()-1;
  }

public slots:

  // here we are (.. suboptimal)
  void setCurrentWidget(QWidget* widget) {
    for(int i=0; i< count(); i++) {
      QWidget* iw= itemAt(i)->widget();
      if(!iw) continue;
      if(iw==widget) iw->show();
      else iw->hide();
    };
  };
};


/*
 *****************************************************
 *****************************************************
 A VioListWidget is simular to a QListWidget except that
 it attepmts to ha a fixed height.

 ( not functional / unfinished )
 *****************************************************
 *****************************************************
 */

class VIODES_API VioListWidget : public QListWidget {

Q_OBJECT

public:

  // constructors and destructors
  VioListWidget(QWidget *parent=0) : QListWidget(parent) {
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
  };
  ~VioListWidget() {};

  // reimplement sizeHint()
  virtual QSize sizeHint(void) const { 
    QSize res = QListWidget::sizeHint();
    res.setHeight(20);
    return res;
  };


};

/*
 *****************************************************
 *****************************************************

 A VioFtuValidator is a validator for faudes time units

 *****************************************************
 *****************************************************
 */


class VIODES_API VioFtuValidator : public QIntValidator {

Q_OBJECT

public:
  // construct
  VioFtuValidator(QObject* parent);
  ~VioFtuValidator(void);

  // reimplement validation
  State validate(QString & input, int & pos) const;
};



/*
 *****************************************************
 *****************************************************

 helper: fix my checkbox: no user tristate

 *****************************************************
 *****************************************************
 */


class VIODES_API VioCheckBox : public QCheckBox {

  Q_OBJECT

  public:
  
  VioCheckBox(QWidget* parent=0) : QCheckBox(parent) {
    setTristate(true);};

  // reset to disabled
  void ResetCheckState(void) {
    setEnabled(false);
    setCheckState(Qt::PartiallyChecked);
  }

  // merge consistent
  void MergeCheckState(bool qs) {
    // set
    if(!isEnabled()) {
      setEnabled(true);
      setCheckState(qs ? Qt::Checked : Qt::Unchecked);
      return;
    }
    // merge
    if(qs && checkState()==Qt::Checked) return;
    if((!qs) && checkState()==Qt::Unchecked) return;
    setCheckState(Qt::PartiallyChecked);
  }
   
  // user progress skip invalid (reimplement base)
  void nextCheckState(void) {
    if(!isEnabled()) return;
    if(checkState()== Qt::Checked) setCheckState(Qt::Unchecked);
    else if(checkState()== Qt::Unchecked) setCheckState(Qt::Checked);
    else if(checkState()== Qt::PartiallyChecked) setCheckState(Qt::Checked);
  }

 private:


};


#endif
