#
# project file for libviodes plugin viomtc, tmoor 2016
#
#

# configuration parameters
VIODES_BASE = ../..
VIODES_TARGET = viomtc

# derive libVIODES configuration
# load common configuration
! include( $${VIODES_BASE}/viodes.pri ) {
    error("### error: libVIODES target configuration file not found" )
}

# depends on viogen
LIBS          += $$VIODES_LIBVIOGEN_DSO
INCLUDEPATH   += $$VIODES_BASE/plugins/viogen/include

# local debug configuration
debug {
DEFINES += FAUDES_DEBUG_VIO
DEFINES += FAUDES_DEBUG_VIO_TYPE
DEFINES += FAUDES_DEBUG_VIO_MTC
#DEFINES += FAUDES_DEBUG_VIO_WIDGETS
}


# viomtc sources

HEADERS      += src/viomtc.h \
                src/viomtcstateattr.h \
                src/viomtcgenstyle.h \
                src/viomtcgenerator.h 

SOURCES      += src/viomtc.cpp \
                src/viomtcstateattr.cpp \
                src/viomtcgenstyle.cpp \
                src/viomtcgenerator.cpp 
