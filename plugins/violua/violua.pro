#
# project file for libviodes plugin violua, tmoor 2016
#
#

# configuration parameters
VIODES_BASE = ../..
VIODES_TARGET = violua

# derive libVIODES configuration
# load common configuration
! include( $${VIODES_BASE}/viodes.pri ) {
    error("### error: libVIODES target configuration file not found" )
}

# local debug configuration
debug {
DEFINES += FAUDES_DEBUG_VIO_LUA
#DEFINES += FAUDES_DEBUG_VIO_WIDGETS
}

# depends on Lua
INCLUDEPATH += $$VIODES_LIBFAUDES/include/lua

# violua headers and sources
HEADERS      += src/violua.h \
                src/violuastyle.h \
                src/violuafunction.h \
                src/violuacode.h 
SOURCES      += src/violua.cpp \
                src/violuastyle.cpp \
                src/violuafunction.cpp \
                src/violuacode.cpp
