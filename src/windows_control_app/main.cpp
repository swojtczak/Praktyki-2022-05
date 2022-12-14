#include "mqtt/client.h"
#include <array>
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

int moved = 0;
int val;
int windowState;

std::map<std::string, int> values =
    {
        {"down", -1},
        {"stop", 0},
        {"up", 1}};

std::array<int, 4> szyby       = {0, 0, 0, 0};
std::array<int, 4> szybyFuture = {0, 0, 0, 0};
const int QOS                  = 1;

const std::string SERVER_ADDRESS{"tcp://localhost:1883"};
std::string CLIENT_ID;
const std::string TOPIC{"/car/window/"};

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

void msgHandling(mqtt::const_message_ptr msg)
{
    std::string topic   = std::string(msg->get_topic());
    int temp            = topic[12] - 48;
    int valTemp         = szybyFuture[temp];
    std::string content = std::string(msg->to_string());
    std::string text    = "received message: \"" + content + "\" on topic: " + msg->get_topic() + "\n";
    writeOnfile(text);

    try {
        valTemp = values.at(content);
    } catch (const std::out_of_range &oor) {
    }
    szybyFuture[temp] = valTemp;
}

void drawWindows()
{
    std::cout << u8"\033[2J\033[1;1H";
    for (int i = 0; i < 10; i++) {
        for (int x = 0; x < 4; x++) {
            if ((i - szyby[x]) > 0) {
                std::cout << "  |||||  ";
            } else {
                std::cout << "         ";
            }
        }
        std::cout << "\n";
    }
}

void my_handler(int s)
{
    writeOnfile("terminated by user.\n");
    exit(1);
}

int main()
{
    CLIENT_ID = uuid::generate_uuid_v4();

    mqtt::async_client cli(SERVER_ADDRESS, CLIENT_ID);

    auto connOpts = mqtt::connect_options_builder().clean_session(true).finalize();

    homeDir = getenv("HOME");
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = my_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);

    try {
        cli.start_consuming();

        auto tok = cli.connect(connOpts);
        auto rsp = tok->get_connect_response();

        for (int i = 0; i < 4; i++) {
            cli.subscribe(TOPIC + std::to_string(i), QOS)->wait();
        }

        writeOnfile("connected\n");

        while (!false) {
            mqtt::const_message_ptr msg;

            if (cli.try_consume_message(&msg)) {
                if (msg == nullptr)
                    break;
                msgHandling(msg);
            }

            drawWindows();
            std::cout.flush();
            sleep(1);
            for (int x = 0; x < 4; x++) {
                val         = szybyFuture[x];
                windowState = szyby[x];
                if (val != 0) {
                    szyby[x] = szyby[x] - (szybyFuture[x] / abs(szybyFuture[x]));
                    if (szyby[x] > 9 || szyby[x] < 0) {
                        szyby[x]       = windowState;
                        szybyFuture[x] = 0;
                    }
                }
            }
        }

        if (cli.is_connected()) {
            std::cout << "\nShutting down and disconnecting from the MQTT server..." << std::flush;
            cli.unsubscribe(TOPIC)->wait();
            cli.stop_consuming();
            cli.disconnect()->wait();
            std::cout << "OK" << std::endl;
            writeOnfile("disconnected from the server.\n");
        } else {
            std::cout << "\nClient was disconnected" << std::endl;
        }
    } catch (const mqtt::exception &exc) {
        std::cerr << "\n  " << exc << std::endl;
        writeOnfile("crashed unexpectedly.\n");
        return 1;
    }
    return 0;
}