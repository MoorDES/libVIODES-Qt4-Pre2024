/* vioedit.cpp  - test application for editable vio widgets */

#include "vioedit.h"

/*
************************************************
************************************************
  
Implementation: VioWindow

************************************************
************************************************
*/

// static: one console
QMainWindow* VioWindow::spConsole = NULL;


// construct
VioWindow::VioWindow() : 
  QMainWindow(0),
  mVioWidget(0) 
{

  // have one console
  if(!spConsole) {
    spConsole= new QMainWindow();
    // have the console widget
    VioConsoleWidget* conwid = new VioConsoleWidget();
    spConsole->setCentralWidget(conwid);
  }

  // set qt window attributes
  setAttribute(Qt::WA_DeleteOnClose);
  setContentsMargins(0,0,0,0);

  // have empty vio widget to start with
  VioWidget* viowid = VioTypeRegistry::NewWidget("System");
  if(!viowid) { // todo: slot for fatal error
     FaudesError("viodes::TypeRegistry: cannot allocate widget for type \"System\"");
     exit(1);
  }

  // set up file name
  CurrentFile("");

  // prepare unser interface
  CreateActions();
  CreateMenus();
  statusBar();
  
  // set the widget
  Widget(viowid);

  // try learn about layouts vs spacing
  /*
  QWidget* wid= new QWidget(0);
  wid->setContentsMargins(0,0,0,0);
  QVBoxLayout* mVbox = new QVBoxLayout(wid);
  mVbox->setMargin(0);
  mVbox->setSpacing(0);
  //mVbox->setContentsMargins(0,0,0,0);
  mVbox->addWidget(new QTextEdit("AAA"));
  mVbox->addWidget(new VioView());
  setCentralWidget(wid);
  */
}

// set vio widget  (incl deleting current widget)
void VioWindow::Widget(VioWidget* viowid) {
  // disconnect old, delete old
  if(mVioWidget) {
    delete mVioWidget;
  }
  // take it
  mVioWidget=viowid;
  mVioWidget->setParent(this);
  setCentralWidget(mVioWidget);
  // connect it
  connect(mVioWidget,SIGNAL(ErrorMessage(const QString&)),this,SLOT(FaudesError(const QString&)));
  connect(mVioWidget,SIGNAL(StatusMessage(const QString&)),this,SLOT(FaudesStatus(const QString&)));
  connect(mVioWidget,SIGNAL(NotifyModified(bool)),this, SLOT(setWindowModified(bool))); 
  // fix modified
  setWindowModified(mVioWidget->Modified());
  // install my menues
  InstallViewMenu();
  // enable view menu if there are actions
  std::cerr << "VioWindow::Widget(): adding entries to view #" <<
    mVioWidget->View()->ViewActions().size() << std::endl;
  mViewMenu->setEnabled(mVioWidget->View()->ViewActions().size()!=0);
  // edit menu too
  InstallEditMenu();
}

// tell (not take!) widget
VioWidget* VioWindow::Widget(void) {
  return mVioWidget;
}


// file manu slots: new
void VioWindow::New() {

  // figure type from action, default to generator
  QString ftype = "System";
  if(QAction *action = qobject_cast<QAction *>(sender())) {
    ftype = action->data().toString();
  }

  // create and set new viowidget 
  VioWidget* viowid=VioTypeRegistry::NewWidget(ftype);

  // have a new window
  VioWindow* nwin = new VioWindow();
  nwin->Widget(viowid);
  QSettings settings("Faudes", "vioDiag");
  nwin->restoreGeometry(settings.value("geometry").toByteArray());
  nwin->move(nwin->pos()+QPoint(10,10));
  nwin->show();
  nwin->CurrentFile("");
}

