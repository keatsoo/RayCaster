cmake_minimum_required(VERSION 3.0.0)
project(KitRayCaster VERSION 0.1.0)

include(CTest)
enable_testing()

set(SDL2_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/include")

set(SDL2_DIR "SDL2/lib/cmake/SDL2")
find_package(SDL2 REQUIRED CONFIG)
include_directories(${SDL2_INCLUDE_DIRS})

set(SDL2_TTF_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/include")

set(SDL2_TTF_DIR "SDL2_TTF/lib/cmake/SDL2_TTF")
find_package(SDL2_TTF REQUIRED CONFIG PATHS ./SDL2_TTF/)
link_directories(${SDL2_TTF_LIBRARY_DIRS})
include_directories(${SDL2_TTF_INCLUDE_DIRS})

add_executable(KitRayCaster src/main.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
target_link_libraries(KitRayCaster ${SDL2_LIBRARIES} ${SDL2_TTF_LIBRARIES})
