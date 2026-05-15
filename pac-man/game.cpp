#include <SFML/Graphics.hpp>

#include <algorithm>
#include <string>
#include <vector>

#include "game.h"
#include "map/map.h"
#include "object/Ghosts.h"
#include "object/layer.h"
#include "object/pac-man.h"

Map gameMap;

static constexpr unsigned int windowWidth = 1000;
static constexpr unsigned int windowHeight = 1000;
static constexpr float tileSize = 25.f;

static pac_man* playerPtr = nullptr;

static float tileToPixel(int tile)
{
    return tile * tileSize + tileSize / 2.f - 10.f;
}

static void setScaledMapView(sf::RenderWindow& window, float mapPixelWidth, float mapPixelHeight)
{
    sf::View view;
    view.setSize({mapPixelWidth, mapPixelHeight});
    view.setCenter({mapPixelWidth / 2.f, mapPixelHeight / 2.f});

    sf::Vector2u windowSize = window.getSize();

    float windowRatio = static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y);
    float mapRatio = mapPixelWidth / mapPixelHeight;

    sf::FloatRect viewport({0.f, 0.f}, {1.f, 1.f});

    if (windowRatio > mapRatio)
    {
        float viewportWidth = mapRatio / windowRatio;
        viewport.position.x = (1.f - viewportWidth) / 2.f;
        viewport.size.x = viewportWidth;
    }
    else
    {
        float viewportHeight = windowRatio / mapRatio;
        viewport.position.y = (1.f - viewportHeight) / 2.f;
        viewport.size.y = viewportHeight;
    }

    view.setViewport(viewport);
    window.setView(view);
}

bool game::init()
{
    if (!gameMap.loadFromFile("data/map/map.json"))
    {
        return false;
    }

    window.create(sf::VideoMode({windowWidth, windowHeight}), "Pac-Man");
    window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(60);

    float mapPixelWidth = static_cast<float>(gameMap.getWidth()) * tileSize;
    float mapPixelHeight = static_cast<float>(gameMap.getHeight()) * tileSize;

    setScaledMapView(window, mapPixelWidth, mapPixelHeight);

    hudFontLoaded = hudFont.openFromFile("data/assets/arial.ttf");

    if (!hudFontLoaded)
    {
        hudFontLoaded = hudFont.openFromFile("C:/Windows/Fonts/arial.ttf");
    }

    cherryTexture.loadFromFile("data/assets/cherry.png");

    createCharacters();
    scanCollectibles();

    sounds.load();
    sounds.playMusic();

    return true;
}

void game::createCharacters()
{
    objects.clear();
    playerPtr = nullptr;

    int mapWidth = gameMap.getWidth();
    int mapHeight = gameMap.getHeight();

    int gateX = mapWidth / 2;

    if (mapWidth % 2 == 0)
    {
        gateX -= 1;
    }

    int houseY = mapHeight / 2;
    int pacmanSpawnY = mapHeight - 2;

    float maxX = static_cast<float>(mapWidth) * tileSize;
    float maxY = static_cast<float>(mapHeight) * tileSize;

    auto player = std::make_unique<pac_man>(
        tileToPixel(gateX),
        tileToPixel(pacmanSpawnY),
        maxY,
        maxX,
        gameMap
    );

    playerPtr = player.get();
    objects.push_back(std::move(player));

    objects.push_back(std::make_unique<ghost>(
        tileToPixel(gateX - 1),
        tileToPixel(houseY),
        gameMap,
        *playerPtr,
        GhostSpriteType::Red,
        50.f,
        GhostAI::DirectChase
    ));

    objects.push_back(std::make_unique<ghost>(
        tileToPixel(gateX),
        tileToPixel(houseY),
        gameMap,
        *playerPtr,
        GhostSpriteType::Blue,
        45.f,
        GhostAI::Random
    ));

    objects.push_back(std::make_unique<ghost>(
        tileToPixel(gateX + 1),
        tileToPixel(houseY),
        gameMap,
        *playerPtr,
        GhostSpriteType::Pink,
        55.f,
        GhostAI::Ambush
    ));

    objects.push_back(std::make_unique<ghost>(
        tileToPixel(gateX - 1),
        tileToPixel(houseY + 1),
        gameMap,
        *playerPtr,
        GhostSpriteType::Green,
        40.f,
        GhostAI::SlowChase
    ));

    objects.push_back(std::make_unique<ghost>(
        tileToPixel(gateX + 1),
        tileToPixel(houseY + 1),
        gameMap,
        *playerPtr,
        GhostSpriteType::Orange,
        55.f,
        GhostAI::Patrol
    ));
}

