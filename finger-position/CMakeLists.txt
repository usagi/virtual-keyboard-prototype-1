cmake_minimum_required(VERSION 2.8.10)

project(finger-position)

find_package(OpenCV REQUIRED)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  find_library(OpenCL_LIBS OpenCL)
  if(NOT OpenCL_LIBS)
    message(FATAL "libOpenCL.so is not found")
  else()
    message(STATUS "OpenCL_LIBS: ${OpenCL_LIBS}")
  endif()
endif()

#find_package(ImageMagick REQUIRED COMPONENTS Magick++)
#include_directories(${ImageMagick_INCLUDE_DIRS})

set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")

add_executable(finger-position finger-position.cxx)

target_link_libraries(finger-position
  ${OpenCV_LIBS}
  ${OpenCL_LIBS}
#  ${ImageMagick_LIBRARIES}
)
