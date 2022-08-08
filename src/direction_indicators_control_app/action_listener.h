#include "string.h"
#include "iostream"
#include "mqtt/client.h"

class action_listener : public virtual mqtt::iaction_listener
{
private:
	std::string name_;

	void on_failure(const mqtt::token& tok) override;
	void on_success(const mqtt::token& tok) override;

public:
	action_listener(const std::string& name);
};