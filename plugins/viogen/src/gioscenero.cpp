/* gioscenero.cpp  - faudes generator as qgraphicsscene */


/*
   Graphical  IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006, 2007  Thomas Moor, Klaus Schmidt, Sebastian Perk

*/


#include <QtGui>
#include <QtSvg>

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <clocale>

#include "gioscenero.h"
#include "gioview.h"

// resolve forward
#include "viogengraph.h"

using namespace std;

// constructor
GioSceneRo::GioSceneRo(VioGeneratorGraphModel* gmodel) : QGraphicsScene(gmodel) {
  FD_DQG("GioSceneRo::GioSceneRo(" << gmodel << ")");
  pGeneratorGraphModel=gmodel;
  pGeneratorModel=gmodel->GeneratorModel();
  pGeneratorConfig=gmodel->GeneratorConfiguration();
  Clear();
  mConsistent= (pGeneratorModel->Size()==0);
  mModified = false;
  mCtrlEditing=false;
  FD_DQG("GioSceneRo::GioSceneRo(): done");
};

// destructor
GioSceneRo::~GioSceneRo(void) {
  FD_DQG("GioSceneRo::~GioSceneRo()");
}

// read only access to faudes generator 
const faudes::vGenerator* GioSceneRo::Generator(void) const {
  return pGeneratorModel->Generator();
};

// clear all (you must test consistency at some point)
int GioSceneRo::Clear(void) {
  FD_DQG("GioSceneRo::Clear()");
  foreach(GioState* state, mStateItems) {
    removeItem(state);
    delete state;
  }
  foreach(GioTrans* trans, mTransItems) {
    removeItem(trans);
    delete trans;
  }    
  mStateItems.clear();
  mTransItems.clear();
  mStateMap.clear();
  mTransMap.clear();
  setSceneRect(QRect(0,0,100,100));
  mCtrlEditing=false;
  FD_DQG("GioSceneRo::Clear(): done");
  return 0;
}


// test consistentcy, return number of missing items
int GioSceneRo::TestConsistent(void) {
  // too many states ?
  if((faudes::Idx) mStateItems.size() > Generator()->Size()) {
    FD_DQG("GioSceneRo::TestConsistent(): to many states");
    return -1;
  }
  //if((faudes::Idx) mTransItems.size() > Generator()->TransRelSize()) return -1;
  // check items
  int count=0;
  faudes::StateSet::Iterator sit=Generator()->StatesBegin();
  for(; sit!=Generator()->StatesEnd(); sit++) 
    if(!mStateMap.contains(*sit)) {
      FD_DQG("GioSceneRo::TestConsistent(): missing state " << *sit);
      count++;
    }
  faudes::TransSet::Iterator tit=Generator()->TransRelBegin();
  for(; tit!=Generator()->TransRelEnd(); tit++) 
    if(!mTransMap.contains(*tit)) {
      FD_DQG("GioSceneRo::TestConsistent(): missing trans " << tit->Str());
      count++;
    }
  if(count>0) {
    FD_DQG("GioSceneRo::TestConsistent(): missing items");
  }
  return count;
}  

// set consistent
void GioSceneRo::Consistent(bool cons) {
  if(cons!=mConsistent) {
    mConsistent=cons;
    emit NotifyConsistent(mConsistent);
  }
}


// ensure the scene to be large enough
void GioSceneRo::AdjustScene(void) {
  // items
  QRectF arect= itemsBoundingRect();
  // cover all viewports
  foreach(QGraphicsView* gview, views()) {
    QPoint vbr(gview->viewport()->size().width(),gview->viewport()->size().height());
    QPoint vtl(0,0);
    QPointF sbr = gview->mapToScene(vbr);
    QPointF stl = gview->mapToScene(vtl);
    QRectF srect(stl,sbr);
    arect |= srect;    
  }
  // threshold
  QRectF drect(sceneRect().topLeft() - arect.topLeft(), sceneRect().bottomRight() - arect.bottomRight());
  drect=drect.normalized();
  qreal gw= pGeneratorConfig->GridWidth();
  if(drect.width() < gw)
  if(drect.height() < gw)
    return;
  // cover largest viewport
  setSceneRect(arect);
}


// clear data
void GioSceneRo::Data::clear(void) {
  mTransItemsData.clear();
  mStateItemsData.clear();
}

// data statistics
void GioSceneRo::Data::statistics(void) const { 
  FD_DQG("GioSceneRo::Data: statistics: " << mStateItemsData.size() << "/"
      << mTransItemsData.size() << " giotrans" );
};
  
// read data from token reader 
void GioSceneRo::Data::read(faudes::TokenReader& rTr) {
  clear();
  try {
    rTr.ReadBegin("GraphData");
    // 1. read states
    rTr.ReadBegin("States");
    while(!rTr.Eos("States")){ 
      GioState::Data sdata;
      sdata.read(rTr);
      mStateItemsData.append(sdata);
    }
    rTr.ReadEnd("States");
    // 2. read transitions
    rTr.ReadBegin("TransRel");
    while(!rTr.Eos("TransRel")){ 
      GioTrans::Data tdata;
      tdata.read(rTr);
      mTransItemsData.append(tdata);
    }
    rTr.ReadEnd("TransRel");
    // done
    rTr.ReadEnd("GraphData");
  }
  catch (faudes::Exception& exception) {  
    FD_DQG("GioSceneRo::Data::read: cannot read data");
    clear();
    throw (exception);
  }
}

// write data to token writer
void GioSceneRo::Data::write(faudes::TokenWriter& rTw, const fGenerator* pGen) const {
  // write graph representation
  rTw.WriteBegin("GraphData");
  // 1. states
  rTw.WriteBegin("States");
  for(int i=0; i<mStateItemsData.size(); i++) {
    mStateItemsData[i].write(rTw,pGen);
  }
  rTw.WriteEnd("States");
  // 2. transitions
  rTw.WriteBegin("TransRel");
  for(int i=0; i<mTransItemsData.size(); i++) {
    mTransItemsData[i].write(rTw, pGen);
  }
  rTw.WriteEnd("TransRel");
  // done
  rTw.WriteEnd("GraphData");
};


// get all data
int GioSceneRo::GioWrite(Data& giodata) {
  FD_DQG("GioSceneRo::GioWrite(data) from scene " << this);
  giodata.clear();
  // update consistent / abort on inconsistent data
  //int res=TestConsistent();
  //Consistent(res!=0);
  if(!Consistent()) return 0;
  // collect data
  for(int i=0; i<mStateItems.size(); i++) {
    giodata.mStateItemsData.append(mStateItems[i]->data());
  }
  for(int i=0; i<mTransItems.size(); i++) {
    giodata.mTransItemsData.append(mTransItems[i]->data());
  }
  return 0;
}

