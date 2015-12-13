cmake_minimum_required(VERSION 2.8)
set(CMAKE_LEGACY_CYGWIN_WIN32 0) # Remove when CMake >= 2.8.4 is required
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -march=native -flto")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Ofast -march=native -flto")
project( test )
find_package( OpenCV REQUIRED )
find_package (Threads)
include_directories( ${OpenCV_INCLUDE_DIRS} galib247 ../C920VideoCap ../networktables/include)

add_executable( test test.cpp objdetect.cpp detectstate.cpp mediain.cpp videoin.cpp camerain.cpp ../C920VideoCap/C920Camera.cpp c920camerain.cpp imagein.cpp track.cpp classifierio.cpp Args.cpp WriteOnFrame.cpp )
IF(CYGWIN)
   find_library (LibNT networktables PATH ../networktables/lib-x86_64-cygwin )
ELSE()
   find_library (LibNT networktables PATH ../networktables/lib-${CMAKE_LIBRARY_ARCHITECTURE} )
ENDIF()
target_link_libraries( test ${OpenCV_LIBS} ${LibNT} ${CMAKE_THREAD_LIBS_INIT} )

find_library (LibGA ga PATH galib247/ga )
add_executable( gaTesting gaTesting.cpp objdetect.cpp )
target_link_libraries( gaTesting ${OpenCV_LIBS} ${LibGA} )

add_executable( gputest gputest.cpp objdetect.cpp classifierio.cpp )
target_link_libraries( gputest ${OpenCV_LIBS} )

add_executable( fpstest fpstest.cpp objdetect.cpp mediain.cpp videoin.cpp )
target_link_libraries( fpstest ${OpenCV_LIBS} )
