#include <iostream>

#include "game.h"


int main() {

    game game;

    std::cout << "compiled" << std::endl;


    game.init();
    game.run();


    std::cout << "prsooses termened" << std::endl;

    return 0;
}