// set all data; return number of missing items
int GioSceneRo::GioRead(const Data& data) {
  FD_DQG("GioSceneRo::GioRead(data) for scene " << this);
  Clear();
  // get gio states that exist in generator
  for(int i=0; i<data.mStateItemsData.size(); i++) {
    if(!Generator()->ExistsState(data.mStateItemsData[i].mIdx))
      continue; 
    GioState* state = new GioState(pGeneratorModel);
    state->setData(data.mStateItemsData[i]);
    addGioState(state);
  }
  // get gio transitions for which the states exist in generator
  for(int i=0; i<data.mTransItemsData.size(); i++) {
    faudes::Transition ftrans;
    ftrans.X1=data.mTransItemsData[i].mIdxA;
    ftrans.Ev=Generator()->EventIndex(data.mTransItemsData[i].mNameEv);
    ftrans.X2=data.mTransItemsData[i].mIdxB;
    //if(!Generator()->ExistsTransition(ftrans)) 
    //  continue; 
    if(!Generator()->ExistsState(ftrans.X1)) 
      continue; 
    if(!Generator()->ExistsState(ftrans.X2)) 
      continue; 
    GioTrans* trans = new GioTrans(pGeneratorModel);
    trans->setData(data.mTransItemsData[i]);
    addGioTrans(trans);
  }
  // done
  int res=TestConsistent();
  Consistent(res!=0);
  // fix scene rect
  AdjustScene();
  return res;
}

// read gio section from token reader 
int GioSceneRo::GioRead(faudes::TokenReader& rTr) {
  // clear
  Clear();
  Modified(false);
  // prepare/read/set data
  Data gdata;
  gdata.read(rTr); // throws excep
  return GioRead(gdata);
};


// write gio section to token writer
int GioSceneRo::GioWrite(faudes::TokenWriter& rTw) {
  // remove unlinked
  UpdateTrimElements();
  // get/write data
  Data gdata;
  GioWrite(gdata);
  gdata.write(rTw, Generator()); 
  // done
  Modified(false); 
  return 0;
};


// export generator to (e)ps file (broken)
int GioSceneRo::EpsWrite(const QString &epsfile) {
  FD_DQG("GioSceneRo::EpsWrite(file)");
  // create a ps printer
  QPrinter printer(QPrinter::HighResolution);
  printer.setOutputFileName(epsfile);
  printer.setOutputFormat(QPrinter::PostScriptFormat);
  printer.setPaperSize(QSizeF(sceneRect().width()+5,sceneRect().height()+5),QPrinter::Point);
  printer.setPageMargins(0,0,0,0,QPrinter::Point);
  // create a painter on that printer
  QPainter painter;
  painter.begin(&printer);
  // doit
  FD_DQG("GioSceneRo::EpsWrite(file): render");
  render(&painter,QRectF(),itemsBoundingRect());
  FD_DQG("GioSceneRo::EpsWrite(file): render done");
  // done
  painter.end();
  return 0;
} 

// export generator to pdf file
int GioSceneRo::PdfWrite(const QString &epsfile) {
  FD_DQG("GioSceneRo::PdfWrite(file)");
  // create a ps printer
  QPrinter printer(QPrinter::HighResolution);
  printer.setOutputFileName(epsfile);
  printer.setOutputFormat(QPrinter::PdfFormat);
  printer.setPaperSize(QSizeF(sceneRect().width()+5,sceneRect().height()+5),QPrinter::Point);
  printer.setPageMargins(0,0,0,0,QPrinter::Point);
  // create a painter on that printer
  QPainter painter;
  painter.begin(&printer);
  // doit
  FD_DQG("GioSceneRo::PdfWrite(file): render");
  render(&painter,QRectF(),itemsBoundingRect());
  FD_DQG("GioSceneRo::PdfWrite(file): render done");
  // done
  painter.end();
  return 0;
} 

// jpg write
int GioSceneRo::JpgWrite(const QString &jpgfile) {
  FD_DQG("GioSceneRo::JpgWrite(file)");
  // todo: configure resolution
  double jscale = pGeneratorConfig->ExportResolution();
  if(jscale*width() > pGeneratorConfig->ExportMaxSize()) jscale = ((double) pGeneratorConfig->ExportMaxSize())/width();
  int jwidth  =  (int) (jscale*width());
  int jheight  = (int) (jscale*height());
  QPixmap pixmap(jwidth,jheight);
  pixmap.fill();
  QPainter painter(&pixmap);
  painter.setRenderHint(QPainter::Antialiasing);
  render(&painter,QRectF(),itemsBoundingRect());
  painter.end();
  bool ok=pixmap.save(jpgfile,"JPG");
  return (ok ? 0 : 1);
}

// png write
int GioSceneRo::PngWrite(const QString &pngfile) {
  FD_DQG("GioSceneRo::PngWrite(file)");
  // todo: configure resolution
  double jscale = pGeneratorConfig->ExportResolution();
  if(jscale*width() > pGeneratorConfig->ExportMaxSize()) jscale = ((double) pGeneratorConfig->ExportMaxSize())/width();
  int jwidth  =  (int) (jscale*width());
  int jheight  = (int) (jscale*height());
  QPixmap pixmap(jwidth,jheight);
  pixmap.fill();
  QPainter painter(&pixmap);
  painter.setRenderHint(QPainter::Antialiasing);
  render(&painter,QRectF(),itemsBoundingRect());
  painter.end();
  bool ok=pixmap.save(pngfile,"PNG");
  return (ok ? 0 : 1);
}

// svg write
int GioSceneRo::SvgWrite(const QString &svgfile) {
  // todo: errors?
  FD_DQG("GioSceneRo::SvgWrite(file)");
  QSvgGenerator svggen;
  svggen.setFileName(svgfile);
  svggen.setSize(QSize((int) width()+1, (int) height()+1));
  QPainter painter(&svggen);
  render(&painter,QRectF(),itemsBoundingRect());
  painter.end();
  return 0;
}

