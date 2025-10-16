cmake_minimum_required(VERSION 3.16)
project(baslerjni LANGUAGES CXX)

# ============================================================
# Build configuration
# ============================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

if(NOT PYLON_ROOT)
    set(PYLON_ROOT "/opt/pylon" CACHE PATH "Basler Pylon SDK root directory")
endif()

# ============================================================
# Find JNI
# ============================================================

find_package(JNI REQUIRED)
include_directories(${JNI_INCLUDE_DIRS})

# ============================================================
# Threads
# ============================================================

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# ============================================================
# OpenCV (using FetchContent similar to PhotonVision)
# ============================================================

include(FetchContent)

set(OPENCV_YEAR "frc2025")
set(OPENCV_VERSION "4.10.0-3")
set(OPENCV_TYPE "")

# OpenCV binary library
fetchcontent_declare(
    opencv_lib
    URL https://frcmaven.wpi.edu/artifactory/release/edu/wpi/first/thirdparty/${OPENCV_YEAR}/opencv/opencv-cpp/${OPENCV_VERSION}/opencv-cpp-${OPENCV_VERSION}-linuxx86-64${OPENCV_TYPE}.zip
)
fetchcontent_makeavailable(opencv_lib)

# OpenCV headers
fetchcontent_declare(
    opencv_header
    URL https://frcmaven.wpi.edu/artifactory/release/edu/wpi/first/thirdparty/${OPENCV_YEAR}/opencv/opencv-cpp/${OPENCV_VERSION}/opencv-cpp-${OPENCV_VERSION}-headers.zip
)
fetchcontent_makeavailable(opencv_header)

file(GLOB_RECURSE OPENCV_LIB_PATH
    "${opencv_lib_SOURCE_DIR}/**/*.so*"
    "${opencv_lib_SOURCE_DIR}/**/*.a"
)
set(OPENCV_INCLUDE_PATH ${opencv_header_SOURCE_DIR})

# ============================================================
# Include directories
# ============================================================

include_directories(
    ${PROJECT_SOURCE_DIR}/src/main/native/include
    ${PYLON_ROOT}/include
    ${OPENCV_INCLUDE_PATH}
    ${JNI_INCLUDE_DIRS}
)

# ============================================================
# Source files
# ============================================================

file(GLOB_RECURSE BASLERJNI_SOURCES
    src/main/native/cpp/*.cpp
    src/main/native/include/*.h
)

# ============================================================
# Create shared library
# ============================================================

add_library(baslerjni SHARED ${BASLERJNI_SOURCES})

target_link_directories(baslerjni PRIVATE ${PYLON_ROOT}/lib ${PYLON_ROOT}/lib64)

target_link_libraries(
    baslerjni
    PRIVATE
        Threads::Threads
        pylonbase
        pylonutility
        ${OPENCV_LIB_PATH}
        ${JNI_LIBRARIES}
)

# ============================================================
# Output
# ============================================================

set_target_properties(baslerjni PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/cmake_build"
    OUTPUT_NAME "baslerjni"
)

# ============================================================
# Optional: Print configuration info
# ============================================================

message(STATUS "Building Basler JNI with:")
message(STATUS "  PYLON_ROOT: ${PYLON_ROOT}")
message(STATUS "  JNI_INCLUDE_DIRS: ${JNI_INCLUDE_DIRS}")
message(STATUS "  OPENCV_INCLUDE_PATH: ${OPENCV_INCLUDE_PATH}")
message(STATUS "  Sources: ${BASLERJNI_SOURCES}")
