#
# project file for libviodes plugin viohio, tmoor 2016 
#
#

# configuration parameters
VIODES_BASE = ../..
VIODES_TARGET = viohio

# derive libVIODES configuration
# load common configuration
! include( $${VIODES_BASE}/viodes.pri ) {
    error("### error: libVIODES target configuration file not found" )
}

# depends on viogen
LIBS          += $$VIODES_LIBVIOGEN_DSO
INCLUDEPATH   += $$VIODES_BASE/plugins/viogen/include

# ocal debug configuration
debug {
DEFINES += FAUDES_DEBUG_VIO
DEFINES += FAUDES_DEBUG_VIO_TYPE
DEFINES += FAUDES_DEBUG_VIO_HIOSYS
}

# viohio headers and sources
HEADERS      += src/viohio.h 
SOURCES      += src/viohio.cpp 