// file manu slots: import faudes file
void VioWindow::ImportFaudes() {

  // save changes
  if(mVioWidget->Modified()) {
    int ret = QMessageBox::warning(this, 
      tr("vioDiag"),
      tr("Do you want to save the current document?"),
      QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, 
      QMessageBox::Save);
    if (ret == QMessageBox::Save) Save();  
    if (ret == QMessageBox::Cancel) return;
  }

  // figure type from action, default to generator
  QString ftype = "System";
  if(QAction *action = qobject_cast<QAction*>(sender())) {
    ftype = action->data().toString();
  }

  // open dialog: restore from settings
  QFileDialog* fdiag = new QFileDialog();
  QSettings settings("Faudes", "vioDiag");
  fdiag->restoreState(settings.value("stateFaudesFileDialog").toByteArray());

  // open dialog: vio files
  QStringList filters; filters 
    << "FAUDES generator files (*.gen)"
    << "Any File (*.*)";
  fdiag->setFilters(filters);
  fdiag->setFileMode(QFileDialog::ExistingFile);
  fdiag->setWindowTitle(QString("Import FAUDES File"));
  fdiag->setAcceptMode(QFileDialog::AcceptOpen);
  fdiag->setLabelText(QFileDialog::Accept,"Import");
  fdiag->setConfirmOverwrite(false);
  fdiag->setDefaultSuffix("gen");

  // open dialog: run and save settings
  QString filename="";
  if(fdiag->exec()) {
    if(fdiag->selectedFiles().size()==1) 
      filename=fdiag->selectedFiles().at(0);
  }
  settings.setValue("stateFaudesFileDialog", fdiag->saveState());
  delete fdiag;

  // bail out on empty
  if(filename=="") return;

  // may take time
  QApplication::setOverrideCursor(Qt::WaitCursor);

  // create new viowidget 
  QString err="";
  VioWidget* tmpviowid=0;
  try { 
    tmpviowid = VioTypeRegistry::FromFaudesFile(ftype,filename);
  } catch (faudes::Exception& fexcep) {
    if(tmpviowid) delete tmpviowid;
    err=QString("Error: ")+VioStyle::QStrFromStr(fexcep.What());
  }

  // fix cursor
  QApplication::restoreOverrideCursor();

  // report error
  if(err!="") {
    QMessageBox::warning(this, 
      tr("vioDiag"),
      tr("<p>Cannot import file %1</p>"
         "<p>%2</p>"
	 ).arg(StrippedName(filename),err));
    return;
  }
  
  // new viowidget (existing widget gets destructed)
  Widget(tmpviowid);

  // no filename
  CurrentFile("");
}


// file manu slots: open
void VioWindow::Open() {


  /*
QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                 "/usr/local",
                                                 tr("Images (*.png *.xpm *.jpg)"));
  */

  // open dialog: restore from settings
  QFileDialog* fdiag = new QFileDialog(this);
  QSettings settings("Faudes", "vioDiag");
  fdiag->restoreState(settings.value("stateFileDialog").toByteArray());

  // open dialog: vio files
  QStringList filters; filters 
    << "VioDES files (*.vio)"
    << "Any File (*.*)";
  fdiag->setFilters(filters);
  fdiag->setFileMode(QFileDialog::ExistingFile);
  fdiag->setWindowTitle(QString("Open VioDES File"));
  fdiag->setAcceptMode(QFileDialog::AcceptOpen);
  fdiag->setLabelText(QFileDialog::Accept,"Open");
  fdiag->setConfirmOverwrite(false);
  fdiag->setDefaultSuffix("vio");

  // open dialog: run and save settings
  QString filename="";
  if(fdiag->exec()) {
    if(fdiag->selectedFiles().size()==1) 
      filename=fdiag->selectedFiles().at(0);
  }
  settings.setValue("stateFileDialog", fdiag->saveState());
  delete fdiag;

  // bail out
  filename=QFileInfo(filename).canonicalFilePath();
  if(filename=="") return;

  // shall we have a new window?
  bool owin=true;
  if(!mVioWidget->Modified() && mCurrentFile=="") owin=false;

  // test all other windows for my new file 
  foreach(QWidget *widget, QApplication::topLevelWidgets()) {
    VioWindow *win = qobject_cast<VioWindow *>(widget);
    if(!win) continue;
    if(win==this) continue;
    if(win->CurrentFile()=="") continue;
    if(win->CurrentFile()!=filename) continue;
    int ret = QMessageBox::warning(this, 
      tr("vioDiag"),
      tr("This file is allready opened in another window. Do you want to open another copy?"),
	QMessageBox::Yes | QMessageBox::No);
    if(ret != QMessageBox::Yes) return;
  }
  
  // is this a revert?
  if(mCurrentFile==filename) {
    owin=false;
    int ret = QMessageBox::warning(this, 
      tr("vioDiag"),
      tr("Revert to file?"),
	QMessageBox::Yes | QMessageBox::No);
    if(ret != QMessageBox::Yes) return;
  }

  // save changes 
  if(!owin) 
  if(mVioWidget->Modified()) {
    int ret = QMessageBox::warning(this, 
      tr("vioDiag"),
      tr("Do you want to save the current document?"),
      QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, 
      QMessageBox::Save);
    if (ret == QMessageBox::Save) Save();  
    if (ret == QMessageBox::Cancel) return;
  }

  // optionally have a new window
  VioWindow* nwin = this;
  if(owin) {
     nwin = new VioWindow();
    //QSettings settings("Faudes", "vioDiag");
    nwin->restoreGeometry(settings.value("geometry").toByteArray());
    nwin->move(nwin->pos()+QPoint((int) (30.0*qrand()/RAND_MAX),(int) (30.0*qrand()/RAND_MAX)));
    nwin->show();
  }

  // do load the file
  nwin->LoadFile(filename);
}

// file manu slots: save
void VioWindow::Save() {
  if (mCurrentFile.isEmpty()) SaveAs();
  else SaveFile(mCurrentFile);
}


