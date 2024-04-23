cmake_minimum_required(VERSION 3.27)
project(minecraft C)

set(CMAKE_C_STANDARD 23)

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug)
endif ()

find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)

add_compile_options(-Wall -Wextra)

set(CMAKE_C_FLAGS_DEBUG "-fsanitize=address -g -Og")
set(CMAKE_LINK_FLAGS_DEBUG "-fsanitize=address")

set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")

add_executable(minecraft
	main.c
	varint.c
	packet.c
	minecraft.h
	status.c
	hexdump.c
	connection.c
	login.c
	crypto.c
	json.c
	nbt.h
	nbt.c
	play.c
	server.c
	world.c
)
target_link_libraries(minecraft OpenSSL::Crypto Threads::Threads ZLIB::ZLIB)

add_executable(nbt_test
	hexdump.c
	nbt.c
	nbt_test.c
)