// construct graph data from generator
int GioSceneRo::GridConstruct(bool clr) {
  FD_DQG("GioSceneRo::GridConstruct(clr): Size #" << Generator()->Size());
  // discard old data
  if(clr) Clear();
  // arrange missing states 
  int width=int(1.3* sqrt(Generator()->Size())+ 0.5 );
  int count=1;
  QRectF rect=itemsBoundingRect();
  QPointF offset=QPointF(0,rect.y()+ rect.height() + 0.5 * pGeneratorConfig->ImportMeshWidthY());
  QPointF pos=QPointF(0,0);
  for(faudes::StateSet::Iterator sit=Generator()->StatesBegin(); sit != Generator()->StatesEnd(); sit++) {
    if(StateItem(*sit)) continue;
    GioState* state = new GioState(pGeneratorModel);
    state->Idx(*sit);
    state->setPos(pos + offset);
    addGioState(state);
    pos.rx()+= pGeneratorConfig->ImportMeshWidthX();
    if(count % width==0) {
      pos.ry()+= pGeneratorConfig->ImportMeshWidthY();
      pos.rx()= 0;
    }
    count++;
  } 
  // arrange missing transitions
  faudes::TransSet::Iterator tit=Generator()->TransRelBegin();
  for(; tit!=Generator()->TransRelEnd(); tit++) {
    if(TransItem(*tit)) continue;
    addGioTrans(*tit);
  }
  // done
  Consistent(true);
  // adjust scene 
  AdjustScene();
  return 0;
}


// construct graph data from old graph data
int GioSceneRo::GioConstruct(void) {
  FD_DQG("GioSceneRo::GioConstruct()");
  // my old gio data refers to the same as viogenerator
  // TODO if(Generator()==pVioGenerator->generator()) return 0;
  // prepare
  /*
  const fGenerator* oldg = Generator();
  const fGenerator* newg = pVioGenerator->generator();
  FD_DQG("GioSceneRo::GridConstruct(...): recover from outdated generator");
  // stage 1: find state correspondence by name
  QMap<int,int> old2new;
  for(faudes::StateSet::Iterator sit=oldg->StatesBegin(); sit != oldg->StatesEnd(); sit++) {
    faudes::Idx oidx=*sit;
    faudes::Idx nidx= newg->StateIndex(oldg->StateName(oidx)); 
    if(nidx !=0 ) {
      FD_DQG("GioSceneRo::GridConstruct(...): " <<  oldg->SStr(oidx) << " -> " << newg->SStr(nidx));
      old2new[oidx]=nidx;
    }
  }
  // stage 2: relink/delete identified states
  for(faudes::StateSet::Iterator sit=oldg->StatesBegin(); sit != oldg->StatesEnd(); sit++) {
    faudes::Idx oidx=*sit;
    faudes::Idx nidx;
    if(GioState* state=gioState(oidx)) {
      if(old2new.contains(oidx)) {
        // silebt relink
        nidx=old2new[oidx];
        state->setFaudes(newg,nidx);
        state->updateData();
      } else {
        // silent remove
        mStateItems.removeAll(state);
        removeItem(state);
        delete state;
      }
    }
  }
  // fix state map
  mStateMap.clear();
  foreach(GioState* state, mStateItems) mStateMap[state->idx()]=state;
  // stage 3: relink/delete identified states
  for(faudes::TransSet::Iterator tit=oldg->TransRelBegin(); tit != oldg->TransRelEnd(); tit++) {
    faudes::Transition otrans=*tit;
    faudes::Transition ntrans=otrans;
    if(GioTrans* trans=gioTrans(otrans)) {
      if(old2new.contains(otrans.X1) && old2new.contains(otrans.X2)) {
        // silent relink
        ntrans.X1=old2new[otrans.X1];
        ntrans.X2=old2new[otrans.X2];
        trans->setFaudes(newg,ntrans);
        trans->updateData();
      } else {
        // silent remove
        mTransItems.removeAll(trans);
        removeItem(trans);
        delete trans;
      }
    }
  }
  // fix trans map
  mTransMap.clear();
  foreach(GioTrans* trans, mTransItems) mTransMap[trans->ftrans()]=trans;
  // relink
  FD_DQG("GioSceneRo::GridConstruct(...): set new generator reference");
  // TODO Generator()=pVioGenerator->generator();
  // discard recover
  if(mStateItems.size()<Generator()->Size()*0.5) {
    Clear();
    return  Generator()->Size(); 
  }
  */
  // arrange missing states 
  return GridConstruct(false);
}





// write to dot file
// note: we use a fixed scaling factor of "100 Qt pixels" to "1 dot inch"
int GioSceneRo::DotWrite(const QString &dotfile, bool mute_layout) {
  std::string cdotfile= VioStyle::LfnFromQStr(dotfile);
  FD_DQG("gioscenero::DotWrite(" << this << ") to " << cdotfile);
  faudes::StateSet::Iterator lit;
  faudes::TransSet::Iterator tit;
  // sort states for output
  QList<faudes::Idx> states;
  for(lit = Generator()->InitStatesBegin(); lit != Generator()->InitStatesEnd(); ++lit) {
    states.append(*lit);
  }
  for(lit = Generator()->MarkedStatesBegin(); lit != Generator()->MarkedStatesEnd(); ++lit){ 
    if(Generator()->ExistsInitState(*lit)) continue;
    states.append(*lit);
  }
  for(lit = Generator()->StatesBegin(); lit != Generator()->StatesEnd(); ++lit) {
    if(Generator()->ExistsInitState(*lit)) continue;
    if(Generator()->ExistsMarkedState(*lit)) continue;
    states.append(*lit);
  }
  // do the writing
  try {
    std::ofstream stream;
    stream.exceptions(std::ios::badbit|std::ios::failbit);
    stream.open(cdotfile.c_str());
    stream << "digraph \"___" << Generator()->Name() << "___\" {" << std::endl;
    stream << "  rankdir=LR" << std::endl;
    stream << "  node [fontsize=10]" << std::endl; // style this!!
    stream << "  node [fontname=\"Arial\"]" << std::endl; // style this!!
    stream << "  edge [fontsize=10]" << std::endl; // style this!!
    stream << "  edge [fontname=\"Arial\"]" << std::endl; // style this!!
    stream << "  node [shape=circle];" << std::endl;
    stream << std::endl;
    if(mute_layout) {
      stream << "  // initial state tips" << std::endl;
      for(lit = Generator()->InitStatesBegin(); lit != Generator()->InitStatesEnd(); ++lit) {
        std::string state=VioStyle::StrFromQStr(
	  VioStyle::DispStateName(Generator(),*lit));
        stream << "  dot_dummyinit_" << *lit << " [shape=none, label=\"\", width=\"0.0\", height=\"0.0\" ];" << std::endl;
        stream << "  dot_dummyinit_" << *lit << " -> \"" << state << "\";" << std::endl; 
      }
    }
    stream << "  // stateset" << std::endl;
    for(int i=0; i< states.size(); i++) {
      std::string state=VioStyle::StrFromQStr(
	VioStyle::DispStateName(Generator(),states.at(i)));
      stream << "  \"" << state << "\"" ;
      if(!mute_layout) {
        GioState* gstate=StateItem(states.at(i));
        if(!gstate) continue;
        double x = gstate->pos().x()/100.0 * 72;
        double y = -gstate->pos().y()/100.0 * 72;
        stream << " [pos=\"" << x << "," << y << "\"]";
      }   
      stream << ";" << std::endl;
    }
    stream << std::endl;
    stream << "  // transition relation" << std::endl;
    for(tit = Generator()->TransRelBegin(); tit != Generator()->TransRelEnd(); ++tit) {
      std::string state1=VioStyle::StrFromQStr(
	VioStyle::DispStateName(Generator(),tit->X1));
      std::string state2=VioStyle::StrFromQStr(
	VioStyle::DispStateName(Generator(),tit->X2));
      std::string event=VioStyle::StrFromQStr(
	 VioStyle::DispEventName(Generator(),tit->Ev));
      stream << "  \"" << state1 
	     << "\" -> \"" << state2
	     << "\" [label=\""<< event << "\"];" << std::endl;
    }
    stream << "}" << std::endl;  // (! no extra colon as of dot 2.36 !)
    stream.close();
  }
  catch (std::ios::failure&) {
    FD_DQG("gioscenero::DotWrite(" << this << "): io error");
    return -1;
  }
  return 0;
}



