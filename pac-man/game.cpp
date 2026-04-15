#include "game.h"
#include "map/map.h"
#include "objects/object.h"

#include <SFML/Window.hpp>

#include <iostream>

bool Game::init()
{
    // Load map information from JSON into object list
    if (!map.loadFromFile("data/map.json"))
    {
        std::cout << "Failed to load map data." << std::endl;
        return false;
    }

    // Standard SFML setup
    window.create(sf::VideoMode({640, 480}), "Mario?");

    // Double the size of the screen
    sf::View view = window.getDefaultView();
    view.setSize({view.getSize().x / 2, view.getSize().y / 2});
    view.setCenter({view.getCenter().x / 2, view.getCenter().y / 2});
    window.setView(view);

    window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(60);

    return true;
}

void Game::run()
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
bool Game::gameTick(float deltaTime)
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
                // Reload map on F5
                if (keyPressed->code == sf::Keyboard::Key::F5)
                {
                    if (!map.loadFromFile("data/map.json"))
                    {
                        std::cout << "Failed to reload map data." << std::endl;
                        return false;
                    }
                }

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

    // Process and render each object
    for (auto &object: map.GetObjects())
    {
        object->process(deltaTime);
        object->draw(window);
    }

    window.display();

    return true;
}
