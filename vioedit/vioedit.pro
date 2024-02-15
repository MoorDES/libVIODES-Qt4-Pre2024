#
# project file for vioedit, tmoor 2016
#
#

# set paths for dependant libraries
VIODES_BASE = ..
VIODES_LIBFAUDES = $$VIODES_BASE/libfaudes_for_viodes

# target setting
TEMPLATE = app
LANGUAGE = C++
QT += core gui svg

# target name
unix:TARGET  =lib/vioedit.bin
macx:TARGET  =VioEdit
win32:TARGET = VioEdit

# lsb compiler options
linux-lsb-g++:LIBS   += --lsb-shared-libs=faudes:luafaudes:viodes
DEFINES += FAUDES_BUILD_APP
DEFINES += VIODES_BUILD_APP


# lib faudes/viodes
LIBS          +=  -L$$VIODES_BASE -lviodes
LIBS          +=  -L$$VIODES_LIBFAUDES -lfaudes 

# qmake paths
INCLUDEPATH += $$VIODES_LIBFAUDES/include 
INCLUDEPATH += $$VIODES_BASE/include 
OBJECTS_DIR = ./obj
MOC_DIR = ./obj


# vioedit sources
HEADERS      += src/vioedit.h                 
SOURCES      += src/vioedit.cpp

# application icon
ICON = ./images/icon_osx.icns 
RC_FILE = ./images/icon_win.rc


# mac: copy libfaudes to bundle 
macx { 
  ContFiles.files += $$VIODES_LIBFAUDES/libfaudes.dylib
  ContFiles.files += $$VIODES_LIBFAUDES/include/libfaudes.rti 
  ContFiles.files += $$VIODES_BASE/libviodes.dylib
  ContFiles.files += $$VIODES_BASE/vioedit/data/vioconfig.txt 
  ContFiles.path = Contents/MacOS
  QMAKE_BUNDLE_DATA += ContFiles
  ViopFiles.files +=  $$VIODES_BASE/libviogen.dylib
  ViopFiles.files +=  $$VIODES_BASE/libviohio.dylib
  ViopFiles.files +=  $$VIODES_BASE/libviomtc.dylib
  ViopFiles.files +=  $$VIODES_BASE/libviosim.dylib
  ViopFiles.files +=  $$VIODES_BASE/libviodiag.dylib
  ViopFiles.files +=  $$VIODES_BASE/libviolua.dylib
  ViopFiles.path = Contents/plugins/viotypes
  QMAKE_BUNDLE_DATA += ViopFiles
}

# mac: fix library paths
macx { 
  # install_name_tool replacement commands for all our libraries
  ITF_LIBFAUDES = -change libfaudes.dylib @executable_path/libfaudes.dylib 
  ITF_LIBVIODES = -change libviodes.dylib @executable_path/libviodes.dylib 
  ITF_LIBVIOGEN = -change libviogen.dylib @executable_path/../plugins/viotypes/libviogen.dylib 
  ITF_ALL = $$ITF_LIBFAUDES $$ITF_LIBVIODES $$ITF_LIBVIOGEN
  QMAKE_EXTRA_TARGETS += macfix
  macfix.target = macfix
  macfix.commands += \
    install_name_tool $$ITF_ALL VioEdit.app/Contents/MacOS/VioEdit && \
    install_name_tool $$ITF_ALL VioEdit.app/Contents/MacOS/libviodes.dylib && \
    install_name_tool $$ITF_ALL VioEdit.app/Contents/plugins/viotypes/libviogen.dylib && \
    install_name_tool $$ITF_ALL VioEdit.app/Contents/plugins/viotypes/libviohio.dylib && \
    install_name_tool $$ITF_ALL VioEdit.app/Contents/plugins/viotypes/libviomtc.dylib && \
    install_name_tool $$ITF_ALL VioEdit.app/Contents/plugins/viotypes/libviosim.dylib && \
    install_name_tool $$ITF_ALL VioEdit.app/Contents/plugins/viotypes/libviodiag.dylib && \
    install_name_tool $$ITF_ALL VioEdit.app/Contents/plugins/viotypes/libviolua.dylib
  QMAKE_POST_LINK += make macfix
}