// construct graph data from dot output file  
// note: dotfile must be gioscenero DotWrite output and dot-processed to format "plain" 
// note: we use a fixed scaling factor of "100 Qt pixels" to "1 dot inch"
int GioSceneRo::DotConstruct(const QString &dotfile) {
  // prepare 
  Clear();
  std::string mydotfile= VioStyle::LfnFromQStr(dotfile);
  FILE* File = fopen(mydotfile.c_str(),"r");
  if(File==0) {
    FD_WARN("GioSceneRo::DotConstruct: cannot open/read dot output file \"" << dotfile << "\"");
    return 1;
  }
  std::string prot;
  char* loc = strdup(setlocale(LC_ALL,NULL));
  setlocale(LC_ALL,"C");
  // plain version
  QMap<faudes::Idx,QPointF> dummyinit;
  double gscale=0, gwidth=0, gheight=0;
  #define MAXNAME 100 // dont change: see ref in format strings 
  char str[MAXNAME];
  int err=0;
  do{
    // be nice (opt: have the progress bar as argument?)
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    // read word until eof
    if(fscanf(File, " %99s", str)!=1) {break;}
    str[MAXNAME-1]=0;
    FD_DQG("GioSceneRo::DotConstruct: parsing \"" << std::string(str) << "\"");      
    // keyword graph  
    if(strcmp(str,"graph")==0) {
      prot.push_back('g');
      if(fscanf(File, " %lf %lf %lf", &gscale,&gwidth,&gheight)!=3) {err=1;break;};
      FD_WARN("GioSceneRo::DotConstruct: scale " << gscale << " width " << gwidth 
        << " height " << gheight);      
    }
    // keyword node 
    if(strcmp(str,"node")==0) {
      prot.push_back('n');
      int id;
      bool dummy=false;
      double x,y,w,h;
      if(fscanf(File, " %99s" , str)!=1) {err=1;break;};
      str[MAXNAME-1]=0;
      FD_DQG("GioSceneRo::DotConstruct:node: "<< std::string(str));
      if(sscanf(str,"dot_dummyinit_%d",&id)==1) {
        FD_DQG("GioSceneRo::DotConstruct:dummy init " << id);      
        dummy=true;
      }
      if(!dummy) {
        id=VioStyle::IdxFromSymbolDq(QString(str),&Generator()->StateSymbolTable());
        if(!Generator()->ExistsState(id)) continue;
        FD_DQG("GioSceneRo::DotConstruct:state " << id);      
      }  
      if(fscanf(File, " %lf %lf %lf %lf",&x,&y,&w,&h)!=4) continue;
      if(!dummy) {
        GioState* state = new GioState(pGeneratorModel);
        state->Idx(id);
        state->setPos(100*QPointF(x,y));
        state->moveL(100*QPointF(x+0.5*w,y+0.5*h));
        addGioState(state);
        FD_DQG("GioSceneRo::DotConstruct:state #" << id << " ok"); 
      } 
      if(dummy) {
        FD_DQG("GioSceneRo::DotConstruct:record dummy at " << x << "," << y); 
        dummyinit[id]=100*QPointF(x,y);
      }
    }
    // keyword  edge
    if(strcmp(str,"edge")==0) {
      prot.push_back('e');
      int idA, idE, idB, nxy,  i;
      bool dummy=false;
      double x,y;
      if(fscanf(File, " %99s" , str)!=1) {err=1;break;};
      str[MAXNAME-1]=0;
      if(sscanf(str,"dot_dummyinit_%d",&idA)==1) {
        FD_DQG("GioSceneRo::DotConstruct: dummy init edge " << idA);      
        dummy=true;
      }
      if(!dummy) {
        idA=VioStyle::IdxFromSymbolDq(QString(str),&Generator()->StateSymbolTable());
      }  
      if(fscanf(File, " %99s" , str)!=1) {err=1;break;};
      str[MAXNAME-1]=0;
      idB=VioStyle::IdxFromSymbolDq(QString(str),&Generator()->StateSymbolTable());
      if(fscanf(File, " %d ", &nxy)!=1) continue;
      if(!dummy) {
        FD_DQG("GioSceneRo::DotConstruct:trans " << idA << " - >"  << idB << " ? (A)");      
        QList<QPointF> ctrlpoints;
        for(i=nxy-1; i>=0; i--){
          if(fscanf(File, " %lf %lf ",&x, &y)!=2) {err=1;break;};
          ctrlpoints.append(100*QPointF(x,y));
        }
        if(fscanf(File, " %99s" , str)!=1) {err=1;break;};
        str[MAXNAME-1]=0;
        FD_DQG("GioSceneRo::DotConstruct:trans " << std::string(str) << " ? (B)");
        idE=VioStyle::IdxFromSymbolDq(QString(str),Generator()->EventSymbolTablep());
        if(fscanf(File, " %lf %lf ",&x, &y)!=2) {err=1;break;};
        if(err) break;
        QPointF labelpos = 100*QPointF(x,y);
        faudes::Transition ftrans(idA,idE,idB); 
        FD_DQG("GioSceneRo::DotConstruct:trans " << idA << " -(" <<idE<<")->"  << idB << " ? (C)");        
        // std behaviour: find and set the transion
        bool succ=false;
        if(Generator()->ExistsTransition(ftrans)) {
          GioTrans* trans=addGioTrans(ftrans);
          if(trans) {
            FD_DQG("GioSceneRo::DotConstruct: trans " << Generator()->TStr(ftrans));      
            trans->moveC(ctrlpoints);
            trans->moveN(labelpos);
	    succ=true;
          }
        }
        // label bug on mac: try to find any one other transition
        if(!succ) {
          prot.push_back('m');
  	  faudes::TransSet::Iterator tit;
          for(tit=Generator()->TransRelBegin((faudes::Idx)idA); tit!=Generator()->TransRelEnd((faudes::Idx) idA); tit++) {
	    if(tit->X2 != (faudes::Idx) idB) continue;
            GioTrans* trans=addGioTrans(*tit);
            if(!trans) continue; 
            FD_DQG("GioSceneRo::DotConstruct: trans macx bug" << Generator()->TStr(ftrans));      
            trans->moveC(ctrlpoints);
            trans->moveN(labelpos);
            break;
          }
        }
      }
    }
    // keyword node 
    if(strcmp(str,"stop")==0) {
      prot.push_back('s');
      break;
    }
    // unknown text
    if(ferror(File)!=0) {
      prot.push_back('u');
      err=1; break;
    }
  } while(!err);
  #undef MAXNAME

  // det init tips
  QMap<faudes::Idx,QPointF>::iterator dit;
  for(dit=dummyinit.begin(); dit!=dummyinit.end(); dit++) {
    GioState* state = StateItem(dit.key());
    if(!state) continue;
    FD_DQG("GioSceneRo::DotConstruct:fix dummy " << state->Idx() << " at " << dit.value().x() << "," << dit.value().y());
    state->moveA(dit.value());
  }

  // errors
  if(File) fclose(File);
  setlocale(LC_ALL,loc);

  if(err) {
    FD_WARN("GioSceneRo::DotConstruct: error while processing dot output: \"" << prot << "\"");      
    Clear();
    Consistent(false);
    return -2;
  }

  int res=TestConsistent();
  Consistent(res==0);

  // adjust scene 
  AdjustScene();

  return res;
}



