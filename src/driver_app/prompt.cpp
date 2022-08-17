#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include "mqtt/client.h"
#include <readline/readline.h>
#include <readline/history.h>
#include "prompt.h"
#include <fstream>
#include <signal.h>
#include <iomanip>
#include <ctime>
#include <sstream>

std::string homeDir;
const std::string ADDRESS {"tcp://localhost:1883"};
const std::string CLIENT_ID {"driver_app"};
const int QOS = 1;
mqtt::connect_options connOpts;
mqtt::client cli(ADDRESS, CLIENT_ID);

void writeOnfile (std::string text) {
    std::string path = homeDir + "/.local/share/autox.log";

    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%d-%m-%Y %H:%M");
    auto str = oss.str();

    text = "[" + str + "] [driver_app:" + CLIENT_ID + "] " + text;

    char* filetext = &text[0];
    std::ofstream myfile;
    myfile.open (path, std::fstream::app);
    
    myfile << filetext;
    myfile.close();
}

void sendMessage(std::string top, std::string data){
    char* payload =  &data[0];
    

    try {
        cli.connect(connOpts);
        cli.publish(top, payload, strlen(payload), 0, false);
        std::string text = "sent message: \"" + data + "\" on topic: " + top + "\n";
        writeOnfile(text);
        cli.disconnect();
    }catch (const mqtt::exception& exc) {
        std::cerr << "Error: " << exc.what() << " [" << exc.get_reason_code() << "]" << std::endl;
        std::string text = "sending message: \"" + data + "\" on topic: " + top + "failed.\n";
        writeOnfile(text);
    }

}

void my_handler(int s){
    writeOnfile("terminated by user.\n");
    exit(1);
}

void repl_loop()
{
    homeDir = getenv("HOME"); 
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = my_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);

    connOpts.set_keep_alive_interval(20);
    connOpts.set_clean_session(true);

    bool status = true;
    int instruction;
    char *line;
    std::string op;
    std::vector<std::string> args;

    writeOnfile("Started\n");

    do {
        // std::cout << PROMPT;
        // std::getline(std::cin, line);
        line = readline(PROMPT);

        if (line == NULL) {
            free(line);
            exit(0);
        } else if (strlen(line) == 0)
            continue;
        else
            add_history(line);
        
        args = split_line(line);
        free(line);
        line = NULL;
        instruction = check_operator(args[0]);
        status = execute_instruction(instruction, args);
    } while (status);
}


int check_operator(std::string op)
{
    int i;
    bool found = false;

    // loop over command_list and check for op
    for (i = 0; i < (sizeof(command_list)/sizeof(command_list[0])); i++) {
        if (!command_list[i].command.compare(op)) {
            found = true;
            break;
        }
    }

    if (found)
        return i;
    else {
        printf("Syntax error: Unknown command - %s\n", op.c_str());
        return -1;
    }
}


std::vector<std::string> split_line(char *line)
{
    std::string in = line;
    std::vector<std::string> ret;

    int start = 0,
        end = in.find(" "); // delimiter between words
    
    while (end != -1) {
        ret.push_back(in.substr(start, end - start));
        start = end + 1;
        end = in.find(" ", start);
    }
    ret.push_back(in.substr(start, end - start));

    return ret;
}


