/* vioedit.h  - test application for editable vio widgets */


#ifndef FAUDES_VIODIAG_H
#define FAUDES_VIODIAG_H

#include <QApplication>
#include <QtGui>
#include "libviodes.h"


/*
************************************************
************************************************
  
A VioWindow is a main window that displays a 
VioWidget. It is equipped with std editor actions
aka new,open,save, etc.

************************************************
************************************************
*/

class VioWindow : public QMainWindow {

 Q_OBJECT

public:

  // constructor
  VioWindow(void);

  // set/get widget (we take ownership)
  void Widget(VioWidget* viowid);
  VioWidget* Widget(void);

  // get parameter
  QString ShortTitle(void);
  QString CurrentFile(void);

public slots:

  // std editor file menu slots
  void About(void);
  void New(void);
  void Open(void);
  void OpenRecent(void);
  void Save(void);
  void SaveAs(void);
  void ImportFaudes(void);
  void ExportFaudes(void);

  // helper slots that actually perform file io
  void LoadFile(const QString &fileName);
  void SaveFile(const QString &fileName);
  void LoadConfig(const QString &fileName);

  // show (faudes) error messages
  void FaudesError(const QString& faudeserror);
  void FaudesStatus(const QString& faudeserror);

  // bring to front
  void ActivateWindow(void);
  void ActivateAllWindows(void);
  void ActivateConsole(void);


protected:

  // reimplement close event handler
  void closeEvent(QCloseEvent * event);

private slots:

  // fix menus
  void InstallViewMenu(void);
  void InstallWindowsMenu(void);
  void InstallEditMenu(void);

  // fix recent files action list
  void ClearRecentFileActions(void);
  void UpdateRecentFileActions(void);

private:

  // initialise
  void CreateActions(void);
  void CreateMenus(void);


  // helper: stripp filename
  QString StrippedName(const QString &fullFileName);

  // helper: set filename incl window title etc
  void CurrentFile(const QString &fullFileName);

  // state: file name
  QString mCurrentFile;

  // central widget (changes on new or open)
  VioWidget* mVioWidget;

  // my menues
  QMenu *mFileMenu;
  QMenu *mNewMenu;
  QMenu *mImportMenu;
  QMenu *mEditMenu;
  QMenu *mViewMenu;
  QMenu *mWindowsMenu;

  // my file actions
  QAction *mAboutAct;
  QAction *mOpenAct;
  QAction *mSaveAct;
  QAction *mSaveAsAct;
  QAction *mExportAct;
  QAction *mExitAct;

  // configurable new actions
  QList<QAction*> mNewActions;
  QList<QAction*> mImportActions;

  // my edit actions 
  QAction *mUndoAct;
  QAction *mRedoAct;
  QAction *mCutAct;
  QAction *mCopyAct;
  QAction *mPasteAct;

  // recent file actions with mutable seperator
  QAction *mSeparatorRecentAct;
  QAction *mClearRecentAct;
  enum { MaxRecentFiles = 5 };
  QAction *mRecentFileActs[MaxRecentFiles];

  // console action
  QAction* mConsoleAct; 

  // have a console
  static QMainWindow* spConsole;

};

#endif
