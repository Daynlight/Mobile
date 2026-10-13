#!/bin/bash

cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE="ndk/android-ndk-r26b/build/cmake/android.toolchain.cmake" -DANDROID_ABI="x86_64" -DANDROID_PLATFORM="android-29" && cmake --build build

rm -rf apk_build/
mkdir -p apk_build/lib/x86_64
cp build/app/libmain.so apk_build/lib/x86_64/
cp build/app/libCore.so apk_build/lib/x86_64/

rm -f app-unaligned.apk app-aligned.apk
aapt package -f -M AndroidManifest.xml -I "$ANDROID_JAR" -F app-unaligned.apk
cd apk_build
zip -r ../app-unaligned.apk lib/
cd ..
zipalign -v -f 4 app-unaligned.apk app-aligned.apk
if [ ! -f "mykey.jks" ]; then
  keytool -genkeypair -validity 10000 -dname "CN=MA,O=MA,C=PL" -keystore mykey.jks -storepass haslo123 -keypass haslo123 -alias mykey -keyalg RSA
fi
apksigner sign --ks mykey.jks --ks-pass pass:haslo123 --key-pass pass:haslo123 --min-sdk-version 21 app-aligned.apk
