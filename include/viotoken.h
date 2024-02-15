/* viotoken.h  - token editor and friends  */


/*
   Graphical IO for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2010 Thomas Moor

*/


#ifndef FAUDES_VIOTOKEN_H
#define FAUDES_VIOTOKEN_H


#include "viostyle.h"



/*
 *****************************************************
 *****************************************************

 A VioTokenHighlighter is a highlighter for faudes token
 formated text.

 *****************************************************
 *****************************************************
 */


class VIODES_API VioTokenHighlighter : public QSyntaxHighlighter {

Q_OBJECT

public:

  // construct/destruct
  VioTokenHighlighter(QTextDocument *parent = 0);
  ~VioTokenHighlighter(void) {};

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
  QTextCharFormat mBinaryFormat;

  // section nesting 
  QRegExp mTagExpression;

public:

  // nested section data 
  class SectionInfo {
  public:
    QString mName;
    int     mPosition;
    int     mLength;
  };
  class BlockData : public QTextBlockUserData {
  public:
    virtual ~BlockData(void) {};
    QVector<SectionInfo> mSectionVector;
    SectionInfo mBeginTag;
    SectionInfo mEndTag;
  };

};


// forward
class LineNumberArea;

/*
 *****************************************************
 *****************************************************

 A VioTokenEditor is a QPlainTextEdit with 
 VioTokenHighlighter. It uses advanced highlighting
 and line numbering (the latter from an qt provided
 example)

 *****************************************************
 *****************************************************
 */


class VIODES_API VioTokenEditor : public QPlainTextEdit {

Q_OBJECT

public:

  // construct/destruct
  VioTokenEditor(QWidget *parent = 0);

  // styles
  QTextCharFormat mTagMatchFormat;
  QTextCharFormat mTagMissFormat;

  // line numbers (from qt example)
  void lineNumberAreaPaintEvent(QPaintEvent *event);
  int lineNumberAreaWidth();

public slots:
  // show line (token error)
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

  // line to highlight (token error)
  int mHighLightLine;


};

/*
 *****************************************************
 *****************************************************

 A LineNumberArea is a utility widget to support
 line numbers withn a VioTokenEditor. The actual darwing
 is within the corresponding VioTokenEdit, this class
 only passes on events/sizehints. The LineNumberArea class is 
 literally taken from an qt provided example, using the
 commercial license (or LGPL)

 *****************************************************
 *****************************************************
 */

class LineNumberArea : public QWidget {
Q_OBJECT
public:
  LineNumberArea(VioTokenEditor *editor) : QWidget(editor) {
         tokenEditor = editor;
  }
  QSize sizeHint() const {
    return QSize(tokenEditor->lineNumberAreaWidth(), 0);
  }
protected:
  void paintEvent(QPaintEvent *event) {
       tokenEditor->lineNumberAreaPaintEvent(event);
  }
private:
  VioTokenEditor *tokenEditor;
};



#endif
