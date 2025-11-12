cmake_minimum_required(VERSION 3.25)
project(CANSecKod)

set(CMAKE_CXX_STANDARD 23)

add_library(CANSecKod STATIC CANSec.cpp
        CANFDStructure/CANFD.cpp)

target_include_directories(CANSecKod PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/CANFDStructure)