cmake_minimum_required(VERSION 3.18)

project(app_so LANGUAGES CXX C)

find_package(imgui CONFIG REQUIRED)
find_package(httplib CONFIG REQUIRED)

add_library(native_app_glue STATIC 
    ${CMAKE_ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
)
target_include_directories(native_app_glue PUBLIC 
    ${CMAKE_ANDROID_NDK}/sources/android/native_app_glue
)
set(src
  App.cpp  
)

add_library(app_so SHARED ${src})
target_include_directories(app_so PRIVATE
  ${CMAKE_SOURCE_DIR}/vendor/x64-android/include
  ${CMAKE_SOURCE_DIR}/vendor/x64-android/include/imgui
)
target_link_libraries(app_so PUBLIC native_app_glue)
target_link_libraries(app_so PRIVATE
  imgui::imgui
  httplib::httplib
  android
  log
  EGL
  GLESv3
)

add_library(main SHARED main.cpp)
target_link_libraries(main PRIVATE app_so 
  android
  native_app_glue
  log
)