bool execute_instruction(int instruction, std::vector<std::string> args)
{
    if (instruction >= 0) {
        if (args.size() > command_list[instruction].len) {
            printf("Syntax error: too many arguments - %zu\n", args.size());
            return true;
        } else if (args.size() < command_list[instruction].len) {
            printf("Syntax error: too little arguments - %zu\n", args.size());
            return true;
        }
    }
    
    std::string win;
    int window = 0;
    std::string indicator = "",
        wipers = "";

    switch (instruction) {
        case -1:
            return true;

        case 0:
            writeOnfile("user left using the exit command\n");
            exit(0);

        case 1:
            printf("%s\n", HELP_MESSAGE);
            break;

        case 2:
            if (args[2].compare("window")) {
                printf("Syntax error: unknown argument - %s\n", args[2].c_str());
                return true;
            }

            if (args[1].compare("front-left") == 0)
                window = 0;
            else if (!args[1].compare("front-right"))
                window = 1;
            else if (!args[1].compare("back-left"))
                window = 2;
            else if (!args[1].compare("back-right"))
                window = 3;
            else {
                printf("Syntax error: unknown argument - %s\n", args[1].c_str());
                return true;
            }
            win = "/car/window/" + std::to_string(window);
            sendMessage(win, "down");
            // printf("/car/window/%d down\n", window);
            break;

        case 3:
            if (args[2].compare("window")) {
                printf("Syntax error: unknown argument - %s\n", args[2].c_str());
                return true;
            }

            if (!args[1].compare("front-left"))
                window = 0;
            else if (!args[1].compare("front-right"))
                window = 1;
            else if (!args[1].compare("back-left"))
                window = 2;
            else if (!args[1].compare("back-right"))
                window = 3;
            else {
                printf("Syntax error: unknown argument - %s\n", args[1].c_str());
                return true;
            }

            
            win = "/car/window/" + std::to_string(window);
            sendMessage(win, "up");
            // printf("/car/window/%d up\n", window);
            break;
        
        case 4:
            if (args[2].compare("indicator")) {
                printf("Syntax error: unknown argument - %s\n", args[2].c_str());
                return true;
            }

            if (!args[1].compare("left"))
                indicator = "left";
            else if (!args[1].compare("right"))
                indicator = "right";
            else if (!args[1].compare("hazard"))
                indicator = "hazard";
            else {
                printf("Syntax error: unknown argument - %s\n", args[1].c_str());
                return true;
            }

            sendMessage(("/car/indicator/" + indicator), "on");
            // printf("/car/indicator/%s { status: 'on' }\n", indicator.c_str());
            break;

        case 5:
            if (args[2].compare("indicator")) {
                printf("Syntax error: unknown argument - %s\n", args[2].c_str());
                return true;
            }

            if (!args[1].compare("left"))
                indicator = "left";
            else if (!args[1].compare("right"))
                indicator = "right";
            else if (!args[1].compare("hazard"))
                indicator = "hazard";
            else {
                printf("Syntax error: unknown argument - %s\n", args[1].c_str());
                return true;
            }

            sendMessage(("/car/indicator/" + indicator), "off");
            // printf("/car/indicator/%s { status: 'off' }\n", indicator.c_str());
            break;

        case 6:
            if (args[1].compare("front") && args[1].compare("back")) {
                printf("Syntax error: unknown argument - %s\n", args[1].c_str());
                return true;
            }

            if (args[2].compare("on") && args[2].compare("once")) {
                printf("Syntax error: unknown argument - %s\n", args[2].c_str());
                return true;
            }

            sendMessage(("/car/wipers/" + args[1]), args[2]);
            // printf("/car/wipers/%s { status: '%s' }\n", args[1].c_str(), args[2].c_str());
            break;

        case 7:
            if (args[1].compare("front") && args[1].compare("back")) {
                printf("Syntax error: unknown argument - %s\n", args[1].c_str());
                return true;
            }

            if (!args[2].compare("on"))
                wipers = "fluid";
            else if (!args[2].compare("once"))
                wipers = "fluid_once";
            else {
                printf("Syntax error: unknown argument - %s\n", args[2].c_str());
                return true;
            }

            sendMessage(("/car/wipers/" + args[1]), wipers);
            // printf("/car/wipers/%s { status: '%s' }\n", args[1].c_str(), wipers.c_str());
            break;

        case 8:
            if (args[1].compare("front") && args[1].compare("back")) {
                printf("Syntax error: unknown argument - %s\n", args[1].c_str());
                return true;
            }

            sendMessage(("/car/wipers/" + args[1]), "off");
            // printf("/car/wipers/%s { status: 'off' }\n", args[1].c_str());
            break;

        case 9:
            if (args[2].compare("window")) {
                printf("Syntax error: unknown argument - %s\n", args[2].c_str());
                return true;
            }

            if (!args[1].compare("front-left"))
                window = 0;
            else if (!args[1].compare("front-right"))
                window = 1;
            else if (!args[1].compare("back-left"))
                window = 2;
            else if (!args[1].compare("back-right"))
                window = 3;
            else {
                printf("Syntax error: unknown argument - %s\n", args[1].c_str());
                return true;
            }

            
            win = "/car/window/" + std::to_string(window);
            sendMessage(win, "stop");
            // printf("/car/window/%d stop\n", window);
            break;
        
    }

    return true;
}
