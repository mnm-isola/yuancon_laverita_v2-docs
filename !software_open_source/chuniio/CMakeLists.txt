cmake_minimum_required(VERSION 3.17)
project(chuniio)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")

include_directories(${chuniio_SOURCE_DIR})
include_directories(${chuniio_SOURCE_DIR}/include)
link_directories(${PROJECT_SOURCE_DIR}/lib)

add_library(hid STATIC IMPORTED)
set_property(TARGET hid PROPERTY IMPORTED_LOCATION ${chuniio_SOURCE_DIR}/lib/hid.lib)
add_library(setupapi STATIC IMPORTED)
set_property(TARGET setupapi PROPERTY IMPORTED_LOCATION ${chuniio_SOURCE_DIR}/lib/setupapi.lib)

add_library(chuniio SHARED chuniio.c config.c hid_impl.c)
target_link_libraries(chuniio hid setupapi)
SET_TARGET_PROPERTIES(chuniio PROPERTIES PREFIX "")


find_path(LIBUSB_INCLUDE_DIR
  NAMES libusb.h
  PATH_SUFFIXES "include" "libusb" "libusb-1.0")
find_library(LIBUSB_LIBRARY NAMES usb-1.0 PATHS ${CMAKE_SOURCE_DIR}/lib)
  
add_executable(chunitest chunitest.c)
target_link_libraries(chunitest chuniio)

#target_compile_options(chuniio PRIVATE -Wall -Wextra -Wpedantic -Werror)