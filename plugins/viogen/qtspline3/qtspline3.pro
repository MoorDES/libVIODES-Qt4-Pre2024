#
# project file for qtspline3, tmoor 2016
#
#

# say hello
message("=== libVIODES independent static library qtspline3")

# set relevant paths

QTS_SRC = ./src
QTS_ALGLIB = ./alglib
QTS_OBJ = ./obj

# target setting

TEMPLATE = lib
CONFIG += staticlib
TARGET = qtspline3
LANGUAGE = C++

# paths and files

INCLUDEPATH += $$QTS_SRC
INCLUDEPATH += $$QTS_ALGLIB
OBJECTS_DIR = $$QTS_OBJ
MOC_DIR = $$QTS_OBJ
DESTDIR = ./

# force win32 plugin to release mode

win32: CONFIG -= debug
win32: CONFIG += release

# alglib widget headers and sources

HEADERS      += $$QTS_ALGLIB/ap.h \
                $$QTS_ALGLIB/apvt.h \
                $$QTS_ALGLIB/stdafx.h \
                $$QTS_ALGLIB/spline3.h 

SOURCES      += $$QTS_ALGLIB/ap.cpp \
                $$QTS_ALGLIB/spline3.cpp 
                
# qt wrapper  headers and sources

HEADERS      += $$QTS_SRC/qtspline3.h

SOURCES      += $$QTS_SRC/qtspline3.cpp
                
