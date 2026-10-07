cmake_minimum_required(VERSION 3.15)

project(MobileApp LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
option(MOBILE_APP_BUILD_TESTS "Build Tests" OFF) # Wyłączone domyślnie dla Androida

if(WIN32)
  set(VCPKG_TARGET_TRIPLET "x64-mingw-static" CACHE STRING "")
  set(VCPKG_INSTALLED_DIR "${CMAKE_SOURCE_DIR}/vendor" CACHE PATH "")
  set(ZLIB_USE_STATIC_LIBS ON CACHE BOOL "Use static zlib") 
  list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/vendor/x64-mingw-static")
elseif(ANDROID)
  set(VCPKG_TARGET_TRIPLET "x86_64-android" CACHE STRING "")
  set(VCPKG_INSTALLED_DIR "${CMAKE_SOURCE_DIR}/vendor" CACHE PATH "")
endif()

add_subdirectory(app)

if(MOBILE_APP_BUILD_TESTS)
  # enable_testing()
  # add_subdirectory(tests)
endif()
