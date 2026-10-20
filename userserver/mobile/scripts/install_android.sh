#!/bin/bash

wget "https://sourceforge.net/projects/android-x86/files/Release%209.0/android-x86_64-9.0-r2.iso/download" -O emulator/images/android-x86.iso
qemu-img create -f qcow2 emulator/android_disk.qcow2 20G

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
