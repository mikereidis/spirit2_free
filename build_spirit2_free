#!/bin/bash
export MY_KEYSTORE_FILE="/home/mike/dev/key/spirit2_free_key"
export MY_KEYSTORE_PASS="password"
export MY_KEY_NAME="mikereidis_key"
export WS="/home/mike/dev"
export PROJ="so"
cd $WS/$PROJ

rm -rf obj  # obj/local/armeabi/*
rm -rf libs # libs/armeabi/

mkdir libs

android update project --target android-20 --path . --name $PROJ

#mkdir -p res/raw
#cp ../raw/spirit_sh ../$PROJ/res/raw/
#cp ../raw/b1_bin    ../$PROJ/res/raw/
#cp ../raw/b2_bin    ../$PROJ/res/raw/

echo ndk-build start
time ndk-build # 2>&1 |grep -i error

mv libs/armeabi/s2d libs/armeabi/libs2d.so
#mv libs/armeabi/ssd libs/armeabi/libssd.so

ant -q clean release

jarsigner -storepass $MY_KEYSTORE_PASS -sigalg MD5withRSA -digestalg SHA1 -keystore $MY_KEYSTORE_FILE -signedjar bin/$PROJ-release-unaligned.apk bin/$PROJ-release-unsigned.apk $MY_KEY_NAME
zipalign -f 4 bin/$PROJ-release-unaligned.apk bin/$PROJ-release.apk

#adb install -r bin/$PROJ-release.apk
