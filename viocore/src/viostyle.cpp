/* viostyle.h  - viodes configuration */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006, 2007  Thomas Moor, Klaus Schmidt, Sebastian Perk

*/

#include "viostyle.h"
#include "viotypes.h"
#include "vioregistry.h"
#include "vioattrstyle.h"


/*
 *************************************
 *************************************
 *************************************

 Implementation of VioEditFunction

 *************************************
 *************************************
 *************************************
 */


// constructor
VioEditFunction::VioEditFunction(void) {};
VioEditFunction::VioEditFunction(const QString& name, const QString& ffnct) :
  mName(name),
  mFFunction(ffnct)
{};

// attribute style relaxed reading 
bool VioEditFunction::Read(faudes::TokenReader& rTr) {
  faudes::Token token;
  rTr.Peek(token);
  if(token.Type()!=faudes::Token::String) return false;
  mName=VioStyle::QStrFromStr(rTr.ReadString());
  rTr.Peek(token);
  if(token.Type()!=faudes::Token::Option) return false;
  mFFunction=VioStyle::QStrFromStr(rTr.ReadOption());
  if(mFFunction.startsWith("faudes::")) mFFunction.remove(0,QString("faudes::").length());
  return true;
}

// execute
void VioEditFunction::Apply(VioModel* model) const {
  FD_DQT("VioEditFunction::Apply(): fnct: " << VioStyle::StrFromQStr(mFFunction));
  (void) model;
}

/*
 *************************************
 *************************************
 *************************************

 Implementation of VioStyle: static vars

 *************************************
 *************************************
 *************************************
 */

// static instance
VioStyle* mpStaticInstance=0;

// static default: faudes names
std::string VioStyle::mStateSymbol="st_1";
std::string VioStyle::mEventSymbol="ev_1";
QString VioStyle::mDotExecutable="dot";

// static default: name of this configuration
QString VioStyle::mConfigName = "VioStdConfig";
QString VioStyle::mConfigFile = "";

// static: license 
QString VioStyle::mLicenseText = 
   "<p><b>License:</b> This software is provided \"as is\" without any warranty. "
   "This software includes components distributed under terms of the LGPL license. "
   "For directions to the corresponding source code and for further legal details, "
   "please consult the provided LICENSE file.</p>";   

// static: faudes break
bool VioStyle::mFaudesBreakFlag=false;


// static default: geometry of arrow
qreal VioStyle::mArrowRatio=0.66;
qreal VioStyle::mArrowSize =8;

// static default: tolerance in selection
qreal VioStyle::mLineShapeSize= 10.0;
qreal VioStyle::mCtrlTolerance = 7;

// static default: mesh width of grid 
qreal VioStyle::mGridWidth=10;

// static default: bitmap for graphics export
qreal VioStyle::mExportResolution = 10; // pixel per point 
qreal VioStyle::mExportMaxSize= 3000;   // pixels

// static pens
QPen*   VioStyle::mpDefaultPen;
QBrush* VioStyle::mpDefaultBrush;
QPen*   VioStyle::mpHighlitePen;
QPen*   VioStyle::mpWarningPen;
QBrush* VioStyle::mpWarningBrush;
QPen* VioStyle::mpDashedPen; 
QPen* VioStyle::mpDottedPen; 
QPen*   VioStyle::mpGridNPen;
QPen*   VioStyle::mpGridBPen;
QPen*   VioStyle::mpGridXPen;

// static cursors
QCursor* VioStyle::mpCursorCross;
QCursor* VioStyle::mpCursorCrossS;
QCursor* VioStyle::mpCursorCrossP;
QCursor* VioStyle::mpCursorCrossC;


// static fonts
QFontMetricsF* VioStyle::mpDefaultFontMetrics;
QFont* VioStyle::mpDefaultFont;

// static color defs
QList<QColor> VioStyle::mColorDefs;
QMap<QString,int>  VioStyle::mColorIndexes;
QMap<int,QString>  VioStyle::mColorNames;



/*
 *************************************
 *************************************
 *************************************

 Implementation of VioStyle: setup

 *************************************
 *************************************
 *************************************
 */



// constructor (fused for derived classes)
VioStyle::VioStyle(const QString& ftype) {
 mFaudesType=ftype; 
 mUserAccess=true;
 mGroupView=false;
} 

