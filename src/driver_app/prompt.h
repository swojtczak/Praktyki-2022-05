#include <string>
#include <vector>

#define PROMPT "-> "

struct command {
    std::string command;
    std::string params;
    int num_of_params;
    std::string action;
};

const struct command command_list[] = {
    {"exit", "", 0, ""},
    {"test", "", 0, "test"},
};


void repl_loop();
int check_operator(std::string op);
std::vector<std::string> split_line(std::string line);
bool execute_instruction(int instruction);