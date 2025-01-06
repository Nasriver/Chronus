cmake_minimum_required(VERSION 3.10)
project(Backtest_cpp VERSION 0.0.1 LANGUAGES CXX)

include(CTest)
enable_testing()
list(APPEND CMAKE_PREFIX_PATH "/usr/local/HDF_Group/HDF5/1.14.5/")
add_subdirectory(external/HighFive)

find_package(MPI REQUIRED)

add_executable(Backtest_cpp 
BunnyGirl.cpp
Participant.hpp
Participant.cpp
Broker.hpp
Broker.cpp
DataHandler.hpp
DataHandler.cpp
Events.hpp
Events.cpp
Helper.hpp
Helper.cpp
)

target_link_libraries(Backtest_cpp MPI::MPI_CXX HighFive)
set_property(TARGET Backtest_cpp PROPERTY CXX_STANDARD 17)