void game::resetGame()
{
    score = 0;
    lives = 3;
    gameWon = false;
    gameLost = false;

    createCharacters();
    scanCollectibles();
    clock.restart();
}

void game::resetAfterLifeLost()
{
    createCharacters();
    clock.restart();
}

void game::run()
{
    float deltaTime = 0;
    clock.restart();

    while (gameTick(deltaTime))
    {
        deltaTime = clock.getElapsedTime().asSeconds();
        clock.restart();
    }
}

bool game::movementKeyPressed() const
{
    return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) ||
           sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) ||
           sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) ||
           sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) ||
           sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W);
}

void game::scanCollectibles()
{
    normalPellets.clear();
    powerPellets.clear();
    fruits.clear();

    for (int y = 0; y < gameMap.getHeight(); y++)
    {
        for (int x = 0; x < gameMap.getWidth(); x++)
        {
            unsigned int tile = gameMap.getTileAt(x, y);

            if (tile == 0)
            {
                normalPellets.push_back({x, y});
            }
            else if (tile == 4)
            {
                powerPellets.push_back({x, y});
            }
            else if (tile == 5)
            {
                fruits.push_back({x, y});
            }
        }
    }
}

void game::checkCollectibles()
{
    if (playerPtr == nullptr || gameWon || gameLost || playerPtr->isDying())
    {
        return;
    }

    sf::Vector2i pacmanTile = playerPtr->getGridPosition();

    auto eraseTile = [pacmanTile](std::vector<sf::Vector2i>& tiles)
    {
        auto it = std::find(tiles.begin(), tiles.end(), pacmanTile);

        if (it != tiles.end())
        {
            tiles.erase(it);
            return true;
        }

        return false;
    };

    if (eraseTile(normalPellets))
    {
        score += 100;
        sounds.playPickUp();
    }

    if (eraseTile(fruits))
    {
        score += 500;
    }

    if (eraseTile(powerPellets))
    {
        score += 1000;
        sounds.playPowerUp(8.f);
        playerPtr->activatePowerMode(8.f);

        for (auto& object : objects)
        {
            ghost* currentGhost = dynamic_cast<ghost*>(object.get());

            if (currentGhost != nullptr)
            {
                currentGhost->makeScared(8.f);
            }
        }
    }

    if (normalPellets.empty() && powerPellets.empty())
    {
        gameWon = true;
    }
}

void game::checkGhostCollisions()
{
    if (playerPtr == nullptr || gameWon || gameLost || playerPtr->isDying())
    {
        return;
    }

    sf::FloatRect pacmanBounds = playerPtr->getBounds();

    for (auto& object : objects)
    {
        ghost* currentGhost = dynamic_cast<ghost*>(object.get());

        if (currentGhost == nullptr)
        {
            continue;
        }

        bool touchingGhost = pacmanBounds.findIntersection(currentGhost->getBounds()).has_value();

        if (!touchingGhost)
        {
            continue;
        }

        if (currentGhost->isScared())
        {
            score += 2000;
            currentGhost->makeDead();
        }
        else if (!currentGhost->isDead())
        {
            score = std::max(0, score - 5000);
            playerPtr->startDeathAnimation();
        }
    }
}

void game::drawNormalPellets()
{
    for (sf::Vector2i pellet : normalPellets)
    {
        sf::CircleShape pelletShape;
        pelletShape.setRadius(2.5f);
        pelletShape.setFillColor(sf::Color::White);
        pelletShape.setPosition({
            pellet.x * tileSize + tileSize / 2.f - 2.5f,
            pellet.y * tileSize + tileSize / 2.f - 2.5f
        });

        window.draw(pelletShape);
    }
}

