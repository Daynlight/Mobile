cmake_minimum_required(VERSION 3.18)

project(MobileApp LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
option(MOBILE_APP_BUILD_TESTS "Build Tests" OFF) # Wyłączone domyślnie dla Androida

set(VCPKG_TARGET_TRIPLET "x64-android" CACHE STRING "")
set(VCPKG_INSTALLED_DIR "${CMAKE_SOURCE_DIR}/vendor" CACHE PATH "")

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/vendor/x64-android")
list(APPEND CMAKE_FIND_ROOT_PATH "${CMAKE_SOURCE_DIR}/vendor/x64-android")

add_subdirectory(app)

if(MOBILE_APP_BUILD_TESTS)
  # enable_testing()
  # add_subdirectory(tests)
endif()
