#include <iostream>
#include "prompt.h"


void repl_loop()
{
    bool status;
    int instruction;
    std::string op, line;
    std::vector<std::string> args;

    do {
        std::cout << PROMPT;
        std::cin >> line;
        args = split_line(line);
        instruction = check_operator(args[0]);
        status = execute_instruction(instruction);
    } while (status);
}


int check_operator(std::string op)
{
    int i;
    bool found = false;

    for (i = 0; i < (sizeof(command_list)/sizeof(command_list[0])); i++) {
        if (!command_list[i].command.compare(op)) {
            found = true;
            break;
        }
    }

    if (found)
        return i;
    else
        return -1;
}


std::vector<std::string> split_line(std::string line)
{
    std::vector<std::string> ret = {};

    int start = 0,
        end = line.find(" ");
    
    while (end != -1) {
        ret.push_back(line.substr(start, end - start));
        start = end + 1;
        end = line.find(" ");
    }
    ret.push_back(line.substr(start, end - start));

    return ret;
}


bool execute_instruction(int instruction)
{
    return true;
}