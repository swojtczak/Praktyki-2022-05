#include "mqtt/client.h"
#include <array>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <signal.h>
#include <sstream>
#include <string>
#include <unistd.h>
#include <random>

std::map<int, char> values =
    {
        {-1, '\\'},
        {0, '|'},
        {1, '/'}};

const int QOS = 1;

const std::string SERVER_ADDRESS{"tcp://localhost:1883"};
std::string CLIENT_ID;
const std::string TOPIC{"/car/wheel/angle"};

std::string homeDir;

namespace uuid
{
static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_int_distribution<> dis(0, 15);
static std::uniform_int_distribution<> dis2(8, 11);

std::string generate_uuid_v4()
{
    std::stringstream ss;
    int i;
    ss << std::hex;
    for (i = 0; i < 8; i++) {
        ss << dis(gen);
    }
    ss << "-";
    for (i = 0; i < 4; i++) {
        ss << dis(gen);
    }
    ss << "-4";
    for (i = 0; i < 3; i++) {
        ss << dis(gen);
    }
    ss << "-";
    ss << dis2(gen);
    for (i = 0; i < 3; i++) {
        ss << dis(gen);
    }
    ss << "-";
    for (i = 0; i < 12; i++) {
        ss << dis(gen);
    };
    return ss.str();
}
} // namespace uuid

void writeOnfile(std::string text)
{
    std::string path = homeDir + "/.local/share/autox.log";

    auto t  = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%d-%m-%Y %H:%M");
    auto str = oss.str();

    text = "[" + str + "] [windows_control_app:" + CLIENT_ID + "] " + text;

    char *filetext = &text[0];
    std::ofstream myfile;
    myfile.open(path, std::fstream::app);

    myfile << filetext;
    myfile.close();
}

void my_handler(int s)
{
    writeOnfile("terminated by user.\n");
    exit(1);
}

int angle    = 0;
int maxAngle = 270;

double tempAngle;

void msgHandling(mqtt::const_message_ptr msg)
{
    tempAngle        = stoi(msg->to_string());
    angle            = (int)tempAngle;
    tempAngle        = std::round(tempAngle / maxAngle);
    std::string text = "received message: \"" + msg->to_string() + "\" on topic: " + msg->get_topic() + "\n";
    writeOnfile(text);
}

void drawWheels()
{
    std::cout << u8"\033[2J\033[1;1H";
    std::cout << angle << std::endl
              << std::endl;
    std::cout << values.at(tempAngle) << std::endl
              << std::endl;
}

int main()
{

    CLIENT_ID = uuid::generate_uuid_v4();

    homeDir = getenv("HOME");
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = my_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);

    mqtt::async_client cli(SERVER_ADDRESS, CLIENT_ID);

    auto connOpts = mqtt::connect_options_builder().clean_session(true).finalize();

    try {
        cli.start_consuming();

        std::cout << "Connecting to the MQTT server..." << std::flush;
        auto tok = cli.connect(connOpts);
        auto rsp = tok->get_connect_response();

        if (!rsp.is_session_present()) {
            cli.subscribe(TOPIC, QOS)->wait();
        }

        writeOnfile("connected\n");

        while (!false) {
            mqtt::const_message_ptr msg;

            if (cli.try_consume_message(&msg)) {
                if (msg == nullptr)
                    break;
                msgHandling(msg);
            }

            drawWheels();
            std::cout.flush();
            usleep(5000);
        }

        if (cli.is_connected()) {
            std::cout << "\nShutting down and disconnecting from the MQTT server..." << std::flush;
            cli.unsubscribe(TOPIC)->wait();
            cli.stop_consuming();
            cli.disconnect()->wait();
            std::cout << "OK" << std::endl;
            writeOnfile("disconnected from the server\n");
        } else {
            std::cout << "\nClient was disconnected" << std::endl;
        }
    } catch (const mqtt::exception &exc) {
        std::cerr << "\n  " << exc << std::endl;
        writeOnfile("unexpected error has occured\n");
        return 1;
    }
    return 0;
}