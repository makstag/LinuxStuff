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

add_executable(${PROJECT_NAME}1 src/ipc1.c)
target_link_libraries(${PROJECT_NAME}1 ${SYSTEM_LIBS} tlpidist)

target_include_directories(${PROJECT_NAME}1 PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

add_executable(${PROJECT_NAME}2 src/ipc2.c)
target_link_libraries(${PROJECT_NAME}2 ${SYSTEM_LIBS} tlpidist)

target_include_directories(${PROJECT_NAME}2 PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
