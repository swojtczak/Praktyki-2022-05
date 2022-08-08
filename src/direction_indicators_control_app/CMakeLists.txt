cmake_minimum_required(VERSION 3.18)
project(DirectionIndicatorControl VERSION 1.0.0)
find_package(PahoMqttCpp REQUIRED)

add_executable(DirectionIndicatorControl main.cpp callback.cpp action_listener.cpp)

target_link_libraries(DirectionIndicatorControl PahoMqttCpp::paho-mqttpp3)