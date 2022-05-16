cmake_minimum_required(VERSION 3.10)

# set the project name
project(TestActsCore)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# add the executable
add_executable(TestActsCore main.cpp)
target_compile_options(TestActsCore PRIVATE -stdpar=gpu)
target_link_options(TestActsCore PRIVATE -stdpar=gpu)
target_compile_options(TestActsCore PRIVATE -gpu=managed:intercept)
target_link_options(TestActsCore PRIVATE -gpu=managed:intercept)

find_package( Eigen3 REQUIRED )

add_subdirectory( extern/acts )
target_link_libraries(TestActsCore PUBLIC ActsCore)
