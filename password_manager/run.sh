#!/bin/bash

adb connect 127.0.0.1:5555
adb -s 127.0.0.1:5555 uninstall com.ma.app
adb -s 127.0.0.1:5555 install -r app-aligned.apk
adb -s 127.0.0.1:5555 shell am start -n com.ma.app/android.app.NativeActivity
adb -s 127.0.0.1:5555 logcat -s MA_APP
