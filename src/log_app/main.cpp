#include <cstddef>
#include <iostream>
#include <string>
#include <unistd.h>
#include "mqtt/client.h"
#include <fstream>
#include <iomanip>
#include <ctime>

const int  QOS = 1;

const std::string SERVER_ADDRESS	{ "tcp://localhost:1883" };
const std::string CLIENT_ID		{ "log_app" };

//main
int main(int argc, char **argv)
{
    std::string homeDir = getenv("HOME"); 
    std::string path = "";
    bool logger = false;
    std::ofstream file;
    int option;

    while ((option = getopt(argc, argv, "s")) != -1) 
    {
        switch (option) 
        {
            case 's':
                path = homeDir + "/.local/share/autox.log";
                break;
            case '?':
                return 1;
                break;
        }
    }
    
    if (!path.empty()) logger = true;

    if (logger) file.open(path, std::fstream::app);

    mqtt::async_client cli(SERVER_ADDRESS, CLIENT_ID);

	auto connOpts = mqtt::connect_options_builder()
		.clean_session(true)
		.finalize();

    try 
    {
		cli.start_consuming();

		std::cout << "Connecting to the MQTT server..." << std::flush;

        if (logger) file << "Connecting to the MQTT server..." << std::endl;

		auto tok = cli.connect(connOpts);
		auto rsp = tok->get_connect_response();

        std::ifstream topic_file("../topics");
        std::string topic;
        std::cout << std::endl;
		if (!rsp.is_session_present())
        {
            while (getline (topic_file, topic)) 
            {
                cli.subscribe(topic, QOS)->wait();
                std::cout << "subscribed to topic: " << topic << std::endl;
                if (logger) file << "subscribed to topic: " << topic << std::endl;
            }
        }

        topic_file.close();
		
		std::cout << "Connected and ready to log" << std::endl;
        if (logger) file << "Connected and ready to log"  << std::endl;

        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
		while (true) 
        {
            mqtt::const_message_ptr msg;

            if (cli.try_consume_message(&msg))
            {
                if (msg == nullptr) break;
                std::cout << std::put_time(&tm, "%d-%m-%Y %H:%M") << " " << msg->get_topic() << " " << msg->get_payload() << std::endl;
                if (logger) file << std::put_time(&tm, "%d-%m-%Y %H:%M") << " " << msg->get_topic() << " " << msg->get_payload() << std::endl;
            }
            
		}

		if (cli.is_connected()) 
        {
			std::cout << "\nShutting down and disconnecting from the MQTT server..." << std::flush;
            if (logger) file << "\nShutting down and disconnecting from the MQTT server..." << std::endl;

            std::ifstream topic_file("filename.txt");
            if (!rsp.is_session_present())
            {
                while (getline (topic_file, topic)) cli.unsubscribe(topic)->wait();
            }
            topic_file.close();
			
			cli.stop_consuming();
			cli.disconnect()->wait();
			std::cout << "OK" << std::endl;
		}
		else 
        {
			std::cout << "\nClient was disconnected" << std::endl;
            if (logger) file << "\nClient was disconnected" << std::endl;
		}
	}
	catch (const mqtt::exception& exc) 
    {
		std::cerr << "\n  " << exc << std::endl;
        if (logger) file.close();
		return 1;
	}

    if (logger) file.close();
    return 0;
}