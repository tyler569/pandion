cmake_minimum_required(VERSION 3.27)
project(minecraft C)

set(CMAKE_C_STANDARD 23)

add_executable(minecraft main.c
        varint.c
        packet.c
        minecraft.h
        handshake.c
        hexdump.c)