// construct graph data via dot (std:: false)
int GioSceneRo::DotConstruct(bool trans_only) {
  FD_DQG("GioSceneRo::DotConstruct: using  " << pGeneratorConfig->DotExecutable());      

  // have temp files
  QTemporaryFile dotin(
    QDir::tempPath()+ QDir::separator()+ "faudes_dotin_XXXXXX");
  dotin.open(); 
  QString dotinname = dotin.fileName(); 
  dotin.close();
  QTemporaryFile 
     dotout(QDir::tempPath()+ QDir::separator()+"faudes_dotout_XXXXXX");
  dotout.open(); 
  QString dotoutname = dotout.fileName(); 
  dotout.close();
  dotin.setAutoRemove(false);
  dotout.setAutoRemove(false);
  // generate dot input file
  FD_DQG("GioSceneRo::DotConstruct: writing tmp dot input file" << dotinname);
  if(DotWrite(dotinname,!trans_only)<0) {
    FD_DQG("GioSceneRo::DotConstruct: error writing dot input file");
    return -2;
  }
  // we used to have an issue with the active window on linux/gnome
  // ... so we manually store/restore
  QWidget* awin=QApplication::activeWindow();
  // have a progress dialog
  QApplication::processEvents() ;
  QProgressDialog* progress= new QProgressDialog("Processing graph data", "Cancel", 0, 0);
  progress->setWindowModality(Qt::ApplicationModal);
  progress->setMinimumDuration(2000);
  progress->setValue(0);
  QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
  // process with format plain
  QStringList dotargs;
  dotargs << "-Tplain"; 
  dotargs << "-y"; 
  if(trans_only) dotargs << "-n";      // no layout of states ...
  if(trans_only) dotargs << "-Kneato"; // ... supported by neato only
  dotargs << dotinname;
  dotargs << "-o" << dotoutname;
  QProcess *dotproc = new QProcess(this);
  dotproc->start(pGeneratorConfig->DotExecutable(), dotargs);
  int i=0;
  while(dotproc->state() != QProcess::NotRunning) {
    QApplication::processEvents(QEventLoop::WaitForMoreEvents,100);
    QApplication::flush();
    if(progress->wasCanceled()) dotproc->kill();
    if(i>20) progress->setValue(1); // wait for 2secs
    i++;
  }
  progress->reset();
  delete progress;
  // restore active window
  if(awin) QApplication::setActiveWindow(awin);
  QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
  // catch error
  if(dotproc->exitStatus() != QProcess::NormalExit) {
    FD_WARN("GioSceneRo::DotConstruct: error while generating dot output");      
    //std::stringstream errstr;
    //errstr << "Exception during dot processing";
    //throw faudes::Exception("GioSceneRo::DotConstruct", errstr.str(), 2);
    return -1;
  }
  // interpret dot output
  int res = DotConstruct(dotoutname);
  QApplication::processEvents();
  // check for errors
  if(res!=0) {
    FD_WARN("GioSceneRo::DotConstruct: error while reading dot output");      
    return -1;
  }
  return res;
}


// access some gio state
GioState* GioSceneRo::StateItem(faudes::Idx index) {
  if(!mStateMap.contains(index)) return NULL;
  return mStateMap[index];
}

// access some gio trans
GioTrans* GioSceneRo::TransItem(const faudes::Transition&  ftrans) {
  if(!mTransMap.contains(ftrans)) return NULL;
  return mTransMap[ftrans];
}

// access gio transitions by target state
// todo: cache reverse sorted translist
QList<GioTrans*> GioSceneRo::TransItemsByTarget(faudes::Idx idxB) {
  FD_DQG("GioSceneRo::gioTransByTarget()");
  QList<GioTrans*> res;
  faudes::TransSet::Iterator tit=Generator()->TransRelBegin();
  for(; tit!=Generator()->TransRelEnd(); tit++) {
    if(tit->X2==idxB) {
      GioTrans* trans=TransItem(*tit);
      if(trans) res.append(trans);
    }
  }
  return res;
}

