# ##########################################
# libVIODES project file 
# tmoor 201602
# ##########################################

# figure version
isEmpty( VIODES_VERSION_MAJOR ): VIODES_VERSION_MAJOR = $$[VIODES_VERSION_MAJOR]
isEmpty( VIODES_VERSION_MINOR ): VIODES_VERSION_MINOR = $$[VIODES_VERSION_MINOR]
isEmpty( VIODES_VERSION_MAJOR ): error("=== error libVIODES major version not configured")
isEmpty( VIODES_VERSION_MINOR ): error("=== error: libVIODES minor version not configured")

# say hello
message("=== libVIODES applications project file")
message("=== using Qt at " $${QMAKE_LIBDIR_QT})
message("=== builing version "$${VIODES_VERSION_MAJOR}"."$${VIODES_VERSION_MINOR})

# do subdirs
CONFIG += ordered
TEMPLATE = subdirs
SUBDIRS  = ./viocore  \
           ./plugins/viogen/qtspline3 \ 
           ./plugins/viogen \ 
           ./plugins/viohio \
           ./plugins/viomtc \
           ./plugins/viosim \
           ./plugins/viodiag \
           ./plugins/violua 

unix:SUBDIRS += ./vioedit



