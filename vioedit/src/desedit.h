/* viogen.h  - example for editable viogenerator */

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

#ifndef VIOWINDOW_H
#define VIOWINDOW_H

#include <QList>
#include <QMainWindow>

//#include "libvio.h"
class VioGenerator;


class QAction;
class QMenu;

class VioWindow : public QMainWindow
{
    Q_OBJECT

public:
    VioWindow();

public slots:
    void newFile();
    void open();
    void save();
    void saveAs();
    void gexport();
    void eexport();
    void jexport();
    void gimport();
    void openRecentFile();
    void about();
    void loadFile(const QString &fileName);
    void saveFile(const QString &fileName);
    void loadConfig(const QString &fileName);
    void gimportFile(const QString &fileName);
    void gexportFile(const QString &fileName);
    void eexportFile(const QString &fileName);
    void jexportFile(const QString &fileName);

protected:
    void closeEvent (QCloseEvent * event );

private:
    void createActions();
    void createMenus();
    void setCurrentFile(const QString &fileName);
    void updateRecentFileActions();
    QString strippedName(const QString &fullFileName);

    QString curFile;

    VioGenerator* vioGenerator;

    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *viewMenu;
    QMenu *helpMenu;
    QMenu *recentFilesMenu;

    QAction *newAct;
    QAction *openAct;
    QAction *saveAct;
    QAction *gimportAct;
    QAction *gexportAct;
    QAction *eexportAct;
    QAction *jexportAct;
    QAction *saveAsAct;
    QAction *exitAct;

    QAction *undoAct;
    QAction *redoAct;
    QAction *cutAct;
    QAction *copyAct;
    QAction *pasteAct;

    QAction *graphviewAct;
    QAction *propviewAct;
    QAction *zoominAct;
    QAction *zoomoutAct;
    QAction *zoomfitAct;

    QAction *aboutAct;
    QAction *aboutQtAct;
    QAction *separatorAct;

    enum { MaxRecentFiles = 5 };
    QAction *recentFileActs[MaxRecentFiles];
};

#endif
