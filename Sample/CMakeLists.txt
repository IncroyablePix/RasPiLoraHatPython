cmake_minimum_required(VERSION 3.16)
project(LoRaDriverSample)

add_executable(${PROJECT_NAME} main.c)

target_include_directories(${PROJECT_NAME}  PUBLIC ../LoRaDraginoDriver)
target_link_libraries(${PROJECT_NAME} PUBLIC
        LoRaDraginoDriver
)
