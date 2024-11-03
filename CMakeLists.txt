cmake_minimum_required(VERSION 3.20)
project(boost VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable AUTOMOC for Qt's signal/slot system
set(CMAKE_AUTOMOC ON)

# Find required packages
find_package(OpenCV REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Include directories
include_directories(${OpenCV_INCLUDE_DIRS} ${Qt6Widgets_INCLUDE_DIRS} src)

# Status messages for debugging
message(STATUS "OpenCV_DIR: ${OpenCV_DIR}")
message(STATUS "OpenCV_INCLUDE_DIRS: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "Qt6Widgets_INCLUDE_DIRS: ${Qt6Widgets_INCLUDE_DIRS}")

# Add executable
add_executable(${PROJECT_NAME}
    src/main.cpp 
    src/LabelImageQt.cpp
    src/ReadImageQt.cpp
    src/utils.cpp
    src/processor.cpp)

# Link libraries
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} Qt6::Widgets)

# CPack configuration
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CPACK_GENERATOR "ZIP")
include(CPack)