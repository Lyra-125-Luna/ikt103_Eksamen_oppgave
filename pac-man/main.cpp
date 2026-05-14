#include <iostream>

#include "game.h"

int main()
{
    game game;

    std::cout << "compiled" << std::endl;

    if (!game.init())
    {
        std::cout << "initialization failed" << std::endl;
        return 1;
    }

    game.run();

    std::cout << "process terminated" << std::endl;

    return 0;
}
