/* vioconsole.cpp  - display faudes console out  */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2010 Thomas Moor

*/

#include "vioconsole.h"

// use my own debugging output to avoid messup with logging console
#undef FD_WARN
//#define FD_WARN(a) std::cout << "CONSOLE DEBUG: " << a << std::endl 
#define FD_WARN(a)


/*
 ************************************************
 ************************************************

 implementation VioFaudesLogger

 ************************************************
 ************************************************
 */

// static singleton
VioFaudesLogger* VioFaudesLogger::mpVInstance = NULL;

// construct
VioFaudesLogger::VioFaudesLogger(QObject* parent)  : QObject(parent), ConsoleOut() {
  FD_WARN("VioConsoleLogger(): construct/install: violog at " << 
     this << " faudes at " << faudes::ConsoleOut::G());
  faudes::ConsoleOut::G()->Redirect(this);
};

// destruct 
VioFaudesLogger::~VioFaudesLogger()  {
  FD_WARN("VioConsoleLogger(): destruct: violog at " << 
     this << " faudes at " << faudes::ConsoleOut::G());
  faudes::ConsoleOut::G()->Redirect(0);
  mpVInstance=NULL;
  FD_WARN("VioConsoleLogger(): destruct done: faudes at " << faudes::ConsoleOut::G());
};

// access singleton
VioFaudesLogger* VioFaudesLogger::G(void) { 
  if(!mpVInstance) mpVInstance = new VioFaudesLogger();
  return mpVInstance;
}

// destruct singleton
void VioFaudesLogger::Destruct(void) {
  if(!mpVInstance) return;
  delete mpVInstance;
  mpVInstance=0;
}
  

// faudes hook ...
// ... is rather fragile due to interference between logging and loop call back
// ... dont use debugging macros FD_xxx here, they will mess up logging
void VioFaudesLogger::DoWrite(const std::string& message,long int cntnow, long int cntdone) {
  // initialize my statics
  static bool doinit=true;
  static QTime recent;
  if(doinit) {
    recent.start();
    doinit=false;
  }
  // conditional logging every 5000ms max
  if(cntnow!=0 || cntdone !=0) {
    if(recent.elapsed() < 5000) {
      //std::cout << "VioFaudesLoggerDoWrite(): swallow ms " << now.secsTo(next) << std::endl;
      return;
    }
  }
  //std::cout << "VioFaudesLoggerDoWrite(): #" << message.size() << std::endl;
  // pass on to base, i.e. std out or file
  faudes::ConsoleOut::DoWrite(message,cntnow,cntdone);

  // filter
  QString qmessage=VioStyle::QStrFromStr(message);
  bool in=true;
  if(qmessage.startsWith("FAUDES_")) in=false;
  if(qmessage.startsWith("DESTOOL_")) in=false;
  if(qmessage.startsWith("FAUDES_WARN")) in=true;
  else if(qmessage.startsWith("FAUDES_EXCEPTION")) in=true;
  else if(qmessage.startsWith("FAUDES_PROGRESS")) in=true;
  else if(qmessage.startsWith("FAUDES_LUAPRINT")) in=true;
  if(!in) return;

  // note: we must ensure to filter out stuff from non-qt threads
  // since they block the queue; e.g. iodevice background thread

  // figure whether this is the main ui thread
  bool inui = (QApplication::instance()->thread()==QThread::currentThread());

  // use std signal when in main ui thread
  if(inui) emit NotifyAppend(qmessage);
  // use alternative signal when called from different thread
  else emit NotifyAltAppend(qmessage);
  // treat timer
  recent.restart();
}



/*
 ************************************************
 ************************************************

  Implementation: highlighter 

 ************************************************
 ************************************************
 */

// construct
VioConsoleHighlighter::VioConsoleHighlighter(QTextDocument *parent)
  : QSyntaxHighlighter(parent)
{

  mLabelFormat.setForeground(VioStyle::Color(VioGrey).light(200));
  mRedFormat.setForeground(VioStyle::Color(VioRed));
  mPromptFormat.setForeground(VioStyle::Color(VioBlue));

  HighlightingRule rule;

  rule.pattern = QRegExp("FAUDES_.*:");
  rule.pattern.setMinimal(true);
  rule.format = mLabelFormat;
  mHighlightingRules.append(rule);

  rule.pattern = QRegExp("FAUDES_WARN:");
  rule.pattern.setMinimal(true);
  rule.format = mRedFormat;
  mHighlightingRules.append(rule);

  rule.pattern = QRegExp("FAUDES_EXCEPTION:");
  rule.pattern.setMinimal(true);
  rule.format = mRedFormat;
  mHighlightingRules.append(rule);

  rule.pattern = QRegExp("^> ");
  rule.pattern.setMinimal(true);
  rule.format = mPromptFormat;
  mHighlightingRules.append(rule);

}

