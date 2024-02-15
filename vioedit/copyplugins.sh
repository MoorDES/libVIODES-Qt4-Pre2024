# install plugins for viodemo

# viodes base dir
VIODES=../

# app bundle dest
VIOAPP=lib

# do copy
mkdir $VIOAPP/plugins
mkdir $VIOAPP/plugins/viotypes

cp -p $VIODES/libviodes*so $VIOAPP
cp $VIODES/libfaudes/libfaudes.so $VIOAPP
cp $VIODES/libfaudes/libluafaudes.so $VIOAPP
cp $VIODES/libfaudes/include/libfaudes.rti $VIOAPP
cp $VIODES/tutorial/data/vioconfig.txt $VIOAPP
cp $VIODES/libviogen*so $VIOAPP/plugins/viotypes
cp $VIODES/libviohio*so $VIOAPP/plugins/viotypes
cp $VIODES/libviomtc*so $VIOAPP/plugins/viotypes
cp $VIODES/libviosim*so $VIOAPP/plugins/viotypes
cp $VIODES/libviodiag*so $VIOAPP/plugins/viotypes
cp $VIODES/libviolua*so $VIOAPP/plugins/viotypes

