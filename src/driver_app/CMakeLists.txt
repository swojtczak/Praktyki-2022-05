cmake_minimum_required(VERSION 3.18)
project(Driver VERSION 1.0.0)

find_package(PahoMqttCpp REQUIRED)

add_executable(Driver main.cpp prompt.cpp)

# target_link_libraries(Driver paho-mqtt3a paho-mqttpp3)
target_link_libraries(Driver PahoMqttCpp::paho-mqttpp3)