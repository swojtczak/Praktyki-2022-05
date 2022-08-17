#include <cstring>
#include <iostream>
#include "prompt.h"

bool debug = false;

int main(int argc, char **argv)
{
    if (argc >= 2 && !strcmp(argv[1], "--debug"))
        debug = true;
    repl_loop(debug);

    return 0;
}