#!/bin/bash

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