// hard-coded defaults
void VioStyle::Initialise(void) {  
  // we are initialized
  if(mpStaticInstance) return;
  // report
  FD_DQ("VioStyle::Initialise() ");
  // static instance 
  mpStaticInstance=new VioStyle();
  // set my faudes loop callback
  faudes::LoopCallback(&FaudesBreakFnct);

  // clear colors
  mColorDefs.clear();
  mColorNames.clear();
  // built in colors (order must match enum ... )
  Color("NoColor",QColor(255, 255, 255));
  Color("Black",  QColor(0, 0, 0));
  Color("White",  QColor(255, 255, 255));
  Color("Grey",   QColor(80, 80, 80));
  Color("LightGrey",   QColor(175, 175, 175));
  Color("Red",    QColor(128, 0, 0));
  Color("Green",  QColor(0, 128, 0));
  Color("Blue",   QColor(0, 0, 128));
  Color("Yellow", QColor(96, 96, 0));
  Color("Magenta",QColor(96, 0, 96));
  Color("Cyan",   QColor(0, 96, 96));
  Color("DarkRed",    QColor(80, 0, 0));
  Color("DarkGreen",  QColor(0, 80, 0));
  Color("DarkBlue",   QColor(0, 0, 80));
  // pen
  mpDefaultPen=new QPen();
  mpDefaultPen->setStyle(Qt::SolidLine);
  mpDefaultPen->setBrush(Qt::SolidPattern);
  mpDefaultPen->setBrush(Qt::black);
  mpDefaultPen->setWidthF(1);
  // alt pens
  mpDashedPen=new QPen(*mpDefaultPen);
  QVector<qreal> dashvec;
  dashvec.append(5);
  dashvec.append(7.5);
  mpDashedPen->setDashPattern(dashvec);
  mpDottedPen=new QPen(*mpDefaultPen);
  QVector<qreal> dotvec;
  dotvec.append(2);
  dotvec.append(3);
  mpDottedPen->setDashPattern(dotvec);
  // brush
  mpDefaultBrush= new QBrush();
  mpDefaultBrush->setStyle(Qt::SolidPattern);
  mpDefaultBrush->setColor(Qt::black);
  // highlight pen   
  mpHighlitePen=new QPen();
  mpHighlitePen->setStyle(Qt::SolidLine);
  mpHighlitePen->setBrush(Qt::SolidPattern);
  mpHighlitePen->setBrush(Qt::red);
  mpHighlitePen->setWidthF(10);
  // warning pen   
  mpWarningPen=new QPen();
  mpWarningPen->setStyle(Qt::SolidLine);
  mpWarningPen->setBrush(VioStyle::Color("Red"));
  mpWarningPen->setWidthF(1);
  // brush
  mpWarningBrush= new QBrush();
  mpWarningBrush->setStyle(Qt::SolidPattern);
  mpWarningBrush->setColor(VioStyle::Color("Red"));
  // grid pen   
  mpGridNPen=new QPen();
  mpGridNPen->setStyle(Qt::SolidLine);
  mpGridNPen->setBrush(VioStyle::Color("Blue").lighter(350));
  mpGridNPen->setWidthF(1);
  mpGridBPen=new QPen();
  mpGridBPen->setStyle(Qt::SolidLine);
  mpGridBPen->setBrush(VioStyle::Color("Blue").lighter(250));
  mpGridBPen->setWidthF(1);
  mpGridXPen=new QPen();
  mpGridXPen->setStyle(Qt::SolidLine);
  //mpGridXPen->setBrush(VioStyle::Color("Red"));
  mpGridXPen->setBrush(Qt::red);
  mpGridXPen->setWidthF(1);
  // fonts
  mpDefaultFont = new QFont();
  mpDefaultFontMetrics = new QFontMetricsF(*mpDefaultFont,0);
  // cursors: cross
  mpCursorCross = new QCursor(Qt::CrossCursor);
  // cursors: variations: 
  /*
  THIS IS NOT FUCNTIONAL ... CANNOT GET STD CURSOR 
  QPixmap scross(mpCursorCross->pixmap());
  FD_WARN("VioStyle(): Cursor: " << scross.width() << " " << scross.height());
     CursorCross->pixmap().height());
   QRectF scrossr = scross.rect();
  QPainter* scrossp = new QPainter(&scross);
  scrossp->fillRect(scrossr, Qt::red);
  scrossp->drawText(scrossr, Qt::AlignCenter,"S");
  delete scrossp;
  mpCursorCrossS = new QCursor(scross);
  */  
  mpCursorCrossS = new QCursor(Qt::CrossCursor);
  mpCursorCrossP = new QCursor(Qt::CrossCursor);
  mpCursorCrossC = new QCursor(Qt::CrossCursor);
  // guess dot executable, trying default "dot" or config-override first
  if(QProcess::execute(mDotExecutable + " -V")!=0) {
  mDotExecutable="dot";
  if(QProcess::execute(mDotExecutable + " -V")!=0) {
  mDotExecutable="/usr/bin/dot";
  if(QProcess::execute(mDotExecutable + " -V")!=0) {
  mDotExecutable="/usr/local/bin/dot";
  if(QProcess::execute(mDotExecutable + " -V")!=0) {
  mDotExecutable="/Applications/Graphviz.app/Contents/MacOS/dot";
  if(QProcess::execute(mDotExecutable + " -V")!=0) {
  mDotExecutable="c:\\Programme\\Graphviz2.16\\bin\\dot.exe";
  if(QProcess::execute(mDotExecutable + " -V")!=0) {
  mDotExecutable="\"c:\\Program Files (x86)\\Graphviz2.38\\bin\\dot.exe\"";
  if(QProcess::execute(mDotExecutable + " -V")!=0) {
  mDotExecutable="dot.exe";
  if(QProcess::execute(mDotExecutable + " -V")!=0) {
  mDotExecutable="(dot not found)";
  }}}}}}}};
  FD_WARN("VioStyle(): using dot executable: " << mDotExecutable);
  mDotExecutable="dot";
}

