cmake_minimum_required(VERSION 3.10)
project(CapiGradMIE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wno-nonnull")

# OpenSSL qutubxonasi (Binance WebSocket uchun)
add_definitions(-DUSE_OPENSSL_NATIVE)

include_directories(include)

add_executable(mie 
    src/data.cpp
    src/nn.cpp
    src/features.cpp
    src/model.cpp
    src/live.cpp
    src/trainer.cpp
    src/main.cpp
)

find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    target_link_libraries(mie OpenSSL::SSL OpenSSL::Crypto pthread)
else()
    target_link_libraries(mie pthread)
endif()