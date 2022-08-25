#include "mqtt/client.h"
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <signal.h>
#include <sstream>
#include <string>
#include <unistd.h>

//temporary cli painting arrays
char indicators_array[4][3][8]{{{'/', '-', '-', '-', '-', '-', '-', '|'},
                                {'|', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
                                {'|', '-', '-', '-', '-', '-', '-', '|'}},
                               {{'/', '-', '-', '-', '-', '-', '-', '|'},
                                {'|', ' ', '#', '#', '#', '#', ' ', '|'},
                                {'|', '-', '-', '-', '-', '-', '-', '|'}},
                               {{'|', '-', '-', '-', '-', '-', '-', '\\'},
                                {'|', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
                                {'|', '-', '-', '-', '-', '-', '-', '|'}},
                               {{'|', '-', '-', '-', '-', '-', '-', '\\'},
                                {'|', ' ', '#', '#', '#', '#', ' ', '|'},
                                {'|', '-', '-', '-', '-', '-', '-', '|'}}};

std::string homeDir;

const int QOS = 1;

const std::string SERVER_ADDRESS{"tcp://localhost:1883"};
const std::string CLIENT_ID{"direction_indicators_control_app"};
const std::string TOPIC{"/car/indicator/"};

// Strunct stores current state of indicators
struct Indicators {
    bool left   = false;
    bool right  = false;
    bool hazard = false;

    void setLeft(bool value)
    {
        left = value;

        if (right == true)
            right = !value;
    }

    void setRight(bool value)
    {
        right = value;

        if (left == true)
            left = !value;
    }

    void setHazard(bool value)
    {
        hazard = value;
    }
} indicators;

// Function select action to be executed
//
// topic - topic from which the message was received
// payload - message payload
void selectAction(std::string topic, std::string payload)
{
    //writeOnfile("received message: \"" + payload + "\" on topic: " + topic + "\n");

    std::string text      = "received message: \"" + payload + "\" on topic: " + topic + "\n";
    std::string direction = topic.erase(0, 15);

    std::map<std::string, int> const topic_cast    = {{"left", 0}, {"right", 1}, {"hazard", 2}};
    std::map<std::string, bool> const payload_cast = {{"off", false}, {"on", true}};

    switch (topic_cast.find(direction)->second) {
        case 0:
            indicators.setLeft(payload_cast.find(payload)->second);
            break;
        case 1:
            indicators.setRight(payload_cast.find(payload)->second);
            break;
        case 2:
            indicators.setHazard(payload_cast.find(payload)->second);
            break;
    }
}

// Function draws indicators from indicator array
//
// state - chooses whether to keep the lights on or not
void drawIndicators(bool state)
{

    std::cout << u8"\033[2J\033[1;1H";

    for (short i = 0; i < 3; i++) {
        for (short j = 0; j < 19; j++) {
            if (j < 8) {
                if (indicators.hazard) {
                    std::cout << indicators_array[state][i][j];
                } else {
                    std::cout << indicators_array[indicators.left ? state : 0][i][j];
                }
            } else if (j < 11) {
                std::cout << ' ';
            } else {
                if (indicators.hazard) {
                    std::cout << indicators_array[state + 2][i][j - 11];
                } else {
                    std::cout << indicators_array[state ? 2 + indicators.right : 2][i][j - 11];
                }
            }
        }
        std::cout << "\n";
    }

    std::cout << "\n";
}

/*//writes info to log file
void writeOnfile (std::string text) {
    std::string path = homeDir + "/.local/share/autox.log";

    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%d-%m-%Y %H:%M");
    auto str = oss.str();

    text = "[" + str + "] [direction_indicators_control_app:" + CLIENT_ID + "] " + text;

    char* filetext = &text[0];
    std::ofstream myfile;
    myfile.open (path, std::fstream::app);
    
    myfile << filetext;
    myfile.close();
}*/

/*//control c handler
void my_handler(int s){
    writeOnfile("terminated by user.\n");
    exit(1);
}*/

// main
int main()
{

    /*homeDir = getenv("HOME"); 
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = my_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);*/

    mqtt::async_client cli(SERVER_ADDRESS, CLIENT_ID);

    auto connOpts = mqtt::connect_options_builder()
                        .clean_session(true)
                        .finalize();

    try {
        cli.start_consuming();

        std::cout << "Connecting to the MQTT server..." << std::flush;
        auto tok = cli.connect(connOpts);
        auto rsp = tok->get_connect_response();

        if (!rsp.is_session_present()) {
            cli.subscribe(TOPIC + "right", QOS)->wait();
            cli.subscribe(TOPIC + "left", QOS)->wait();
            cli.subscribe(TOPIC + "hazard", QOS)->wait();
        }

        std::cout << "Connected and redy to run" << std::endl;
        //writeOnfile("connected\n");

        bool state = false;

        while (true) {
            mqtt::const_message_ptr msg;

            if (cli.try_consume_message(&msg)) {
                if (msg == nullptr)
                    break;

                selectAction(msg->get_topic(), msg->get_payload());
            }

            drawIndicators(state);
            usleep(100000);
            state = !state;
        }

        if (cli.is_connected()) {
            std::cout << "\nShutting down and disconnecting from the MQTT server..." << std::flush;
            //writeOnfile("disconnected from the server.\n");
            cli.unsubscribe(TOPIC)->wait();
            cli.stop_consuming();
            cli.disconnect()->wait();
            std::cout << "OK" << std::endl;
        } else {
            std::cout << "\nClient was disconnected" << std::endl;
        }
    } catch (const mqtt::exception &exc) {
        std::cerr << "\n  " << exc << std::endl;
        //writeOnfile("crashed unexpectedly.\n")
        return 1;
    }

    return 0;
}