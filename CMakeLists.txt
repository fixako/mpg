cmake_minimum_required(VERSION 3.7)
project(mpg)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES
        main.cpp
	mpg_view.cpp mpg_view.hpp mpg_model.cpp mpg_model.hpp mpg_controller.cpp mpg_controller.hpp)

add_executable(mpg ${SOURCE_FILES})

find_package(OpenGL REQUIRED)
find_package(Boost REQUIRED)

set(GLFW_INCLUDE_DIRS C:/local/glfw-3.2.1.bin.WIN32/include)
set(GLFW_LIBRARIES C:/local/glfw-3.2.1.bin.WIN32/lib-vc2015/glfw3.lib)

set(GLEW_INCLUDE_DIRS C:/local/glew-2.0.0/include)
set(GLEW_LIBRARIES C:/local/glew-2.0.0/lib/Release/Win32/glew32.lib)
set(GLM_INCLUDE_DIRS C:/local/glm)

include_directories(${OPENGL_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS})
target_link_libraries(mpg ${OPENGL_LIBRARIES} ${Boost_LIBRARY_DIRS} ${GLEW_LIBRARIES} ${GLFW_LIBRARIES})
