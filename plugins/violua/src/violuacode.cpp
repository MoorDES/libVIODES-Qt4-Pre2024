/* violuacode.cpp  - Lua code editor and friends  */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2010 Thomas Moor

*/

#include "violuacode.h"

/*
 ************************************************
 ************************************************

  Helper: parse begin/end/multiline strings

 ************************************************
 ************************************************
 */

// parse block of LuaCodes
void VioLuaCodeParser(const QTextBlock& block, VioLuaCodeHighlighter::BlockData* newstate, int position=-1) {

  // search regex (is static ok ??)
  static QRegExp mControlStartExpression("\\b(for|function|if|repeat|while)\\b");
  static QRegExp mMultilineStartExpression("(--\\[\\[)|(\\[=*\\[)");
  static QRegExp mFilterExpression("(--(?!\\[\\[)([^\\n]*))|(\"((\\\\.)|([^\\\\\"]))*\")|('((\\\\.)|([^\\\\']))*')"); // got it ;-)  comments,strings, sq strings
  QRegExp mStopExpression;
  // have an accumulator, initialise with previous state
  VioLuaCodeHighlighter::BlockData* state = 0;
  state = dynamic_cast< VioLuaCodeHighlighter::BlockData* >( block.previous().userData() );
  if(state) *newstate=*state;
  state = newstate;
  FD_DQ("VioLuaCodeParser(): state #" << state->mSectionVector.size());
  // clear position result
  state->mControlBegin.mName="";
  state->mControlEnd.mName="";
  state->mComments.clear();
  state->mStrings.clear();
  // parse this block for tags
  QString text= block.text();
  int offset=block.position();
  FD_DQ("VioLuaCodeParser(): block " << block.blockNumber() << " text " << VioStyle::StrFromQStr(text));
  // remove single line comments and strings (if not in multiline mode ... this is not perfect:
  // technically, multilinemode can take place in a single line, at least for strings  ...)
  bool filter=true;
  if(state->mSectionVector.size()>0) 
  if(state->mSectionVector.back().mType==VioLuaCodeHighlighter::String) 
    filter=false;
  if(state->mSectionVector.size()>0) 
  if(state->mSectionVector.back().mType==VioLuaCodeHighlighter::Comment) 
    filter=false;
  int index=0;
  if(filter)
  while(index>=0) {
    index = mFilterExpression.indexIn(text,index);
    if(index<0) break;
    int length = mFilterExpression.matchedLength();
    // replace
    text.replace(index,length,'x');
    // proceed with next match
    index = index+length;
  }
  FD_DQ("VioLuaCodeParser(): filter " << block.blockNumber() << " text " << VioStyle::StrFromQStr(text));
  // go for relevant keys
  index = 0;
  while(index >= 0) {
    // seek for control/ml starts
    int index_ctrl = mControlStartExpression.indexIn(text,index);
    int index_ml   = mMultilineStartExpression.indexIn(text,index);
    // seek for relevant stop
    int index_stop = -1;
    if(state->mSectionVector.size()>0) {
      mStopExpression.setPattern(state->mSectionVector.back().mStopExpression);
      index_stop=mStopExpression.indexIn(text,index);
    }
    // hack: ensure progress
    index++; 
    // dont allow for starts in multiline
    if(state->mSectionVector.size()>0) 
      if(state->mSectionVector.back().mType==VioLuaCodeHighlighter::String) {
        index_ctrl=-1;
        index_ml=-1;
      }
    if(state->mSectionVector.size()>0) 
      if(state->mSectionVector.back().mType==VioLuaCodeHighlighter::Comment) {
        index_ctrl=-1;
        index_ml=-1;
      }
    // case a) stop is first
    if(   index_stop>=0 && 
	 (index_stop <= index_ctrl || index_ctrl <0) && 
         (index_stop <= index_ml   || index_ml <0) ) 
    {
      int length = mStopExpression.matchedLength();
      QString match = text.mid(index_stop,length);        
      FD_DQ("VioLuaCodeParser(): stop " << VioStyle::StrFromQStr(match) << " tag at " << 
        index_stop << " (#" << length <<")");
      // if in target position, record begin and end tag
      if(position >= offset+index_stop)
      if(position <  offset+index_stop+length) {
        FD_DQ("VioLuaCodeParser(): end label at current position");
     	state->mControlEnd.mPosition = offset+index_stop;
	state->mControlEnd.mLength = length;
        state->mControlEnd.mName = match;
        if(state->mSectionVector.size()>0) 
          state->mControlBegin=state->mSectionVector.back();
      }
      // if we have a string, record in strings
      if(state->mSectionVector.size()>0) 
      if(state->mSectionVector.back().mType==VioLuaCodeHighlighter::String) {
        VioLuaCodeHighlighter::SectionInfo sinfo;
        sinfo.mPosition=state->mSectionVector.back().mPosition;
        sinfo.mLength=offset+index_stop+length - sinfo.mPosition;          
        state->mStrings.append(sinfo);
      }
      // if we have a comment, record in comment
      if(state->mSectionVector.size()>0) 
      if(state->mSectionVector.back().mType==VioLuaCodeHighlighter::Comment) {
        VioLuaCodeHighlighter::SectionInfo sinfo;
        sinfo.mPosition=state->mSectionVector.back().mPosition;
        sinfo.mLength=offset+index_stop+length - sinfo.mPosition;          
        state->mComments.append(sinfo);
      }
      // pop the state
      if(state->mSectionVector.size()>0) {
        state->mSectionVector.pop_back();       
      } 
      index=index_stop+length;
      continue;
    } 
    // case b) start control is first
    if(   index_ctrl>=0 && 
	 (index_ctrl <= index_stop || index_stop <0) && 
         (index_ctrl <= index_ml   || index_ml <0) ) 
    {
      int length = mControlStartExpression.matchedLength();
      QString match = text.mid(index_ctrl,length);        
      FD_DQ("VioLuaCodeParser(): start ctrl " << VioStyle::StrFromQStr(match) << 
        " tag at " << index_ctrl << " (#" << length <<")");
      VioLuaCodeHighlighter::SectionInfo sinfo;
      sinfo.mName=match;
      sinfo.mPosition=offset+index_ctrl;
      sinfo.mLength=length;
      sinfo.mType=VioLuaCodeHighlighter::Control;
      if(match=="for") sinfo.mStopExpression = "\\bend\\b"; 
      else if(match=="function") sinfo.mStopExpression = "\\bend\\b"; 
      else if(match=="if") sinfo.mStopExpression = "\\bend\\b"; 
      else if(match=="repeat") sinfo.mStopExpression = "\\buntil\\b"; 
      else if(match=="while") sinfo.mStopExpression = "\\bend\\b"; 
      state->mSectionVector.push_back(sinfo);
      index=index_ctrl+length;
      continue;
    }
    // case c) start multiline is first
    if(   index_ml>=0 && 
	 (index_ml <= index_ctrl || index_ctrl <0) && 
         (index_ml <= index_stop   || index_stop <0) ) {
      int length = mMultilineStartExpression.matchedLength();
      QString match = text.mid(index_ml,length);        
      FD_DQ("VioLuaCodeParser(): start ml " << VioStyle::StrFromQStr(match) << 
        " tag at " << index_ml << " (#" << length <<")");
      VioLuaCodeHighlighter::SectionInfo sinfo;
      sinfo.mName=match;
      sinfo.mPosition=offset+index_ml;
      sinfo.mLength=length;
      sinfo.mType=VioLuaCodeHighlighter::String;
      if(match=="--[[") sinfo.mType=VioLuaCodeHighlighter::Comment;
      QString endex=match;
      endex.replace('[',"\\]");
      sinfo.mStopExpression = endex;
      state->mSectionVector.push_back(sinfo);
      index=index_ml+length;
    }
    // no matches at all: done
    break;
  } // parse loop

  // record continuing strings
  if(state->mSectionVector.size()>0) 
  if(state->mSectionVector.back().mType==VioLuaCodeHighlighter::String) {
     VioLuaCodeHighlighter::SectionInfo sinfo;
     sinfo.mPosition=state->mSectionVector.back().mPosition;
     sinfo.mLength=offset+block.length() - sinfo.mPosition;          
     state->mStrings.append(sinfo);
  }
  // record continuing comments
  if(state->mSectionVector.size()>0) 
  if(state->mSectionVector.back().mType==VioLuaCodeHighlighter::Comment) {
     VioLuaCodeHighlighter::SectionInfo sinfo;
     sinfo.mPosition=state->mSectionVector.back().mPosition;
     sinfo.mLength=offset+block.length() - sinfo.mPosition;          
     state->mComments.append(sinfo);
  }


}