// file manu slots: save as 
void VioWindow::SaveAs() {

  // save dialog: restore from settings
  QFileDialog* fdiag = new QFileDialog();
  QSettings settings("Faudes", "vioDiag");
  fdiag->restoreState(settings.value("stateFileDialog").toByteArray());

  // save dialog: vio files
  QStringList filters; filters 
    << "VioDES files (*.vio)"
    << "Any File (*.*)";
  fdiag->setFilters(filters);
  fdiag->setFileMode(QFileDialog::AnyFile);
  fdiag->setWindowTitle(QString("Save Document as VioDES File"));
  fdiag->setAcceptMode(QFileDialog::AcceptSave);
  fdiag->setLabelText(QFileDialog::Accept,"Save");
  fdiag->setConfirmOverwrite(true);
  fdiag->setDefaultSuffix("vio");

  // open dialog: run and save settings
  QString filename="";
  if(fdiag->exec()) {
    if(fdiag->selectedFiles().size()==1) 
      filename=fdiag->selectedFiles().at(0);
  }
  settings.setValue("stateFileDialog", fdiag->saveState());
  delete fdiag;

  // bail out on empty
  if(filename=="") return;

  // do save
  SaveFile(filename);
}

// file manu slots: save as 
void VioWindow::ExportFaudes() {

  // save dialog: restore from settings
  QFileDialog* fdiag = new QFileDialog();
  QSettings settings("Faudes", "vioDiag");
  fdiag->restoreState(settings.value("stateFaudesFileDialog").toByteArray());

  // save dialog: vio files
  QStringList filters; filters 
    << "FAUDES generator files (*.gen)"
    << "Any File (*.*)";
  fdiag->setFilters(filters);
  fdiag->setFileMode(QFileDialog::AnyFile);
  fdiag->setWindowTitle(QString("Save Document as FAUDES File"));
  fdiag->setAcceptMode(QFileDialog::AcceptSave);
  fdiag->setLabelText(QFileDialog::Accept,"Export");
  fdiag->setConfirmOverwrite(true);
  fdiag->setDefaultSuffix("gen"); // todo: have suffix

  // open dialog: run and save settings
  QString filename="";
  if(fdiag->exec()) {
    if(fdiag->selectedFiles().size()==1) 
      filename=fdiag->selectedFiles().at(0);
  }
  settings.setValue("stateFaudesFileDialog", fdiag->saveState());
  delete fdiag;

  // bail out on empty
  if(filename=="") return;

  // do export
  mVioWidget->ExportFaudesFile(filename);
}

// file menu slots: open recent
void VioWindow::OpenRecent() {
  
  // bail out if not triggered by action
  QAction *action = qobject_cast<QAction *>(sender());
  if(!action) return;

  // figure filename
  QString filename = action->data().toString();
  if(filename=="") return;

  // shall we have a new window?
  bool owin=true;
  if(!mVioWidget->Modified() && mCurrentFile=="") owin=false;

  // test all other windows for my new file 
  foreach(QWidget *widget, QApplication::topLevelWidgets()) {
    VioWindow *win = qobject_cast<VioWindow *>(widget);
    if(!win) continue;
    if(win==this) continue;
    if(win->CurrentFile()=="") continue;
    if(win->CurrentFile()!=filename) continue;
    int ret = QMessageBox::warning(this, 
      tr("vioDiag"),
      tr("This file is allready opened in another window. Do you want to open another copy?"),
	QMessageBox::Yes | QMessageBox::No);
    if(ret != QMessageBox::Yes) return;
  }
  
  // is this a revert?
  if(mCurrentFile==filename) {
    owin=false;
    int ret = QMessageBox::warning(this, 
      tr("vioDiag"),
      tr("Revert to file?"),
	QMessageBox::Yes | QMessageBox::No);
    if(ret != QMessageBox::Yes) return;
  }

  // save changes 
  if(!owin) 
  if(mVioWidget->Modified()) {
    int ret = QMessageBox::warning(this, 
      tr("vioDiag"),
      tr("Do you want to save the current document?"),
      QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, 
      QMessageBox::Save);
    if (ret == QMessageBox::Save) Save();  
    if (ret == QMessageBox::Cancel) return;
  }

  // optionally have a new window
  VioWindow* nwin = this;
  if(owin) {
    nwin = new VioWindow();
    QSettings settings("Faudes", "vioDiag");
    nwin->restoreGeometry(settings.value("geometry").toByteArray());
    nwin->move(nwin->pos()+QPoint((int) (30.0*qrand()/RAND_MAX),(int) (30.0*qrand()/RAND_MAX)));
    nwin->show();
  }

  // do load the file
  nwin->LoadFile(filename);

}

