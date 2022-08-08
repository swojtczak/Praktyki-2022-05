#include "callback.h"
#include "settings.h"

void callback::reconnect() 
{
    std::this_thread::sleep_for(std::chrono::milliseconds(2500));
    try {
        cli_.connect(connOpts_, nullptr, *this);
    }
    catch (const mqtt::exception& exc) {
        std::cerr << "Error: " << exc.what() << std::endl;
        exit(1);
    }
}

void callback::on_failure(const mqtt::token& tok){
    std::cout << "Connection attempt failed" << std::endl;
    
    if (++nretry_ > N_RETRY_ATTEMPTS){
        exit(1);
    }
    
    reconnect();
}

void callback::on_success(const mqtt::token& tok) {}

void callback::connected(const std::string& cause)
{
    std::cout << "\nConnection success" << std::endl;
    std::cout << "\nSubscribing to topic '" << TOPIC << "'\n" << "\tfor client " << CLIENT_ID << " using QoS" << QOS << "\n" << "\nPress Q<Enter> to quit\n" << std::endl;
    
    
    cli_.subscribe(TOPIC, QOS, nullptr, subListener_);
}

void callback::connection_lost(const std::string& cause)
{
    std::cout << "\nConnection lost" << std::endl;
    if (!cause.empty()){
        std::cout << "\tcause: " << cause << std::endl;
    }

    std::cout << "Reconnecting..." << std::endl;
    nretry_ = 0;
    reconnect();
}

void callback::message_arrived(mqtt::const_message_ptr msg)
{
    std::cout << "Message arrived" << std::endl;
    std::cout << "\ttopic: '" << msg->get_topic() << "'" << std::endl;
    std::cout << "\tpayload: '" << msg->to_string() << "'\n" << std::endl;
}

void callback::delivery_complete(mqtt::delivery_token_ptr token) {}

callback::callback(mqtt::async_client& cli, mqtt::connect_options& connOpts) : nretry_(0), cli_(cli), connOpts_(connOpts), subListener_("Subscription") {}