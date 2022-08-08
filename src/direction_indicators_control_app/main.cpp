#include <iostream>
#include <string>
#include <unistd.h>

bool indicatorsON;
bool left = true;

void drawIndicators(){
    for(int i = 0; i < 3; i++){
        if(left){
            std::cout<< u8"\033[2J\033[1;1H";
            std::cout<< "<\n";
            std::cout.flush();
            sleep(1);
            std::cout<< u8"\033[2J\033[1;1H\n";
            std::cout.flush();
            sleep(1);
        }
        else{
            std::cout<< u8"\033[2J\033[1;1H";
            std::cout<< ">\n";
            std::cout.flush();
            sleep(1);
            std::cout<< u8"\033[2J\033[1;1H\n";
            std::cout.flush();
            sleep(1);
        }
    }
}

int main(){
    std::string direction;

    while(true){
        drawIndicators();
        std::cin >> direction;
        left = (direction == "left");
    }
    return 0;
}