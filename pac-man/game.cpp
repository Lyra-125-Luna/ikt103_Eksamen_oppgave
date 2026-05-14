// c++ libraries and SFML
#include <iostream>
#include <filesystem>

#include <SFML/Window.hpp>

// external
#include "game.h"
#include "object/Ghosts.h"
#include "object/pac-man.h"

#include "map/map.h"
#include "object/layer.h"

Map gameMap;

#define x_acess 875
#define y_acess 625

bool game::init()
{
    std::cout << "Working dir: " << std::filesystem::current_path() << std::endl;

    // Loading map
    if (!gameMap.loadFromFile("data/map/map.json"))
    {
        std::cout << "Failed to load map data." << std::endl;
        return false;
    }

    // Standard SFML setup
    window.create(sf::VideoMode({x_acess, y_acess}), "Pac-Man");

    window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(60);

    float max_X = x_acess;
    float max_Y = y_acess;

    auto player = std::make_unique<pac_man>(
        17 * 25.f + 12.5f - 10.f,
        18 * 25.f + 12.5f - 10.f,
        max_Y,
        max_X,
        gameMap
    );

    pac_man* playerPtr = player.get();

    objects.push_back(std::move(player));

    objects.push_back(std::make_unique<ghost>(
        16 * 25.f + 12.5f - 10.f,
        11 * 25.f + 12.5f - 10.f,
        gameMap,
        *playerPtr,
        sf::Color::Red,
        50.f,
        GhostAI::DirectChase
    ));

    objects.push_back(std::make_unique<ghost>(
        17 * 25.f + 12.5f - 10.f,
        11 * 25.f + 12.5f - 10.f,
        gameMap,
        *playerPtr,
        sf::Color::Cyan,
        45.f,
        GhostAI::Random
    ));

    objects.push_back(std::make_unique<ghost>(
        18 * 25.f + 12.5f - 10.f,
        11 * 25.f + 12.5f - 10.f,
        gameMap,
        *playerPtr,
        sf::Color::Magenta,
        55.f,
        GhostAI::Ambush
    ));

    objects.push_back(std::make_unique<ghost>(
        16 * 25.f + 12.5f - 10.f,
        12 * 25.f + 12.5f - 10.f,
        gameMap,
        *playerPtr,
        sf::Color::Green,
        40.f,
        GhostAI::SlowChase
    ));

    objects.push_back(std::make_unique<ghost>(
        18 * 25.f + 12.5f - 10.f,
        12 * 25.f + 12.5f - 10.f,
        gameMap,
        *playerPtr,
        sf::Color::White,
        55.f,
        GhostAI::Patrol
    ));

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

// Process and draw one frame of the game
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

        if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
        {
            // Exit program on escape
            if (keyPressed->code == sf::Keyboard::Key::Escape)
            {
                window.close();
                return false;
            }
        }
    }

    for (auto& object : objects)
    {
        object->logic(deltaTime);
    }

    window.clear(sf::Color::Black);

    // Draw map first
    for (auto& obj : gameMap.GetObjects())
    {
        obj->draw(window);
    }

    // Draw Pac-Man and ghosts on top
    for (auto& object : objects)
    {
        object->draw(window);
    }

    window.display();

    return true;
}