// file menu slots: about box
void VioWindow::About() {
  
  // use qt about box
  QMessageBox aboutbox;
  aboutbox.setText(QString(
    "<p><b>vioDiag - a  test application for libVioDES plug-ins</b></p>")); 
  aboutbox.setInformativeText(QString(
    "<p><b>Version:</b> %1 with %2 plug-ins</p> " 
    "<p><b>Configuration:</b> %3</p> " 
    "<p><b>VioPlugIns:</b> %4</p> " 
    "<p><b>Credits:</b> %5</p> " 
    "<p><b>(c) 2010 Thomas Moor</b></p> "
    ).arg(
      faudes::VersionString().c_str(),
      faudes::PluginsString().c_str(),
      VioStyle::ConfigName(),
      VioStyle::PluginsString(),
      faudes::ContributorsString().c_str()));
    aboutbox.setWindowTitle("About DESTool"),
    aboutbox.setIcon(QMessageBox::Information);
    aboutbox.addButton(QMessageBox::Ok);
    aboutbox.exec();
}

// initialise: have actions
void VioWindow::CreateActions() {

  // about
  mAboutAct = new QAction(tr("About vioDiag"), this);
  mAboutAct->setStatusTip(tr("Show About-Box"));
  connect(mAboutAct, SIGNAL(triggered()), this, SLOT(About()));

  // new by type
  QStringList vtypelist=VioTypeRegistry::Types(); 
  vtypelist.sort();
  for(int i=0; i < vtypelist.size(); i++) {
    QAction* action = new QAction(tr("test"), this);
    action->setText(vtypelist.at(i));
    action->setData(vtypelist.at(i));
    action->setStatusTip(tr("Create a new VioDES object"));
    connect(action, SIGNAL(triggered()), this, SLOT(New()));
    mNewActions.append(action);
  }

  // import by type
  const QStringList& ftypelist=VioTypeRegistry::Types(); 
  for(int i=0; i < ftypelist.size(); i++) {
    QAction* action = new QAction(tr("test"), this);
    action->setText(ftypelist.at(i));
    action->setData(ftypelist.at(i));
    action->setStatusTip(tr("Import from FAUDES file"));
    connect(action, SIGNAL(triggered()), this, SLOT(ImportFaudes()));
    mImportActions.append(action);
  }

  // open
  mOpenAct = new QAction(tr("&Open..."), this);
  mOpenAct->setShortcut(tr("Ctrl+O"));
  mOpenAct->setStatusTip(tr("Open an existing file"));
  connect(mOpenAct, SIGNAL(triggered()), this, SLOT(Open()));

  // save
  mSaveAct = new QAction(tr("&Save"), this);
  mSaveAct->setShortcut(tr("Ctrl+S"));
  mSaveAct->setStatusTip(tr("Save the document to disk"));
  connect(mSaveAct, SIGNAL(triggered()), this, SLOT(Save()));

  // save as
  mSaveAsAct = new QAction(tr("Save &As..."), this);
  mSaveAsAct->setStatusTip(tr("Save the document under a new name"));
  connect(mSaveAsAct, SIGNAL(triggered()), this, SLOT(SaveAs()));

  // clear recent
  mClearRecentAct = new QAction(tr("Clear Recent"), this);
  mClearRecentAct->setStatusTip(tr("Clear list of recently visited filed"));
  connect(mClearRecentAct, SIGNAL(triggered()), this, SLOT(ClearRecentFileActions()));


  // faudes export
  mExportAct = new QAction(tr("Export ..."), this);
  mExportAct->setStatusTip(tr("Export in FAUDES file format"));
  connect(mExportAct, SIGNAL(triggered()), this, SLOT(ExportFaudes()));

  // open recenet
  for (int i = 0; i < MaxRecentFiles; ++i) {
    mRecentFileActs[i] = new QAction(this);
    mRecentFileActs[i]->setVisible(false);
    connect(mRecentFileActs[i], SIGNAL(triggered()),
	    this, SLOT(OpenRecent()));
  }

  // exit
  mExitAct = new QAction(tr("&Quit"), this);
  mExitAct->setShortcut(tr("Ctrl+Q"));
  mExitAct->setStatusTip(tr("Quit"));
  connect(mExitAct, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));

  // undo (should use provided action)
  mUndoAct = new QAction(tr("Undo"), this);
  mUndoAct->setShortcut(tr("Ctrl+Z"));
  mUndoAct->setStatusTip(tr("Undo the last operation"));
  mUndoAct->setEnabled(true);

  // redo (should use provided action)
  mRedoAct = new QAction(tr("Redo"), this);
  mRedoAct->setShortcut(tr("Ctrl+Shift+Z"));
  mRedoAct->setStatusTip(tr("Redo the last operation"));
  mRedoAct->setEnabled(true);

  // cut (should use provided action)
  mCutAct = new QAction(tr("Cut"), this);
  mCutAct->setShortcut(tr("Ctrl+X"));
  mCutAct->setStatusTip(tr("Cut the current selection's contents to the clipboard"));
  mCutAct->setEnabled(false);

  // copy  (should use provided action)
  mCopyAct = new QAction(tr("Copy"), this);
  mCopyAct->setShortcut(tr("Ctrl+C"));
  mCopyAct->setStatusTip(tr("Copy the current selection's contents to the clipboard"));
  mCopyAct->setEnabled(false);

  // paste  (should use provided action)
  mPasteAct = new QAction(tr("Paste"), this);
  mPasteAct->setShortcut(tr("Ctrl+V"));
  mPasteAct->setStatusTip(tr("Paste the clipboard's contents into the current selection"));
  mPasteAct->setEnabled(false);

  // about
  mConsoleAct = new QAction(tr("Show Console"), this);
  mConsoleAct->setStatusTip(tr("Opens a console window error log"));
  connect(mConsoleAct, SIGNAL(triggered()), this, SLOT(ActivateConsole()));
}

