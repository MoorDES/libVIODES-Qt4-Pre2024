#
# project file for core libVIODES, tmoor 2016
#
#

# configuration parameters
VIODES_BASE = ..
VIODES_TARGET = viodes

# derive libVIODES configuration
# load common configuration
! include( $${VIODES_BASE}/viodes.pri ) {
    error("### error: libVIODES target configuration file not found" )
}


# local debug configuration
debug {
#DEFINES += FAUDES_DEBUG_VIO
#DEFINES += FAUDES_DEBUG_VIO_TYPE
#DEFINES += FAUDES_DEBUG_VIO_ATTRIBUTE
DEFINES += FAUDES_DEBUG_VIO_NAMESET
DEFINES += FAUDES_DEBUG_VIO_HELPER
#DEFINES += FAUDES_DEBUG_VIO_WIDGETS
}



# component sources
HEADERS      += $$VIODES_INCLUDE/viostyle.h \
                $$VIODES_INCLUDE/vioelement.h \ 
                $$VIODES_INCLUDE/viosymbol.h \ 
                $$VIODES_INCLUDE/viotoken.h \ 
                $$VIODES_INCLUDE/vioconsole.h \ 
                $$VIODES_INCLUDE/viotypes.h \ 
                $$VIODES_INCLUDE/vioregistry.h \ 
                $$VIODES_INCLUDE/vioattrstyle.h \
                $$VIODES_INCLUDE/vioattribute.h \
                $$VIODES_INCLUDE/vionsetstyle.h \
                $$VIODES_INCLUDE/vionameset.h \                
                src/lionameset.h \                
                $$VIODES_INCLUDE/libviodes.h 
SOURCES      += src/viostyle.cpp \
                src/vioelement.cpp \ 
                src/viosymbol.cpp \ 
                src/viotoken.cpp \ 
                src/vioconsole.cpp \ 
                src/viotypes.cpp \
                src/vioregistry.cpp \
                src/vioattrstyle.cpp \
                src/vioattribute.cpp \ 
                src/vionsetstyle.cpp \
                src/vionameset.cpp \ 
                src/lionameset.cpp                

