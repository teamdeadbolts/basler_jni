cmake_minimum_required(VERSION 3.16)
project(baslerjni LANGUAGES CXX)

# ============================================================
# Build configuration
# ============================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT PYLON_ROOT)
    set(PYLON_ROOT "/opt/pylon" CACHE PATH "Basler Pylon SDK root directory")
endif()


find_package(JNI REQUIRED)

include_directories(
    ${JNI_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/src/main/native/include
    ${PYLON_ROOT}/include
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

# ============================================================
# Link Basler Pylon SDK
# ============================================================

target_link_directories(baslerjni PRIVATE ${PYLON_ROOT}/lib ${PYLON_ROOT}/lib64)

target_link_libraries(baslerjni
    ${JNI_LIBRARIES}
    pylonbase
    pylonutility
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
message(STATUS "  Sources: ${BASLERJNI_SOURCES}")
