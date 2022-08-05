find_package(PahoMqttCpp REQUIRED)

add_executable(data_publish_example data_publish_example.cpp)

# target_link_libraries(data_publish_example paho-mqtt3a paho-mqttpp3)
target_link_libraries(data_publish_example PahoMqttCpp::paho-mqttpp3)
