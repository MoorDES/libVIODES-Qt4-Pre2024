# ##########################################
# qmake include for libFAUDES based libraries
# 
# This project file supports the configuration of Qt library projects 
# that link against libFAUDES. It is complemented by the shell scripts 
# "copyfaudes.sh" and "distclean.sh", operational on MacOsX and Linux 
# platforms. Effectively, a properly configured and compiles libFAUDES
# is expected to reside in the specified path.
# 
# Relevant settings in this version are tailored for directory layout
# used for libVIODES including its Qt plug-ins
#
# Referenced variables, to be set in the main project file
# - VIODES_TARGET          target name, e.g., viocore, viogen etc.
# - VIODES_BASE            base dir of libVIODES, must contain ./libfaudes_for_viodes
#
# tmoor 201602
# ##########################################

# figure version numbers (from qmake database or main project)
isEmpty( VIODES_VERSION_MAJOR ): VIODES_VERSION_MAJOR = $$[VIODES_VERSION_MAJOR]
isEmpty( VIODES_VERSION_MINOR ): VIODES_VERSION_MINOR = $$[VIODES_VERSION_MINOR]
isEmpty( VIODES_VERSION_MAJOR ): error("=== error: libVIODES major version not configured")
isEmpty( VIODES_VERSION_MINOR ): error("=== error: libVIODES minor version not configured")
VIODES_VERSION = $${VIODES_VERSION_MAJOR}.$${VIODES_VERSION_MINOR}

# say hello
message("=== libVIODES base at" $${VIODES_BASE})
message("=== libVIODES component" $${VIODES_TARGET})
message("=== libVIODES version" $${VIODES_VERSION})
message("=== using Qt at" $${QMAKE_LIBDIR_QT})

# libVIODES directory layout
VIODES_LIBFAUDES = $$VIODES_BASE/libfaudes_for_viodes
VIODES_INCLUDE = $$VIODES_BASE/include

# qmake target setting
TEMPLATE = lib
CONFIG += shared plugin
TARGET = $$VIODES_TARGET
DESTDIR = $$VIODES_BASE
LANGUAGE = C++
QT += core gui svg



# platform dependand library names
unix {
  VIODES_LIBFAUDES_DSO = -L$${VIODES_LIBFAUDES} -lfaudes
  VIODES_LIBVIODES_DSO = -L$${VIODES_BASE} -lviodes
  VIODES_LIBVIOGEN_DSO = -L$${VIODES_BASE} -lviogen
}
win32 {
  VIODES_LIBFAUDES_DSO = $${VIODES_LIBFAUDES}\\faudes.lib
  VIODES_LIBVIODES_DSO = $${VIODES_BASE}\\viodes.lib
  VIODES_LIBVIOGEN_DSO = $${VIODES_BASE}\\viogen.lib
}


# link to libviodes and libfaudes
LIBS += $${VIODES_LIBFAUDES_DSO} 
!equals( VIODES_TARGET , viodes ) {
LIBS += $${VIODES_LIBVIODES_DSO} 
}

# extra lsb compiler options (must preceed libraries)
linux-lsb-g++:QMAKE_LFLAGS   += --lsb-shared-libs=faudes:viodes:viogen


# force win32 plugin to release mode
win32: CONFIG -= debug
win32: CONFIG += release

# extra cflags
win32: QMAKE_CXXFLAGS += /EHsc 


# dll export/import switch
DEFINES += $$upper($${VIODES_TARGET})_BUILD_LIB
DEFINES += FAUDES_BUILD_APP


# pass on directory layout to qmake
INCLUDEPATH += $$VIODES_LIBFAUDES/include 
INCLUDEPATH += $$VIODES_INCLUDE
INCLUDEPATH += ./include
INCLUDEPATH += ./src
OBJECTS_DIR = $$VIODES_BASE/obj
MOC_DIR = $$VIODES_BASE/obj


# pass on config to compiler
DEFINES += VIODES_VERSION='\\"$${VIODES_VERSION}\\"'
DEFINES += VIODES_TARGET='\\"$${VIODES_TARGET}\\"'

