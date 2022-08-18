#include <cstring>
#include <iostream>
#include <unistd.h>
#include "prompt.h"

bool debug = false;


int main(int argc, char **argv)
{
    int option;

    while ((option = getopt(argc, argv, "dh")) != -1) {
        switch (option) {
            case 'd':
                debug = true;
                break;
            case 'h':
                printf("Autox v0.1\nOptions:\n-d\t\t-> turn on the debug mode\n-h\t\t-> view this help message\n\n");
                return 0;
                break;
            case '?':
                return 1;
                break;
        }
    }
    
    repl_loop(debug);

    return 0;
}