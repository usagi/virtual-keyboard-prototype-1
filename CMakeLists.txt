cmake_minimum_required(VERSION 2.8)

project(virtual-keyboard-prototype-1)

if(NOT CMAKE_CXX_COMPILER)
  set(CMAKE_CXX_COMPILER "clang++")
endif()

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

set(CMAKE_CXX_FLAGS "-Wall -pedantic-errors -Werror ${CMAKE_CXX_FLAGS}")

find_package(Threads REQUIRED)
find_package(Boost 1.49.0 REQUIRED system program_options)
find_package(OpenCV REQUIRED)

add_executable(sender sender.cxx)
target_link_libraries(sender ${CMAKE_THREAD_LIBS_INIT} ${Boost_SYSTEM_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY})

add_executable(reciever reciever.cxx)
target_link_libraries(reciever ${CMAKE_THREAD_LIBS_INIT} ${Boost_SYSTEM_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY})

add_executable(keyboard-writer keyboard-writer.cxx)
target_link_libraries(keyboard-writer ${Boost_PROGRAM_OPTIONS_LIBRARY})

add_executable(virtual-keyboard-tester virtual-keyboard-tester.cxx)
target_link_libraries(virtual-keyboard-tester ${Boost_PROGRAM_OPTIONS_LIBRARY} -lsqlite3)

add_custom_command(TARGET virtual-keyboard-tester POST_BUILD
  COMMAND ${PROJECT_SOURCE_DIR}/virtual-keyboard.build.sh \"${PROJECT_SOURCE_DIR}\" \"${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/virtual-keyboard-tester.dir\" \"${CMAKE_CURRENT_BINARY_DIR}\"
  DEPENDS ${PROJECT_SOURCE_DIR}/virtual-keyboard.csv
)

add_executable(virtual-keyboard-viewer virtual-keyboard-viewer.cxx)
target_link_libraries(virtual-keyboard-viewer ${Boost_PROGRAM_OPTIONS_LIBRARY} -lsqlite3 ${OpenCV_LIBS})

install(TARGETS sender RUNTIME DESTINATION bin)
install(TARGETS reciever RUNTIME DESTINATION bin)
install(TARGETS keyboard-writer RUNTIME DESTINATION bin)
install(TARGETS virtual-keyboard-tester RUNTIME DESTINATION bin)
install(TARGETS virtual-keyboard-viewer RUNTIME DESTINATION bin)