// helper function for derived classes
faudes::TokenReader* VioStyle::NewStyleReader(const QString& section, const QString& filename) {
  // open that file if specified/exists
  std::string fname=VioStyle::LfnFromQStr(filename);
  std::string fsection=VioStyle::StrFromQStr(section);
  if(fname=="") fname=VioStyle::LfnFromQStr(VioStyle::ConfigFile());
  faudes::TokenReader* trp=0;
  try { 
    FD_DQT("VioAttributeStyle::Config: read file " << fname);
    trp = new faudes::TokenReader(fname);
    trp->ReadBegin("VioConfig");
  } catch (faudes::Exception& fexcep) {
    FD_DQT("VioAttributeStyle::Config: cannot open/read file " << fname);
    return 0;
  }
  // find my section
  try {
    if(trp->ExistsBegin(fsection)) {
      trp->ReadBegin(fsection);
      return trp;
    }    
  } catch (faudes::Exception& fexcep) {
    FD_DQT("VioAttributeStyle::Config: corrupted file " << fname);
  }
  // failure
  delete trp;
  return 0;
}


// read config file 
void VioStyle::ReadFile(const QString& filename) {
  // start with defaults
  Initialise();
  // record filename
  if(filename!="") mConfigFile=filename;
  // built in default
  if(mConfigFile=="") mConfigFile=QCoreApplication::applicationDirPath() + "/vioconfig.txt";
  // have token reader
  std::string fname=VioStyle::LfnFromQStr(mConfigFile);
  FD_DQ("VioStyle::Config: read file " << fname);
  faudes::TokenReader tr(fname);
  // find my section
  tr.ReadBegin("VioConfig");
  // record name of configuration
  mConfigName=QStrFromStr(tr.ReadString());
  // loop my section
  while(!tr.Eos("VioConfig")) {
    faudes::Token token;
    tr.Peek(token);
    if(token.Type()!=faudes::Token::Begin) { tr.Get(token); continue; }
    // dotpath
    if(token.StringValue()=="DotPath") {
      tr.ReadBegin("DotPath");
      mDotExecutable=VioStyle::QStrFromStr(tr.ReadString());
      tr.ReadEnd("DotPath");
      continue;
    }
    // ... add more sections here (todo: fonts, colors etc)
    // skip unkown section
    tr.ReadBegin(token.StringValue());
    tr.ReadEnd(token.StringValue());
  }
  // done
  tr.ReadEnd("VioConfig");
  FD_DQ("VioStyle::Config:done ");
}


// have a static instance
VioStyle* VioStyle::G(void) {
  Initialise();
  return mpStaticInstance;
}


// configuration report
QString VioStyle::FaudesType(void) const { 
  return mFaudesType;
};


// configuration report
bool VioStyle::UserAccess(void) const {
  return mUserAccess;
}

