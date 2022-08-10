#include <string>
#include <vector>

#define PROMPT "-> "
#define HELP_MESSAGE "autox - v0.1\n\
Commands:\n\
exit \t\t-> exit this prompt\n\
help \t\t-> write this help message\n\
test \t\t-> write test123\n\
mykmyk \t\t-> write 123mykmyk"

struct command {
    std::string command;
    int len;
};

const struct command command_list[] = {
    {"exit",        1},
    {"help",        1},
    {"open",        3},
    {"close",       3},
    {"turn_on",     3},
    {"turn_off",    3},
    {"wipe",        3},
    {"wipe_w_fluid", 3},
    {"wipe_off",    2},
};


void repl_loop();
int check_operator(std::string op);
std::vector<std::string> split_line(std::string line);
bool execute_instruction(int instruction, std::vector<std::string> args);
