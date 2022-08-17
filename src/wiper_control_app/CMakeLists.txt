cmake_minimum_required(VERSION 3.18)
project(WiperControll VERSION 1.0.0)

find_package(PahoMqttCpp REQUIRED)

add_executable(WiperControll main.cpp)

target_link_libraries(WiperControll PahoMqttCpp::paho-mqttpp3)