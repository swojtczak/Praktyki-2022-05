#include <iostream>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include "mqtt/client.h"


//std::cout<< u8"\033[2J\033[1;1H"; - ANSI Escape sequence clearing terminal

const std::string SERVER_ADDRESS { "tcp://localhost:1883" };
const std::string CLIENT_ID { "wiper_controll_app" };
const std::string TOPIC_WF { "/car/wipers/front" };
const std::string TOPIC_WB { "/car/wipers/back" };

const int QOS = 1;

bool stopWiping;
int mode = 0;
bool turnedOn;
bool rerender = true;

void drawWipers(int state, bool sprinklersOn){
    int lenghtOfWipers = 6;
    switch(state){

        //Wipers don't move, facing left
        case 0:
            if(!sprinklersOn){
                for(short spacing = 0; spacing < lenghtOfWipers - 1; spacing++) std::cout << "\n";
            }
            std::cout << "<";
            for(short count = 0; count <= lenghtOfWipers * 2 + 1; count++){
                std::cout << "--";
                
                if(count == lenghtOfWipers) std::cout << " <";
            }

            std::cout << std::endl;
            break;

        //Wipers move, tilted to the left
        case 1:
            for(short count = 0; count < lenghtOfWipers; count++){
                for(short spacing = 0; spacing < count; spacing++) std::cout << " ";
                std::cout << "       \\";
                std::cout << "          \\" << std::endl; 
            }
            break;

        //Wipers move, stand vertically
        case 2:
            for(short count = 0; count < lenghtOfWipers; count++) std::cout << "             |          |" << std::endl; 

            break;

        //Wipers move, tilted to the right
        case 3:
            for(short count = 0; count < lenghtOfWipers; count++){
                for(short spacing = lenghtOfWipers * 2; spacing > count; spacing--) std::cout << " ";
                std::cout << "       /";
                std::cout << "          /" << std::endl; 
            }
            break;

        //Wipers don't move, facing right
        case 4:
            for(short spacing = 0; spacing < lenghtOfWipers - 1; spacing++) std::cout << "\n";
            std::cout << "             ";
            for(short count = 0; count <= lenghtOfWipers * 2 + 1; count++){

                std::cout << "--";
                
                if(count == lenghtOfWipers) std::cout << "> ";
            }

            std::cout << ">" << std::endl;
            break;
    }
}

//Function animating wipers
void wipe(short mode){
    short wiperState = 0;

    if(mode == 1 || mode == 2 || mode == 6 || mode == 7) stopWiping = true;

    //Wipers with sprinklers
    if(mode == 2 || mode == 3 || mode == 7 || mode == 8){
        int count = 0;
        int sprinklersCount = 0;
        for(;;){
            if(count == 5) break;
            std::cout<< u8"\033[2J\033[1;1H";
    
            for(short spacing = 3 - sprinklersCount; spacing >= 0; spacing--) std::cout << "\n";
            std::cout << "        T               T\n";

            if(sprinklersCount > 0){
                for(short sprinklers = 0; sprinklers < sprinklersCount; sprinklers++){
                    std::cout << "        |               |\n";
                }
            }

            count++;
            sprinklersCount++;

            drawWipers(0, true);

            if(mode >= 0 && mode <= 4) std::cout << "              front\n\n";
            else if(mode >= 5 && mode <= 9) std::cout << "              back\n\n";
            
            std::cout.flush();
            usleep(125000);
        }
        usleep(500000);
    }
    
    //Rest of wiper variants
    for(;;){
        std::cout<< u8"\033[2J\033[1;1H";
        drawWipers(wiperState, false);

        if(mode >= 0 && mode <= 4) std::cout << "              front\n\n";
        else if(mode >= 5 && mode <= 9) std::cout << "              back\n\n";
 
        wiperState++;
        std::cout.flush();
        usleep(500000);
        if(wiperState > 4){
            for(short backwardRun = 3; backwardRun >= 0; backwardRun--){
                std::cout<< u8"\033[2J\033[1;1H";
                drawWipers(backwardRun, false);

                if(mode >= 0 && mode <= 4) std::cout << "              front\n\n";
                else if(mode >= 5 && mode <= 9) std::cout << "              back\n\n";

                std::cout.flush();
                usleep(500000);
            }
            wiperState = -1;
           
        }  
        if(wiperState == -1) return; 

    }
}

