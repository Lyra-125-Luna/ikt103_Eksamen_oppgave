#ifndef IKT103_EKSAMEN_OPPGAVE_GAME_H
#define IKT103_EKSAMEN_OPPGAVE_GAME_H

#include <iostream>

#include "map/map.h"

#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>

#include <list>

class Object;


class game
{
public:
    bool innit();
    void run()


protected:

    bool gameTick(float deltaTime);

    sf::Clock clock;

    sf::RenderWindow window;

    Map map;

}



#endif //IKT103_EKSAMEN_OPPGAVE_GAME_H