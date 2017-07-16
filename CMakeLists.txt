cmake_minimum_required(VERSION 3.7)
project(mpg)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES
        main.cpp
        mpg.hpp mpg.cpp mpg_view.cpp mpg_view.hpp mpg_model.cpp mpg_model.hpp mpg_controller.cpp mpg_controller.hpp defaul_values.hpp)

add_executable(mpg ${SOURCE_FILES})

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(Boost REQUIRED)

include_directories(${OPENGL_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
target_link_libraries(mpg glfw ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${Boost_LIBRARY_DIRS})