// configuration report
bool VioStyle::GroupView(void) const {
  return mGroupView;
}

/*
 *************************************
 *************************************
 *************************************

 Implementation of VioStyle: faudes interface

 *************************************
 *************************************
 *************************************
 */


// stateset as qstrings (except states with no name)
void VioStyle::StatesQStrList(QList<QString>& states, const fGenerator* pGen) {
  states.clear();
  faudes::StateSet::Iterator sit;
  for(sit=pGen->StatesBegin(); sit!=pGen->StatesEnd(); sit++) {
    std::string name=pGen->StateName(*sit); 
    if(name!="") states.append(VioStyle::QStrFromStr(name));
  }
  qSort(states);
}

// alphabet as qstrings 
void VioStyle::EventsQStrList(QList<QString>& events, const faudes::EventSet* pAlph) {
  events.clear();
  faudes::EventSet::Iterator eit;
  for(eit=pAlph->Begin(); eit!=pAlph->End(); eit++) {
    std::string name=pAlph->SymbolicName(*eit); 
    if(name!="") events.append(VioStyle::QStrFromStr(name));
  }
  qSort(events);
}

// alphabet as qstrings 
void VioStyle::EventsQStrList(QList<QString>& events, const fGenerator* pGen) {
  EventsQStrList(events,&pGen->Alphabet());
}


// string conversion
std::string VioStyle::StrFromQStr(const QString& qstr) { 
  return std::string(qstr.toUtf8().constData()); }
QString VioStyle::QStrFromStr(const std::string& str) { 
  return QString::fromUtf8(str.c_str());}
std::string VioStyle::LfnFromQStr(const QString& qstr) { 
  return std::string(QFile::encodeName(qstr).constData()); }


// string conversion FTU
QString VioStyle::QStrFromFtu(faudes::Time::Type ftu) {
  if(ftu>=faudes::Time::Max()) return QString("inf");
  if(ftu<=faudes::Time::Min()) return QString("-inf");
  return QString("%1").arg(ftu);
}
faudes::Time::Type VioStyle::FtuFromQStr(const QString& str) {
  if(str=="inf") return faudes::Time::Max();
  if(str=="+inf") return faudes::Time::Max();
  if(str=="-inf") return faudes::Time::Min();
  return str.toInt();
}

// string conversion Float
QString VioStyle::QStrFromFloat(faudes::Float x) {
  if(x>=std::numeric_limits<faudes::Float>::max()) return QString("inf");
  if(x<= -1*std::numeric_limits<faudes::Float>::max()) return QString("-inf");
  return QString("%1").arg(x);
}
faudes::Float VioStyle::FloatFromQStr(const QString& str) {
  if(str=="inf") return std::numeric_limits<faudes::Float>::max();
  if(str=="+inf") return std::numeric_limits<faudes::Float>::max();
  if(str=="-inf") return -1*std::numeric_limits<faudes::Float>::max();
  return str.toDouble();
}

// display version of state name
QString VioStyle::DispStateName(const fGenerator* gen, faudes::Idx idx) {
  if(!gen) return SymbolFromIdx(idx);
  return SymbolFromIdx(idx,&gen->StateSymbolTable()); 
}

// display version of event name
QString VioStyle::DispEventName(const fGenerator* gen, faudes::Idx idx) {
  if(!gen) return SymbolFromIdx(idx);
  return SymbolFromIdx(idx,gen->EventSymbolTablep()); 
}

// display version of event name
QString VioStyle::DispEventName(const faudes::EventSet* set, faudes::Idx idx) {
  if(!set) return SymbolFromIdx(idx);
  return SymbolFromIdx(idx,set->SymbolTablep()); 
}


// sort string: fix digits: sugar or waste?
QString VioStyle::SortName(const QString& str){
  QString res;
  int src=0, numstart, numstop, miss;
  // fix autoappendix digits
  for(src=0; src<str.size(); src++) {
    const QChar c=str[src];
    if(!c.isDigit()) 
      { res.append(c); continue; };
    numstart=src;
    while(src<str.size()) 
      { if(!str[src].isDigit()) break; src++; }
    numstop=src;
    miss=5-(numstop-numstart);
    for(;miss>0;miss--) res.append('0');
    for(;numstart<numstop; numstart++) res.append(str[numstart]);
  }
  // fix prefic #
  if(res.size()>=1) 
  if(res[0]=='#') res[0]=127;
  FD_DQ("VioStyle::SortName("<< str << "): " << res);
  return res;
}

