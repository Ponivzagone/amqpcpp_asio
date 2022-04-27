cmake_minimum_required(VERSION 3.17)

project(amqp-test VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BOOST_USE_STATIC_LIBS ON)
find_package(Boost 1.79 COMPONENTS system program_options REQUIRED)

set(AMQPCPP_ROOT "C:/lib/amqpcpp")
set(AMQPCPP_INCLUDE_DIR ${AMQPCPP_ROOT}/include)
set(AMQPCPP_LIBRARIES_DIR ${AMQPCPP_ROOT}/lib)

include_directories(include ${AMQPCPP_INCLUDE_DIR})

add_executable(producer producer.cpp src/amqp_asio_handler.cpp)
target_link_libraries(producer ${Boost_LIBRARIES} ${AMQPCPP_LIBRARIES_DIR}/libamqpcpp.a ws2_32)

add_executable(consumer consumer.cpp src/amqp_asio_handler.cpp)
target_link_libraries(consumer ${Boost_LIBRARIES} ${AMQPCPP_LIBRARIES_DIR}/libamqpcpp.a ws2_32)
