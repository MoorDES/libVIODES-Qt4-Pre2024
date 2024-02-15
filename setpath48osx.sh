echo ==================== set qt path

export PATH=/usr/local/qt-4.8.7-osx11/bin:/opt/lsb/bin:$PATH

echo ==================== set version in qmake database

. ./VERSION
VIODES_VERSION=$VIODES_VERSION_MAJOR.$VIODES_VERSION_MINOR
qmake -set VIODES_VERSION $VIODES_VERSION 
qmake -set VIODES_VERSION_MAJOR $VIODES_VERSION_MAJOR 
qmake -set VIODES_VERSION_MINOR $VIODES_VERSION_MINOR 