// initialise: menues
void VioWindow::CreateMenus(void) {

  // new submenu
  mNewMenu = new QMenu(this);
  mNewMenu->setTitle("&New");
  for(int i=0; i < mNewActions.size(); i++) {
    mNewMenu->addAction(mNewActions.at(i));
  } 

  // import submenu
  mImportMenu = new QMenu(this);
  mImportMenu->setTitle("Import");
  for(int i=0; i < mImportActions.size(); i++) {
    mImportMenu->addAction(mImportActions.at(i));
  } 

  // file menu
  mFileMenu = menuBar()->addMenu(tr("&File"));
  mFileMenu->addAction(mAboutAct);
  mFileMenu->addSeparator();
  mFileMenu->addMenu(mNewMenu);
  mFileMenu->addAction(mOpenAct);
  mFileMenu->addAction(mSaveAct);
  mFileMenu->addAction(mSaveAsAct);
  mFileMenu->addSeparator();
  mFileMenu->addMenu(mImportMenu);
  mFileMenu->addAction(mExportAct);
  mSeparatorRecentAct = mFileMenu->addSeparator();
  for(int i = 0; i < MaxRecentFiles; ++i)
    mFileMenu->addAction(mRecentFileActs[i]);
  mFileMenu->addAction(mClearRecentAct);
#ifndef Q_WS_MAC  
  mFileMenu->addSeparator();
#endif
  mFileMenu->addAction(mExitAct);
  UpdateRecentFileActions();

  // edit menu
  mEditMenu = menuBar()->addMenu(tr("&Edit"));
  mEditMenu->setEnabled(false);

  // view menu
  mViewMenu = menuBar()->addMenu(tr("&View"));
  mViewMenu->setEnabled(false);

  // windows menu
  mWindowsMenu = menuBar()->addMenu(tr("&Windows"));
  mWindowsMenu->setEnabled(true);
  connect(mWindowsMenu, SIGNAL(aboutToShow()), this, SLOT(InstallWindowsMenu()));
}

// update view menu
void VioWindow::InstallViewMenu(void) {
  // bail out (qt mouse over menu inconvenience)
  if(mViewMenu->isVisible()) return;
  std::cout << "VioWindow::InstallViewMenu()\n";
  // clear
  mViewMenu->clear();
  mViewMenu->setEnabled(false);
  // get action from current widget
  if(mVioWidget)
    foreach(QAction* vaction,mVioWidget->View()->ViewActions()) 
      mViewMenu->addAction(vaction);
  // ennable (relevant if called programmatically)
  if(mVioWidget)
     mViewMenu->setEnabled(mVioWidget->View()->ViewActions().size()!=0);
}

// update edit menu
void VioWindow::InstallEditMenu(void) {
  // bail out (qt mouse over menu inconvenience)
  if(mEditMenu->isVisible()) return;
  std::cout << "VioWindow::InstallEditMenu()\n";
  // clear
  mEditMenu->clear();
  // std actions
  mEditMenu->addAction(mUndoAct);
  mEditMenu->addAction(mRedoAct);
  mEditMenu->addSeparator();
  mEditMenu->addAction(mCutAct);
  mEditMenu->addAction(mCopyAct);
  mEditMenu->addAction(mPasteAct);
  // connect std
  disconnect(mCutAct,SIGNAL(triggered()));
  mCutAct->setEnabled(false);
  if(mVioWidget) connect(mCutAct, SIGNAL(triggered()), mVioWidget, SLOT(Cut()));
  if(mVioWidget) mCutAct->setEnabled(mVioWidget->CutAction()->isEnabled());
  disconnect(mCopyAct,SIGNAL(triggered()));
  mCopyAct->setEnabled(false);
  if(mVioWidget) connect(mCopyAct, SIGNAL(triggered()), mVioWidget, SLOT(Copy()));
  if(mVioWidget) mCopyAct->setEnabled(mVioWidget->CopyAction()->isEnabled());
  disconnect(mPasteAct,SIGNAL(triggered()));
  mPasteAct->setEnabled(false);
  if(mVioWidget) connect(mPasteAct, SIGNAL(triggered()), mVioWidget, SLOT(Paste()));
  if(mVioWidget) mPasteAct->setEnabled(mVioWidget->PasteAction()->isEnabled());
  disconnect(mUndoAct,SIGNAL(triggered()));
  mUndoAct->setEnabled(true);
  if(mVioWidget) connect(mUndoAct, SIGNAL(triggered()), mVioWidget, SLOT(Undo()));
  disconnect(mRedoAct,SIGNAL(triggered()));
  mRedoAct->setEnabled(true);
  if(mVioWidget) connect(mRedoAct, SIGNAL(triggered()), mVioWidget, SLOT(Redo()));
  // get action from current widget
  if(mVioWidget) 
  if(mVioWidget->View()->EditActions().size()>0) {
    mEditMenu->addSeparator();
    foreach(QAction* vaction,mVioWidget->View()->EditActions()) 
      mEditMenu->addAction(vaction);
  }
  // be enabled
  mEditMenu->setEnabled(true);
}


