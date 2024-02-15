/* violuacode.h  - code editor and friends  */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2010 Thomas Moor

*/


#ifndef FAUDES_VIOLUACODE_H
#define FAUDES_VIOLUACODE_H


#include "viostyle.h"





/*
 *****************************************************
 *****************************************************

 A VioLuaCodeHighlighter is a highlighter for faudes luacode
 formated text.

 *****************************************************
 *****************************************************
 */


class VioLuaCodeHighlighter : public QSyntaxHighlighter {

Q_OBJECT

public:

  // construct/destruct
  VioLuaCodeHighlighter(QTextDocument *parent = 0);
  ~VioLuaCodeHighlighter(void) {};

protected:
  
  // highlight re-implementation 
  void highlightBlock(const QString &text);

private:

  // my highlighting rules
  struct HighlightingRule {
    QRegExp pattern;
    QTextCharFormat format;
  };
  QVector<HighlightingRule> mHighlightingRules;

  
  // styles
  QTextCharFormat mCommentFormat;
  QTextCharFormat mSectionFormat;
  QTextCharFormat mStringFormat;
  QTextCharFormat mKeywordFormat;

  // section nesting 
  QRegExp mTagExpression;

public:

  // section types
  typedef enum { None, Control, String, Comment } SectionType;

  // section data 
  class SectionInfo {
  public:
    SectionInfo() { mType=None;};
    QString mName;
    int     mPosition;
    int     mLength;
    QString mStopExpression;
    SectionType mType; 
  };

  // block state
  class BlockData : public QTextBlockUserData {
  public:
    virtual ~BlockData(void) {};
    QVector<SectionInfo> mSectionVector; // state = accumulated section starts
    SectionInfo mControlBegin;           // highlight matching tags     
    SectionInfo mControlEnd;             // highlight matching tags
    QVector<SectionInfo> mStrings;       // highlight strings
    QVector<SectionInfo> mComments;      // highlight comments
  };

};


// forward
class LineNumberArea;

/*
 *****************************************************
 *****************************************************

 A VioLuaCodeEditor is a QPlainTextEdit with 
 VioLuaCodeHighlighter. It uses advanced highlighting
 and line numbering (the latter from an qt provided
 example)

 *****************************************************
 *****************************************************
 */


class VioLuaCodeEditor : public QPlainTextEdit {

Q_OBJECT

public:

  // construct/destruct
  VioLuaCodeEditor(QWidget *parent = 0);

  // styles
  QTextCharFormat mTagMatchFormat;
  QTextCharFormat mTagMissFormat;

  // line numbers (from qt example)
  void lineNumberAreaPaintEvent(QPaintEvent *event);
  int lineNumberAreaWidth();

  // set/get size
  int FontSize(void);
  void FontSize(int sz);

public slots:

  // show line (luacode error)
  void ShowLine(int line=-1);

protected:

  // reimplement
  void resizeEvent(QResizeEvent *event);

private slots:

  // track cursor (advanced highlight: tag matching)
  void TrackCursor(void);

  //  line numbers update (qt example)
  void updateLineNumberAreaWidth(int newBlockCount);
  void updateLineNumberArea(const QRect &, int);

private:

  // line numbers (qt example)
  QWidget *lineNumberArea;

  // line to highlight (luacode error)
  int mHighLightLine;

  // my font
  QFont mFont;


};

/*
 *****************************************************
 *****************************************************

 A LineNumberArea is a utility widget to support
 line numbers withn a VioLuaCodeEditor. The actual darwing
 is within the corresponding VioLuaCodeEdit, this class
 only passes on events/sizehints. The LineNumberArea class is 
 literally taken from an qt provided example, using the
 commercial license (or LGPL)

 *****************************************************
 *****************************************************
 */

class VioLCLineNumberArea : public QWidget {
Q_OBJECT
public:
  VioLCLineNumberArea(VioLuaCodeEditor *editor) : QWidget(editor) {
         luacodeEditor = editor;
  }
  QSize sizeHint() const {
    return QSize(luacodeEditor->lineNumberAreaWidth(), 0);
  }
protected:
  void paintEvent(QPaintEvent *event) {
       luacodeEditor->lineNumberAreaPaintEvent(event);
  }
private:
  VioLuaCodeEditor *luacodeEditor;
};



#endif
