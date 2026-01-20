# Explicitly specify C++ source files
FILE(GLOB_RECURSE app_sources 
    ${CMAKE_SOURCE_DIR}/src/*.cpp
    ${CMAKE_SOURCE_DIR}/lib/*/*.cpp
)

idf_component_register(
    SRCS ${app_sources}
    INCLUDE_DIRS "."
                 "${CMAKE_SOURCE_DIR}/include"
                 "${CMAKE_SOURCE_DIR}/lib/Debug"
                 "${CMAKE_SOURCE_DIR}/lib/Logger"
                 "${CMAKE_SOURCE_DIR}/lib/UART"
                 "${CMAKE_SOURCE_DIR}/lib/WebServer"
                 "${CMAKE_SOURCE_DIR}/lib/WiFi"
)
