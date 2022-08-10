cmake_minimum_required(VERSION 3.18)
project(WindowsControll VERSION 1.0.0)

find_package(PahoMqttCpp REQUIRED)

add_executable(WindowsControll main.cpp)

# target_link_libraries(WindowsControll paho-mqtt3a paho-mqttpp3)
target_link_libraries(WindowsControll PahoMqttCpp::paho-mqttpp3)