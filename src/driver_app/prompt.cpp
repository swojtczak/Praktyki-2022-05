#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <readline/chardefs.h>
#include <string>
#include "mqtt/client.h"
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include "prompt.h"
#include <fstream>
#include <signal.h>
#include <iomanip>
#include <ctime>
#include <sstream>

std::string homeDir;
bool scenario_mode = false,
     debug_mode = false,
     recording = false;
std::ifstream sfile;

std::string scenarioDir = "../Scenarios/";
std::string fileName;
std::ofstream scenarioFile;
bool commandExecuted;

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

void sendMessage(std::string top, std::string data)
{
    char* payload =  &data[0];
    

    if (debug_mode) printf("[DEBUG] Trying to send the message: %s %s...\n", top.c_str(), data.c_str());

    try {
        if (debug_mode) printf("[DEBUG] Connecting to the broker...\n");
        cli.connect(connOpts);
        if (debug_mode) printf("[DEBUG] Connected!\n[DEBUG] Publishing the message...\n");
        cli.publish(top, payload, strlen(payload), 0, false);
        std::string text = "sent message: \"" + data + "\" on topic: " + top + "\n";
        writeOnfile(text);
        if (debug_mode) printf("[DEBUG] Publishing successful!\n[DEBUG] Disconnecting from the broker...\n");
        cli.disconnect();
        if (debug_mode) printf("[DEBUG] Disconnected!\n");
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


void repl_loop(bool debug)
{
    homeDir = getenv("HOME"); 
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = my_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);

    connOpts.set_keep_alive_interval(20);
    connOpts.set_clean_session(true);

    debug_mode = debug;

    bool status = true;
    int instruction;
    char *line;
    std::string temp;
    std::vector<std::string> args;

    writeOnfile("Started\n");

    do {
        if (scenario_mode) {
            free(line);
            if(!std::getline(sfile, temp)){
                sfile.close();
                scenario_mode = false;
            }
            line = strdup(temp.c_str());
        } else {
            line = readline(PROMPT);

            if (line == NULL) {
                free(line);
                exit(0);
            } else if (strlen(line) == 0)
                continue;
            else
                add_history(line);
        }

        args = split_line(line);
        
        /*if(recording && args[0] != "help" && args[0] != "stop_rec" && args.size() == command_list[check_operator(args[0], true)].len){
            recordAction(line, fileName);
        }*/

        if (debug_mode) printf("[DEBUG] Looking up the command index...\n");
        instruction = check_operator(args[0], false);
        if (debug_mode && instruction != -1) printf("[DEBUG] %s index found! It's %d\n[DEBUG] Trying to execute the instruction...\n", args[0].c_str(), instruction);
        status = execute_instruction(instruction, args);
        
        if(recording && status && commandExecuted && args[0] != "help" && args[0] != "stop_rec" && args[0] != "record" && instruction != -1) recordAction(line, fileName);
        commandExecuted = false;

        free(line);
        line = NULL;
    } while (true);
}


int check_operator(std::string op, bool silentRun)
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

    if (found) {
        return i;
    } else {
        if(!silentRun) printf("Syntax error: Unknown command - %s\n", op.c_str());
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
            return false;
        } else if (args.size() < command_list[instruction].len) {
            printf("Syntax error: too little arguments - %zu\n", args.size());
            return false;
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
            if(!recording)
                win = "/car/window/" + std::to_string(window);

            sendMessage(win, "down");
            commandExecuted = true;
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

            if(!recording)
                win = "/car/window/" + std::to_string(window);

            sendMessage(win, "up");
            commandExecuted = true;
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
            if(!recording)
                sendMessage(("/car/indicator/" + indicator), "on");

            commandExecuted = true;
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
            if(!recording)
                sendMessage(("/car/indicator/" + indicator), "off");

            commandExecuted = true;
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
            if(!recording)
                sendMessage(("/car/wipers/" + args[1]), args[2]);

            commandExecuted = true;
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
            if(!recording)
                sendMessage(("/car/wipers/" + args[1]), wipers);

            commandExecuted = true;
            break;

        case 8:
            if (args[1].compare("front") && args[1].compare("back")) {
                printf("Syntax error: unknown argument - %s\n", args[1].c_str());
                return true;
            }
            if(!recording)
                sendMessage(("/car/wipers/" + args[1]), "off");

            commandExecuted = true;
            break;

        case 9:
            if (args[2].compare("window")) {
                printf("Syntax error: unknown argument - %s\n", args[2].c_str());
                return false;
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
                return false;
            }
            
            win = "/car/window/" + std::to_string(window);

            if(!recording)
                sendMessage(win, "stop");
                
            commandExecuted = true;
            break;

        case 10:
            if (!scenario_mode && !recording)
                printf("Syntax error: delay command can only be used in scenario or record mode\n");
            else {
                commandExecuted = true;
                
                if(recording) break;

                int ms = stoi(args[1]);
                if (ms >= 0)
                    usleep(ms * 1000);
            }
            break;

        case 11:
            if (scenario_mode || recording) {
                printf("Syntax error: run command cannot be used in scenario or recording mode\n");
                return true;
            }

            if (access(args[1].c_str(), F_OK) == 0) {
                sfile.open(args[1]);

                if(!sfile) {
                    printf("Error: Couldn't open the scenario file - %s\n", args[1].c_str());
                    return true;
                }
                std::string tmp;
                std::getline(sfile, tmp);
                if(tmp != "autox")
                {
                    printf("Error: Not a vaild scenario file - %s\n", args[1].c_str());
                    sfile.close();
                    return true;
                }
                scenario_mode = true;
            }
            break;
        case 12:
            if (scenario_mode) {
                printf("Syntax error: record command cannot be used in scenario mode\n");
                return true;
            }

            if(args[1].empty()){
                 printf("Syntax error: no file name given %s\n", args[1].c_str());
                return true;
            }
            else{
                fileName = args[1];;

                //Clearing content of scenario file
                scenarioFile.open(scenarioDir + fileName + ".autox", std::ofstream::out | std::ofstream::trunc);
                scenarioFile.close();

                scenarioFile.open(scenarioDir + fileName + ".autox",std::ios::out|std::ios::app);
                scenarioFile << "autox";
                scenarioFile.close();

                recording = true;
            }
            break;

        case 13:
            if (scenario_mode) {
                printf("Syntax error: stop_rec command cannot be used in scenario mode\n");
                return true;
            }

            if(!recording){
                printf("Syntax error: stop_rec command cannot be used in while not recording\n");
                return true;
            }

            if(args.size() != 1){
                printf("Syntax error: too much arguments %s\n", args[1].c_str());
                return true;
            }

            if(scenarioFile.is_open()) scenarioFile.close();

            printf("Recording stopped\n");

            recording = false;
            break;

    }

    return true;
}

void recordAction(char *command, std::string fileName){
    scenarioFile.open(scenarioDir + fileName + ".autox",std::ios::out|std::ios::app);
    scenarioFile << std::endl << command;
    scenarioFile.close();
}