// re-implement highlighting
void VioConsoleHighlighter::highlightBlock(const QString &text) {

  // test rules for std highlighting
  foreach(const HighlightingRule &rule, mHighlightingRules) {
    QRegExp expression(rule.pattern);
    int index = expression.indexIn(text);
    while (index >= 0) {
      int length = expression.matchedLength();
      setFormat(index, length, rule.format);
      index = expression.indexIn(text, index + length);
    }
  }

}




/*
 ************************************************
 ************************************************

 implementation VioConsoleWidget

 ************************************************
 ************************************************
 */

// construct
VioConsoleWidget::VioConsoleWidget(QWidget* parent) : QWidget(parent) {
  FD_WARN("VioConsoleWidget(): construct");
  // my widgets
  mConsoleText = new QPlainTextEdit();
  mConsoleText->setReadOnly(false);
  mConsoleText->setCenterOnScroll(true);
  mConsoleText->setMaximumBlockCount(1000);
  mConsoleText->setUndoRedoEnabled(false);
  mConsoleText->setContextMenuPolicy(Qt::NoContextMenu);
  // install myself as filter
  mConsoleText->installEventFilter(this);
  // my find dialog
  mFindDialog = new VioFindDialog(this);
  mFindPattern="";
  mFindFlags=0;
  // text appearance
  QFont font;
  font.setFamily("Courier");
  font.setFixedPitch(true);
  mConsoleText->setFont(font);
  // highlighter
  VioConsoleHighlighter* highlighter = new VioConsoleHighlighter(mConsoleText->document());
  (void) highlighter;
  // my layout
  mVbox = new QVBoxLayout(this);
  mVbox->setMargin(0);
  mVbox->setSpacing(5);
  mVbox->addWidget(mConsoleText);
  // appearance
  setMinimumWidth(600);
  setMinimumHeight(200);
  // Clear all, say hello
  Clear();
  // instantiage logger
  VioFaudesLogger::G();
  // connect to logger
  connect(VioFaudesLogger::G(),SIGNAL(NotifyAppend(QString)), this, SLOT(AppendFaudes(QString)));
  connect(VioFaudesLogger::G(),SIGNAL(NotifyAltAppend(QString)), this, SLOT(AppendFaudes(QString)),Qt::BlockingQueuedConnection);
}

// destruct
VioConsoleWidget::~VioConsoleWidget(void) {
  FD_WARN("VioConsoleWidget(): detsruct");
  // destruct logger
  VioFaudesLogger::Destruct();
}

// configure
void VioConsoleWidget::BufferSize(int max) {
  mConsoleText->setMaximumBlockCount(max); 
}

// clear all history
void VioConsoleWidget::Clear(void) {
  mConsoleText->clear();
  mFindPattern="";
  mFindFlags=0;
  mConsoleText->appendPlainText("libFAUDES console");
  mPrePrompt=mConsoleText->textCursor();
  mPostPrompt=mConsoleText->textCursor();
  AppendNewLine();
  AppendPrompt();
  mHistoryCurrent=mHistory.size();
}

// have new line
void VioConsoleWidget::AppendNewLine(void) {
  mConsoleText->moveCursor(QTextCursor::End);
  mConsoleText->insertPlainText("\n");
  mPrePrompt=mConsoleText->textCursor();
  mPostPrompt=mConsoleText->textCursor();
}

// have a prompt
void VioConsoleWidget::AppendPrompt(void) {
  mConsoleText->moveCursor(QTextCursor::End);
  int pppos = mConsoleText->textCursor().position();
  mConsoleText->insertPlainText("> ");
  mPostPrompt.setPosition(mConsoleText->textCursor().position());
  mPrePrompt.setPosition(pppos);
}