// new viodes style update interface: global
void GioSceneRo::UpdateTrimElements(void) { 
  // remove all gio transitions with unknown states
  FD_DQG("GioSceneRo::UpdateTrimElements(): T");
  foreach(GioTrans* trans, mTransItems) {
    if(!Generator()->ExistsState(trans->FTrans().X1)) 
      removeGioTrans(trans);
    if(!Generator()->ExistsState(trans->FTrans().X2)) 
      removeGioTrans(trans);
  }
  // remove all gio states  that dont exist in generator
  FD_DQG("GioSceneRo::UpdateTrimElements(): S");
  foreach(GioState* state, mStateItems) 
    if(!Generator()->ExistsState(state->Idx())) 
       removeGioState(state);
  FD_DQG("GioSceneRo::UpdateTrimElements(): done");
};

// new viodes style update interface: global
// (incl file io, edit functions ect, thus: we are strict)
void GioSceneRo::UpdateAnyChange(void) { 
  FD_DQG("GioSceneRo::UpdateAnyChange(): A");
  // remove all gio transitions that dont exist in generator
  foreach(GioTrans* trans, mTransItems) { 
    faudes::Transition ftrans=trans->FTrans();
    if(Generator()->ExistsTransition(ftrans)) 
      continue; 
    if(Generator()->ExistsState(ftrans.X1)) 
    if(Generator()->ExistsState(ftrans.X2)) 
      continue; 
    removeGioTrans(trans);
  }
  // remove all gio states  that dont exist in generator
  foreach(GioState* state, mStateItems) 
    if(!Generator()->ExistsState(state->Idx())) 
       removeGioState(state);
  FD_DQG("GioSceneRo::UpdateAnyChange(): B");
  int missing=TestConsistent();
  // do it
  if(missing<20) { // TODO: style this
    if(missing>0) GridConstruct(false); 
    UpdateAnyAttr();
    Consistent(true);
    return;
  }
  // reject
  // Clear();  
  Consistent(false);
  FD_DQG("GioSceneRo::UpdateAnyChange(): done");
};

// new viodes style update interface: global
void GioSceneRo::UpdateAnyAttr(void) {
  FD_DQG("GioSceneRo::UpdateAnyAttr()");
  // all gio states  
  foreach(GioState* state, mStateItems) 
    state->updateData();
  // all gio transitions 
  foreach(GioTrans* trans, mTransItems) 
    trans->updateData();
  FD_DQG("GioSceneRo::UpdateAnyAttr(): done");
}
 
// new viodes style update interface: global
void GioSceneRo::UpdateNewModel(void) { 
  FD_DQG("GioSceneRo::UpdateNewModel()");
  UpdateAnyChange(); // pass on (not implemented)
};

// new viodes style update interface: by element
void GioSceneRo::UpdateElementIns(const VioElement& elem) {
  FD_DQG("GioSceneRo::UpdateElementIns(): " << elem.Str());
  // switch type ...
  switch(elem.Type()) {
  case VioElement::ETrans: addGioTrans(elem.Trans()); break;
  case VioElement::EState: addGioState(elem.State()); break;
  default: break;
  }
  FD_DQG("GioSceneRo::UpdateElementIns(): done");
}; 

// new viodes style update interface: by element
void GioSceneRo::UpdateElementDel(const VioElement& elem) {
  FD_DQG("GioSceneRo::UpdateElementDel(): " << elem.Str());
  // switch type ...
  switch(elem.Type()) {
  case VioElement::ETrans: removeGioTrans(elem.Trans()); break;
  case VioElement::EState: removeGioState(elem.State()); break;
  default: break;
  }
  FD_DQG("GioSceneRo::UpdateElementDel(): done");
}

// new viodes style update interface: by element
void GioSceneRo::UpdateElementEdit(const VioElement& selem, const VioElement& delem) {
  // ignore invalid
  if(selem.Type()!=delem.Type()) return;
  FD_DQG("GioSceneRo::UpdateElementEdit(): " << selem.Str() << " >>> " << delem.Str());
  // switch type ...
  switch(selem.Type()) {
  case VioElement::ETrans: moveGioTrans(selem.Trans(),delem.Trans()); break;
  case VioElement::EState: moveGioState(selem.State(),delem.State()); break;
  default: break;
  }
  FD_DQG("GioSceneRo::UpdateElementEdit(): done");
}

// new viodes style update interface: by element
void GioSceneRo::UpdateElementProp(const VioElement& elem) {
  FD_DQG("GioSceneRo::UpdateElementProp(): " << elem.Str());
  // switch type ...
  switch(elem.Type()) {
  case VioElement::ETrans: {
    GioTrans* trans = TransItem(elem.Trans());
    if(trans) trans->updateData();
    break;
  }
  case VioElement::EState: { 
    GioState* state = StateItem(elem.State());
    if(state) state->updateData();
    break;
  }
  case VioElement::EEvent: {
    faudes::TransSet::Iterator tit;
    for(tit=Generator()->TransRelBegin(); tit!=Generator()->TransRelEnd(); tit++) {
      if(tit->Ev!=elem.Event()) continue;
      GioTrans* trans = TransItem(*tit);
      if(trans) trans->updateData();
    }
    break;
  }
  default: break;
  }
}



// add a complete state item
GioState* GioSceneRo::addGioState(GioState* state) {
  FD_DQG("GioSceneRo::addGioState: gio state with idx " << state->Idx());
  faudes::Idx index = state->Idx();
  // insist in state item  to be new
  if(StateItem(index)) return NULL;
  if(mStateItems.contains(state)) return NULL;
  // add item to scene
  addItem(state);
  mStateItems.append(state);
  // link 
  mStateMap[index]=state;
  // update graphics
  state->updateData();
  return state;
}


// add a state item by index and position
GioState* GioSceneRo::addGioState(faudes::Idx index, QPointF pos) {
  FD_DQG("GioSceneRo::addGioState: new state by idx and pos");
  // if exists, just move
  GioState* state = StateItem(index);;
  if(state) {
    state->hintA(pos);
    return NULL;
  }
  // create item
  state = new GioState(pGeneratorModel);
  state->Idx(index);
  state->hintA(pos);
  // add item
  return addGioState(state);
}

/*
// add a state item by postion
GioState* GioSceneRo::addGioState(QPointF pos) {
  FD_DQG("GioSceneRo::addGioState: new state by Pos");
  return state;
}
*/

// add a state item by index
GioState* GioSceneRo::addGioState(faudes::Idx index) {
  FD_DQG("GioSceneRo::addGioState: new state by index");
  GioState* state=StateItem(index);
  // insist in state to be new
  if(state) return NULL;
  // guess position and add
  QRectF rect=itemsBoundingRect();
  QPointF pos= rect.center();
  return addGioState(index,pos); 
}