// Install windows menu
void VioWindow::InstallWindowsMenu(void) {
  // bail out (qt mouse over menu inconvenience)
  if(mWindowsMenu->isVisible()) return;
  std::cout << "VioWindow::InstallWindowsMenu()\n";
  // clear
  mWindowsMenu->clear();
  // todo: delete actions
  // fill
  foreach(QWidget *widget, QApplication::topLevelWidgets()) {
    VioWindow *win = qobject_cast<VioWindow *>(widget);
    if(!win) continue;
    // create action
    QAction* wact = new QAction(win->ShortTitle(), this);
    connect(wact, SIGNAL(triggered()), win, SLOT(ActivateWindow()));
    mWindowsMenu->addAction(wact);
  }
  // separate
  mWindowsMenu->addSeparator();
  // console
  mWindowsMenu->addAction(mConsoleAct);
  // all
  QAction* wact = new QAction(tr("Bring all to front"), this);
  connect(wact, SIGNAL(triggered()), this, SLOT(ActivateAllWindows()));
  mWindowsMenu->addAction(wact);
}

// doit: load a file
void VioWindow::LoadFile(const QString &fileName) {

  // bail out on non-existent
  if(fileName=="") return;

  // may take time
  QApplication::setOverrideCursor(Qt::WaitCursor);

  // catch faudes errors
  QString err="";
  VioWidget* viowid=0;
  try { 
    viowid = VioTypeRegistry::FromFile(fileName);
  } catch (faudes::Exception& fexcep) {
    if(viowid) delete viowid;
    viowid=0;
    err=QString("Error: ")+VioStyle::QStrFromStr(fexcep.What());
  }

  // fix cursor
  QApplication::restoreOverrideCursor();

  // report error
  if(err!="") {
    QMessageBox::warning(this, 
      tr("vioDiag"),
      tr("<p>Cannot read file %1</p>"
         "<p>%2</p>"
	 ).arg(StrippedName(fileName),err));
    return;
  }
  
  // set the widget
  Widget(viowid);

  // read ok
  CurrentFile(fileName);
  statusBar()->showMessage(tr("File loaded"), 2000);
}

// doit: save file
void VioWindow::SaveFile(const QString &fileName) {

  // may take time
  QApplication::setOverrideCursor(Qt::WaitCursor);

  // catch faudes errors
  QString err="";
  try { 
    mVioWidget->Write(fileName);
  } catch (faudes::Exception& fexcep) {
    err=QString("Error: ")+VioStyle::QStrFromStr(fexcep.What());
  }

  // fix cursor
  QApplication::restoreOverrideCursor();

  // report error
  if(err!="") {
    QMessageBox::warning(this, 
      tr("vioDiag"),
      tr("<p>Cannot write file %1</p>"
         "<p>%2</p>"
	 ).arg(fileName,err));
    return;
  }
  
  // record filename
  CurrentFile(fileName);
  statusBar()->showMessage(tr("File saved"), 2000);
}


// rerort faudes error
void VioWindow::FaudesError(const QString& faudeserror) {

  // fix cursor
  QApplication::restoreOverrideCursor();

  //say hello
  QMessageBox::warning(this, 
    tr("vioDiag"),
    tr("<p>vioDiag sensed the following libFAUDES error:</p>"
       "<p>%1</p>"
    ).arg(faudeserror));
  return;
}

// report faudes status
void VioWindow::FaudesStatus(const QString& faudeserror) {
  statusBar()->showMessage(faudeserror, 5000);
  return;
}

// doit: load viostyle config
void VioWindow::LoadConfig(const QString &fileName) {
  // todo: fix viostyle to be less static
  (void) fileName;
}

