cmake_minimum_required(VERSION 2.8)
project( MOST )
find_package( OpenCV 4.0 REQUIRED )

find_library(GEOS_C geos_c)
find_path(GEOS_INC geos_c.h)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

message(${OpenCV_INCLUDE_DIRS})

include_directories( ${OpenCV_INCLUDE_DIRS} "./inc" "./preprocessing_geometry/inc" "/usr/include" "/usr/local/include")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_executable(segmenter src/segmenter_main.cpp)
target_link_libraries(segmenter ${OpenCV_LIBS})

add_executable(simplifier src/simplifier_main.cpp preprocessing_geometry/src/polygon.cpp preprocessing_geometry/src/simplifier.cpp)
target_link_libraries(simplifier ${OpenCV_LIBS} ${GEOS_C})

add_executable(frame_extractor src/frame_extractor_main.cpp)
target_link_libraries(frame_extractor ${OpenCV_LIBS})

add_executable(hsv src/hsv_filter_main.cpp)
target_link_libraries(hsv ${OpenCV_LIBS})

add_executable(auto_segmenter src/auto_segmenter_main.cpp preprocessing_geometry/src/polygon.cpp)
target_link_libraries(auto_segmenter ${OpenCV_LIBS} ${GEOS_C})	

add_executable(cell_extraction src/cell_extraction_main.cpp)
target_link_libraries(cell_extraction ${OpenCV_LIBS})

add_executable(warp src/warp_main.cpp)
target_link_libraries(warp ${OpenCV_LIBS})

add_executable(draw_wkt src/draw_wkt.cpp preprocessing_geometry/src/polygon.cpp preprocessing_geometry/src/simplifier.cpp)
target_link_libraries(draw_wkt ${OpenCV_LIBS} ${GEOS_C})
