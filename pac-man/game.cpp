
// c++ libereys and SFML
#include <iostream>

#include <SFML/Window.hpp>

// exterel

#include "game.h"
#include "object/enemy_gost_1.h"
#include "object/pac-man.h"

#include "map/map.h"
#include "object/layer.h"

Map gameMap;

#define x_acess 1000
#define y_acess 840


#include <filesystem>

bool game::init()
{

    std::cout << "Working dir: " << std::filesystem::current_path() << std::endl;
    // lowiding map
    if (!gameMap.loadFromFile("data/map/map.json"))
    {
        std::cout << "Failed to load map data." << std::endl;
        return false;
    }



    // Standard SFML setup
    window.create(sf::VideoMode({x_acess, y_acess}), "Pac-Man");

    // Double the size of the screen
    sf::View view = window.getDefaultView();
    view.setSize({view.getSize().x / 2, view.getSize().y / 2});
    view.setCenter({view.getCenter().x / 2, view.getCenter().y / 2});
    window.setView(view);

    window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(60);

    float  shapeWith = x_acess / 2.f;
    float shapehigth = y_acess / 2.f;

    float max_X = shapeWith;
    float max_Y = shapehigth;

    objects.push_back(std::make_unique<pac_man>(160.f, 120.f, max_Y, max_X));
    objects.push_back(std::make_unique<gost_1>(160.f, 120.f, max_X, max_Y));



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


    for (auto& object : objects)
    {
        object->logic(deltaTime);
    }

    window.clear(sf::Color::Black);

    for (auto& object : objects)
    {
        object->draw(window);
    }

    for (auto& obj : gameMap.GetObjects())
    {
        obj->draw(window);
    }

    // Then draw game objects on top
    for (auto& object : objects)
    {
        object->draw(window);
    }

    window.display();

    return true;
}
