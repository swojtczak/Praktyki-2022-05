#include "string.h"
#include "iostream"
#include "mqtt/client.h"
#include "action_listener.h"

class callback : public virtual mqtt::callback, public virtual mqtt::iaction_listener
{
private:
    const int N_RETRY_ATTEMPTS = 5;
    const int QOS = 1;

	int nretry_;
	mqtt::async_client& cli_;
	mqtt::connect_options& connOpts_;
	action_listener subListener_;

	void reconnect();
	void on_failure(const mqtt::token& tok) override;
	void on_success(const mqtt::token& tok) override;
	void connected(const std::string& cause) override;
	void connection_lost(const std::string& cause) override;
	void message_arrived(mqtt::const_message_ptr msg) override;
	void delivery_complete(mqtt::delivery_token_ptr token) override;

public:
	callback(mqtt::async_client& cli, mqtt::connect_options& connOpts);
};