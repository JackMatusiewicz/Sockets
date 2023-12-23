cmake_minimum_required(VERSION 3.27)
project(Sockets)

set(CMAKE_CXX_STANDARD 20)

add_executable(Sockets main.cpp
        lib/SocketType.h
        lib/SocketInfo.h
        lib/ConnectedSocketClient.h
        lib/result/Result.h
)
