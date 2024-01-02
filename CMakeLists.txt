cmake_minimum_required(VERSION 3.27.6)
project(ipc LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 17)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUC OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "^MINGW")
    set(SYSTEM_LIBS -lstdc++)
else()
    set(SYSTEM_LIBS)
endif()

add_library(tlpidist STATIC IMPORTED)
set_target_properties(tlpidist PROPERTIES
    IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/tlpi-dist/libtlpi.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/tlpi-dist/lib"
)

add_executable(server src/server.c)
target_link_libraries(server ${SYSTEM_LIBS} tlpidist)

#target_include_directories(server PUBLIC
#  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#  $<INSTALL_INTERFACE:include>
#)

add_executable(client src/client.c)
target_link_libraries(client ${SYSTEM_LIBS} tlpidist)

#target_include_directories(client PUBLIC
#  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#  $<INSTALL_INTERFACE:include>
#)
