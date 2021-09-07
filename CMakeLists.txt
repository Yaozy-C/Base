cmake_minimum_required(VERSION 3.16)
project(Base)

set(CMAKE_CXX_STANDARD 17)

include_directories(MiddleWare)
include_directories(Net)
include_directories(Public)
include_directories(Thread)

add_executable(Base
#        MiddleWare/Redis.cpp
#        MiddleWare/Redis.h
        Net/Acceptor.cpp
        Net/Acceptor.h
        Net/CallBack.h
        Net/Connection.cpp
        Net/Connection.h
        Net/ConnectManager.cpp
        Net/ConnectManager.h
        Net/Epoller.cpp
        Net/Epoller.h
        Net/Event.h
        Net/InetAddress.cpp
        Net/InetAddress.h
        Net/Socket.cpp
        Net/Socket.h
        Net/SocketOpt.cpp
        Net/SocketOpt.h
        Net/TcpServer.cpp
        Net/TcpServer.h
        Net/Timer.cpp
        Net/Timer.h
        Public/Buffer.cpp
        Public/Buffer.h
        Public/cJSON.c
        Public/cJSON.h
#        Public/Curl.cpp
#        Public/Curl.h
        Public/DataPacket.cpp
        Public/DataPacket.h
        Public/Log.cpp
        Public/Log.h
        Public/Tools.cpp
        Public/Tools.h
        Thread/IndependentThread.h
        Thread/IndependentThreadPool.h
        Thread/Pool.cpp
        Thread/Pool.h
        Thread/SafeQue.h
        main.cpp)

target_link_libraries(Base -lpthread)