// doit: fix current file name
void VioWindow::CurrentFile(const QString &fileName) {

  // record
  mCurrentFile = QFileInfo(fileName).canonicalFilePath();
  if(mCurrentFile=="") mCurrentFile=fileName;

  // default window title
  if (mCurrentFile.isEmpty()) {
    setWindowTitle(tr("vioDiag: Untitled[*]"));
    return;
  }

  // file name window title
  setWindowTitle(tr("vioDiag: %1[*]").arg(StrippedName(mCurrentFile)));

  // fix recent files incl settings
  QSettings settings("Faudes", "vioDiag");
  QStringList files = settings.value("recentFileList").toStringList();
  files.removeAll(fileName);
  files.prepend(fileName);
  while (files.size() > MaxRecentFiles)
    files.removeLast();
  settings.setValue("recentFileList", files);

  // update open recent in all open windows
  foreach(QWidget *widget, QApplication::topLevelWidgets()) {
    VioWindow *win = qobject_cast<VioWindow *>(widget);
    if(win) win->UpdateRecentFileActions();
  }
}

// helper
QString VioWindow::CurrentFile(void) {
  return mCurrentFile;
}

// doit: fix open recent actions
void VioWindow::UpdateRecentFileActions() {

  // get from settings
  QSettings settings("Faudes", "vioDiag");
  QStringList files = settings.value("recentFileList").toStringList();

  // iterate list
  int i=0;  
  for(; i < files.size(); ++i) {
    if(i>(int)MaxRecentFiles) break;
    QString text = tr("&%1 %2").arg(i + 1).arg(StrippedName(files[i]));
    mRecentFileActs[i]->setText(text);
    mRecentFileActs[i]->setData(files[i]);
    mRecentFileActs[i]->setVisible(true);
  }
  for(; i < MaxRecentFiles; ++i)
    mRecentFileActs[i]->setVisible(false);

  // show/hide seperator
  mSeparatorRecentAct->setVisible(files.size() > 0);
  mClearRecentAct->setVisible(files.size() > 0);
}

// doit: fix clear recent actions
void VioWindow::ClearRecentFileActions() {

  // claer in  settings
  QSettings settings("Faudes", "vioDiag");
  QStringList files;
  settings.setValue("recentFileList", files);

  // update open recent in all open windows
  foreach(QWidget *widget, QApplication::topLevelWidgets()) {
    VioWindow *win = qobject_cast<VioWindow *>(widget);
    if(win) win->UpdateRecentFileActions();
  }
}

// doit: close the window (reimplemet qt window)
void VioWindow::closeEvent ( QCloseEvent * event ){

  // save changes
  if(mVioWidget->Modified()) {
    int ret = QMessageBox::warning(this, 
      tr("vioDiag"),
      tr("Do you want to save the current document?"),
      QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, 
      QMessageBox::Save);
    if (ret == QMessageBox::Save) Save();  
    if (ret == QMessageBox::Cancel) { event->ignore() ; return; };
  }

  // save window geometry
  QSettings settings("Faudes", "vioDiag");
  settings.setValue("geometry", saveGeometry());

  // call base
  QMainWindow::closeEvent(event);
}

// helper
QString VioWindow::StrippedName(const QString &fullFileName) {
  return QFileInfo(fullFileName).fileName();
}

// helper
QString VioWindow::ShortTitle() {
  QString filename = QFileInfo(mCurrentFile).fileName();
  QString res="Untitled" ;
  if(filename!="") res = filename;
  return res;
}

// helper
void VioWindow::ActivateWindow(void) {
  activateWindow();
  raise();
}

// helper
void VioWindow::ActivateConsole(void) {
  if(!spConsole) return;
  spConsole->show();
  spConsole->activateWindow();
  spConsole->raise();
}

// helper
void VioWindow::ActivateAllWindows(void) {
  foreach(QWidget *widget, QApplication::topLevelWidgets()) {
    VioWindow *win = qobject_cast<VioWindow *>(widget);
    if(!win) continue;
    if(win==this) continue;
    win->activateWindow();
    win->raise();
  }
  ActivateWindow();
}

/*
************************************************
************************************************
  
Implementation: main()

************************************************
************************************************
*/


