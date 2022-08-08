#include <iostream>
#include <string>
#include <unistd.h>
#include "mqtt/client.h"
#include "callback.h"
#include "settings.h"

bool left = true;

void drawIndicators()
{
    for(int i = 0; i < 3; i++)
    {
        if(left)
        {
            std::cout<< u8"\033[2J\033[1;1H";
            std::cout<< "<\n";
            std::cout.flush();
            sleep(1);
            std::cout<< u8"\033[2J\033[1;1H\n";
            std::cout.flush();
            sleep(1);
        }
        else
        {
            std::cout<< u8"\033[2J\033[1;1H";
            std::cout<< ">\n";
            std::cout.flush();
            sleep(1);
            std::cout<< u8"\033[2J\033[1;1H\n";
            std::cout.flush();
            sleep(1);
        }
    }
}


int main()
{

    mqtt::async_client cli(SERVER_ADDRESS, CLIENT_ID);

    mqtt::connect_options connOpts;
	connOpts.set_clean_session(false);

    callback cb(cli, connOpts);
	cli.set_callback(cb);

    try {
		//std::cout << "Connecting to the MQTT server...\n" << std::flush;
		cli.connect(connOpts, nullptr, cb);

	}
	catch (const mqtt::exception& exc) {
		std::cerr << "\nERROR: Unable to connect to MQTT server: '" << SERVER_ADDRESS << "'" << exc << std::endl;
	}

    std::string direction;

    while(true)
    {
        //drawIndicators();
        //std::cin >> direction;
        //left = (direction == "left");
    }
    return 0;
}