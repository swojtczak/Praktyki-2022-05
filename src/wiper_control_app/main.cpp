#include <iostream>
#include <string>
#include <unistd.h>
#include <stdlib.h>


void drawWipers(int state, bool sprinklersOn){
    int lenghtOfWipers = 6;
    switch(state){
        case 0:
            if(!sprinklersOn){
                for(short spacing = 0; spacing < lenghtOfWipers - 1; spacing++) std::cout << "\n";
            }
            std::cout << "<";
            for(short count = 0; count <= lenghtOfWipers * 2 + 1; count++){
                std::cout << "--";
                
                if(count == lenghtOfWipers) std::cout << " <";
            }

            std::cout << std::endl;
            break;
        case 1:
            for(short count = 0; count < lenghtOfWipers; count++){
                for(short spacing = 0; spacing < count; spacing++) std::cout << " ";
                std::cout << "       \\";
                std::cout << "          \\" << std::endl; 
            }
            break;
        case 2:
            for(short count = 0; count < lenghtOfWipers; count++) std::cout << "             |          |" << std::endl; 

            break;
        case 3:
            for(short count = 0; count < lenghtOfWipers; count++){
                for(short spacing = lenghtOfWipers * 2; spacing > count; spacing--) std::cout << " ";
                std::cout << "       /";
                std::cout << "          /" << std::endl; 
            }
            break;
        case 4:
            for(short spacing = 0; spacing < lenghtOfWipers - 1; spacing++) std::cout << "\n";
            std::cout << "             ";
            for(short count = 0; count <= lenghtOfWipers * 2 + 1; count++){

                std::cout << "--";
                
                if(count == lenghtOfWipers) std::cout << "> ";
            }

            std::cout << ">" << std::endl;
            break;
        
        default:
            std::cout << "Choose option from the range (0-3)\n";
    }
}

bool stopWiping;

void wipe(short mode){
    short wiperState = 0;
    if(mode == 2){
        int count = 0;
        int sprinklersCount = 0;
        for(;;){
            if(count == 5) break;
            std::cout<< u8"\033[2J\033[1;1H";
    
            for(short spacing = 3 - sprinklersCount; spacing >= 0; spacing--) std::cout << "\n";
            std::cout << "        T               T\n";

            if(sprinklersCount > 0){
                for(short sprinklers = 0; sprinklers < sprinklersCount; sprinklers++){
                    std::cout << "        |               |\n";
                }
            }

            count++;
            sprinklersCount++;

            drawWipers(0, true);
            std::cout.flush();
            usleep(125000);
        }
        usleep(500000);
    }
    for(;;){
        std::cout<< u8"\033[2J\033[1;1H";
        drawWipers(wiperState, false);
        wiperState++;
        std::cout.flush();
        usleep(500000);
        if(wiperState > 4){
            for(short backwardRun = 3; backwardRun > 0; backwardRun--){
                std::cout<< u8"\033[2J\033[1;1H";
                drawWipers(backwardRun, false);
                std::cout.flush();
                usleep(500000);
            }
            wiperState = 0;
            if(mode == 1 || stopWiping) return;
        } 
    }
}

int main(){
    int mode = 0;
    bool turnedOn = false;

    for(;;){
        if(!turnedOn) drawWipers(0, false);
        if(turnedOn){
            system("clear");
            switch(mode){
                //Wipe continuously 
                case 0:
                    wipe(0);
                    break;

                //Wipe once
                case 1:
                    wipe(1);
                    break;

                //Wipe once with fluid
                case 2:
                    wipe(2);
                    break;
                
                //Stop wiping
                case 3:
                    stopWiping = true;
                    break;
                default:

                break;

            }
            turnedOn = false;
        }
        else{
            std::cout << "\n\n";
            std::cin >> mode;
            turnedOn = true;
        }
    }
}