// remove line
void VioConsoleWidget::RemoveLastLine(void) {
  mPrePrompt.setPosition(mPrePrompt.position(), QTextCursor::MoveAnchor);
  mPrePrompt.movePosition(QTextCursor::End, QTextCursor::KeepAnchor, 1);
  mPrePrompt.removeSelectedText();
  mConsoleText->moveCursor(QTextCursor::End);
  mPrePrompt=mConsoleText->textCursor();
  mPostPrompt=mConsoleText->textCursor();
}

// append faudes message before promt
void VioConsoleWidget::AppendFaudes(QString message) {
  //std::cout << "VioFaudesLogger::Append(#"<<message.size()<<")" << std::endl;
  // mConsoleText->appendPlainText(message);
  // mConsoleText->moveCursor(QTextCursor::End);
  // mConsoleText->insertPlainText(message);
  mPrePrompt.insertText(message);
  mConsoleText->setTextCursor(mPostPrompt);
  if(!mConsoleText->verticalScrollBar()->isSliderDown())
     mConsoleText->ensureCursorVisible();
}

// append keys
void VioConsoleWidget::AppendPlain(const QString& txt) {
  // bail out
  if(txt=="") return;
  // goto last line
  //std::cout << " A pos " << mConsoleText->textCursor().position() <<
  //  " ppos " << mPrePrompt.position() << std::endl;
  if(mConsoleText->textCursor().position() < mPrePrompt.position())
    mConsoleText->moveCursor(QTextCursor::End);
  if(mConsoleText->textCursor().position() < mPrePrompt.position()+2)
    mConsoleText->moveCursor(QTextCursor::Right);
  if(mConsoleText->textCursor().position() < mPrePrompt.position()+2)
    mConsoleText->moveCursor(QTextCursor::Right);
  //std::cout << " B pos " << mConsoleText->textCursor().position() <<
  //  " ppos " << mPrePrompt.position() << std::endl;
  // append
  int pppos = mPrePrompt.position();
  mConsoleText->textCursor().insertText(txt);
  mConsoleText->ensureCursorVisible();
  mPostPrompt.setPosition(mConsoleText->textCursor().position());
  mPrePrompt.setPosition(pppos);
}


// execute recent statement
void VioConsoleWidget::Execute(void) {
  // goto end
  mConsoleText->moveCursor(QTextCursor::End);
  // figure statement
  mPostPrompt.setPosition(mPrePrompt.position(), QTextCursor::MoveAnchor);
  mPostPrompt.movePosition(QTextCursor::End, QTextCursor::KeepAnchor, 1);
  QString cmd=mPostPrompt.selectedText();
  //cmd = mConsoleText->textCursor().block().text();
  if(cmd.startsWith("> ")) cmd=cmd.mid(2);
  //cmd=cmd.simplified();
  QString scmd;
  for(int i=0; i< cmd.size(); i++) {
    char ch = cmd.at(i).toAscii();
    if(ch!=0) { scmd.append(ch); continue; }
    scmd.append(" ");
  }
  //std::cout << "VioConsole::Execute(): " << VioStyle::StrFromQStr(scmd) << std::endl;
  // record with history
  while(mHistory.size() > 50) mHistory.removeFirst(); // style this
  if(!scmd.isEmpty()) mHistory.append(cmd);
  if(mHistory.size()>=2)
  if(mHistory.at(mHistory.size()-2)==cmd)
    mHistory.removeLast();
  mHistoryCurrent=mHistory.size();
  // add new-line
  AppendNewLine();
  // have extra thread to evaluate
  VioConsoleEvaluate* eval= new VioConsoleEvaluate(cmd);
  QString err= eval->Execute();
  // Report error, if any
  if(err!="") {
    if(err.startsWith("[string \"string\"]")) err=QString("[line]")+err.mid(17);
    AppendFaudes(QString("FAUDES_EXCEPTION: ")+err+"\n");
  }
  // add prompt
  AppendPrompt();
}


