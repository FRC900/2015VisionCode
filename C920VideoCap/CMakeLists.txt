cmake_minimum_required(VERSION 2.8)
set(CMAKE_LEGACY_CYGWIN_WIN32 0)
project( foo)
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Ofast")

add_executable( C920VideoCap C920VideoCap.cpp C920Camera.cpp)
target_link_libraries( C920VideoCap ${OpenCV_LIBS} )
