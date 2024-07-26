cmake_minimum_required(VERSION 3.28)
project(myFTA C)

set(CMAKE_C_STANDARD 11)

add_executable(myFTAClient
        Client.c
        CommonFunction/TransferFunction.c
        CommonFunction/TransferFunction.h)
add_executable(myFTAServer
        Server.c
        ServerFunction/StartServer.c
        CommonFunction/TransferFunction.c
        CommonFunction/TransferFunction.h)