// del a state item (incl all transitions)
void GioSceneRo::removeGioState(GioState* state) {
  FD_DQG("GioSceneRo::removeGioState: del state");
  faudes::Idx index=state->Idx();
  // remove all related transitions
  QList<GioTrans*> rem;
  foreach(GioTrans* trans, mTransItems) {
    if((trans->IdxA() == index) || (trans->IdxB() ==index)){
      rem.append(trans);
    } 
  }
  foreach(GioTrans* trans, rem) removeGioTrans(trans); 
  // remove state item from lists 
  if(StateItem(index)) {    
    mStateMap.remove(index);
    mStateItems.removeAll(state);
  }
  removeItem(state);
  delete state;
  FD_DQG("GioSceneRo::removeGioState: done");
}

// del a state item by index (incl all transitions) 
void GioSceneRo::removeGioState(faudes::Idx index) {
  FD_DQG("GioSceneRo::removeGioState: del state by index");
  if(GioState* state=StateItem(index)) removeGioState(state);
}

// add a >new< gio transition item incl faudes id
// (does nothing if faudes id or giotrans allready exist;
// does not insert in viogenerator) 
GioTrans* GioSceneRo::addGioTrans(GioTrans* trans) {
  faudes::Transition ftrans=trans->FTrans();
  FD_DQG("GioSceneRo::addGioTrans: new?");
  // if gio trans is known: bail out
  if(mTransItems.contains(trans)) return NULL;
  // faudes id is known: bail out
  if(TransItem(ftrans)) return NULL;
  FD_DQG("GioSceneRo::addGioTrans: new!");
  // add item to scene
  addItem(trans);
  mTransItems.append(trans);
  // link 
  mTransMap[ftrans]=trans;
  // update view
  trans->updateData();
  return trans;
}
  

// update state properties
GioState* GioSceneRo::updateGioState(faudes::Idx idx) {
  FD_DQG("GioSceneRo::updateGioState(...)");
  GioState* state = StateItem(idx);
  if(state)  state->updateData();
  return state;
}

// move faudes state to new faudes id; if possible, keep geoometry (e.g. name changed)
void GioSceneRo::moveGioState(GioState* state, faudes::Idx nidx) {
  FD_DQG("GioSceneRo::moveGioState: to: " << Generator()->SStr(nidx));
  // no such gio state
  if(!state) return;
  if(!mStateItems.contains(state)) return;
  // get old faudes state
  faudes::Idx oidx =state->Idx();
  FD_DQG("GioSceneRo::moveGioState: from: " << Generator()->SStr(oidx));
  // no change in faudes id
  if(oidx==nidx) return;
  // update faudes data in gio state
  state->Idx(nidx);
  // if new faudes state exists ... remove
  if(mStateMap.contains(nidx)) {
    removeGioState(mStateMap[nidx]);
  }
  // re-link gio state with updated faudes id
  mStateMap.remove(oidx);
  mStateMap[nidx]=state;
  // draw update
  state->updateData();
  // done
}

// move faudes transition to new faudes id, keep geoometry (e.e. event changed)
void GioSceneRo::moveGioState(faudes::Idx oidx, faudes::Idx nidx) {
  if(GioState* state=StateItem(oidx)) moveGioState(state,nidx);
}

// add or update a transition item by faudes trans
// note: we insist to have the respective states
// note: we allow the event to be invalid
GioTrans* GioSceneRo::addGioTrans(const faudes::Transition& ftrans) {
  FD_DQG("GioSceneRo::addGioTrans: new ftrans?");
  // see whether it exists
  GioTrans* trans = TransItem(ftrans);
  if(trans) return NULL;
  // insist in existence of states in generator     
  if(!Generator()->ExistsState(ftrans.X1)) return NULL;
  if(!Generator()->ExistsState(ftrans.X2)) return NULL;
  // insist in  existence of states in scene   
  if(!StateItem(ftrans.X1)) return NULL;
  if(!StateItem(ftrans.X2)) return NULL;
  // create item
  trans = new GioTrans(pGeneratorModel);
  trans->FTrans(ftrans);
  // add item
  trans= addGioTrans(trans);
  // fix geometry
  GioState* stateA=StateItem(ftrans.X1);
  GioState* stateB=StateItem(ftrans.X2);
  trans->hintAB(stateA->pos(),stateB->pos());
  // fix flags (required for new tranitions)
  trans->updateData();
  return trans;
}

// add a transition item by stateA, event and stateB
GioTrans* GioSceneRo::addGioTrans(faudes::Idx idxA, std::string event, faudes::Idx idxB) {
    FD_DQG("GioSceneRo::addGioTrans new trans by stateA, event and stateB");
    faudes::Idx ev=Generator()->EventIndex(event);
    faudes::Transition ftrans(idxA, ev, idxB);
    // add item
    return addGioTrans(ftrans);
}

// del a transition item from gio
void GioSceneRo::removeGioTrans(GioTrans* trans) {
  FD_DQG("GioSceneRo::removeGioTrans: del trans?");
  // set up faudes transition
  faudes::Transition ftrans=trans->FTrans();
  // insist in existance in gio 
  if(!TransItem(ftrans)) return;
  FD_DQG("GioSceneRo::removeGioTrans: del trans ! ");
  // remove from gio
  mTransMap.remove(ftrans);
  mTransItems.removeAll(trans);
  removeItem(trans);
  delete trans;
  FD_DQG("GioSceneRo::removeGioTrans: done ");
}

// del a transition item by faudes id
void GioSceneRo::removeGioTrans(const faudes::Transition& ftrans) {
  FD_DQG("GioSceneRo::removeGioTrans: del trans by faudes id");
  if(GioTrans* trans=TransItem(ftrans)) removeGioTrans(trans);
}


// update transition properties
GioTrans* GioSceneRo::updateGioTrans(const faudes::Transition& ftrans) {
  FD_DQG("GioSceneRo::updateGioTrans(...)");
  GioTrans* trans = TransItem(ftrans);
  if(trans)  trans->updateData();
  return trans;
}

