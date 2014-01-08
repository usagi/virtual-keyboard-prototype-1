cmake_minimum_required(VERSION 2.8)

project(virtual-keyboard-prototype-1)

include_directories(
  libWRP-SQLite3/include
  libWRP-key/include
)

find_program(SQLITE3 sqlite3 HINTS ~/opt/bin /opt/local/bin)
if(NOT SQLITE3)
  message(FATAL_ERROR "sqlite3 is not found")
else()
  message(STATUS "sqlite3: ${SQLITE3}")
endif()

find_path(SQLITE3_H sqlite3.h HINTS ~/opt/include /opt/local/include)
if(NOT SQLITE3_H)
  message(FATAL_ERROR "sqlite3.h is not found")
else()
  message(STATUS "sqlite3.h: ${SQLITE3_H}")
endif()

find_library(SQLITE3_LIB sqlite3 HINTS ~/opt/lib /opt/local/lib)
if(NOT SQLITE3_LIB)
  message(FATAL_ERROR "libsqlite3 is not found")
else()
  message(STATUS "libsqlite3: ${SQLITE3_LIB}")
endif()

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
endif(APPLE)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.7")
    message(FATAL_ERROR "Insufficient g++ version; require g++-4.7")
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "3.2")
    message(FATAL_ERROR "Insufficient clang++ version; require clang++-3.2")
  endif()
else()
  message(WARNING "Unexpected compiler: ${CMAKE_CXX_COMPILER_ID}")
endif()

add_definitions(-std=c++11)

set(CMAKE_CXX_FLAGS "-Wall -pedantic-errors ${CMAKE_CXX_FLAGS}")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native")
set(CMAKE_CXX_FLAGS_DEBUG   "-O0 -g")

find_package(Threads REQUIRED)
find_package(Boost 1.49.0 REQUIRED system program_options)
find_package(OpenCV REQUIRED)

add_executable(sender sender.cxx)
target_link_libraries(sender ${CMAKE_THREAD_LIBS_INIT} ${Boost_SYSTEM_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY})

add_executable(reciever reciever.cxx)
target_link_libraries(reciever ${CMAKE_THREAD_LIBS_INIT} ${Boost_SYSTEM_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY})

add_executable(keyboard-writer keyboard-writer.cxx)
target_link_libraries(keyboard-writer ${Boost_PROGRAM_OPTIONS_LIBRARY})
if(APPLE)
  target_link_libraries(keyboard-writer ${OSX_CG_LIB} ${OSX_CF_LIB})
endif(APPLE)

add_executable(virtual-keyboard-tester virtual-keyboard-tester.cxx)
target_link_libraries(virtual-keyboard-tester ${Boost_PROGRAM_OPTIONS_LIBRARY} ${SQLITE3_LIB})

add_custom_command(TARGET virtual-keyboard-tester POST_BUILD
  COMMAND ${PROJECT_SOURCE_DIR}/virtual-keyboard.build.sh \"${PROJECT_SOURCE_DIR}\" \"${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/virtual-keyboard-tester.dir\" \"${CMAKE_CURRENT_BINARY_DIR}\"
  DEPENDS ${PROJECT_SOURCE_DIR}/virtual-keyboard.csv
)

add_executable(virtual-keyboard-viewer virtual-keyboard-viewer.cxx)
target_link_libraries(virtual-keyboard-viewer ${Boost_PROGRAM_OPTIONS_LIBRARY} ${SQLITE3_LIB} ${OpenCV_LIBS})

add_executable(camera-preview camera-preview.cxx)
target_link_libraries(camera-preview ${Boost_PROGRAM_OPTIONS_LIBRARY} ${OpenCV_LIBS})

add_executable(pixel-to-realspace pixel-to-realspace.cxx)
target_link_libraries(pixel-to-realspace ${Boost_PROGRAM_OPTIONS_LIBRARY})
