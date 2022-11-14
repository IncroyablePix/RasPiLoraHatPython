# Python module CMakeLists
cmake_minimum_required(VERSION 3.16)
project(LoRaDraginoPython)

# Find the Python interpreter
find_package(PythonInterp 3.6 REQUIRED)

# Find the Python libraries
find_package(PythonLibs 3.6 REQUIRED)

add_custom_target(
    ${PROJECT_NAME} ALL
    COMMAND pip3 install .
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Building and installing ${PROJECT_NAME}"
)