// completer
void VioConsoleWidget::Complete(void) {
  // figure last word
  QString line = mConsoleText->textCursor().block().text();
  int wpos=line.indexOf(QRegExp("[a-zA-Z0-9_.:]+$"));
  QString word="";
  if(wpos>=0) word=line.mid(wpos);
  // get list of completions
  std::list< std::string > mlist =  faudes::LuaState::G()->Complete(VioStyle::StrFromQStr(word));
  QStringList list;
  std::list< std::string >::iterator lit;
  for(lit=mlist.begin(); lit!=mlist.end();  lit++) 
    list.push_back(VioStyle::QStrFromStr(*lit));
  // cases none, one, multiple completions
  switch(list.count()) {
    // no completions: do nothing
    case 0:
      break;
    // unique completions: insert
    case 1: 
      if(list.at(0).size()<=word.size()) break;
      AppendPlain(list.at(0).mid(word.size()));
      break;
    // multiple candidates ... insert common prefix, print all
    default: 
      // print list
      list.sort();
      for(int i=1; i<list.size(); i++) {
	AppendNewLine();
        AppendPlain(list.at(i));
      }
      // print line incl common prefix
      if(list.at(0).size()>word.size())
        line.append(list.at(0).mid(word.size()));
      AppendNewLine();
      AppendPlain(line);
  }
}

						  


// track keys, version A
bool VioConsoleWidget::eventFilter(QObject *obj, QEvent *event) {
  // only filter for my text object
  if(obj!=mConsoleText) return false;
  // keys
  if(event->type() == QEvent::KeyPress) {
    QKeyEvent *e = static_cast<QKeyEvent*>(event);
    //std::cout << "Filter key press " << e->key() << std::endl;
    // input a character: we doit
    if(e->key() >= Qt::Key_Space && e->key() <= Qt::Key_AsciiTilde) 
    if(!(e->modifiers() & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier ))){
      QString txt = e->text();
      AppendPlain(txt);
      return true;
    }
    // last line navigation left: we doit 
    if(e->key()	== Qt::Key_Left) {
      if(mConsoleText->textCursor().position() < mPrePrompt.position()) {
        mConsoleText->moveCursor(QTextCursor::End);
        return true;
      }
      if(mConsoleText->textCursor().position() > mPrePrompt.position()+2) 
        mConsoleText->moveCursor(QTextCursor::Left);
      return true;
    }
    // last line navigation right: we doit 
    if(e->key()	== Qt::Key_Right) {
      if(mConsoleText->textCursor().position() < mPrePrompt.position()) {
        mConsoleText->moveCursor(QTextCursor::End);
        return true;
      }
      mConsoleText->moveCursor(QTextCursor::Right);
      return true;
    }
    // last line delete: textedit does it 
    if(e->key()	== Qt::Key_Delete || e->key() == Qt::Key_Backspace ) {
      if(mConsoleText->textCursor().position() <= mPrePrompt.position()+2)
        return true;
      return false;
    }
    // history: we doit
    if(e->key()	== Qt::Key_Up || e->key() == Qt::Key_Down) {
      RemoveLastLine();
      AppendPrompt();
      if(e->key()== Qt::Key_Up) mHistoryCurrent--;
      if(e->key()== Qt::Key_Down) mHistoryCurrent++;
      if(mHistoryCurrent < 0) mHistoryCurrent=0;
      if(mHistoryCurrent > mHistory.size()) mHistoryCurrent=mHistory.size();
      if(mHistoryCurrent < mHistory.size()) AppendPlain(mHistory.at(mHistoryCurrent));
      return true;
    }
    // execute: we doit
    if(e->key()	== Qt::Key_Return || e->key() == Qt::Key_Enter) {
      Execute();
      return true;
    }
    // completer: we doit
    if(e->key()	== Qt::Key_Tab) {
      Complete();
      return true;
    }
    // swallow all other keys
    if(!(e->modifiers() & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier )))
      return true;
    e->ignore();
    return false;
  }
  // filter out other than left mouse button ( not functional ???)
  if(event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease) {
    QMouseEvent *e = static_cast<QMouseEvent*>(event);
    if(e->button() == Qt::LeftButton) return false;
    return true;
    
  }
  // dont filter
  //std::cout << "Filter type " << event->type() << std::endl;
  return false;
}


// track keys, version B
void VioConsoleWidget::keyPressEvent(QKeyEvent * e) {
  QWidget::keyPressEvent(e);
  /*
  // std ascii  editing
  if(e->key() >= Qt::Key_Space && e->key() <= Qt::Key_AsciiTilde) {
    QString txt = e->text();
    if(txt!="") AppendPlain(txt);
  }
  // allways accept
  e->accept();
  */
}



// copy to clippboard
void VioConsoleWidget::Copy(void) {
  mConsoleText->copy();
}