int main(int argc, char *argv[]) {

  // let Qt see commandline
  QApplication app(argc, argv);
 

#ifdef Q_OS_WIN32  
  // redirect libfaudes error to file (will be rdirected again for console widget)
  QTemporaryFile violog(QDir::tempPath()+ QDir::separator()+ "viodiag_log_XXXXXX");
  violog.setAutoRemove(false);
  violog.open(); 
  QString viologname = violog.fileName(); 
  violog.close();
  faudes::ConsoleOut::G()->ToFile(VioStyle::StrFromQStr(viologname));
  FD_WARN("viodiag logfile");
#endif

#ifdef Q_OS_WIN32  
#ifdef FAUDES_IODEVICE_SIMPLENET
  // if we have a net device on windows, initialise winsocks
  FD_DS("Main(): init winsock");
  WSADATA wsaData; 
  if(WSAStartup(MAKEWORD(2,2), &wsaData)!=0) {
    std::cout << "cannot open netdevice (network error)" << std::endl;  
    return 1;
  }
#endif
#endif



  // add my searchpath for plugins (obsolete?)
  QApplication::addLibraryPath(QCoreApplication::applicationDirPath() + QDir::separator()+ "plugins"); 

  // default args 
  QString  cfgname="";
  QString vioname="";
  
  // lazy commandline ...
  bool ok=false;
  // start with new viodes object
  if(argc ==1) { ok=true; } 
  // start with open spezified file
  if(argc ==2) { vioname=argv[1]; ok=true; }
  // start with nonstandard config
  if(argc ==3)
    if(app.arguments().at(1)=="-c") {
      cfgname=argv[2];
      ok=true;
    }
  // start with nonstandard config and file
  if(argc ==4)
    if(app.arguments().at(1)=="-c") {
      cfgname=argv[2];
      vioname=argv[3];
      ok=true;
    }
  // report error
  if(!ok) {
    std::cout << "usage: viodiag [-c config.txt] [generator.vio]" << std::endl;  
    return 1;
  }

  // configure: defaults
  VioStyle::Initialise();


  // configure: try read specified file
  if(cfgname!="") {
    try { 
      VioStyle::ReadFile(cfgname); 
    } catch (faudes::Exception& fexcep) {
      QString err=QString("Error: ")+VioStyle::QStrFromStr(fexcep.What());
      QMessageBox::warning(0,"vioDiag",
	QString("<p>Cannot read configuration file %1</p><p>%2</p>").arg(cfgname,err));
      exit(1);
    }
  }

  // configure: try read default file
  if(cfgname=="") {
    try { 
      cfgname =  QCoreApplication::applicationDirPath() + "/vioconfig.txt";
      VioStyle::ReadFile(cfgname);
    } catch (faudes::Exception& fexcep) {
    }
  }

  // disclaimer
  FD_WARN("viodiag: show disclaimer");
  {
    QSettings settings("Faudes", "vioDiag");
    QString never = settings.value("disclaimer3").toString();
    if(never!="dontShowDisclaimer") {
    QMessageBox disclaimer;
      disclaimer.setText(QString(
      "<p><b>This software is for testing libVioDES plug-ins.</b></p>"
      "<p><b>Do you want to proceed?</b></p>"));
      disclaimer.setInformativeText(QString(
       "<p>If you are not developing a libVioDES plug-in, it is very unlikely "
       "that you want to use vioDiag. </p> "
        "<p> %1 </p> ").arg(VioStyle::LicenseText())); 
      disclaimer.setWindowTitle("vioDiag");
      disclaimer.setIcon(QMessageBox::Information);
      QPushButton* disno = disclaimer.addButton("No", QMessageBox::NoRole);
      QPushButton* disnever = disclaimer.addButton("Yes, dont ask again", QMessageBox::YesRole);
      QPushButton* disyes = disclaimer.addButton("Yes", QMessageBox::YesRole);
      disclaimer.exec();
      if(disclaimer.clickedButton() == disno) exit(1);
      if(disclaimer.clickedButton() == disnever) settings.setValue("disclaimer3", "dontShowDisclaimer");
      (void) disyes;
    }
  }

  // initialise faudes registry (todo: path)
  FD_WARN("viodiag: load libFAUDES regsitry");
  QString rtifile = QCoreApplication::applicationDirPath() + QDir::separator()+ "libfaudes.rti"; 
  faudes::LoadRegistry(VioStyle::StrFromQStr(rtifile));

 
  // check for my plugins (file errors)
  FD_WARN("viodiag: load vio plug-ins");
  try { 
    VioTypeRegistry::Initialise();
  } catch (faudes::Exception& fexcep) {
    QString err=QString("Error: ")+VioStyle::QStrFromStr(fexcep.What());
    QMessageBox::warning(0,"vioDiag",
    QString("<p>Error while loading plugins.</p><p>%1</p>").arg(err));
    exit(1);
  }

  // set up function registry
  FD_WARN("viodiag: initialize vio function registry");
  try { 
    VioFunctionRegistry::Initialise();
  } catch (faudes::Exception& fexcep) {
    QString err=QString("Error: ")+VioStyle::QStrFromStr(fexcep.What());
    QMessageBox::warning(0,"vioDiag",
    QString("<p>Error while initialising functions.</p><p>%1</p>").arg(err));
    exit(1);
  }

  // load file and go
  FD_WARN("viodiag: open main window");
  VioWindow *vioWin = new VioWindow;
  QSettings settings("Faudes", "vioDiag");
  vioWin->restoreGeometry(settings.value("geometry").toByteArray());
  if(vioname!="") vioWin->LoadFile(vioname);
  vioWin->show();
  return app.exec();

}
