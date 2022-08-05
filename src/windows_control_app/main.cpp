#include <iostream>
#include <string>
#include <array>
#include <unistd.h>

std::array<int, 4> szyby = {0,0,0,0};

void drawWindows(){
    std::cout<< u8"\033[2J\033[1;1H";
    for(int i = 0; i < 10; i++){
        for(int x = 0; x < 4; x++){
            if((i-szyby[x]) > 0){
                std::cout << "  |||||  ";
            }else{
                std::cout << "         ";
            }
        }
        std::cout << "\n";
    }
}

int main(){
    int counter = 0;
    bool isMoving = false;
    int moved = 0;
    int val = 1;
    int windowState;
    while(true){
        drawWindows();
        if(isMoving){
            while(val != 0){
                windowState = szyby[moved];
                szyby[moved] = szyby[moved] - (val / abs(val));
                if(szyby[moved] > 9 || szyby[moved] < 0){
                    szyby[moved] = windowState;
                    isMoving = false;
                    break;
                }
                drawWindows();
                std::cout.flush();
                sleep(1);
                if(val > 0){
                    val--;
                }else{
                    val++;
                }
            }
            isMoving = false;
        }else{
            std::cin >> moved >> val;
            isMoving = true;
        }
    }
    return 0;
}