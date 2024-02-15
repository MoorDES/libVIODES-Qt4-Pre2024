echo ==================== set qt path


export PATH=/opt/qt-4.4.2-lsb-3.2/bin:/opt/lsb/bin:$PATH


echo ==================== set version in qmake database

. ./VERSION
VIODES_VERSION=$VIODES_VERSION_MAJOR.$VIODES_VERSION_MINOR
qmake -set VIODES_VERSION $VIODES_VERSION 
qmake -set VIODES_VERSION_MAJOR $VIODES_VERSION_MAJOR 
qmake -set VIODES_VERSION_MINOR $VIODES_VERSION_MINOR 