// cut to clippboard
void VioConsoleWidget::Cut(void) {
  mConsoleText->cut();
}

// paste from clippboard
void VioConsoleWidget::Paste(void) {
  // figure clipboard
  const QClipboard *clipboard = QApplication::clipboard();
  QString txt=clipboard->text();
  if(txt.size()==0) return;
  // do the paste
  AppendPlain(txt);
}


// find string
void VioConsoleWidget::Find(const QString& pattern, QTextDocument::FindFlags flags) {
  FD_DQH("VioConsoleWidget::Find(" << VioStyle::StrFromQStr(pattern) << ")");
  mFindPattern=pattern;
  mFindFlags=flags;
  // reset position
  if(mFindFlags & 0x1000) {
    if(mFindFlags & QTextDocument::FindBackward)
      mConsoleText->moveCursor(QTextCursor::End);
    else
      mConsoleText->moveCursor(QTextCursor::Start);
  }
  // doit
  bool res=mConsoleText->find(mFindPattern, mFindFlags & (~ 0xf000));
  if(res) mConsoleText->ensureCursorVisible();
}

// find string
void VioConsoleWidget::FindAgain(void) {
  Find(mFindPattern, mFindFlags & ((QTextDocument::FindFlag) (~ 0x1000)));
}

// find string
void VioConsoleWidget::FindDialog(void) {
  FD_DQH("VioConsoleWidget::FindDialog()");
  if(mFindDialog->exec()==QDialog::Accepted) 
    Find(mFindDialog->Pattern(), mFindDialog->Flags());
  FD_DQH("VioConsoleWidget::FindDialog(): done");
}

// reset lua state
void VioConsoleWidget::Reset(void) {
  faudes::LuaState::G()->Reset();
  mConsoleText->appendPlainText("Lua Reset");
  mPrePrompt=mConsoleText->textCursor();
  mPostPrompt=mConsoleText->textCursor();
  AppendNewLine();
  AppendPrompt();
  mHistoryCurrent=mHistory.size();
}

/*
 ************************************************
 ************************************************

 implementation VioFindDialog

 ************************************************
 ************************************************
 */


VioFindDialog::VioFindDialog(QWidget *parent) : QDialog(parent) {

  mReplaceFlag=false;

  // my main layout
  mVbox = new QVBoxLayout(this);
  mVbox->setMargin(5);
  mVbox->setSpacing(5);

  // pattern line
  QLabel* label = new QLabel(tr("Find:"));
  mPatternEdit = new QLineEdit;
  label->setBuddy(mPatternEdit);
  QHBoxLayout* hbox1= new QHBoxLayout();
  hbox1->setMargin(2);
  hbox1->setSpacing(5);
  hbox1->addWidget(label);
  hbox1->addWidget(mPatternEdit);
  mVbox->addLayout(hbox1);

  // replace line
  QLabel* rlabel = new QLabel(tr("Replace with:"));
  mReplaceEdit = new QLineEdit();
  label->setBuddy(mReplaceEdit);
  mReplaceLine= new QWidget();
  QHBoxLayout* hbox4 = new QHBoxLayout(mReplaceLine);
  hbox4->setMargin(2);
  hbox4->setSpacing(5);
  hbox4->addWidget(rlabel);
  hbox4->addWidget(mReplaceEdit);
  mReplaceLine->hide();
  mVbox->addWidget(mReplaceLine);


  // checkbox line
  mCaseCheck = new QCheckBox(tr("Match Case"));
  mFromStartCheck = new QCheckBox(tr("From Start"));
  mBackwardCheck = new QCheckBox(tr("Backward Search"));
  QHBoxLayout* hbox2= new QHBoxLayout();
  hbox2->setMargin(2);
  hbox2->setSpacing(10);
  hbox2->addWidget(mCaseCheck);
  hbox2->addWidget(mFromStartCheck);
  hbox2->addWidget(mBackwardCheck);
  hbox2->addStretch(1);
  mVbox->addLayout(hbox2);

  // button line
  mFindButton = new QPushButton(tr("Find"));
  mFindButton->setDefault(true);
  mReplaceButton = new QPushButton(tr("Replace"));
  mReplaceButton->hide();
  mCancelButton = new QPushButton(tr("Cancel"));
  QDialogButtonBox* bbox = new QDialogButtonBox(Qt::Horizontal);
  bbox->addButton(mFindButton,QDialogButtonBox::ApplyRole); 
  bbox->addButton(mReplaceButton,QDialogButtonBox::ApplyRole); 
  bbox->addButton(mCancelButton,QDialogButtonBox::RejectRole);
  QHBoxLayout* hbox3= new QHBoxLayout();
  hbox3->setMargin(2);
  hbox3->setSpacing(5);
  hbox3->addWidget(bbox);
  mVbox->addLayout(hbox3);

  // connect
  connect(mFindButton,SIGNAL(pressed(void)),this,SLOT(accept(void)));
  connect(mReplaceButton,SIGNAL(pressed(void)),this,SLOT(accept(void)));
  connect(mCancelButton,SIGNAL(pressed(void)),this,SLOT(reject(void)));
  connect(mPatternEdit,SIGNAL(returnPressed(void)),this,SLOT(accept(void)));
}

