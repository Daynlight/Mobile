#!/bin/bash

adb connect 127.0.0.1:5555
adb reverse tcp:3000 tcp:3000
adb -s 127.0.0.1:5555 uninstall com.password_manager.app
adb -s 127.0.0.1:5555 install -r apk/app-aligned.apk
adb -s 127.0.0.1:5555 shell am start -n com.password_manager.app/android.app.NativeActivity
adb -s 127.0.0.1:5555 logcat -s MA_APP
