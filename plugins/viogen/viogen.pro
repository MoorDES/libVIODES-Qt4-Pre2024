#
# project file for libVIODES plugin viogen, tmoor 2016
#
#

# configuration parameters
VIODES_BASE = ../..
VIODES_TARGET = viogen

# derive libVIODES configuration
# load common configuration
! include( $${VIODES_BASE}/viodes.pri ) {
    error("### error: libVIODES target configuration file not found" )
}


# local debug configuration
debug {
#DEFINES += FAUDES_DEBUG_VIO
#DEFINES += FAUDES_DEBUG_VIO_TYPE
#DEFINES += FAUDES_DEBUG_VIO_GENERATOR
#DEFINES += FAUDES_DEBUG_VIO_WIDGETS
}

# using alglib spline
VIO_SPLINE = ./qtspline3
INCLUDEPATH += $$VIO_SPLINE/src
INCLUDEPATH += $$VIO_SPLINE/alglib
unix:LIBS += $$VIO_SPLINE/libqtspline3.a
win32:LIBS += $$VIO_SPLINE/qtspline3.lib
DEFINES += USING_SPLINE


# vio generator widget headers and sources
HEADERS      += include/viogen.h \
                include/viogenstyle.h \
                include/viogenerator.h \
                include/viogenlist.h \
                include/viogengraph.h \
                src/liotselist.h \
                src/liotseview.h \
                src/piotseview.h \
                src/gioitem.h \ 
                src/giostate.h \
                src/giotrans.h \
                src/gioscenero.h \
                src/gioscene.h \
                src/gioview.h 
SOURCES      += src/viogen.cpp \
                src/viogenstyle.cpp \
                src/viogenerator.cpp \
                src/viogenlist.cpp \
                src/viogengraph.cpp \
                src/liotselist.cpp \
                src/liotseview.cpp \
                src/piotseview.cpp \
                src/gioitem.cpp \ 
                src/giostate.cpp \
                src/giotrans.cpp \
                src/gioscenero.cpp \
                src/gioscene.cpp \
                src/gioview.cpp 

