# CMakeList.txt : CMake project for raytracer, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (raytracer "src/raytracer.cpp" "src/tgaimage.cpp" "src/raytracer.h")

# TODO: Add tests and install targets if needed.
