cmake_minimum_required(VERSION 3.18)
project(WiperControl VERSION 1.0.0)

find_package(PahoMqttCpp REQUIRED)

add_executable(WiperControl main.cpp)

target_link_libraries(WiperControl PahoMqttCpp::paho-mqttpp3)