cmake_minimum_required(VERSION 3.28)
project(final_project)

set(CMAKE_CXX_STANDARD 17)

add_executable(final_project serverM.cpp
        client.cpp
        client.h
        serverC.cpp
        serverC.h
        serverEEB.cpp
        serverEEB.h
        serverRTH.cpp
        serverRTH.h
        serverC.h)