void chooseMode(){
        if(!turnedOn && rerender){
            std::cout<< u8"\033[2J\033[1;1H";
            drawWipers(0, false);
            rerender = false;
        } 
        if(turnedOn){
            std::cout<< u8"\033[2J\033[1;1H";
            switch(mode){
                //Wipe front continuously 
                case 0:
                    wipe(0);
                    return;

                //Wipe front once
                case 1:
                    wipe(1);
                    return;

                //Wipe front once with fluid
                case 2:
                    wipe(2);
                    return;
                
                //Wipe front continuously with fluid
                case 3:
                    wipe(3);
                    return;

                //Stop wiping front
                case 4:
                    stopWiping = true;
                    return;

                //Wipe back continuously 
                case 5:
                    wipe(5);
                    return;

                //Wipe back once
                case 6:
                    wipe(6);
                    return;

                //Wipe back once with fluid
                case 7:
                    wipe(7);
                    return;

                //Wipe back continuously with fluid
                case 8: 
                    wipe(8);
                    return;
        
                //Stop wiping back
                case 9:
                    stopWiping = true;
                    return;
                default:

                break;

            }
            turnedOn = false;
        }
}

int main(){
    
    mqtt::async_client cli(SERVER_ADDRESS, CLIENT_ID);

	auto connOpts = mqtt::connect_options_builder()
		.clean_session(true)
		.finalize();

   try {
		cli.start_consuming();

		auto tok = cli.connect(connOpts);

		auto rsp = tok->get_connect_response();

		if (!rsp.is_session_present()){
			cli.subscribe(TOPIC_WF, QOS)->wait();
            cli.subscribe(TOPIC_WB, QOS)->wait();
        }

        chooseMode();

        mqtt::const_message_ptr msgPrev;

		while (true) 
        {
            mqtt::const_message_ptr msg;

            if (cli.try_consume_message(&msg))
            {
                for(;;){

                    if (msg == nullptr) break;
                
                    std::string option = msg->to_string();

                    if(msg->get_topic() == "/car/wipers/front"){
                        if(option == "on"){
                            stopWiping = false;
                            turnedOn = true;
                            mode = 0;
                        }
                        else if(option == "off"){ 
                            stopWiping = true;
                            turnedOn = true;
                            mode = 4;
                        }
                        else if(option == "once"){
                            turnedOn = true;
                            mode = 1; 
                        }
                        else if(option == "fluid"){
                            stopWiping = false;
                            turnedOn = true;
                            mode = 3;
                        }
                        else if(option == "fluid_once"){ 
                            turnedOn = true;
                            mode = 2;
                        }
                    }
                    else if(msg->get_topic() == "/car/wipers/back"){
                        if(option == "on"){
                            turnedOn = true;
                            stopWiping = false;
                            mode = 5;
                        }
                        else if(option == "off"){ 
                            turnedOn = true;
                            stopWiping = true;
                            mode = 9;
                        }
                        else if(option == "once"){
                            turnedOn = true;
                            mode = 6; 
                        }
                        else if(option == "fluid"){
                            turnedOn = true;
                            stopWiping = false;
                            mode = 8;
                        }
                        else if(option == "fluid_once"){ 
                            turnedOn = true;
                            mode = 7;
                        }
                    }

                    mqtt::const_message_ptr msgNew;

                    if(cli.try_consume_message(&msgNew)){
                        msg = msgNew;
                        stopWiping = false;
                        continue;
                    }
                    if(!stopWiping) chooseMode();    
                }
            } 
		}



		if (cli.is_connected()) 
        {
			std::cout << "\nShutting down and disconnecting from the MQTT server..." << std::flush;
			cli.unsubscribe(TOPIC_WF)->wait();
            cli.unsubscribe(TOPIC_WB)->wait();
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
}
