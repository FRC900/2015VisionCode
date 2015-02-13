cmake_minimum_required(VERSION 2.8)
project( test )
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

add_executable( camera_cal camera_cal.cpp )
target_link_libraries( camera_cal ${OpenCV_LIBS} )
