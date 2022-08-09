#include <iostream>
#include <string>
#include <unistd.h>
#include "mqtt/client.h"
#include <map>

//temporary cli painting arrays
char indicators_array[4][3][8]  {{{'/', '-', '-', '-', '-', '-', '-', '|'},
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

const int  QOS = 1;

const std::string SERVER_ADDRESS	{ "tcp://localhost:1883" };
const std::string CLIENT_ID		{ "paho_cpp_async_consume" };
const std::string TOPIC 			{ "/car/indicator/" };

struct Indicators 
{
    bool left = false;
    bool right = false;
    bool prev_left = false;
    bool prev_right = false;

    void setLeft(bool value)
    {
        left = value;

        if(right == true) right = !value;
    }

    void setRight(bool value)
    {
        right = value;

        if(left == true) left = !value;
    }

    void setHazard(bool value)
    {
        if (!value)
        {
            left = prev_left;
            right = prev_right;
        }
        else
        {   
            prev_left = left;
            prev_right = right;
            left = value;
            right = value;
        }

        
    }
}indicators;

void selectAction(std::string topic, std::string payload)
{
    std::string direction = topic.erase(0, 15);
    std::map<std::string, int> const topic_cast = { {"left", 0}, {"right", 1}, {"hazard", 2}};
    std::map<std::string, bool> const payload_cast = { {"off", false}, {"on", true}};

    switch(topic_cast.find(direction)->second)
    {
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


void drawIndicators(bool state)
{

    std::cout<< u8"\033[2J\033[1;1H";

    for (short i = 0; i < 3; i++)
    {
        for (short j = 0; j < 19; j++)
        {
            if(j < 8)
            {
                std::cout << indicators_array[indicators.left ? state : 0][i][j];
            }
            else if (j < 11)
            {
                std::cout << ' ';
            }
            else
            {
                std::cout << indicators_array[state ? (1 << 1) + indicators.right : 2][i][j - 11];
            }
        }
        std::cout << "\n";
    }
        
    std::cout << "\n";
}

int main()
{
    mqtt::async_client cli(SERVER_ADDRESS, CLIENT_ID);

	auto connOpts = mqtt::connect_options_builder()
		.clean_session(false)
		.finalize();

    try 
    {
		cli.start_consuming();

		std::cout << "Connecting to the MQTT server..." << std::flush;
		auto tok = cli.connect(connOpts);
		auto rsp = tok->get_connect_response();

		// If there is no session present, then we need to subscribe, but if
		// there is a session, then the server remembers us and our
		// subscriptions.
		if (!rsp.is_session_present())
			cli.subscribe(TOPIC + "right", QOS)->wait();
            cli.subscribe(TOPIC + "left", QOS)->wait();
            cli.subscribe(TOPIC + "hazard", QOS)->wait();

		std::cout << "Connected and redy to run" << std::endl;

        bool state = false;

		while (true) 
        {
            mqtt::const_message_ptr msg;

            if (cli.try_consume_message(&msg))
            {
                if (msg == nullptr) break;

                selectAction(msg->get_topic(), msg->get_payload());
            }
            

            drawIndicators(state);
            sleep(1);
            state = !state;
		}



		if (cli.is_connected()) 
        {
			std::cout << "\nShutting down and disconnecting from the MQTT server..." << std::flush;
			cli.unsubscribe(TOPIC)->wait();
			cli.stop_consuming();
			cli.disconnect()->wait();
			std::cout << "OK" << std::endl;
		}
		else 
        {
			std::cout << "\nClient was disconnected" << std::endl;
		}
	}
	catch (const mqtt::exception& exc) {
		std::cerr << "\n  " << exc << std::endl;
		return 1;
	}
    return 0;
}