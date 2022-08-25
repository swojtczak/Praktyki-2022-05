#include "mqtt/client.h"
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mqtt/message.h>
#include <signal.h>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <random>

std::string homeDir;

//std::cout<< u8"\033[2J\033[1;1H"; - ANSI Escape sequence clearing terminal

const std::string SERVER_ADDRESS{"tcp://localhost:1883"};
const std::string TOPIC_WF{"/car/wipers/front"};
const std::string TOPIC_WB{"/car/wipers/back"};

const int QOS = 1;

enum modes { frontOn,
             frontOnce,
             frontFluidOnce,
             frontFluid,
             frontOff,
             backOn,
             backOnce,
             backFluidOnce,
             backFluid,
             backOff 
            };

enum infoFromBroker { on,
                      off,
                      once,
                      fluid,
                      fluid_once 
                    };
                    
std::map<std::string, infoFromBroker> infoFromBrokerMap;

bool stopWiping;
int mode = 0;
bool turnedOn;
bool rerender = true;
bool blockLoop;

std::string storedComm, storedTopic;

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

mqtt::async_client cli(SERVER_ADDRESS, uuid::generate_uuid_v4());

/*void writeOnfile(std::string text)
{
    std::string path = homeDir + "/.local/share/autox.log";

    auto t  = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%d-%m-%Y %H:%M");
    auto str = oss.str();

    text = "[" + str + "] [wipers_control_app:" + CLIENT_ID + "] " + text;

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
}*/

//Function drawing wipers depending on state passed as argument
void drawWipers(int state, bool sprinklersOn)
{
    int lenghtOfWipers = 6;
    switch (state) {

        //Wipers don't move, facing left
        case 0:
            if (!sprinklersOn) {
                for (short spacing = 0; spacing < lenghtOfWipers - 1; spacing++)
                    std::cout << "\n";
            }
            std::cout << "<";
            for (short count = 0; count <= lenghtOfWipers * 2 + 1; count++) {
                std::cout << "--";

                if (count == lenghtOfWipers)
                    std::cout << " <";
            }

            std::cout << std::endl;
            break;

        //Wipers move, tilted to the left
        case 1:
            for (short count = 0; count < lenghtOfWipers; count++) {
                for (short spacing = 0; spacing < count; spacing++)
                    std::cout << " ";
                std::cout << "       \\";
                std::cout << "          \\" << std::endl;
            }
            break;

        //Wipers move, stand vertically
        case 2:
            for (short count = 0; count < lenghtOfWipers; count++)
                std::cout << "             |          |" << std::endl;

            break;

        //Wipers move, tilted to the right
        case 3:
            for (short count = 0; count < lenghtOfWipers; count++) {
                for (short spacing = lenghtOfWipers * 2; spacing > count; spacing--)
                    std::cout << " ";
                std::cout << "       /";
                std::cout << "          /" << std::endl;
            }
            break;

        //Wipers don't move, facing right
        case 4:
            for (short spacing = 0; spacing < lenghtOfWipers - 1; spacing++)
                std::cout << "\n";
            std::cout << "             ";
            for (short count = 0; count <= lenghtOfWipers * 2 + 1; count++) {

                std::cout << "--";

                if (count == lenghtOfWipers)
                    std::cout << "> ";
            }

            std::cout << ">" << std::endl;
            break;
    }
}

//Function animating wipers
void wipe(short mode)
{
    short wiperState = 0;

    if (mode == frontOnce || mode == frontFluidOnce || mode == backOnce || mode == backFluidOnce){
        stopWiping = true;
        blockLoop = true;
    }
    else blockLoop = false;

    //Wipers with sprinklers
    if (mode == frontFluidOnce || mode == frontFluid || mode == backFluidOnce || mode == backFluid) {
        int count           = 0;
        int sprinklersCount = 0;
        for (;;) {
            if (count == 5)
                break;
            std::cout << u8"\033[2J\033[1;1H";

            for (short spacing = 3 - sprinklersCount; spacing >= 0; spacing--)
                std::cout << "\n";
            std::cout << "        T               T\n";

            if (sprinklersCount > 0) {
                for (short sprinklers = 0; sprinklers < sprinklersCount; sprinklers++) {
                    std::cout << "        |               |\n";
                }
            }

            count++;
            sprinklersCount++;

            drawWipers(0, true);

            if (mode >= frontOn && mode <= frontOff)
                std::cout << "              front\n\n";
            else if (mode >= backOn && mode <= backOff)
                std::cout << "              back\n\n";

            std::cout.flush();
            usleep(125000);
        }
        usleep(500000);
    }

    //Rest of wiper variants
    for (;;) {
        std::cout << u8"\033[2J\033[1;1H";
        drawWipers(wiperState, false);

        if (mode >= frontOn && mode <= frontOff)
            std::cout << "              front\n\n";
        else if (mode >= backOn && mode <= backOff)
            std::cout << "              back\n\n";

        wiperState++;
        std::cout.flush();
        usleep(500000);
        if (wiperState > 4) {
            for (short backwardRun = 3; backwardRun >= 0; backwardRun--) {
                std::cout << u8"\033[2J\033[1;1H";
                drawWipers(backwardRun, false);

                if (mode >= frontOn && mode <= frontOff)
                    std::cout << "              front\n\n";
                else if (mode >= backOn && mode <= backOff)
                    std::cout << "              back\n\n";

                std::cout.flush();
                usleep(500000);
            }
            wiperState = -1;
        }

        if (wiperState == -1 || mode == frontOff || mode == backOff) {
            mqtt::const_message_ptr msg;

            while(cli.try_consume_message(&msg)){
                if(msg->to_string() != "once" && msg->to_string() != "fluid_once"){
                    storedComm = msg->to_string();
                    storedTopic = msg->get_topic();
                    break;
                }
            };

            return;
        }
    }
}

