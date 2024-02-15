/* viogen.cpp  - example for editable viogenerator */

/* 
   This demo has been derived from a trolltech qt example; since
   we hold a commercercial license i understand that using this
   file does not turn our project into a gpl project; we will 
   rewrite this demo on a later stage anyway. tmoor 2006 

*/


/****************************************************************************
**
** Copyright (C) 2006-2006 Trolltech ASA. All rights reserved.
**
** This file is part of the example classes of the Qt Toolkit.
**
** Licensees holding a valid Qt License Agreement may use this file in
** accordance with the rights, responsibilities and obligations
** contained therein.  Please consult your licensing agreement or
** contact sales@trolltech.com if any conditions of this licensing
** agreement are not clear to you.
**
** Further information about Qt licensing is available at:
** http://www.trolltech.com/products/qt/licensing.html or by
** contacting info@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QApplication>
#include <QtGui>

#include "viogen.h"
#include "libvio.h"

VioWindow::VioWindow()
{
    setAttribute(Qt::WA_DeleteOnClose);

    vioGenerator  = new VioGenerator(this);

    setCentralWidget(vioGenerator);
    setCurrentFile("");
    connect(vioGenerator, SIGNAL(Changed(bool)),
             this, SLOT(setWindowModified(bool))); 

    createActions();
    createMenus();
    (void)statusBar();

    setWindowTitle(tr("vioGen[*]"));
    //resize(500, 500);

}

void VioWindow::newFile()
{
   // single window
   /*
   if(vioGenerator->hasChanged()) {
      int ret = QMessageBox::warning(this, tr("vioGen"),
		  tr("Do you want to save the current document?"),
                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
        if (ret == QMessageBox::Save) save();  
        if (ret == QMessageBox::Cancel) return;

    }
    vioGenerator->Clear();
    setCurrentFile("");
    */

    // multi window alternative:
    VioWindow *other = new VioWindow;
    QSettings settings("Faudes", "vioGen");
    other->restoreGeometry(settings.value("geometry").toByteArray());
    other->move(other->pos()+QPoint((int) (30.0*qrand()/RAND_MAX),(int) (30.0*qrand()/RAND_MAX)));
    other->show();
}

