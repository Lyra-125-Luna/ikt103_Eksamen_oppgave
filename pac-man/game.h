#ifndef IKT103_EKSAMEN_OPPGAVE_GAME_H
#define IKT103_EKSAMEN_OPPGAVE_GAME_H

#include <iostream>


#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>

#include <list>

class Object;


class game
{
public:
    bool init();
    void run();


protected:

    bool gameTick(float deltaTime);

    sf::Clock clock;

    sf::RenderWindow window;

};

#endif //IKT103_EKSAMEN_OPPGAVE_GAME_H