# Mobile



## About



## TOC



## Architecture
### Backend
Backend on nestjs with postgresql database.  
In database only hashes of passwords.  
Users don't send passwords via net.  
Connection via TLS.  
Docker and docker compose for containers.
backend/cert with https certificates.



### Mobile App
C++ with openssl and [Boost.Asio](https://github.com/boostorg/beast)  
UI with OpenGL ES and [idk here ui lib]()
Packages via vcpkg  

#### Installation and Usage
##### Android Image
```bash
wget "https://sourceforge.net/projects/android-x86/files/Release%209.0/android-x86_64-9.0-r2.iso/download" -O emulator/images/android-x86.iso
``` 

##### Create Disk
```bash
qemu-img create -f qcow2 emulator/android_disk.qcow2 20G
```

##### Run qemu and install android
```bash
qemu-system-x86_64 \
  -enable-kvm \
  -m 4096 \
  -smp 4 \
  -hda emulator/android_disk.qcow2 \
  -cdrom emulator/images/android-x86.iso \
  -boot d \
  -netdev user,id=net0,hostfwd=tcp::5555-:5555 \
  -device e1000,netdev=net0 \
  -vga std \
  -usb -device usb-tablet \
  -display gtk
```

##### Run android from disk
```bash
qemu-system-x86_64 \
  -enable-kvm \
  -m 4096 \
  -smp 4 \
  -hda emulator/android_disk.qcow2 \
  -netdev user,id=net0,hostfwd=tcp::5555-:5555 \
  -device e1000,netdev=net0 \
  -vga std \
  -usb -device usb-tablet \
  -display gtk
```

#### Compile Apk and Run
##### Vcpkg
```bash
vcpkg install --triplet=x64-android --x-install-root=./vendor
```

##### Install Dependencies
```bash
sudo apt update && sudo apt install -y cmake ninja-build zip aapt zipalign apksigner adb android-framework-res
wget https://dl.google.com/android/repository/android-ndk-r26b-linux.zip -O ndk.zip
unzip -q ndk.zip -d ndk/
rm ndk.zip
export ANDROID_JAR="/usr/share/android-framework-res/framework-res.apk"
```

##### Compile
```bash
cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE="ndk/android-ndk-r26b/build/cmake/android.toolchain.cmake" -DANDROID_ABI="x86_64" -DANDROID_PLATFORM="android-29" && cmake --build build

rm -rf apk_build/
mkdir -p apk_build/lib/x86_64
cp build/app/libmain.so apk_build/lib/x86_64/
cp build/app/libapp_so.so apk_build/lib/x86_64/

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
```

##### Install apk and run on qemu
```bash
adb connect 127.0.0.1:5555
adb -s 127.0.0.1:5555 install -r app-aligned.apk
adb -s 127.0.0.1:5555 shell am start -n com.ma.app/android.app.NativeActivity
adb -s 127.0.0.1:5555 logcat -s MA_APP
```

##### All in one
```bash
cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE="ndk/android-ndk-r26b/build/cmake/android.toolchain.cmake" -DANDROID_ABI="x86_64" -DANDROID_PLATFORM="android-29" && cmake --build build

rm -rf apk_build/
mkdir -p apk_build/lib/x86_64
cp build/app/libmain.so apk_build/lib/x86_64/
cp build/app/libapp_so.so apk_build/lib/x86_64/

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

adb connect 127.0.0.1:5555
adb -s 127.0.0.1:5555 install -r app-aligned.apk
adb -s 127.0.0.1:5555 shell am start -n com.ma.app/android.app.NativeActivity
adb -s 127.0.0.1:5555 logcat -s MA_APP
```
