cmake_minimum_required(VERSION 2.8)

set(CMAKE_LEGACY_CYGWIN_WIN32 0) # Remove when CMake >= 2.8.4 is required
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast -flto")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Ofast -flto")

project(fvs)
#find_package(Boost COMPONENTS geometry)
include_directories(${Boost_INCLUDE_DIR} src)
#link_directories(${Boost_LIBRARY_DIR})
add_executable(fvs fvs.cpp)
#target_link_libraries(fvs ${Boost_LIBRARIES})