// convenience function for sort names
QString VioStyle::SortStateName(const fGenerator* gen, faudes::Idx idx){ 
    return SortName(DispStateName(gen,idx));};
QString VioStyle::SortEventName(const fGenerator* gen, faudes::Idx idx){ 
    return SortName(QStrFromStr(gen->EventName(idx)));};
QString VioStyle::SortEventName(const faudes::EventSet* set, faudes::Idx idx){ 
    return SortName(QStrFromStr(set->SymbolicName(idx)));};


// get index from symbol (incl non strict symbols)  
faudes::Idx VioStyle::IdxFromSymbol(const QString& symbol, const faudes::SymbolTable* pSymbolTable) {
  faudes::Idx res;
  // try table first
  if(pSymbolTable) {
    res=pSymbolTable->Index(StrFromQStr(symbol));
    if(res>0) return res;
  }
  // try fake index
  QString cand=symbol;
  if(cand.length()<=1) return 0;
  if(cand.mid(0,1)!= "#") return 0;
  cand=cand.remove(0,1);
  cand=cand.trimmed();
  // check index
  bool ok;
  res=cand.toULong(&ok,10);
  res = ok ? res : 0;
  FD_DQ("VioStyle::IndexFromSymbol("<< symbol << "): " << res );
  return res;
}

// get index from symbol (incl non strict symbols, drop quotes if any)  
faudes::Idx VioStyle::IdxFromSymbolDq(const QString& symbol, const faudes::SymbolTable* pSymbolTable) {
  QString sym=symbol;
  // drop quotes
  if(sym.size()>1)
    if(sym.at(0)=='"' ) sym=symbol.mid(1,sym.length()-2);
  // doit
  return IdxFromSymbol(sym,pSymbolTable);
}



// get index from symbol (incl non strict symbols)  
faudes::Idx VioStyle::IdxFromSymbol(const std::string& symbol, const faudes::SymbolTable* pSymbolTable) {
  return IdxFromSymbol(QStrFromStr(symbol),pSymbolTable);
}

// get symbolic name from index (incl fake symbols)  
QString VioStyle::SymbolFromIdx(faudes::Idx idx, const faudes::SymbolTable* pSymbolTable) {
  // try table first
  if(pSymbolTable) {
    QString res=QStrFromStr(pSymbolTable->Symbol(idx));
    if(res!="") return res;
  }
  // have fake name
  return QString("#%1").arg(idx);
}

// valid faudes symbol (no index names)
bool VioStyle::ValidSymbol(const std::string& name) {
  return faudes::SymbolTable::ValidSymbol(name);
}


// valid faudes symbol (no index names)
bool VioStyle::ValidSymbol(const QString& name) {
  return faudes::SymbolTable::ValidSymbol(VioStyle::StrFromQStr(name));
}

// valid faudes symbol (incl fake index)
bool VioStyle::ValidFakeSymbol(const QString& name) {
  // its a strict symbol anyway
  if(faudes::SymbolTable::ValidSymbol(VioStyle::StrFromQStr(name)))
     return true;
  // can convert to valid index ?
  return IdxFromSymbol(name)>0;
}
 

// valid faudes fake symbol 
bool VioStyle::ValidFakeSymbol(const std::string& name) {
  return ValidFakeSymbol(VioStyle::QStrFromStr(name));
}


/*
 *************************************
 *************************************
 *************************************

 Implementation of VioStyle: misc

 *************************************
 *************************************
 *************************************
 */


// other helper: viodes plugin info
QString VioStyle::PluginsString(void) {
  QString res;
  foreach(QString plugin, VioTypeRegistry::Plugins()) {
    if(res!="") res.append(", ");
    res.append(plugin);
  }
  return res;
}

// faudes callback function 
// .. needs mutex for shared data
// .. must not use debug macros FD_xxx in order not to mess up console logging
bool VioStyle::FaudesBreakFnct(void) {
#ifdef FAUDES_DEBUG_SCRIPT
  // std::cout << "FaudesBreakFnct(): " << mFaudesBreakFlag << std::endl;
#endif
  if(!mFaudesBreakFlag) return false;
#ifdef FAUDES_DEBUG_SCRIPT
  std::cout << "FAUDES_STDOUT: FaudesBreakFnct(): " << mFaudesBreakFlag << std::endl;
#endif
  mFaudesBreakFlag=false;
  return true;
}
// trigger break
void VioStyle::FaudesBreakSet(void) {
  mFaudesBreakFlag=true;
}
// trigger break
void VioStyle::FaudesBreakClr(void) {
  mFaudesBreakFlag=false;
}