// config
void VioFindDialog::Replace(bool on) {
  mReplaceLine->setVisible(on);
  mReplaceButton->setVisible(on);
}

// reimplement
void VioFindDialog::accept(void) {
  mReplaceFlag = false;
  QPushButton* button = qobject_cast< QPushButton*>(sender());
  if(button==mReplaceButton) mReplaceFlag = true;
  // call base
  QDialog::accept();
}

// reimplement 
void VioFindDialog::showEvent(QShowEvent * event) {
  // fix button state mac osx
  mFindButton->setDown(false);
  mReplaceButton->setDown(false);
  mCancelButton->setDown(false);
  // call base
  QDialog::showEvent(event);
} 


// access
QString VioFindDialog::Pattern(void) {
  return mPatternEdit->text();
}

// replace
QString VioFindDialog::Replace(void) {
  return mReplaceEdit->text();
}

// access
bool VioFindDialog::CaseSensitive(void) {
  return mCaseCheck->isChecked();
}

// access
bool VioFindDialog::FromStart(void) {
  return mFromStartCheck->isChecked();
}

// access
bool VioFindDialog::Backward(void) {
  return mBackwardCheck->isChecked();
}

// access 
QTextDocument::FindFlags VioFindDialog::Flags(void) {
  QTextDocument::FindFlags res=0;
  if(CaseSensitive()) res |= QTextDocument::FindCaseSensitively;
  if(Backward()) res |= QTextDocument::FindBackward;
  if(FromStart()) res |= (QTextDocument::FindFlag) 0x1000;
  if(ReplaceFlag()) res |= (QTextDocument::FindFlag) 0x2000;
  return res;
}



/*
 ************************************************
 ************************************************

 implementation of VioConsoleEvaluate 

 ************************************************
 ************************************************
 */


// construct/destruct
VioConsoleEvaluate::VioConsoleEvaluate(QString command) : QThread(0) {
  mCommand=command;
};

// destruct  
VioConsoleEvaluate::~VioConsoleEvaluate(void) {
};


// run (this is the thread itself, called by start()
void VioConsoleEvaluate::run(void) {
  // execute this statment: should go in  a thread ??
  mErrStr="";
  try{
    faudes::LuaState::G()->Evaluate(VioStyle::StrFromQStr(mCommand));
  } catch( faudes::Exception fex) {
    mErrStr=VioStyle::QStrFromStr(fex.What());
  }
};


// api: this is callers thread
QString VioConsoleEvaluate::Execute(void) {
  // install progress dialog
  QProgressDialog progress("Evaluating", "Cancel", 0, 0, 0); 
  progress.setWindowModality(Qt::ApplicationModal);
  progress.setValue(0);
  QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
  QApplication::flush();
  // clear global break flag
  VioStyle::FaudesBreakClr(); 
  // run my extra thread
  start();
  // process applicationevent loop while running
  long int i=100; 
  while(isRunning() && (--i)>0) {
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents,10);
    QApplication::flush();
    wait(10);
  }
  bool canceled=false;
  while(isRunning()) {
    QApplication::processEvents(QEventLoop::AllEvents,10);
    QApplication::flush();
    if(progress.wasCanceled() && !canceled) {
       VioStyle::FaudesBreakSet(); 
       canceled=true;
    }
    progress.setValue(1); 
    wait(10);
  }
  // clear global break flag
  VioStyle::FaudesBreakClr(); 
  // done
  return mErrStr;
}



