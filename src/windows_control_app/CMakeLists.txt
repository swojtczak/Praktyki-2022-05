cmake_minimum_required(VERSION 3.18)
project(WindowsControl VERSION 1.0.0)

find_package(PahoMqttCpp REQUIRED)

add_executable(WindowsControl main.cpp)

# target_link_libraries(WindowsControl paho-mqtt3a paho-mqttpp3)
target_link_libraries(WindowsControl PahoMqttCpp::paho-mqttpp3)