/*
 ************************************************
 ************************************************

  Implementation: highlighter 

 ************************************************
 ************************************************
 */

// construct
VioLuaCodeHighlighter::VioLuaCodeHighlighter(QTextDocument *parent)
  : QSyntaxHighlighter(parent)
{

  HighlightingRule rule;

  mStringFormat.setFontItalic(true);
  mStringFormat.setForeground(VioStyle::Color(VioGreen));
  rule.pattern = QRegExp("(\"((\\\\.)|([^\\\\\"]))*\")|('((\\\\.)|([^\\\\']))*')");
  rule.format = mStringFormat;
  mHighlightingRules.append(rule);

  mKeywordFormat.setForeground(VioStyle::Color(VioBlue));
  rule.pattern = QRegExp("\\b(and|break|do|else|elseif|end|false|for|function|if|in|local|nil|not|or|repeat|return|then|true|until|while)\\b");
  rule.format = mKeywordFormat;
  mHighlightingRules.append(rule);

  mCommentFormat.setForeground(VioStyle::Color(VioRed));
  rule.pattern = QRegExp("--[^\\n]*");
  rule.format = mCommentFormat;
  mHighlightingRules.append(rule);


}

// re-implement highlighting
void VioLuaCodeHighlighter::highlightBlock(const QString &text) {

  // prepare my set of expressions
  QMap<const HighlightingRule*, QRegExp> regex;
  foreach(const HighlightingRule &rule, mHighlightingRules) 
     regex[&rule]=QRegExp(rule.pattern);

  // test rules for std highlighting
  int index=0;
  while(index >= 0) {
    int mindex=-1;
    int mlength=0;
    const HighlightingRule* mrule=0;
    foreach(const HighlightingRule &rule, mHighlightingRules) {
      QRegExp& expression=regex[&rule];
      int pindex = expression.indexIn(text,index);
      int plength = expression.matchedLength();
      if(pindex>=0 &&  (pindex < mindex || mindex<0)) {
        mindex=pindex;
        mlength=plength;
        mrule=&rule;
      }
    }
    if(mindex>=0) {
      setFormat(mindex, mlength, mrule->format);
    }
    index=mindex+mlength;
  }

  // parse for tags to propage state
  BlockData* newstate = new BlockData();
  VioLuaCodeParser(currentBlock(),newstate);
  setCurrentBlockUserData(newstate);
  
  // highligt multiline strings that occred during parse
  for(int i=0; i<newstate->mStrings.size(); i++) {
    int index = newstate->mStrings.at(i).mPosition - currentBlock().position();
    int length = newstate->mStrings.at(i).mLength;
    if(index<0) { length+=index; index=0;}
    setFormat(index,length,mStringFormat);
  }

  // highligt multiline comments that occred during parse
  for(int i=0; i<newstate->mComments.size(); i++) {
    int index = newstate->mComments.at(i).mPosition - currentBlock().position();
    int length = newstate->mComments.at(i).mLength;
    if(index<0) { length+=index; index=0;}
    setFormat(index,length,mCommentFormat);
  }

  // toggle qt state to trigger subsequent updates
  // todo: only when state changed
  setCurrentBlockState( currentBlockState()>0 ? 0 : 1);


}



