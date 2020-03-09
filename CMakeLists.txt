cmake_minimum_required(VERSION 3.15)
project(COMP4981Server)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(rapidjson)
include_directories(rapidjson/error)
include_directories(rapidjson/internal)
include_directories(rapidjson/msinttypes)

add_executable(COMP4981Server
        rapidjson/error/en.h
        rapidjson/error/error.h
        rapidjson/internal/biginteger.h
        rapidjson/internal/clzll.h
        rapidjson/internal/diyfp.h
        rapidjson/internal/dtoa.h
        rapidjson/internal/ieee754.h
        rapidjson/internal/itoa.h
        rapidjson/internal/meta.h
        rapidjson/internal/pow10.h
        rapidjson/internal/regex.h
        rapidjson/internal/stack.h
        rapidjson/internal/strfunc.h
        rapidjson/internal/strtod.h
        rapidjson/internal/swap.h
        rapidjson/msinttypes/inttypes.h
        rapidjson/msinttypes/stdint.h
        rapidjson/allocators.h
        rapidjson/cursorstreamwrapper.h
        rapidjson/document.h
        rapidjson/encodedstream.h
        rapidjson/encodings.h
        rapidjson/filereadstream.h
        rapidjson/filewritestream.h
        rapidjson/fwd.h
        rapidjson/istreamwrapper.h
        rapidjson/memorybuffer.h
        rapidjson/memorystream.h
        rapidjson/ostreamwrapper.h
        rapidjson/pointer.h
        rapidjson/prettywriter.h
        rapidjson/rapidjson.h
        rapidjson/reader.h
        rapidjson/schema.h
        rapidjson/stream.h
        rapidjson/stringbuffer.h
        rapidjson/writer.h
        ConnectionType.cpp
        ConnectivityManager.cpp
        ConnectivityManager.hpp
        messageFunctions.c
        messageFunctions.h
        tcpServer.cpp)