/*
 *************************************
 *************************************
 *************************************

 Implementation of VioStyle: graphics

 *************************************
 *************************************
 *************************************
 */




// helper: fix hue and sat, keep val
QColor VioStyle::ColorTaint(const QColor& valcol, const QColor& huecol, qreal sat) {
  qreal hue    =  huecol.hueF();
  qreal val    =  valcol.valueF();
  //qreal valsat =  valcol.saturationF();
  QColor result(0,0,0);
  result.setHsvF(hue,sat,val);
  return result;
}


// helper: euclidian norm (or zero if too small)
qreal VioStyle::NormF(const QPointF& point) {
  qreal len=sqrt(pow(point.x(),2)+pow(point.y(),2));
  if(len < 1.0) len=0;
  return len;
} 

// helper: scale to euclidian norm (or zero if too small)
QPointF VioStyle::NormalizeF(const QPointF& point) {
  qreal len = NormF(point);
  if(len==0.0) return QPointF(0,0);
  return 1/len * point;
}

// helper: normal aka orthogonal unit length (or zero if too small)
QPointF VioStyle::NormalF(const QPointF& point) {
  qreal len = NormF(point);
  if(len==0.0) return QPointF(0,0);
  QPointF res;
  res.ry()=point.x();
  res.rx()=-point.y();
  return 1/len * res;
}

// helper: extend base to "lot (EL?)" of x (base being normalized)
QPointF VioStyle::LotF(const QPointF& base, const QPointF& point) {
  return base * (base.x()*point.x()+base.y()*point.y());
} 

// helper: scalar product
qreal VioStyle::ScalarF(const QPointF& b, const QPointF& a) {
  return a.x()*b.x()+a.y()*b.y();
} 

// arrow tip: given line segment from a to b, construct tip at b
void VioStyle::ArrowTip(QPainterPath& arrow, const QPointF& pointA, const QPointF& pointB) {
  QPointF  pointD=mArrowSize*NormalizeF(pointB-pointA);
  QPointF  pointO=mArrowRatio*mArrowSize* NormalF(pointB-pointA);
  arrow=QPainterPath();
  arrow.setFillRule(Qt::WindingFill);
  arrow.moveTo(pointB);
  arrow.lineTo(pointB-pointD-pointO);
  arrow.lineTo(pointB-pointD+pointO);
  arrow.lineTo(pointB);
}

// fix arrow: given line segment from a to b, move b back until it is outside pathB
void VioStyle::FixArrow(QPointF& pointA, const QPointF &pointB, const QPainterPath& pathB) {
  if(pointA==pointB) return;
  if(!pathB.contains(pointB)) return;
  QPointF dir=pointB-pointA;
  QPointF pA=pointA;
  QPointF pB=pointB;
  QPointF pI;
  for(int i=0; i<10; i++) {
    if(!pathB.contains(pA)) break;
    pA-=dir;
    dir*=2;
  }
  for(int i=0; i<10; i++) {
    pI=0.5* pA + 0.5 * pB;
    if(pathB.contains(pI)) pB=pI;
    else pA=pI;
  }
  pointA=pI;
}

// add color to Qt color
void VioStyle::Color(const QString& name, const QColor& color) {
  int pos;
  if(mColorIndexes.contains(name)) {
    FD_DQ("VioStyle::Color(): redefining " << name);
    pos=mColorIndexes[name];
    mColorDefs[pos]=color;
  } else {
    pos=mColorDefs.size();
    FD_DQ("VioStyle::Color(): defining " << name << " with idx " << pos);
    mColorIndexes[name]=pos;
    mColorNames[pos]=name;
    mColorDefs.append(color);
 }    
}

// number of colors defined
int VioStyle::Colors(void) {
  return mColorDefs.size();
}

// translate symbolic color to Qt color
const QColor& VioStyle::Color(int color) {
  //FD_DQ("VioStyle::Color(): looking up " << color);
  if(color>=0 && color < mColorDefs.size()) return mColorDefs.at(color);
  return mColorDefs.at((int) VioBlack);
}

