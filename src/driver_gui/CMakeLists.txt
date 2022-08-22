cmake_minimum_required(VERSION 3.18)
project(DriverGUI VERSION 1.0.0)

find_package(PahoMqttCpp REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(GTK REQUIRED gtkmm-3.0)

pkg_check_modules(GTK REQUIRED gtkmm-3.0)


INCLUDE_DIRECTORIES(${GTK_INCLUDE_DIRS})
LINK_DIRECTORIES(${GTK_LIBRARY_DIRS})

ADD_DEFINITIONS(${GTK_CFLAGS_OTHER})

add_executable(DriverGUI main.cpp)
target_link_libraries(DriverGUI ${GTKMM_LIBRARIES})
TARGET_LINK_LIBRARIES(DriverGUI ${GTK_LIBRARIES})

target_include_directories(DriverGUI PRIVATE ${GTKMM_INCLUDE_DIRS})
target_link_directories(DriverGUI PRIVATE ${GTKMM_LIBRARY_DIRS})
target_link_libraries(DriverGUI PahoMqttCpp::paho-mqttpp3)

target_compile_definitions(DriverGUI PRIVATE ${GTKMM_CFLAGS_OTHER})