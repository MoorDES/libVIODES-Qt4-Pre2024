#
# project file for libviodes plugin viodiag, tmoor 2016
#
#

# configuration parameters
VIODES_BASE = ../..
VIODES_TARGET = viodiag

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
DEFINES += FAUDES_DEBUG_VIO_DIAG
#DEFINES += FAUDES_DEBUG_VIO_WIDGETS
}


# viomtc headers and sources
HEADERS      += src/viodiag.h \
                src/viodiagstateattr.h \
                src/viodiaggenstyle.h 
SOURCES      += src/viodiag.cpp \
                src/viodiagstateattr.cpp \
                src/viodiaggenstyle.cpp 