/*
 ************************************************
 ************************************************

  Implementation: editor

 ************************************************
 ************************************************
 */

// construct
VioLuaCodeEditor::VioLuaCodeEditor(QWidget *parent) :
  QPlainTextEdit(parent) {

  // default font
  mFont.setFamily("Courier");
  mFont.setFixedPitch(true);
  setFont(mFont);

  // color scheme
  mTagMatchFormat.setBackground(VioStyle::Color(VioGreen).light(300));
  mTagMatchFormat.setForeground(VioStyle::Color(VioBlack));
  mTagMissFormat.setBackground(VioStyle::Color(VioRed).light(300));
  mTagMissFormat.setForeground(VioStyle::Color(VioBlack));

  // dont want the std context menu
  setContextMenuPolicy(Qt::PreventContextMenu);

  // have my highlighter
  new VioLuaCodeHighlighter(document());

  // track cursor for advanced highlighting
  connect(this,SIGNAL(cursorPositionChanged(void)),this,SLOT(TrackCursor(void)));

  // have/link to line number widget
  lineNumberArea = new VioLCLineNumberArea(this);
  mHighLightLine=-1;
  connect(this, SIGNAL(textChanged()), this, SLOT(ShowLine()));
  connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
  connect(this, SIGNAL(updateRequest(const QRect &, int)), this, SLOT(updateLineNumberArea(const QRect &, int)));
  //connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
  updateLineNumberAreaWidth(0);

} 

