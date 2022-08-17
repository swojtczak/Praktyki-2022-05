#include <iostream>
#include <string>
#include <unistd.h>
#include "mqtt/client.h"
#include <map>
#include <array>
#include <fstream>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

std::string homeDir;

int moved = 0;
int val;
int windowState;

std::map<std::string, int> values =
{
    {"down", -1},
    {"stop", 0},
    {"up", 1}
};

std::array<int, 4> szyby = {0,0,0,0};
std::array<int, 4> szybyFuture = {0,0,0,0};
const int  QOS = 1;

const std::string SERVER_ADDRESS	{ "tcp://localhost:1883" };
const std::string CLIENT_ID		{ "paho_cpp_async_consume" };
const std::string TOPIC 			{ "/car/window/" };

void writeOnfile (std::string text) {
    std::string path = homeDir + "/.local/share/autox.log";
    char* filetext = &text[0];
    std::ofstream myfile;
    //myfile.open ("/var/log/autox.log", std::fstream::app);
    myfile.open (path, std::fstream::app);
    
    myfile << filetext;
    myfile.close();
}

void msgHandling(mqtt::const_message_ptr msg){
    std::string topic = std::string(msg->get_topic());
    int temp = topic[12] - 48;
    int valTemp = szybyFuture[temp];
    std::string content = std::string(msg->to_string());
    std::string text = "Windows control app received message: \"" + content + "\" on topic: " + msg->get_topic() + "\n";
    writeOnfile(text);

    try{
        valTemp = values.at(content);
    }catch(const std::out_of_range& oor){

    }
    szybyFuture[temp] = valTemp;
}

void drawWindows(){
    std::cout<< u8"\033[2J\033[1;1H";
    for(int i = 0; i < 10; i++){
        for(int x = 0; x < 4; x++){
            if((i-szyby[x]) > 0){
                std::cout << "  |||||  ";
            }else{
                std::cout << "         ";
            }
        }
        std::cout << "\n";
    }
}

void my_handler(int s){
    writeOnfile("Application terminated by user.\n");
    exit(1);
}

int main(){
    homeDir = getenv("HOME"); 
    mqtt::async_client cli(SERVER_ADDRESS, CLIENT_ID);

	auto connOpts = mqtt::connect_options_builder().clean_session(false).finalize();

            struct sigaction sigIntHandler;
            sigIntHandler.sa_handler = my_handler;
            sigemptyset(&sigIntHandler.sa_mask);
            sigIntHandler.sa_flags = 0;
            sigaction(SIGINT, &sigIntHandler, NULL);
    try{
		cli.start_consuming();

		auto tok = cli.connect(connOpts);
		auto rsp = tok->get_connect_response();

        for(int i = 0; i < 4; i++){
            cli.subscribe(TOPIC + std::to_string(i), QOS)->wait();
        }

        writeOnfile("Windows control app connected\n");

        while(!false){
            mqtt::const_message_ptr msg;

            if(cli.try_consume_message(&msg)){
                if (msg == nullptr) break;
                msgHandling(msg);
            }


            drawWindows();
            std::cout.flush();
            sleep(1);
            for(int x = 0; x < 4; x++){
                val = szybyFuture[x];
                windowState = szyby[x];
                if(val != 0){
                    szyby[x] = szyby[x] - (szybyFuture[x] / abs(szybyFuture[x]));
                    if(szyby[x] > 9 || szyby[x] < 0){
                        szyby[x] = windowState;
                        szybyFuture[x] = 0;
                    }
                }
            }
        }


        if (cli.is_connected()){
            std::cout << "\nShutting down and disconnecting from the MQTT server..." << std::flush;
            cli.unsubscribe(TOPIC)->wait();
            cli.stop_consuming();
            cli.disconnect()->wait();
            std::cout << "OK" << std::endl;
        }else{
            std::cout << "\nClient was disconnected" << std::endl;
        }
    }catch (const mqtt::exception& exc) {
        std::cerr << "\n  " << exc << std::endl;
        writeOnfile("Windows control app crashed unexpectedly. \n");
        return 1;
    }
    return 0;
}