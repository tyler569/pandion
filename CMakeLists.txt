cmake_minimum_required(VERSION 3.27)
project(minecraft C)

set(CMAKE_C_STANDARD 23)

find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)

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
)

target_link_libraries(minecraft OpenSSL::Crypto Threads::Threads ZLIB::ZLIB)
