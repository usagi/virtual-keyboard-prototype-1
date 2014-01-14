cmake_minimum_required(VERSION 2.8.10)
project(etupirka)

set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")

add_executable(etupirka
  main.cxx
  etupirka.cxx
  logger.cxx
  commandline_helper.cxx
  camera-capture.cxx
  finger-detector.cxx
  space-converter.cxx
  virtual-keyboard.cxx
  udp-sender.cxx
  udp-reciever.cxx
  key-invoker.cxx
)

if(CMAKE_BUILD_TYPE STREQUAL debug)
  find_package(PkgConfig REQUIRED)
  pkg_search_module(GLOG REQUIRED libglog)
  include_directories(${GLOG_INCLUDE_DIRS})
  target_link_libraries(etupirka ${GLOG_LIBRARIES})
endif()
