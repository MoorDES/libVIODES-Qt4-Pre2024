#
# project file for libviodes plugin viosim, tmoor 2016
#
#

# configuration parameters
VIODES_BASE = ../..
VIODES_TARGET = viosim

# derive libVIODES configuration
# load common configuration
! include( $${VIODES_BASE}/viodes.pri ) {
    error("### error: libVIODES target configuration file not found" )
}

# depends on viogen
LIBS          += $$VIODES_LIBVIOGEN_DSO
INCLUDEPATH   += $$VIODES_BASE/plugins/viogen/include

# local debug confgurataion
debug {
#DEFINES += FAUDES_DEBUG_VIO
#DEFINES += FAUDES_DEBUG_VIO_TYPE
DEFINES += FAUDES_DEBUG_VIO_SIM
#DEFINES += FAUDES_DEBUG_VIO_WIDGETS
}

# viosim sources
HEADERS      += src/viosim.h \
                src/viosimcondattr.h 
SOURCES      += src/viosim.cpp \
                src/viosimcondattr.cpp 
