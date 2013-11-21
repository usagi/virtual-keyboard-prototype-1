cmake_minimum_required(VERSION 2.8)

project(virtual-keyboard-prototype-1)

add_definitions(-std=c++11)

find_package(Threads)
find_package(Boost 1.49.0 REQUIRED system program_options)

add_executable(sender sender.cxx)
target_link_libraries(sender ${CMAKE_THREAD_LIBS_INIT} ${Boost_SYSTEM_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY})

add_executable(reciever reciever.cxx)
target_link_libraries(reciever ${CMAKE_THREAD_LIBS_INIT} ${Boost_SYSTEM_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY})

add_executable(keyboard-writer keyboard-writer.cxx)
target_link_libraries(keyboard-writer ${Boost_PROGRAM_OPTIONS_LIBRARY})

install(TARGETS sender RUNTIME DESTINATION bin)
install(TARGETS reciever RUNTIME DESTINATION bin)
install(TARGETS keyboard-writer RUNTIME DESTINATION bin)
