cmake_minimum_required(VERSION 3.26.4)
project(SR VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME}
  src/main.cpp
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
  "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

target_include_directories(${PROJECT_NAME}
  PRIVATE ${PROJECT_SOURCE_DIR}/include
  PUBLIC ${PROJECT_SOURCE_DIR}/src
  PUBLIC /opt/homebrew/Cellar/sdl2/2.28.1/include
  PUBLIC /opt/homebrew/Cellar/glm/0.9.9.8/include
)

target_link_libraries(${PROJECT_NAME}
  ${SDL2_LIBRARIES}
)
