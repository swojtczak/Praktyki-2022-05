#include <iostream>
#include <string>
#include <unistd.h>
#include "mqtt/client.h"
#include <map>
#include <array>
#include <cmath>

std::map<int, char> values = 
{
    {-1, '\\'},
    {0, '|'},
    {1, '/'}
};


const int  QOS = 1;

const std::string SERVER_ADDRESS	{ "tcp://localhost:1883" };
const std::string CLIENT_ID		{ "wheel_control_app" };
const std::string TOPIC 			{ "/car/wheel/angle" };
int angle = 0;
int maxAngle = 270;

double tempAngle;

void msgHandling(mqtt::const_message_ptr msg){
    std::cout << msg->to_string() << " topic: " << msg->get_topic();
    tempAngle = stoi(msg->to_string());
    angle = (int)tempAngle;
    tempAngle = std::round(tempAngle / maxAngle);
}

void drawWheels(){
    std::cout << u8"\033[2J\033[1;1H";
    std::cout << angle << std::endl << std::endl;
    std::cout << values.at(tempAngle) << std::endl << std::endl;
}


int main(){
    mqtt::async_client cli(SERVER_ADDRESS, CLIENT_ID);

	auto connOpts = mqtt::connect_options_builder().clean_session(true).finalize();

    try{
		cli.start_consuming();

		std::cout << "Connecting to the MQTT server..." << std::flush;
		auto tok = cli.connect(connOpts);
		auto rsp = tok->get_connect_response();
        
		if(!rsp.is_session_present()){
            cli.subscribe(TOPIC, QOS)->wait();
            std::cout << "Connecting to " << TOPIC <<  "..." << std::flush;
        }
    
        while(!false){
            mqtt::const_message_ptr msg;

            if(cli.try_consume_message(&msg)){
                if (msg == nullptr) break;
                msgHandling(msg);
            }


            drawWheels();
            std::cout.flush();
            sleep(1);
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
        return 1;
    }
    return 0;
}