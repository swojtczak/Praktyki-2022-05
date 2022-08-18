cmake_minimum_required(VERSION 3.18)
project(WheelControl VERSION 1.0.0)

find_package(PahoMqttCpp REQUIRED)

add_executable(WheelControl main.cpp)

# target_link_libraries(WheelControl paho-mqtt3a paho-mqttpp3)
target_link_libraries(WheelControl PahoMqttCpp::paho-mqttpp3)