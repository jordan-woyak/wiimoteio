project(simple-tests)

include_directories(../../include)

set(CMAKE_CXX_FLAGS "-Wall -std=gnu++0x")

add_executable(simple-tests main.cpp)