//Function executing wipe() function in desired mode
void chooseMode()
{
    if (!turnedOn && rerender) {
        std::cout << u8"\033[2J\033[1;1H";
        drawWipers(0, false);
        rerender = false;
    }
    if (turnedOn) {
        std::cout << u8"\033[2J\033[1;1H";
        switch (mode) {
            //Wipe front continuously
            case frontOn:
                wipe(frontOn);
                return;

            //Wipe front once
            case frontOnce:
                wipe(frontOnce);
                return;

            //Wipe front once with fluid
            case frontFluidOnce:
                wipe(frontFluidOnce);
                return;

            //Wipe front continuously with fluid
            case frontFluid:
                wipe(frontFluid);
                return;

            //Stop wiping front
            case frontOff:
                wipe(frontOff);
                stopWiping = true;
                return;

            //Wipe back continuously
            case backOn:
                wipe(backOn);
                return;

            //Wipe back once
            case backOnce:
                wipe(backOnce);
                return;

            //Wipe back once with fluid
            case backFluidOnce:
                wipe(backFluidOnce);
                return;

            //Wipe back continuously with fluid
            case backFluid:
                wipe(backFluid);
                return;

            //Stop wiping back
            case backOff:
                wipe(backOff);
                stopWiping = true;
                return;
            default:

                break;
        }
        turnedOn = false;
    }
}

int main()
{

    infoFromBrokerMap.insert({"on", on});
    infoFromBrokerMap.insert({"off", off});
    infoFromBrokerMap.insert({"once", once});
    infoFromBrokerMap.insert({"fluid", fluid});
    infoFromBrokerMap.insert({"fluid_once", fluid_once});

    /*std::string previousText;

    homeDir = getenv("HOME");
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = my_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);*/

    auto connOpts = mqtt::connect_options_builder()
                        .clean_session(true)
                        .finalize();

    try {
        cli.start_consuming();

        auto tok = cli.connect(connOpts);

        auto rsp = tok->get_connect_response();

        if (!rsp.is_session_present()) {
            cli.subscribe(TOPIC_WF, QOS)->wait();
            cli.subscribe(TOPIC_WB, QOS)->wait();
        }

        //writeOnfile("connected\n");
        chooseMode();
        
        while (true) {
            mqtt::const_message_ptr msg;
            std::string prevTopic, prevComm;

            if (cli.try_consume_message(&msg) || !storedComm.empty()) {
                for (;;) {
                    

                    if (msg == nullptr)
                        break;

                    std::string option;
                    std::string topic;

                    if(storedComm.empty()){
                        option = msg->to_string();
                        topic = msg->get_topic();
                    }

                    if (!storedComm.empty()){
                        option = storedComm;
                        topic = storedTopic;
                        
                    }
                    

                    //std::string text   = "received message: \"" + option + "\" on topic: " + msg->get_topic() + "\n";

                    /*if (previousText != text) {
                        writeOnfile(text);
                    }

                    previousText = text;*/

                    if (topic == "/car/wipers/front") {
                        switch (infoFromBrokerMap[option]) {
                            case on:
                                stopWiping = false;
                                turnedOn   = true;
                                mode       = frontOn;
                                break;

                            case off:
                                stopWiping = true;
                                turnedOn   = true;
                                mode       = frontOff;
                                break;

                            case once:
                                turnedOn = true;
                                mode     = frontOnce;
                                break;

                            case fluid:
                                stopWiping = false;
                                turnedOn   = true;
                                mode       = frontFluid;
                                break;

                            case fluid_once:
                                turnedOn = true;
                                mode     = frontFluidOnce;
                                break;
                        }
                    } else if (topic == "/car/wipers/back") {
                        switch (infoFromBrokerMap[option]) {
                            case on:
                                stopWiping = false;
                                turnedOn   = true;
                                mode       = backOn;
                                break;

                            case off:
                                stopWiping = true;
                                turnedOn   = true;
                                mode       = backOff;
                                break;

                            case once:
                                turnedOn = true;
                                mode     = backOnce;
                                break;

                            case fluid:
                                stopWiping = false;
                                turnedOn   = true;
                                mode       = backFluid;
                                break;

                            case fluid_once:
                                turnedOn = true;
                                mode     = backFluidOnce;
                                break;
                        }
                    }

                    storedTopic.clear();
                    storedComm.clear();

                    mqtt::const_message_ptr msgNew;

                    if (cli.try_consume_message(&msgNew)) {

                        msg        = msgNew;
                        stopWiping = false;
                        continue;
                    }

                    if (!stopWiping){
                        chooseMode();
                    }
                    else{
                        if(mode == frontOff || mode == backOff){
                            mqtt::const_message_ptr msgTmp;
                            msg = msgTmp;
                            continue;
                        }
                        
                        if(!blockLoop){ 
                            stopWiping = false;
                        }
                    }

                }
            }
        }

        if (cli.is_connected()) {
            std::cout << "\nShuttinga down and disconnecting from the MQTT server..." << std::flush;
            cli.unsubscribe(TOPIC_WF)->wait();
            cli.unsubscribe(TOPIC_WB)->wait();
            cli.stop_consuming();
            cli.disconnect()->wait();
            std::cout << "OK" << std::endl;
            //writeOnfile("disconnected from the server\n");
        } else {
            std::cout << "\nClient was disconnected" << std::endl;
        }
    } catch (const mqtt::exception &exc) {
        std::cerr << "\n  " << exc << std::endl;
        //writeOnfile("crashed unexpectedly.\n");
        return 1;
    }
}
