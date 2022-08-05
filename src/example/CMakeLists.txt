find_package(PahoMqttCpp REQUIRED)

add_executable(data_publish data_publish.cpp)

# target_link_libraries(data_publish paho-mqtt3a paho-mqttpp3)
target_link_libraries(data_publish PahoMqttCpp::paho-mqttpp3)
