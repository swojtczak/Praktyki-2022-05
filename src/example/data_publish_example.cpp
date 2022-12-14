#include "mqtt/client.h"
#include <string>

const std::string ADDRESS{"tcp://localhost:1883"};
const int QOS = 1;

int main(int argc, char *argv[])
{
    std::string TOPIC{"/car/wheel/angle"};
    char *PAYLOAD2 = argv[1];

    // Create a client

    mqtt::client cli(ADDRESS, "data_publish_example");
    mqtt::connect_options connOpts;
    connOpts.set_keep_alive_interval(20);
    connOpts.set_clean_session(true);

    try {
        // Connect to the client

        cli.connect(connOpts);

        // Publish using a message pointer.

        std::cout << TOPIC << " " << PAYLOAD2 << std::endl;
        //msg->set_qos(QOS);

        cli.publish(TOPIC, PAYLOAD2, strlen(PAYLOAD2), 0, false);

        //cli.publish(msg);

        // Now try with itemized publish.

        // Disconnect

        cli.disconnect();
    } catch (const mqtt::exception &exc) {
        std::cerr << "Error: " << exc.what() << " [" << exc.get_reason_code() << "]" << std::endl;
        return 1;
    }

    return 0;
}
