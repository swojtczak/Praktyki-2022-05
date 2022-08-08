#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <cctype>
#include <thread>
#include <chrono>
#include "mqtt/async_client.h"
#include <array>
#include <unistd.h>

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
/*

              .-=====-.
              |       |
              |       |
              |       |
              |       |
 .-==========-'       '-==========-.
 |                                 |
 |                                 |
 |                                 |
 '-==========-.       .-==========-'
              |       |
              |       |               you are stepping into a godless teritorry, take this with you.
              |       |
              |       |
              |       |
              |       |
              |       |
              |       |
              |       |
              '-=====-'
*/

const std::string SERVER_ADDRESS("tcp://localhost:1883");
const std::string CLIENT_ID("paho_cpp_async_subcribe");
const std::string TOPIC("/car/window/");

const int	QOS = 1;
const int	N_RETRY_ATTEMPTS = 5;

// Callbacks for the success or failures of requested actions.
// This could be used to initiate further action, but here we just log the
// results to the console.

class action_listener : public virtual mqtt::iaction_listener
{
	std::string name_;

	void on_failure(const mqtt::token& tok) override {
		
        if (tok.get_message_id() != 0){
			std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
        }

        std::cout << std::endl;
	}

	void on_success(const mqtt::token& tok) override {
		std::cout << name_ << " success";
		
        if (tok.get_message_id() != 0){
			std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
        }
		
        auto top = tok.get_topics();
		
        if (top && !top->empty()){
			std::cout << "\ttoken topic: '" << (*top)[0] << "', ..." << std::endl;
        }

		std::cout << std::endl;
	}

public:
	action_listener(const std::string& name) : name_(name) {}
};

/**
 * Local callback & listener class for use with the client connection.
 * This is primarily intended to receive messages, but it will also monitor
 * the connection to the broker. If the connection is lost, it will attempt
 * to restore the connection and re-subscribe to the topic.
 */

class callback : public virtual mqtt::callback, public virtual mqtt::iaction_listener{
	// Counter for the number of connection retries
	int nretry_;
	// The MQTT client
	mqtt::async_client& cli_;
	// Options to use if we need to reconnect
	mqtt::connect_options& connOpts_;
	// An action listener to display the result of actions.
	action_listener subListener_;

	// This deomonstrates manually reconnecting to the broker by calling
	// connect() again. This is a possibility for an application that keeps
	// a copy of it's original connect_options, or if the app wants to
	// reconnect with different options.
	// Another way this can be done manually, if using the same options, is
	// to just call the async_client::reconnect() method.
	void reconnect() {
		std::this_thread::sleep_for(std::chrono::milliseconds(2500));
		try {
			cli_.connect(connOpts_, nullptr, *this);
		}
		catch (const mqtt::exception& exc) {
			std::cerr << "Error: " << exc.what() << std::endl;
			exit(1);
		}
	}

	// Re-connection failure
	void on_failure(const mqtt::token& tok) override {
		std::cout << "Connection attempt failed" << std::endl;
		
        if (++nretry_ > N_RETRY_ATTEMPTS){
			exit(1);
        }
		
        reconnect();
	}

	// (Re)connection success
	// Either this or connected() can be used for callbacks.
	void on_success(const mqtt::token& tok) override {}

	// (Re)connection success
	void connected(const std::string& cause) override {
		//std::cout << "\nConnection success" << std::endl;
		//std::cout << "\nSubscribing to topic '" << TOPIC << "'\n" << "\tfor client " << CLIENT_ID << " using QoS" << QOS << "\n" << "\nPress Q<Enter> to quit\n" << std::endl;
        
        for(int i = 0; i < 4; i++){
            cli_.subscribe((TOPIC + std::to_string(i)), QOS, nullptr, subListener_);
        }
	}

	// Callback for when the connection is lost.
	// This will initiate the attempt to manually reconnect.
	void connection_lost(const std::string& cause) override {
		std::cout << "\nConnection lost" << std::endl;
		if (!cause.empty()){
			std::cout << "\tcause: " << cause << std::endl;
        }

		std::cout << "Reconnecting..." << std::endl;
		nretry_ = 0;
		reconnect();
	}

	// Callback for when a message arrives.
	void message_arrived(mqtt::const_message_ptr msg) override {
        int temp = std::string(msg->get_topic())[7] - 48;
        int valTemp = szybyFuture[temp];
        try{
            valTemp = values.at(std::string(msg->to_string()));
        }catch(const std::out_of_range& oor){

        }
        
        szybyFuture[temp] = valTemp;
        //std::cout << "Message arrived: " << msg->to_string() << std::endl;
		//std::cout << "\ttopic: '" << msg->get_topic() << "'" << std::endl;
	}

	void delivery_complete(mqtt::delivery_token_ptr token) override {}

public:
	callback(mqtt::async_client& cli, mqtt::connect_options& connOpts) : nretry_(0), cli_(cli), connOpts_(connOpts), subListener_("Subscription") {}
};

void drawWindows(){
    std::cout<< u8"\033[2J\033[1;1H";
   for(int i = 0; i < 10; i++){
        for(int x = 0; x < 4; x++){
            if((i-szyby[x]) >= 0){
                std::cout << "  |||||  " << szybyFuture[x];
            }else{
                std::cout << "         " << szybyFuture[x];
            }
        }
        std::cout << "\n";
    }
}

int main(int argc, char* argv[])
{
	// A subscriber often wants the server to remember its messages when its
	// disconnected. In that case, it needs a unique ClientID and a
	// non-clean session.

	mqtt::async_client cli(SERVER_ADDRESS, CLIENT_ID);

	mqtt::connect_options connOpts;
	connOpts.set_clean_session(false);

	// Install the callback(s) before connecting.
	callback cb(cli, connOpts);
	cli.set_callback(cb);

	// Start the connection.
	// When completed, the callback will subscribe to topic.

	try {
		std::cout << "Connecting to the MQTT server...\n" << std::flush;
		cli.connect(connOpts, nullptr, cb);
	}
	catch (const mqtt::exception& exc) {
		std::cerr << "\nERROR: Unable to connect to MQTT server: '" << SERVER_ADDRESS << "'" << exc << std::endl;
		return 1;
	}

	while(!false){
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

	// Disconnect

	try {
		std::cout << "\nDisconnecting from the MQTT server..." << std::flush;
		cli.disconnect()->wait();
		std::cout << "OK" << std::endl;
	}
	catch (const mqtt::exception& exc) {
		std::cerr << "D U P A" << exc << std::endl;
		return 1;
	}

 	return 0;
}