#include "action_listener.h"

void action_listener::on_failure(const mqtt::token& tok)
{
    
    if (tok.get_message_id() != 0)
    {
        //std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
    }
}

void action_listener::on_success(const mqtt::token& tok)
{
    std::cout << name_ << " success";
    
    if (tok.get_message_id() != 0)
    {
        //std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
    }
    
    auto top = tok.get_topics();
    
    if (top && !top->empty())
    {
        std::cout << "\ttoken topic: '" << (*top)[0] << "', ..." << std::endl;
    }

    std::cout << std::endl;
}

action_listener::action_listener(const std::string& name) : name_(name) {}