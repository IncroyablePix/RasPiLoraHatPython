cmake_minimum_required(VERSION 3.16)
project(LoRaDraginoDriver)

find_library(WIRINGPI_LIB wiringPi)

# Set static library

add_library("${PROJECT_NAME}_static" STATIC Wiring.c LoRaCommunicator.c)
add_library("${PROJECT_NAME}" SHARED Wiring.c LoRaCommunicator.c)

message("${WIRINGPI_LIB}")
target_link_libraries(${PROJECT_NAME} PUBLIC ${WIRINGPI_LIB})

install(TARGETS "${PROJECT_NAME}" DESTINATION /usr/local/opt/LoRaDraginoDriver/lib)
install(TARGETS "${PROJECT_NAME}_static" DESTINATION /usr/local/opt/LoRaDraginoDriver/lib)
install(FILES LoRaCommunicator.h Sf.h Sx.h Bytes.h Wiring.h
        DESTINATION /usr/local/opt/LoRaDraginoDriver/include)

