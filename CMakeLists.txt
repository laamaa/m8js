cmake_minimum_required(VERSION 3.10)

# Project name and version
project(m8js VERSION 0.1)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSERIALPORT REQUIRED libserialport)

# Specify source files
set(SOURCE_FILES
        src/main.c
        src/serial.c
        src/slip.c
        src/command.c
        src/command.c
        src/virtualjoystick.c
        src/include/virtualjoystick.h
        # Add more source files here
)

# Add executable target
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE src/include)

set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_RELEASE "-O2")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build, options are: Debug Release" FORCE)
endif()

# Output directories for different build types
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/release)

# Add link-time optimization and other release-specific options
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

target_link_libraries(${PROJECT_NAME} ${LIBSERIALPORT_LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC ${LIBSERIALPORT_INCLUDE_DIRS})
target_compile_options(${PROJECT_NAME} PUBLIC ${LIBSERIALPORT_CFLAGS_OTHER})


