cmake_minimum_required(VERSION 3.18)
project(CPP_Concurrency)

set(CMAKE_CXX_STANDARD 17)

add_executable(CPP_Concurrency main.cpp chapter3/dnsCache.cpp chapter4/simplifiedImplementationOfConditionVariable.cpp chapter4/threadsafeQueue.hpp chapter4/threadsafeQueue.cpp chapter4/packagedTask.cpp)
