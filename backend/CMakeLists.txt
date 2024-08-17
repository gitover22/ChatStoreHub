# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project name
project(ChatStoreHub)

# Set C standard
set(CMAKE_C_STANDARD 11)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(MICROHTTPD REQUIRED libmicrohttpd)
pkg_check_modules(MYSQL REQUIRED mysqlclient)

# Include directories
include_directories(${MICROHTTPD_INCLUDE_DIRS})
include_directories(${MySQL_INCLUDE_DIRS})

# Source directories
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)

# Source files
set(LOGIN_SOURCES ${SRC_DIR}/login.c ${SRC_DIR}/log.c)
set(DIALOG_SOURCES ${SRC_DIR}/dialog.c ${SRC_DIR}/log.c)

# Add executable for login
add_executable(login ${LOGIN_SOURCES})

# Link libraries to login executable
target_link_libraries(login ${MICROHTTPD_LIBRARIES} /usr/lib/x86_64-linux-gnu/libmysqlclient.so)

# Add executable for dialog
add_executable(dialog ${DIALOG_SOURCES})

# Link libraries to dialog executable
target_link_libraries(dialog ${MICROHTTPD_LIBRARIES})

# Define the run.sh script
set(RUN_SCRIPT ${CMAKE_SOURCE_DIR}/execute.sh)

# Add a custom command to run the script after build
add_custom_command(
    TARGET login POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Running kill and launch script"
    COMMAND bash ${RUN_SCRIPT}
)

# Optionally, you can install binaries (if needed)
install(TARGETS login dialog RUNTIME DESTINATION bin)