// translate color name to Qt color
const QColor& VioStyle::Color(const QString& color) {
  //FD_DQ("VioStyle::Color(): looking up " << VioStyle::StrFromQStr(color));
  if(mColorIndexes.contains(color)) return Color(mColorIndexes[color]);
  return mColorDefs.at((int) VioBlack);
}

// translate color name to color index
int VioStyle::ColorIndex(const QString& color) {
  //FD_DQ("VioStyle::Color(): looking up " << VioStyle::StrFromQStr(color));
  if(!mColorIndexes.contains(color)) return VioNoColor;
  return mColorIndexes[color];
}

// translate color index to color name
const QString& VioStyle::ColorName(int idx) {
  //FD_DQ("VioStyle::Color(): looking up " << idx );
  if(!mColorNames.contains(idx)) return ColorName(VioNoColor);
  return mColorNames[idx];
}

// return all known colors
QList<QString> VioStyle::ColorNames(void) {
  QList<QString> res;
  foreach(const QString& name, mColorNames)
    res.append(name);
  return res;
}

// helper: magnetic grid
QPointF VioStyle::GridPoint(const QPointF& point) {
  QPointF res;
  if(point.x()>=0) 
    res.rx()=((long) (point.x()/mGridWidth+0.5))*mGridWidth;
  else
    res.rx()=-((long) (-point.x()/mGridWidth+0.5))*mGridWidth;
  if(point.y()>=0) 
    res.ry()=((long) (point.y()/mGridWidth+0.5))*mGridWidth;
  else
    res.ry()=-((long) (-point.y()/mGridWidth+0.5))*mGridWidth;
  return res;
}

// draw text, lower left point given
void VioStyle::TextLL(QPainterPath& path, const QPointF& pos, const QString& text) {
  path.addText(pos,*mpDefaultFont,text);
}

// text rect, lower left point given
void VioStyle::TextLL(QRectF& drect, const QPointF& pos, const QString& text) {
  drect=mpDefaultFontMetrics->boundingRect(text);
  drect.translate(pos);
}

// draw text, center point given
void VioStyle::TextCP(QPainterPath& path, const QPointF& center, const QString& text) {
  QRectF outer;
  TextLL(outer,center,text);
  QPointF offset=center - outer.center(); 
  TextLL(path,center+offset,text);
}

// text rect, center point given
void VioStyle::TextCP(QRectF& drect, const QPointF& center, const QString& text) {
  TextLL(drect,center,text);
  QPointF offset=center - drect.center(); 
  drect.translate(offset);
}

// draw text, center given by rect
void VioStyle::TextCR(QPainterPath& path, const QRectF& rect, const QString& text) {
  TextCP(path,rect.center(),text);
}

//  text rect, center given by rect
void VioStyle::TextCR(QRectF& drect, const QRectF& rect, const QString& text) {
  TextCP(drect,rect.center(),text);
}


// operator << for qstrings
std::ostream& operator<<(std::ostream& o, const QString& qstr) {
  return (o << std::string(qstr.toAscii().constData())); 
}

/*
 *************************************
 *************************************
 *************************************

 Implementation of VioStyle: should go to
 a derived class for VioModel and friends

 *************************************
 *************************************
 *************************************
 */



// add edit function
void VioStyle::InsEditFunction(const VioEditFunction& vfnct) {
  FD_DQT("VioStyle::InsEditFunction(): insert " << vfnct.mName);
  mEditFunctions.append(vfnct);
}



/* 
 ******************************************
 ******************************************
 ******************************************

 implementation of ftu validator

 ******************************************
 ******************************************
 ******************************************
 */

// construct
VioFtuValidator::VioFtuValidator(QObject* parent) :  
  QIntValidator(parent) {
};

// destruct
VioFtuValidator::~VioFtuValidator(void) {  
  FD_DQ("VioFtuValidator::~VioFtuValidator("<<this<<")");
};

// validate
VioFtuValidator::State VioFtuValidator::validate(QString& input, int & pos) const {
  // test basis for int
  State intres = QIntValidator::validate(input,pos);
  if(intres==Acceptable) return Acceptable;
  if(intres==Intermediate) return Intermediate;
  // test infty
  if(input=="inf") return Acceptable;
  if(input=="+inf") return Acceptable;
  if(input=="-inf") return Acceptable;
  if(QString("inf").startsWith(input)) return Intermediate;
  if(QString("+inf").startsWith(input)) return Intermediate;
  if(QString("-inf").startsWith(input)) return Intermediate;
  // fail
  return Invalid;
};


