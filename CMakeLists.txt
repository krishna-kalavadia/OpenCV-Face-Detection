cmake_minimum_required(VERSION 3.16)
project(OpenCV-Face-Detection-Image-processing)
set(CMAKE_CXX_STANDARD 14)

find_package(OpenCV)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(OpenCV-Face-Detection-Image-processing face_detection.cpp)
target_link_libraries(OpenCV-Face-Detection-Image-processing ${OpenCV_LIBS})