#include "game.h"


#include <SFML/Window.hpp>

#include <iostream>

int map = 0;

bool game::init()
{

    // Standard SFML setup
    window.create(sf::VideoMode({1000, 840}), "Pac-Man");

    // Double the size of the screen
    sf::View view = window.getDefaultView();
    view.setSize({view.getSize().x / 2, view.getSize().y / 2});
    view.setCenter({view.getCenter().x / 2, view.getCenter().y / 2});
    window.setView(view);

    window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(60);

    return true;
}

void game::run()
{
    float deltaTime = 0;
    clock.restart();

    // Game loop
    while (gameTick(deltaTime))
    {
        deltaTime = clock.getElapsedTime().asSeconds();
        clock.restart();
    }
}

// Process and draws one frame of the game
bool game::gameTick(float deltaTime)
{
    // Process events from the OS
    while (const std::optional event = window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
        {
            window.close();
            return false;
        }

        if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>())
        {
                // Exit program on escape
                if (keyPressed->code == sf::Keyboard::Key::Escape)
                {
                    window.close();
                    return false;
                }
                break;
        }
    }

    window.clear(sf::Color::Black);

    window.display();

    return true;
}
