cmake_minimum_required(VERSION 2.8.9)
project(etupirka)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.7")
    message(FATAL_ERROR "Insufficient g++ version; require g++-4.7")
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "3.3")
    message(FATAL_ERROR "Insufficient clang++ version; require clang++-3.3")
  endif()
else()
  message(WARNING "Unexpected compiler: ${CMAKE_CXX_COMPILER_ID}")
endif()

set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "-Wall -pedantic-errors ${CMAKE_CXX_FLAGS}")

set(CMAKE_CXX_FLAGS_RELEASE        "-O3 -march=native -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -march=native -g -pg")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -march=native -g -pg")

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
  gui.cxx
)

add_custom_command(TARGET etupirka POST_BUILD
  COMMAND ${PROJECT_SOURCE_DIR}/virtual-keyboard.build.sh \"${PROJECT_SOURCE_DIR}\" \"${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/etupirka.dir\" \"${CMAKE_CURRENT_BINARY_DIR}\"
  DEPENDS ${PROJECT_SOURCE_DIR}/virtual-keyboard.csv
)

include_directories(
  libWRP-SQLite3/include
  libWRP-key/include
)

find_package(Threads REQUIRED)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.8)
  message(STATUS "GCC<4.8: link boost(system thread)")
  find_package(Boost 1.49.0 REQUIRED system thread)
else()
  find_package(Boost 1.49.0 REQUIRED system)
endif()
find_package(OpenCV REQUIRED core highgui imgproc)

find_library(LIBTBB NAMES tbb HINTS /opt/local/lib)
if(LIBTBB STREQUAL "LIBTBB-NOTFOUND")
  message(STATUS "TBB is not found, try no use.")
  set(LIBTBB "")
endif()
message(STATUS "LIBTBB: ${LIBTBB}")

find_file(LIBWRPSQLITE3 WonderRabbitProject/SQLite3.hpp
  PATHS
    libWRP-SQLite3/include
    ~/opt/include
    /usr/local/lib/include
    /usr/lib/include
    /lib/include
)
if(NOT LIBWRPSQLITE3)
  message(FATAL_ERROR "libWRP-SQLite3 is not found")
else()
  message(STATUS "libWRP-SQLite3: ${LIBWRPSQLITE3}")
endif()

find_file(LIBWRPKEY WonderRabbitProject/key.hxx
  PATHS
    libWRP-key/include
    ~/opt/include
    /usr/local/lib/include
    /usr/lib/include
    /lib/include
)
if(NOT LIBWRPKEY)
  message(FATAL_ERROR "libWRP-key is not found")
else()
  message(STATUS "libWRP-key: ${LIBWRPKEY}")
endif()

find_package(PkgConfig REQUIRED)

if(APPLE)
  find_library(LIBSQLITE3_LIBRARIES NAMES sqlite3 HINTS /opt/local/lib)
else()
  pkg_search_module(LIBSQLITE3 REQUIRED sqlite3)
  message(STATUS "libsqlite3: ${LIBSQLITE3_LIBRARIES}")
endif()

target_link_libraries(etupirka
  ${CMAKE_THREAD_LIBS_INIT}
  ${Boost_LIBRARIES}
  ${SQLITE3_LIB}
  ${OpenCV_LIBS}
  ${LIBTBB}
  ${LIBSQLITE3_LIBRARIES}
)

if(APPLE)
  find_file(OSX_CG_LIB CoreGraphics HINTS /System/Library/Frameworks/CoreGraphics.framework/Versions/A)

  if(NOT OSX_CG_LIB)
    message(FATAL "OSX CoreGraphics is not found")
  else()
    message(STATUS "OSX CoreGraphics: ${OSX_CG_LIB}")
  endif()

  find_file(OSX_CF_LIB CoreFoundation HINTS /System/Library/Frameworks/CoreFoundation.framework/Versions/A)

  if(NOT OSX_CF_LIB)
    message(FATAL "OSX CoreFoundation is not found")
  else()
    message(STATUS "OSX CoreFoundation: ${OSX_CF_LIB}")
  endif()
  
  target_link_libraries(etupirka
    ${OSX_CG_LIB}
    ${OSX_CF_LIB}
  )
endif(APPLE)


#if(CMAKE_BUILD_TYPE STREQUAL debug)
  pkg_search_module(GLOG REQUIRED libglog)
  include_directories(${GLOG_INCLUDE_DIRS})
  target_link_libraries(etupirka ${GLOG_LIBRARIES})
#endif()

find_program(SQLITE3 sqlite3 HINTS ~/opt/bin /opt/local/bin)
if(NOT SQLITE3)
  message(FATAL_ERROR "sqlite3 is not found")
else()
  message(STATUS "sqlite3: ${SQLITE3}")
endif()