void game::drawPowerPellets()
{
    for (sf::Vector2i pellet : powerPellets)
    {
        sf::CircleShape pelletShape;
        pelletShape.setRadius(6.f);
        pelletShape.setFillColor(sf::Color::White);
        pelletShape.setPosition({
            pellet.x * tileSize + tileSize / 2.f - 6.f,
            pellet.y * tileSize + tileSize / 2.f - 6.f
        });

        window.draw(pelletShape);
    }
}

void game::drawFruits()
{
    for (sf::Vector2i fruit : fruits)
    {
        sf::Sprite fruitSprite(cherryTexture);
        sf::Vector2u size = cherryTexture.getSize();

        if (size.x > 0 && size.y > 0)
        {
            fruitSprite.setOrigin({
                static_cast<float>(size.x) / 2.f,
                static_cast<float>(size.y) / 2.f
            });

            fruitSprite.setScale({
                20.f / static_cast<float>(size.x),
                20.f / static_cast<float>(size.y)
            });

            fruitSprite.setPosition({
                fruit.x * tileSize + tileSize / 2.f,
                fruit.y * tileSize + tileSize / 2.f
            });

            window.draw(fruitSprite);
        }
    }
}

void game::drawScore()
{
    if (!hudFontLoaded)
    {
        return;
    }

    int gateX = gameMap.getWidth() / 2;

    if (gameMap.getWidth() % 2 == 0)
    {
        gateX -= 1;
    }

    int houseY = gameMap.getHeight() / 2;

    sf::Text scoreText(hudFont);
    scoreText.setString("Score: " + std::to_string(score) + "\nLives: " + std::to_string(lives));
    scoreText.setCharacterSize(12);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setStyle(sf::Text::Bold);
    scoreText.setPosition({
        gateX * tileSize - 35.f,
        houseY * tileSize + 14.f
    });

    window.draw(scoreText);
}

void game::drawWinMessage()
{
    if (!gameWon || !hudFontLoaded)
    {
        return;
    }

    float mapPixelWidth = static_cast<float>(gameMap.getWidth()) * tileSize;
    float mapPixelHeight = static_cast<float>(gameMap.getHeight()) * tileSize;

    sf::RectangleShape background;
    background.setSize({mapPixelWidth * 0.75f, mapPixelHeight * 0.35f});
    background.setOrigin({
        background.getSize().x / 2.f,
        background.getSize().y / 2.f
    });
    background.setPosition({mapPixelWidth / 2.f, mapPixelHeight / 2.f});
    background.setFillColor(sf::Color(0, 0, 0, 220));
    background.setOutlineColor(sf::Color::Yellow);
    background.setOutlineThickness(3.f);

    window.draw(background);

    sf::Text title(hudFont);
    title.setString("YOU WIN");
    title.setCharacterSize(36);
    title.setFillColor(sf::Color::Yellow);
    title.setStyle(sf::Text::Bold);

    sf::FloatRect titleBounds = title.getLocalBounds();
    title.setOrigin({
        titleBounds.position.x + titleBounds.size.x / 2.f,
        titleBounds.position.y + titleBounds.size.y / 2.f
    });
    title.setPosition({mapPixelWidth / 2.f, mapPixelHeight / 2.f - 45.f});

    window.draw(title);

    sf::Text finalScore(hudFont);
    finalScore.setString("Score: " + std::to_string(score));
    finalScore.setCharacterSize(24);
    finalScore.setFillColor(sf::Color::White);
    finalScore.setStyle(sf::Text::Bold);

    sf::FloatRect scoreBounds = finalScore.getLocalBounds();
    finalScore.setOrigin({
        scoreBounds.position.x + scoreBounds.size.x / 2.f,
        scoreBounds.position.y + scoreBounds.size.y / 2.f
    });
    finalScore.setPosition({mapPixelWidth / 2.f, mapPixelHeight / 2.f + 15.f});

    window.draw(finalScore);
}

