cmake_minimum_required(VERSION 3.15)

project(app_so LANGUAGES CXX)
add_library(app_so SHARED App.cpp)

project(app LANGUAGES CXX C)

set(NATIVE_APP_GLUE_DIR "${ANDROID_NDK}/sources/android/native_app_glue")
include_directories(${NATIVE_APP_GLUE_DIR})

add_library(main SHARED 
  main.cpp 
  ${NATIVE_APP_GLUE_DIR}/android_native_app_glue.c
)

target_link_libraries(main PRIVATE app_so android log)
target_link_options(main PRIVATE -u ANativeActivity_onCreate)
target_link_libraries(app_so PRIVATE log)
