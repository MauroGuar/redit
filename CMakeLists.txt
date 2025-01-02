# Minimum CMake version required
cmake_minimum_required(VERSION 3.22.1)

# Project name and language
project(redit LANGUAGES C)

# Set the C standard
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Set build type to Release by default if not specified
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug, Release, RelWithDebInfo, MinSizeRel)" FORCE)
endif ()

# Configure compiler flags for Release and Debug
set(CMAKE_C_FLAGS_DEBUG "-g -Wall -Wextra -Wpedantic")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG -Wall -Wextra -Wpedantic")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG -Wall -Wextra -Wpedantic")
set(CMAKE_C_FLAGS_MINSIZEREL "-Os -DNDEBUG -Wall -Wextra -Wpedantic")

# Specify the output directories for build artifacts
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Add the executable and its sources
add_executable(
        redit
        src/main.c
        src/error_handler.c
        src/flags_handler.c
        src/paths_handler.c
        src/modes_handler.c
        src/file_utils.c
        src/file_operations.c
)

# Include directories for headers
target_include_directories(redit PRIVATE include)

# Add cargs library (subdirectory)
add_subdirectory(lib/cargs)

# Link the cargs library to the project
target_link_libraries(redit PRIVATE cargs)

# Enable stricter warnings and useful debug/release flags
target_compile_options(redit PRIVATE
        $<$<CONFIG:DEBUG>:-g -Og -Wall -Wextra -Wpedantic>
        $<$<CONFIG:RELEASE>:-O3 -DNDEBUG -Wall -Wextra -Wpedantic>
)

# Install the executable for system-wide usage
install(TARGETS redit RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)