void game::drawLoseMessage()
{
    if (!gameLost || !hudFontLoaded)
    {
        return;
    }

    float mapPixelWidth = static_cast<float>(gameMap.getWidth()) * tileSize;
    float mapPixelHeight = static_cast<float>(gameMap.getHeight()) * tileSize;

    sf::RectangleShape background;
    background.setSize({mapPixelWidth * 0.8f, mapPixelHeight * 0.42f});
    background.setOrigin({
        background.getSize().x / 2.f,
        background.getSize().y / 2.f
    });
    background.setPosition({mapPixelWidth / 2.f, mapPixelHeight / 2.f});
    background.setFillColor(sf::Color(0, 0, 0, 230));
    background.setOutlineColor(sf::Color::Red);
    background.setOutlineThickness(3.f);

    window.draw(background);

    sf::Text title(hudFont);
    title.setString("YOU LOSE");
    title.setCharacterSize(36);
    title.setFillColor(sf::Color::Red);
    title.setStyle(sf::Text::Bold);

    sf::FloatRect titleBounds = title.getLocalBounds();
    title.setOrigin({
        titleBounds.position.x + titleBounds.size.x / 2.f,
        titleBounds.position.y + titleBounds.size.y / 2.f
    });
    title.setPosition({mapPixelWidth / 2.f, mapPixelHeight / 2.f - 55.f});

    window.draw(title);

    sf::Text finalScore(hudFont);
    finalScore.setString("Score: " + std::to_string(score));
    finalScore.setCharacterSize(24);
    finalScore.setFillColor(sf::Color::White);
    finalScore.setStyle(sf::Text::Bold);

    sf::FloatRect scoreBounds = finalScore.getLocalBounds();
    finalScore.setOrigin({
        scoreBounds.position.x + scoreBounds.size.x / 2.f,
        scoreBounds.position.y + scoreBounds.size.y / 2.f
    });
    finalScore.setPosition({mapPixelWidth / 2.f, mapPixelHeight / 2.f + 5.f});

    window.draw(finalScore);

    sf::Text restartText(hudFont);
    restartText.setString("Move to restart");
    restartText.setCharacterSize(18);
    restartText.setFillColor(sf::Color::White);

    sf::FloatRect restartBounds = restartText.getLocalBounds();
    restartText.setOrigin({
        restartBounds.position.x + restartBounds.size.x / 2.f,
        restartBounds.position.y + restartBounds.size.y / 2.f
    });
    restartText.setPosition({mapPixelWidth / 2.f, mapPixelHeight / 2.f + 55.f});

    window.draw(restartText);
}

bool game::gameTick(float deltaTime)
{
    while (const std::optional event = window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
        {
            window.close();
            return false;
        }

        if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
        {
            if (keyPressed->code == sf::Keyboard::Key::Escape)
            {
                window.close();
                return false;
            }
        }
    }

    if (gameLost)
    {
        if (movementKeyPressed())
        {
            resetGame();
        }
    }
    else if (!gameWon)
    {
        if (playerPtr != nullptr && playerPtr->isDying())
        {
            playerPtr->logic(deltaTime);

            if (playerPtr->isDeathAnimationFinished())
            {
                lives--;

                if (lives > 0)
                {
                    resetAfterLifeLost();
                }
                else
                {
                    gameLost = true;
                }
            }
        }
        else
        {
            bool ghostsCanMove = playerPtr != nullptr && playerPtr->hasStartedMoving();

            for (auto& object : objects)
            {
                ghost* currentGhost = dynamic_cast<ghost*>(object.get());

                if (currentGhost != nullptr && !ghostsCanMove)
                {
                    continue;
                }

                object->logic(deltaTime);
            }

            checkCollectibles();
            checkGhostCollisions();
        }
    }

    window.clear(sf::Color::Black);
    sounds.updatePowerUpState(deltaTime);

    for (auto& object : gameMap.GetObjects())
    {
        object->draw(window);
    }

    drawNormalPellets();
    drawPowerPellets();
    drawFruits();

    for (auto& object : objects)
    {
        object->draw(window);
    }

    drawScore();
    drawWinMessage();
    drawLoseMessage();

    window.display();

    return true;
}
