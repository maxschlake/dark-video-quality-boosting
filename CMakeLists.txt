cmake_minimum_required(VERSION 3.20)
project(opencvex VERSION 0.1.0)

find_package(OpenCV REQUIRED)
include_directories("C:/opt/opencv/build/include")

message(STATUS "OpenCV_DIR: ${OpenCV_DIR}")
message(STATUS "OpenCV_INCLUDE_DIRS: ${OpenCV_INCLUDE_DIRS}")

add_executable(${PROJECT_NAME} src/utils.cpp)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})