/* viotoken.cpp  - token editor and friends  */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2009 Ruediger Berndt, Thomas Moor;

*/

#include "viotoken.h"

/*
 ************************************************
 ************************************************

  Helper: parse tags

 ************************************************
 ************************************************
 */

// parse block of tokens
void VioTokenParser(const QTextBlock& block, VioTokenHighlighter::BlockData* newstate, int position=-1) {
  // search regex (is static ok ??)
  static QRegExp mTagExpression("<[^>]*>");
  // have an accumulator, initialise with previous state
  VioTokenHighlighter::BlockData* state = 0;
  state = dynamic_cast< VioTokenHighlighter::BlockData* >( block.previous().userData() );
  if(state) *newstate=*state;
  state = newstate;
  FD_DQ("VioTokenParser(): state #" << state->mSectionVector.size());
  // clear position result
  state->mBeginTag.mName="";
  state->mEndTag.mName="";
  // parse this block for tags
  QString text= block.text();
  int offset=block.position();
  FD_DQ("VioTokenParser(): block " << block.blockNumber() << " text " << VioStyle::StrFromQStr(text));
  int index = mTagExpression.indexIn(text);
  while(index >= 0) {
    int length = mTagExpression.matchedLength();
    QString match = text.mid(index,length);        
    // end section tag 
    if(length>3) 
    if(match.at(1)==QChar('/')) {
      QString label = text.mid(index+2,length-3);         
      FD_DQ("VioTokenParser(): end " << VioStyle::StrFromQStr(label) << " tag at " << 
        index << " (#" << length <<")");
      if(position >= offset+index)
      if(position <  offset+index+length) {
        FD_DQ("VioTokenParser(): end label at current position");
     	state->mEndTag.mPosition = offset+index;
	state->mEndTag.mLength = length;
        state->mEndTag.mName = label;
        if(state->mSectionVector.size()>0)
          state->mBeginTag=state->mSectionVector.back();
      }
      // pop matching begin tag
      if(state->mSectionVector.size()>0)
      if(state->mSectionVector.back().mName == label) {
        FD_DQ("VioTokenParser(): pop matching begin " << VioStyle::StrFromQStr(label));
        state->mSectionVector.pop_back();        
      }
    }
    // begin section tag 
    if(length>2) 
    if(match.at(1)!=QChar('/')) {
      QString label = text.mid(index+1,length-2);         
      FD_DQ("VioTokenParser(): begin " << VioStyle::StrFromQStr(label) << " tag at " << 
        index << " (#" << length <<")");
      VioTokenHighlighter::SectionInfo sinfo;
      sinfo.mName=label;
      sinfo.mPosition=offset+index;
      sinfo.mLength=length;
      state->mSectionVector.push_back(sinfo);
    }
    // proceed with next match
    index = mTagExpression.indexIn(text, index + length);
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
VioTokenHighlighter::VioTokenHighlighter(QTextDocument *parent)
  : QSyntaxHighlighter(parent)
{

  HighlightingRule rule;

  mCommentFormat.setForeground(VioStyle::Color(VioRed));
  rule.pattern = QRegExp("%[^\n]*");
  rule.format = mCommentFormat;
  mHighlightingRules.append(rule);

  mSectionFormat.setForeground(VioStyle::Color(VioBlue));
  rule.pattern = QRegExp("<.*>");
  rule.format = mSectionFormat;
  mHighlightingRules.append(rule);


  mStringFormat.setFontItalic(true);
  mStringFormat.setForeground(VioStyle::Color(VioGreen));
  rule.pattern = QRegExp("\".*[^\\\\]\"");
  rule.format = mStringFormat;
  //mHighlightingRules.append(rule);

  mBinaryFormat.setFontItalic(true);
  mBinaryFormat.setForeground(VioStyle::Color(VioGrey));
  rule.pattern = QRegExp("=.*=(?!=)");
  rule.format = mBinaryFormat;
  mHighlightingRules.append(rule);

  mTagExpression = QRegExp("<[^>]*>");

}

// re-implement highlighting
void VioTokenHighlighter::highlightBlock(const QString &text) {

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

  // parse for tags to propage state
  BlockData* newstate = new BlockData();
  VioTokenParser(currentBlock(),newstate);
  setCurrentBlockUserData(newstate);

  // toggle state to trigger subsequent updates
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
VioTokenEditor::VioTokenEditor(QWidget *parent) :
  QPlainTextEdit(parent) {

  // adjust my outfit
  QFont font;
  font.setFamily("Courier");
  font.setFixedPitch(true);
  setFont(font);
  mTagMatchFormat.setBackground(VioStyle::Color(VioGreen).light(300));
  mTagMatchFormat.setForeground(VioStyle::Color(VioBlack));
  mTagMissFormat.setBackground(VioStyle::Color(VioRed).light(300));
  mTagMissFormat.setForeground(VioStyle::Color(VioBlack));

  // dont want the std context menu
  setContextMenuPolicy(Qt::PreventContextMenu);

  // have my highlighter
  VioTokenHighlighter* highlighter = new VioTokenHighlighter(document());
  (void) highlighter;

  // track cursor for advanced highlighting
  connect(this,SIGNAL(cursorPositionChanged(void)),this,SLOT(TrackCursor(void)));


  // have/link to line number widget
  lineNumberArea = new LineNumberArea(this);
  mHighLightLine=-1;
  connect(this, SIGNAL(textChanged()), this, SLOT(ShowLine()));
  connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
  connect(this, SIGNAL(updateRequest(const QRect &, int)), this, SLOT(updateLineNumberArea(const QRect &, int)));
  //connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
  updateLineNumberAreaWidth(0);

} 

// show line
void VioTokenEditor::ShowLine(int line) {
  mHighLightLine=line;
  lineNumberArea->update();
}


// line numbers: width
int VioTokenEditor::lineNumberAreaWidth() {
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
void VioTokenEditor::updateLineNumberAreaWidth(int blocks) {
  (void) blocks;
  setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

// line numbers: pass on upate
void VioTokenEditor::updateLineNumberArea(const QRect &rect, int dy) {
  if(dy) lineNumberArea->scroll(0, dy);
  else lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
  if(rect.contains(viewport()->rect())) updateLineNumberAreaWidth(0);
}

// line numbers: track resize
void VioTokenEditor::resizeEvent(QResizeEvent *e){
  QPlainTextEdit::resizeEvent(e);
  QRect cr = contentsRect();
  lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

// line numbers: do draw numbers
void VioTokenEditor::lineNumberAreaPaintEvent(QPaintEvent *event) {
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
void VioTokenEditor::TrackCursor(void) {
  FD_DQ("VioTokenEditorTrackCursor()");
  // retrieve highlighters state
  const QTextCursor& cursor = textCursor();
  const QTextBlock& block = cursor.block();
  VioTokenHighlighter::BlockData posstate;
  VioTokenParser(block,&posstate,cursor.position());  
  FD_DQ("VioTokenEditorTrackCursor(): posstate #" << posstate.mSectionVector.size());
  // set visual feed back
  QList<QTextEdit::ExtraSelection> xsellist;
  QTextEdit::ExtraSelection xselect;
  xselect.cursor=QTextCursor(document());
  if(posstate.mEndTag.mName==posstate.mBeginTag.mName) 
    xselect.format=mTagMatchFormat;
  else
    xselect.format=mTagMissFormat;
  // visual end tag
  if(posstate.mEndTag.mName!="") {
    FD_DQ("VioTokenHighlighter(): found cursor at end label pos " << posstate.mEndTag.mPosition);
    xselect.cursor.setPosition(posstate.mEndTag.mPosition,QTextCursor::MoveAnchor);
    xselect.cursor.setPosition(posstate.mEndTag.mPosition+posstate.mEndTag.mLength,QTextCursor::KeepAnchor);
    xsellist.append(xselect);
  }
  // visual begin tag
  if(posstate.mBeginTag.mName!="") {
    FD_DQ("VioTokenHighlighter(): found relevant begin label pos " << posstate.mBeginTag.mPosition);
    xselect.cursor.setPosition(posstate.mBeginTag.mPosition,QTextCursor::MoveAnchor);
    xselect.cursor.setPosition(posstate.mBeginTag.mPosition+posstate.mBeginTag.mLength,QTextCursor::KeepAnchor);
    xsellist.append(xselect);
  }
  // apply
  setExtraSelections(xsellist); 
}

