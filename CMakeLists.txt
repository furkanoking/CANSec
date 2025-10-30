cmake_minimum_required(VERSION 3.25)
project(CANSecKod)

set(CMAKE_CXX_STANDARD 23)

add_library(CANSecKod STATIC CANSec.cpp)
