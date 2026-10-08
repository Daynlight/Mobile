cmake_minimum_required(VERSION 3.18)
## =========
## Packages
## =========
find_package(imgui CONFIG REQUIRED)
find_package(httplib CONFIG REQUIRED)

## =========
## Core
## =========
project(Core LANGUAGES CXX C)

add_library(native_app_glue STATIC ${CMAKE_ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
target_include_directories(native_app_glue PUBLIC ${CMAKE_ANDROID_NDK}/sources/android/native_app_glue)

set(src
  Core.cpp  
)

add_library(Core SHARED ${src})

target_link_libraries(Core PUBLIC native_app_glue)
target_include_directories(Core PRIVATE
  ${CMAKE_SOURCE_DIR}/vendor/x64-android/include
  ${CMAKE_SOURCE_DIR}/vendor/x64-android/include/imgui
)
target_link_libraries(Core PRIVATE
  imgui::imgui
  httplib::httplib
  android
  log
  EGL
  GLESv3
)

## =========
## Main
## =========
set(src
  main.cpp
)

add_library(main SHARED ${src})
target_link_libraries(main PRIVATE 
  Core 
  android
  native_app_glue
  log
)