// set/get size
int VioLuaCodeEditor::FontSize(void) {
  return mFont.pointSize();
}

// set/get size
void VioLuaCodeEditor::FontSize(int sz) {
  if(sz>=5 && sz <= 24) {
    mFont.setPointSize(sz);
    setFont(mFont);
  }
}

// show line
void VioLuaCodeEditor::ShowLine(int line) {
  mHighLightLine=line;
  lineNumberArea->update();
}


// line numbers: width
int VioLuaCodeEditor::lineNumberAreaWidth() {
  int digits = 1;
  int max = qMax(1, blockCount());
  while (max >= 10) {
    max /= 10;
    ++digits;
  }
  int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;
  return space;
}

// line numbers: update width
void VioLuaCodeEditor::updateLineNumberAreaWidth(int blocks) {
  (void) blocks;
  setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

// line numbers: pass on upate
void VioLuaCodeEditor::updateLineNumberArea(const QRect &rect, int dy) {
  if(dy) lineNumberArea->scroll(0, dy);
  else lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
  if(rect.contains(viewport()->rect())) updateLineNumberAreaWidth(0);
}

// line numbers: track resize
void VioLuaCodeEditor::resizeEvent(QResizeEvent *e){
  QPlainTextEdit::resizeEvent(e);
  QRect cr = contentsRect();
  lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

// line numbers: do draw numbers
void VioLuaCodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event) {
  QPainter painter(lineNumberArea);
  painter.fillRect(event->rect(), Qt::lightGray);

  QTextBlock block = firstVisibleBlock();
  int blockNumber = block.blockNumber();
  int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
  int bottom = top + (int) blockBoundingRect(block).height();

  while(block.isValid() && top <= event->rect().bottom()) {
    if(block.isVisible() && bottom >= event->rect().top()) {
      if(blockNumber +1 == mHighLightLine) {
        QRect hirect=QRect(0,top,lineNumberArea->width(),fontMetrics().height());
        painter.fillRect(hirect,Qt::red);
      }
      QString number = QString::number(blockNumber + 1);
      painter.setPen(Qt::black);
      painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                          Qt::AlignRight, number);
    }
    block = block.next();
    top = bottom;
    bottom = top + (int) blockBoundingRect(block).height();
    ++blockNumber;
  }
}

// track tags 
void VioLuaCodeEditor::TrackCursor(void) {
  FD_DQ("VioLuaCodeEditorTrackCursor()");
  // retrieve highlighters state
  const QTextCursor& cursor = textCursor();
  const QTextBlock& block = cursor.block();
  VioLuaCodeHighlighter::BlockData posstate;
  VioLuaCodeParser(block,&posstate,cursor.position());  
  FD_DQ("VioLuaCodeEditorTrackCursor(): posstate #" << posstate.mSectionVector.size());
  // set visual feed back
  QList<QTextEdit::ExtraSelection> xsellist;
  QTextEdit::ExtraSelection xselect;
  xselect.cursor=QTextCursor(document());
  if(posstate.mControlEnd.mName==posstate.mControlBegin.mName) 
    xselect.format=mTagMatchFormat;
  else
    xselect.format=mTagMissFormat;
  // visual end tag
  if(posstate.mControlEnd.mName!="") {
    FD_DQ("VioLuaCodeHighlighter(): found cursor at end label pos " << posstate.mControlEnd.mPosition);
    xselect.cursor.setPosition(posstate.mControlEnd.mPosition,QTextCursor::MoveAnchor);
    xselect.cursor.setPosition(posstate.mControlEnd.mPosition+posstate.mControlEnd.mLength,QTextCursor::KeepAnchor);
    xsellist.append(xselect);
  }
  // visual begin tag
  if(posstate.mControlBegin.mName!="") {
    FD_DQ("VioLuaCodeHighlighter(): found relevant begin label pos " << posstate.mControlBegin.mPosition);
    xselect.cursor.setPosition(posstate.mControlBegin.mPosition,QTextCursor::MoveAnchor);
    xselect.cursor.setPosition(posstate.mControlBegin.mPosition+posstate.mControlBegin.mLength,QTextCursor::KeepAnchor);
    xsellist.append(xselect);
  }
  // apply
  setExtraSelections(xsellist); 
}


