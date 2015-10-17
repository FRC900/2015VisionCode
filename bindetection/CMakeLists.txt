cmake_minimum_required(VERSION 2.8)
set(CMAKE_LEGACY_CYGWIN_WIN32 0) # Remove when CMake >= 2.8.4 is required
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast -flto")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Ofast -flto")
project( test )

find_package( OpenCV REQUIRED )
find_package (Threads)
find_package(ZED 0.8.2 REQUIRED)
find_package(CUDA 6.5 REQUIRED)

include_directories( ${OpenCV_INCLUDE_DIRS} galib247 ../C920VideoCap ../networktables/include)
include_directories(${ZED_INCLUDE_DIRS})
include_directories(${CUDA_INCLUDE_DIRS})

add_executable( test test.cpp imagedetect.cpp videoin_c920.cpp ../C920VideoCap/C920Camera.cpp track.cpp classifierio.cpp Args.cpp WriteOnFrame.cpp )
add_executable( gaTesting gaTesting.cpp imagedetect.cpp )

IF(CYGWIN)
   find_library (LibNT networktables PATH ../networktables/lib-x86_64-cygwin )
ELSE()
   find_library (LibNT networktables PATH ../networktables/lib-${CMAKE_LIBRARY_ARCHITECTURE} )
ENDIF()

link_directories(${ZED_LIBRARY_DIR})
link_directories(${CUDA_LIBRARY_DIRS})

set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS} -std=c++11)
add_definitions(-std=c++11)

target_link_libraries( test ${OpenCV_LIBS} ${LibNT} ${CMAKE_THREAD_LIBS_INIT} ${ZED_LIBRARIES} ${CUDA_LIBRARIES} ${CUDA_nppi_LIBRARY} ${CUDA_npps_LIBRARY})
find_library (LibGA ga PATH galib247/ga )
target_link_libraries( gaTesting ${OpenCV_LIBS} ${LibGA} )

add_executable( gputest gputest.cpp imagedetect.cpp classifierio.cpp )
target_link_libraries( gputest ${OpenCV_LIBS} )
add_executable( fpstest fpstest.cpp imagedetect.cpp videoin.cpp )
target_link_libraries( fpstest ${OpenCV_LIBS} )