void VioWindow::open()
{
    if(vioGenerator->hasChanged()) {
        int ret = QMessageBox::warning(this, tr("vioGen"),
	       tr("Do you want to save the current document \"%1\"?").arg(strippedName(curFile)),
               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
        if (ret == QMessageBox::Save) save();  
        if (ret == QMessageBox::Cancel) return;
    }
    QString fileName=QFileDialog::getOpenFileName(this,"Open","./","Vio-Files (*.vio)");
    if (fileName.isEmpty())
        return;
    loadFile(fileName);
}

void VioWindow::save()
{
    if (curFile.isEmpty())
        saveAs();
    else
        saveFile(curFile);
}

void VioWindow::saveAs()
{
    QFileDialog filedialog;
    filedialog.setDefaultSuffix(".vio");
    QString fileName=filedialog.getSaveFileName(this,"Save","./","Vio-Files (*.vio)");
    if (fileName.isEmpty())
        return;
    saveFile(fileName);
}

void VioWindow::gexport()
{
    QFileDialog filedialog;
    filedialog.setDefaultSuffix(".gen");
    QString fileName=filedialog.getSaveFileName(this,"Export","./","Gen-Files (*.gen)");
    if (fileName.isEmpty())
        return;
    gexportFile(fileName);
}

void VioWindow::eexport()
{
    QFileDialog filedialog;
    filedialog.setDefaultSuffix(".svg");
    QString fileName=filedialog.getSaveFileName(this,"Export","./","Svg-Files (*.svg)");
    if (fileName.isEmpty())
        return;
    eexportFile(fileName);
}

void VioWindow::jexport()
{
    QFileDialog filedialog;
    filedialog.setDefaultSuffix(".jpg");
    QString fileName=filedialog.getSaveFileName(this,"Export","./","Jpg-Files (*.jpg)");
    if (fileName.isEmpty())
        return;
    jexportFile(fileName);
}

void VioWindow::gimport()
{
    if(vioGenerator->hasChanged()) {
        int ret = QMessageBox::warning(this, tr("vioGen"),
	       tr("Do you want to save the current document \"%1\"?").arg(strippedName(curFile)),
               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
        if (ret == QMessageBox::Save) save();  
        if (ret == QMessageBox::Cancel) return;
    }

    QString fileName=QFileDialog::getOpenFileName(this,"Import","./","Gen-Files (*.gen)");
    if (fileName.isEmpty())
        return;
    gimportFile(fileName);
}

void VioWindow::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if(!action) return;

    if(vioGenerator->hasChanged()) {
        int ret = QMessageBox::warning(this, tr("vioGen"),
	       tr("Do you want to save the current document \"%1\"?").arg(strippedName(curFile)),
               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
        if (ret == QMessageBox::Save) save();  
        if (ret == QMessageBox::Cancel) return;
    }

    loadFile(action->data().toString());
}

void VioWindow::about()
{
  QMessageBox::about(this, tr("About vioGen"),
    tr( "<b>vioGen</b> is a graphical editor for faudes generators"  
	"<p>Version: %1 %2 </p> " 
	"<p>Configuration: %3</p> " 
	"<p>Credits: %4</p> " 
        "<p>(c) 2006, 2008 Moor, Schmidt, Perk</p> "
     ).arg(
        faudes::FDVersionString().c_str(),
        faudes::FDPluginsString().c_str(),
        VioStyle::CONFIGURATION,
        faudes::FDContributorsString().c_str()));
}

void VioWindow::createActions() 
{
    newAct = new QAction(tr("&New"), this);
    newAct->setShortcut(tr("Ctrl+N"));
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));

    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcut(tr("Ctrl+O"));
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    saveAct = new QAction(tr("&Save"), this);
    saveAct->setShortcut(tr("Ctrl+S"));
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    saveAsAct = new QAction(tr("Save &As..."), this);
    saveAsAct->setStatusTip(tr("Save the document under a new name"));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    gimportAct = new QAction(tr("&Import (.gen)"), this);
    gimportAct->setStatusTip(tr("Import FAUDES file"));
    connect(gimportAct, SIGNAL(triggered()), this, SLOT(gimport()));

    gexportAct = new QAction(tr("&Export (.gen)"), this);
    gexportAct->setStatusTip(tr("Export FAUDES file"));
    connect(gexportAct, SIGNAL(triggered()), this, SLOT(gexport()));

    eexportAct = new QAction(tr("&Export (.svg)"), this);
    eexportAct->setStatusTip(tr("Export vector graphics file"));
    connect(eexportAct, SIGNAL(triggered()), this, SLOT(eexport()));

    jexportAct = new QAction(tr("&Export (.jpg)"), this);
    jexportAct->setStatusTip(tr("Export image as jpeg"));
    connect(jexportAct, SIGNAL(triggered()), this, SLOT(jexport()));

    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActs[i] = new QAction(this);
        recentFileActs[i]->setVisible(false);
        connect(recentFileActs[i], SIGNAL(triggered()),
                this, SLOT(openRecentFile()));
    }

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr(""));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    undoAct = new QAction(tr("&Undo"), this);
    undoAct->setShortcut(tr("Ctrl+Z"));
    undoAct->setStatusTip(tr("Undo the last operation"));
    connect(undoAct, SIGNAL(triggered()), vioGenerator, SLOT(Undo()));

    redoAct = new QAction(tr("&Redo"), this);
    redoAct->setShortcut(tr("Ctrl+Y"));
    redoAct->setStatusTip(tr("Redo the last operation"));
    connect(redoAct, SIGNAL(triggered()), vioGenerator, SLOT(Redo()));

    cutAct = new QAction(tr("Cu&t"), this);
    cutAct->setShortcut(tr("Ctrl+X"));
    cutAct->setStatusTip(tr("Cut the current selection's contents to the clipboard"));
    connect(cutAct, SIGNAL(triggered()), vioGenerator, SLOT(Cut()));

    copyAct = new QAction(tr("&Copy"), this);
    copyAct->setShortcut(tr("Ctrl+C"));
    copyAct->setStatusTip(tr("Copy the current selection's contents to the clipboard"));
    connect(copyAct, SIGNAL(triggered()), vioGenerator, SLOT(Copy()));

    pasteAct = new QAction(tr("&Paste"), this);
    pasteAct->setShortcut(tr("Ctrl+V"));
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current selection"));
    connect(pasteAct, SIGNAL(triggered()), vioGenerator, SLOT(Paste()));

    graphviewAct = new QAction(tr("Graph/List"), this);
    graphviewAct->setShortcut(tr("Ctrl+#"));
    graphviewAct->setStatusTip(tr("show generator as graph or list"));
    connect(graphviewAct, SIGNAL(triggered(bool)), vioGenerator, SLOT(ToggleGraphList(void)));

    propviewAct = new QAction(tr("Properties"), this);
    propviewAct->setStatusTip(tr("show item properties"));
    connect(propviewAct, SIGNAL(triggered(bool)), vioGenerator, SLOT(ToggleProp(void)));

    zoominAct = new QAction(tr("Zoom In "), this);
    zoominAct->setShortcut(tr("Ctrl++"));
    zoominAct->setStatusTip(tr("enlarge generator image"));
    connect(zoominAct, SIGNAL(triggered()), vioGenerator, SLOT(ZoomIn()));

    zoomoutAct = new QAction(tr("Zoom Out "), this);
    zoomoutAct->setShortcut(tr("Ctrl+-"));
    zoomoutAct->setStatusTip(tr("shrink generator image"));
    connect(zoomoutAct, SIGNAL(triggered()), vioGenerator, SLOT(ZoomOut()));

    zoomfitAct = new QAction(tr("Fit to Window"), this);
    zoomfitAct->setShortcut(tr("Ctrl+*"));
    zoomfitAct->setStatusTip(tr("fit selection to view/window"));
    connect(zoomfitAct, SIGNAL(triggered()), vioGenerator, SLOT(ZoomFit()));


}

void VioWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    separatorAct = fileMenu->addSeparator();
    fileMenu->addAction(gimportAct);
    fileMenu->addAction(gexportAct);
    fileMenu->addAction(eexportAct);
    fileMenu->addAction(jexportAct);
    separatorAct = fileMenu->addSeparator();
    for (int i = 0; i < MaxRecentFiles; ++i)
        fileMenu->addAction(recentFileActs[i]);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);
    updateRecentFileActions();

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(undoAct);
    editMenu->addAction(redoAct);
    editMenu->addSeparator();
    editMenu->addAction(cutAct);
    editMenu->addAction(copyAct);
    editMenu->addAction(pasteAct);
    editMenu->addSeparator();
    editMenu->addAction(vioGenerator->mInsStateAction);
    editMenu->addAction(vioGenerator->mInsTransAction);
    editMenu->addSeparator();
    editMenu->addAction(vioGenerator->mSelectAllStatesAction);
    editMenu->addAction(vioGenerator->mSelectAllEventsAction);
    editMenu->addAction(vioGenerator->mSelectAllTransAction);
    editMenu->addSeparator();
    editMenu->addAction(vioGenerator->mUpdateAttributesAction);


    viewMenu = menuBar()->addMenu(tr("V&iew"));
    viewMenu->addAction(graphviewAct);
    viewMenu->addAction(propviewAct);
    viewMenu->addAction(vioGenerator->mClearGraphAction);
    viewMenu->addSeparator();
    viewMenu->addAction(zoominAct);
    viewMenu->addAction(zoomoutAct);
    viewMenu->addAction(zoomfitAct);

    //menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
}

void VioWindow::loadFile(const QString &fileName)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    int err=vioGenerator->VioRead(fileName);
    QApplication::restoreOverrideCursor();
    if(err!=0) {
        QMessageBox::warning(this, tr("vioGen"),
                             tr("Cannot read file %1")
                             .arg(fileName));
        return;
    }
    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File loaded"), 2000);
}

void VioWindow::saveFile(const QString &fileName)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    int err=vioGenerator->VioWrite(fileName);
    QApplication::restoreOverrideCursor();
    if(err!=0) {
        QMessageBox::warning(this, tr("vioGen"),
                             tr("Cannot write file %1")
                             .arg(fileName));
        return;
    }
    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File saved"), 2000);
}

void VioWindow::loadConfig(const QString &fileName)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    int err=0; // vioGenerator->Config(fileName); todo
    QApplication::restoreOverrideCursor();
    if(err!=0) {
        QMessageBox::warning(this, tr("vioGen"),
                             tr("Cannot read config file %1")
                             .arg(fileName));
        return;
    }
    statusBar()->showMessage(tr("Config complete"), 2000);
}

void VioWindow::gexportFile(const QString &fileName)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    int err=vioGenerator->GenWrite(fileName);
    QApplication::restoreOverrideCursor();
    if(err!=0) {
        QMessageBox::warning(this, tr("vioGen"),
                             tr("Cannot write file %1")
                             .arg(fileName));
        return;
    }
    statusBar()->showMessage(tr("Generator exported"), 2000);
}

void VioWindow::eexportFile(const QString &fileName)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    int err=vioGenerator->SvgWrite(fileName); 
    QApplication::restoreOverrideCursor();
    if(err!=0) {
        QMessageBox::warning(this, tr("vioGen"),
                             tr("Cannot write file %1")
                             .arg(fileName));
        return;
    }
    statusBar()->showMessage(tr("Vectorgraphic generated"), 2000);
}

void VioWindow::jexportFile(const QString &fileName)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    int err=vioGenerator->JpgWrite(fileName); 
    QApplication::restoreOverrideCursor();
    if(err!=0) {
        QMessageBox::warning(this, tr("vioGen"),
                             tr("Cannot write file %1")
                             .arg(fileName));
        return;
    }
    statusBar()->showMessage(tr("Image generated"), 2000);
}

void VioWindow::gimportFile(const QString &fileName)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    int err=vioGenerator->GenRead(fileName);
    QApplication::restoreOverrideCursor();
    if(err!=0) {
        QMessageBox::warning(this, tr("vioGen"),
                             tr("Cannot read file %1")
                             .arg(fileName));
        return;
    }
    setCurrentFile(""); 
    statusBar()->showMessage(tr("Generator imported"), 2000);
}

