cmake_minimum_required(VERSION 3.27)
project(minecraft C)

set(CMAKE_C_STANDARD 23)

find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)

add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

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
