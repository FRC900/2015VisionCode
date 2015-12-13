cmake_minimum_required(VERSION 2.8)
set(CMAKE_LEGACY_CYGWIN_WIN32 0) # Remove when CMake >= 2.8.4 is required
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -march=native -flto")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Ofast -march=native -flto")
project( gaTesting )
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} galib247 ../bindetection ../C920VideoCap )

find_library (LibGA ga PATH galib247/ga )
add_executable( gaTesting gaTesting.cpp ../bindetection/objdetect.cpp )
target_link_libraries( gaTesting ${OpenCV_LIBS} ${LibGA} )