void VioWindow::setCurrentFile(const QString &fileName)
{
    curFile = fileName;
    if (curFile.isEmpty())
        setWindowTitle(tr("vioGen[*]"));
    else {
        setWindowTitle(tr("%1[*] - %2").arg(strippedName(curFile))
                                    .arg(tr("vioGen")));

      QSettings settings("Faudes", "vioGen");
      QStringList files = settings.value("recentFileList").toStringList();
      files.removeAll(fileName);
      files.prepend(fileName);
      while (files.size() > MaxRecentFiles)
          files.removeLast();

      settings.setValue("recentFileList", files);

      foreach (QWidget *widget, QApplication::topLevelWidgets()) {
          VioWindow *mainWin = qobject_cast<VioWindow *>(widget);
          if (mainWin)
              mainWin->updateRecentFileActions();
      }
    }
}

void VioWindow::updateRecentFileActions()
{

    QSettings settings("Faudes", "vioGen");
    QStringList files = settings.value("recentFileList").toStringList();

    int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);

    for (int i = 0; i < numRecentFiles; ++i) {
        QString text = tr("&%1 %2").arg(i + 1).arg(strippedName(files[i]));
        recentFileActs[i]->setText(text);
        recentFileActs[i]->setData(files[i]);
        recentFileActs[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
        recentFileActs[j]->setVisible(false);

    separatorAct->setVisible(numRecentFiles > 0);
}

void VioWindow::closeEvent ( QCloseEvent * event ){
  if(vioGenerator->hasChanged()) {
      int ret = QMessageBox::warning(this, tr("vioGen"),
	       tr("Do you want to save the current document \"%1\"?").arg(strippedName(curFile)),
                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
        if (ret == QMessageBox::Save) save();  
        if (ret == QMessageBox::Cancel) { event->ignore(); return;}
  }
  QSettings settings("Faudes", "vioGen");
  settings.setValue("geometry", saveGeometry());
  QMainWindow::closeEvent(event);
}

QString VioWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}



int main(int argc, char *argv[])
{
    // let Qt see commandline
    QApplication app(argc, argv);
 
    // add my searchpath for plugins (obsolete?)
    QApplication::addLibraryPath(QCoreApplication::applicationDirPath() + QDir::separator()+ "plugins"); 

    // default args 
    QString  cfgname =  QCoreApplication::applicationDirPath() + "/vioconfig.txt";
    QString vioname="";
  
    // lazy commandline
    bool ok=false;
    if(argc ==1) {
      ok=true;    
    }
    if(argc ==2) {
      vioname=argv[1];
      ok=true;
    }
    if(argc ==3)
    if(app.arguments().at(1)=="-c") {
      cfgname=argv[2];
      ok=true;
    }
    if(argc ==4)
    if(app.arguments().at(1)=="-c") {
      cfgname=argv[2];
      vioname=argv[3];
      ok=true;
    }
    if(!ok) {
      cout << "usage: viogen [-c config.txt] [generator.vio]" << endl;  
      return 1;
    }

    // configure
    VioStyle::Default();
    if(cfgname!="") VioStyle::Config(cfgname); 
    

    // disclaimer
    {
    QSettings settings("Faudes", "vioGen");
    QString never = settings.value("disclaimer3").toString();
    if(never!="dontShowDisclaimer") {

      QMessageBox disclaimer;
      disclaimer.setText(QString(
             "<p><b>Disclaimer:</b> This software is provided as a developer preview. It "
             "may exit unexpectedly and miss crucial functions, etc. </p>"
             "%1"
             "<p><b>Do you want to proceed?</b></p>").arg(VioStyle::LICENSE));
      disclaimer.setWindowTitle("vioGen");
      disclaimer.setIcon(QMessageBox::Information);
      QPushButton* disno = disclaimer.addButton(
        "No", QMessageBox::NoRole);
      QPushButton* disnever = disclaimer.addButton(
        "Yes, dont ask again", QMessageBox::YesRole);
      QPushButton* disyes = disclaimer.addButton(
        "Yes", QMessageBox::YesRole);
      disclaimer.exec();
      if( disclaimer.clickedButton() == disno) exit(1);
      if( disclaimer.clickedButton() == disnever) 
        settings.setValue("disclaimer3", "dontShowDisclaimer");
      (void) disyes;
    }
    }

    // load file and go
    VioWindow *vioWin = new VioWindow;
    QSettings settings("Faudes", "vioGen");
    vioWin->restoreGeometry(settings.value("geometry").toByteArray());
    if(vioname!="") vioWin->loadFile(vioname);
    vioWin->show();
    return app.exec();

}
