cmake_minimum_required(VERSION 3.18)
project(LogApp VERSION 1.0.0)
find_package(PahoMqttCpp REQUIRED)

add_executable(LogApp main.cpp)

target_link_libraries(LogApp PahoMqttCpp::paho-mqttpp3)