// move faudes transition to new faudes id; if possible, keep geoometry (e.g. event changed)
void GioSceneRo::moveGioTrans(GioTrans* trans, const faudes::Transition& nftrans) {
  FD_DQG("GioSceneRo::moveGioTrans: to: " << Generator()->TStr(nftrans));
  // no such gio transition
  if(!trans) return;
  if(!mTransItems.contains(trans)) return;
  // get old faudes transition
  faudes::Transition oftrans=trans->FTrans();
  FD_DQG("GioSceneRo::moveGioTrans: from: " << Generator()->TStr(oftrans));
  // no change in faudes transition
  if(oftrans==nftrans) return;
  // update faudes data in gio transition
  trans->FTrans(nftrans);
  // if new faudes transition allready exits ... remove
  if(mTransMap.contains(nftrans)) {
    removeGioTrans(mTransMap[nftrans]);
  }
  // re-link gio trans with updated faudes transition
  mTransMap.remove(oftrans);
  mTransMap[nftrans]=trans;
  // re-construct if states changed
  if((nftrans.X1 != oftrans.X1)|| (nftrans.X2 != oftrans.X2)) {
    GioState* nstateA=StateItem(nftrans.X1);
    GioState* nstateB=StateItem(nftrans.X2);
    GioState* ostateA=StateItem(oftrans.X1);
    GioState* ostateB=StateItem(oftrans.X2);
    bool recon=false;
    if(nstateA && ostateA)
    if(VioStyle::NormF(nstateA->pos() - ostateA->pos()) > pGeneratorConfig->CtrlTolerance())
      recon=true;
    if(nstateB && ostateB)
    if(VioStyle::NormF(nstateB->pos() - ostateB->pos()) > pGeneratorConfig->CtrlTolerance())
      recon=true;
    if(nstateA && nstateB && recon) 
      trans->hintAB(nstateA->pos(),nstateB->pos());
  }
  // draw update
  trans->updateData();
  // done
}

// move faudes transition to new faudes id, keep geoometry (e.e. event changed)
void GioSceneRo::moveGioTrans(const faudes::Transition& oftrans, const faudes::Transition& nftrans) {
  if(GioTrans* trans=TransItem(oftrans)) moveGioTrans(trans,nftrans);
  else addGioTrans(nftrans);
}



// programmatic interface
void GioSceneRo::InsGioState(GioState* state) {
  FD_DQG("GioSceneRo::InsGioState: gio state with idx " << state->Idx());
  faudes::Idx index = state->Idx();
  // insist in state item  to be new to gioscene
  if(StateItem(index)) return;
  if(mStateItems.contains(state)) return;
  // insist item to exist in generator
  if(!pGeneratorModel->ElementExists(VioElement::FromState(index))) return;
  // add item to scene
  addGioState(state);
}

// programmatic interface
// note: we dont care whether this trans exists/is valid in the generator
void GioSceneRo::InsGioTrans(GioTrans* trans) {
  FD_DQG("GioSceneRo::InsGioTrans: gio trans " << trans->FTrans().Str());
  faudes::Transition ftrans = trans->FTrans();
  // insist in trans item  to be new to gioscene
  if(TransItem(ftrans)) return;
  if(mTransItems.contains(trans)) return;
  // add item to scene
  addGioTrans(trans);
}





// Report changes
bool GioSceneRo::Modified(void) {
  return mModified;
};

// reset changes
void GioSceneRo::Modified(bool ch) {
  FD_DQG("GioSceneRo::Modified("<<ch<<")");
  // set
  if(!mModified && ch) {
    mModified=true;
    emit NotifyModified(mModified);
  }
  // clr
  if(mModified && !ch) {
    mModified=false;
    emit NotifyModified(mModified);
  }
}  

// collect and pass on changes from child
void GioSceneRo::ChildModified(bool changed) { 
  // ignore netagtives
  if(!changed) return;
  // report
  FD_DQG("GioSceneRo::ChildModified(1): model modified " << mModified);
  Modified(true);
}



// handle my events: mouse press: emit signal application via viogenartor
void GioSceneRo::mousePressEvent(QGraphicsSceneMouseEvent *event) {
  FD_DQG("GioSceneRo::mousePress"); 
  QGraphicsItem* item = itemAt(event->scenePos());
  if(!item) {
    emit MouseClick(VioElement::FromType());
  }
  if(GioState* state= qgraphicsitem_cast<GioState *>(item)) {
    FD_DQG("GioSceneRo::mousePress on state " << state->Idx()); 
    emit MouseClick(VioElement::FromState(state->Idx()));
  }
  if(GioTrans* trans= qgraphicsitem_cast<GioTrans *>(item)){
    faudes::Transition ftrans(trans->FTrans());
    FD_DQG("GioSceneRo::mousePress on transition "<< Generator()->TStr(ftrans));
    emit MouseClick(VioElement::FromTrans(ftrans));
  }
  event->accept();
}

// handle my events: mouse double click: emit signal to application
void GioSceneRo::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) { 
  FD_DQG("GioSceneRo::mouseDoubleClick"); 
  QGraphicsItem* item = itemAt(event->scenePos());
  if(GioState* state= qgraphicsitem_cast<GioState *>(item)) {
    FD_DQG("GioSceneRo::mouseDoubleClick on state " << state->Idx()); 
    emit MouseDoubleClick(VioElement::FromState(state->Idx()));
  }
  if(GioTrans* trans= qgraphicsitem_cast<GioTrans *>(item)){
    faudes::Transition ftrans(trans->FTrans());
    FD_DQG("GioSceneRo::mouseDoubleClick on transition "<< Generator()->TStr(ftrans));
    emit MouseDoubleClick(VioElement::FromTrans(ftrans));
  }
  event->accept();
}

// ignore other mouse events
void GioSceneRo::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) { (void) event; }
void GioSceneRo::mouseMoveEvent(QGraphicsSceneMouseEvent *event)    { (void) event; }


// highlite element
void GioSceneRo::Highlite(const VioElement& elem, bool on) {
  // switch type ...
  switch(elem.Type()) {
  // ... highlite transition
  case VioElement::ETrans: {
    if(GioTrans* trans=TransItem(elem.Trans())) trans->highlite(on);
    break;
  }
  // ... highlite state
  case VioElement::EState: {
    if(GioState* state=StateItem(elem.State())) state->highlite(on);
    break;
  }
  // ... highlite event aka all relevant transitions
  case VioElement::EEvent: {
    foreach(GioTrans* trans, mTransItems) 
      if(trans->IdxEv()==elem.Event()) trans->highlite(on);
    break;
  }
  default: break;
  }
}

// clear highlite all
void GioSceneRo::HighliteClear(void) {
  foreach(GioState* state, mStateItems) 
    state->highlite(false);
  foreach(GioTrans* trans, mTransItems) 
    trans->highlite(false);
}


// ctrl editing
void GioSceneRo::CtrlEditing(void) {
  mCtrlEditing=false;
  invalidate(QRectF(),QGraphicsScene::ForegroundLayer);
}
void GioSceneRo::CtrlEditing(const QPointF& pos) {
  mCtrlEditing=true;
  mCtrlPosition=pos;
  invalidate(QRectF(),QGraphicsScene::ForegroundLayer);
}
