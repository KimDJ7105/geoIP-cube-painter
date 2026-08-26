cmake_minimum_required(VERSION 3.15)
project(GeoIPGame)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(unofficial-usockets CONFIG REQUIRED)
find_package(unofficial-uwebsockets CONFIG REQUIRED)
find_package(maxminddb CONFIG REQUIRED)
find_package(ZLIB REQUIRED)

add_executable(Server
    src/main.cpp
    src/GeoIPManager.cpp
    src/GameServer.cpp
    src/CountryCentroids.cpp
)

target_include_directories(Server PRIVATE 
    /home/cheese/vcpkg/installed/x64-linux/include
    include
)

# maxminddb를 원시 문자열이 아닌 공식 타겟 명칭(maxminddb::maxminddb)으로 지정하여 경로 유실을 막습니다.
target_link_libraries(Server PRIVATE
    unofficial::uwebsockets::uwebsockets
    unofficial::usockets::usockets
    maxminddb::maxminddb
    ZLIB::ZLIB
)
