#ifndef IKT103_EKSAMEN_OPPGAVE_GAME_H
#define IKT103_EKSAMEN_OPPGAVE_GAME_H

#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>
#include <memory>
#include <vector>

#include "data/audio/sounds.h"
#include "object/object.h"
#include "object/pac-man.h"

class game
{
public:
    bool init();
    void run();

    void scanCollectibles();
    void checkCollectibles();
    void checkGhostCollisions();

    void drawNormalPellets();
    void drawPowerPellets();
    void drawFruits();
    void drawScore();
    void drawWinMessage();
    void drawLoseMessage();

    void resetGame();

protected:
    bool gameTick(float deltaTime);

    void createCharacters();
    void resetAfterLifeLost();
    bool movementKeyPressed() const;

    sf::Clock clock;
    sf::RenderWindow window;

    std::vector<std::unique_ptr<Object>> objects;

    std::vector<sf::Vector2i> normalPellets;
    std::vector<sf::Vector2i> powerPellets;
    std::vector<sf::Vector2i> fruits;

    sf::Font hudFont;
    bool hudFontLoaded = false;

    sf::Texture cherryTexture;

    SoundManager sounds;

    int score = 0;
    int lives = 3;

    bool gameWon = false;
    bool gameLost = false;
};

#endif
