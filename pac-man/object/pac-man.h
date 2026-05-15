#ifndef PAC_MAN_PAC_MAN_H
#define PAC_MAN_PAC_MAN_H

#include <SFML/Graphics.hpp>
#include <random>
#include <iostream>
#include <cmath>
#include <array>
#include <string>

#include "object/object.h"
#include "map/map.h"

class pac_man : public Object
{
public:

  pac_man(float x, float y, float maxY, float maxX, Map& map)
      : Object(x, y), max_Y(maxY), max_X(maxX), map(map), sprite(animationTextures[0])
  {
    srand(static_cast<unsigned>(time(0)));

    loadSprites();

    spawnX = x;
    spawnY = y;

    snapToTileCenter();
    sprite.setPosition(getPacmanCenter());
  }

  void collision_handler(const sf::FloatRect &wall)
  {
    // Not used right now.
  }

  void logic(float deltaTime) override
  {
    if (dying)
    {
      updateDeathAnimation(deltaTime);
      sprite.setPosition(getPacmanCenter());
      return;
    }

    updatePowerMode(deltaTime);

    handleConnectionMessage();
    handleInput();

    if (!isMoving)
    {
      chooseNextTile();
    }

    moveToTargetTile(deltaTime);
    updateAnimation(deltaTime);
    updateSpriteDirection();

    sprite.setPosition(getPacmanCenter());
  }

  void draw(sf::RenderWindow &window) override
  {
    window.draw(sprite);
  }

  sf::Vector2i getGridPosition() const
  {
    return getCurrentTile();
  }

  sf::Vector2i getDirection() const
  {
    return direction;
  }

  bool hasStartedMoving() const
  {
    return startedMoving;
  }

  void activatePowerMode(float duration)
  {
    if (dying)
    {
      return;
    }

    powerModeTimer = duration;
    speed = normalSpeed * 1.35f;
  }

  bool isPowerModeActive() const
  {
    return powerModeTimer > 0.f;
  }

  void startDeathAnimation()
  {
    if (dying)
    {
      return;
    }

    dying = true;
    deathAnimationFinished = false;
    deathFrame = 0;
    deathTimer = 0.f;

    direction = {0, 0};
    wantedDirection = {0, 0};
    isMoving = false;
    speed = normalSpeed;
    powerModeTimer = 0.f;

    sprite.setRotation(sf::degrees(0));
    sprite.setTexture(deathTextures[0], true);
    setupSpriteFromTexture(deathTextures[0]);
    sprite.setPosition(getPacmanCenter());
  }

  bool isDying() const
  {
    return dying;
  }

  bool isDeathAnimationFinished() const
  {
    return deathAnimationFinished;
  }

  void resetForRestart()
  {
    x = spawnX;
    y = spawnY;

    direction = {0, 0};
    wantedDirection = {0, 0};
    isMoving = false;
    startedMoving = false;

    dying = false;
    deathAnimationFinished = false;
    deathFrame = 0;
    deathTimer = 0.f;

    speed = normalSpeed;
    powerModeTimer = 0.f;

    sprite.setTexture(animationTextures[0], true);
    setupSpriteFromTexture(animationTextures[0]);
    sprite.setRotation(sf::degrees(0));

    snapToTileCenter();
    sprite.setPosition(getPacmanCenter());
  }

  sf::FloatRect getBounds() const
  {
    return sprite.getGlobalBounds();
  }

protected:

  static constexpr float radius = 10.f;
  static constexpr float tileSize = 25.f;

  std::array<sf::Texture, 5> animationTextures;
  std::array<sf::Texture, 6> deathTextures;
  sf::Sprite sprite;

  int animationFrame = 0;
  float animationTimer = 0.f;

  int deathFrame = 0;
  float deathTimer = 0.f;
  bool dying = false;
  bool deathAnimationFinished = false;

  unsigned int joystickId = 0;
  bool wasConnected = false;

  float max_Y = 0.f;
  float max_X = 0.f;

  float spawnX = 0.f;
  float spawnY = 0.f;

  Map& map;

  bool startup = true;
  bool startedMoving = false;

  sf::Vector2i direction{0, 0};
  sf::Vector2i wantedDirection{0, 0};

  bool isMoving = false;

  sf::Vector2f targetCenter{0.f, 0.f};

  float normalSpeed = 60.f;
  float speed = normalSpeed;

  float powerModeTimer = 0.f;

  void loadSprites()
  {
    for (int i = 0; i < 5; i++)
    {
      std::string path = "data/assets/pacman/pacman" + std::to_string(i + 1) + ".png";

      if (!animationTextures[i].loadFromFile(path))
      {
        std::cout << "Failed to load Pac-Man sprite: " << path << std::endl;
      }
    }

    for (int i = 0; i < 6; i++)
    {
      std::string path = "data/assets/pacman/pacmandie" + std::to_string(i + 1) + ".png";

      if (!deathTextures[i].loadFromFile(path))
      {
        std::cout << "Failed to load Pac-Man death sprite: " << path << std::endl;
      }
    }

    sprite.setTexture(animationTextures[0], true);
    setupSpriteFromTexture(animationTextures[0]);
  }

  void setupSpriteFromTexture(const sf::Texture& texture)
  {
    sf::Vector2u size = texture.getSize();

    if (size.x == 0 || size.y == 0)
    {
      return;
    }

    sprite.setOrigin({
        static_cast<float>(size.x) / 2.f,
        static_cast<float>(size.y) / 2.f
    });

    float diameter = radius * 2.f;

    sprite.setScale({
        diameter / static_cast<float>(size.x),
        diameter / static_cast<float>(size.y)
    });
  }

  void updateAnimation(float deltaTime)
  {
    if (direction == sf::Vector2i{0, 0})
    {
      return;
    }

    animationTimer += deltaTime;

    if (animationTimer >= 0.07f)
    {
      animationTimer = 0.f;
      animationFrame = (animationFrame + 1) % static_cast<int>(animationTextures.size());
      sprite.setTexture(animationTextures[animationFrame], true);
      setupSpriteFromTexture(animationTextures[animationFrame]);
    }
  }

  void updateDeathAnimation(float deltaTime)
  {
    if (deathAnimationFinished)
    {
      return;
    }

    deathTimer += deltaTime;

    if (deathTimer >= 0.14f)
    {
      deathTimer = 0.f;
      deathFrame++;

      if (deathFrame >= static_cast<int>(deathTextures.size()))
      {
        deathFrame = static_cast<int>(deathTextures.size()) - 1;
        deathAnimationFinished = true;
      }

      sprite.setTexture(deathTextures[deathFrame], true);
      setupSpriteFromTexture(deathTextures[deathFrame]);
    }
  }

  void updateSpriteDirection()
  {
    if (direction == sf::Vector2i{1, 0})
    {
      sprite.setRotation(sf::degrees(0));
    }
    else if (direction == sf::Vector2i{-1, 0})
    {
      sprite.setRotation(sf::degrees(180));
    }
    else if (direction == sf::Vector2i{0, -1})
    {
      sprite.setRotation(sf::degrees(270));
    }
    else if (direction == sf::Vector2i{0, 1})
    {
      sprite.setRotation(sf::degrees(90));
    }
  }

  void updatePowerMode(float deltaTime)
  {
    if (powerModeTimer <= 0.f)
    {
      return;
    }

    powerModeTimer -= deltaTime;

    if (powerModeTimer <= 0.f)
    {
      powerModeTimer = 0.f;
      speed = normalSpeed;
    }
  }

  void handleConnectionMessage()
  {
    bool isConnected = sf::Joystick::isConnected(joystickId);

    if (startup)
    {
      if (isConnected && !wasConnected)
      {
        std::cout << "Joystick connected\n";
      }
      else if (!isConnected && wasConnected)
      {
        std::cout << "Joystick disconnected\n";
      }

      startup = false;
    }

    wasConnected = isConnected;
  }

  void handleInput()
  {
    bool isConnected = sf::Joystick::isConnected(joystickId);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
    {
      wantedDirection = {1, 0};
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
    {
      wantedDirection = {-1, 0};
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
    {
      wantedDirection = {0, -1};
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
    {
      wantedDirection = {0, 1};
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
    {
      direction = {0, 0};
      wantedDirection = {0, 0};
      isMoving = false;
    }

    if (isConnected && sf::Joystick::isButtonPressed(joystickId, 0))
    {
      wantedDirection = {0, 1};
    }
    else if (isConnected && sf::Joystick::isButtonPressed(joystickId, 1))
    {
      wantedDirection = {1, 0};
    }
    else if (isConnected && sf::Joystick::isButtonPressed(joystickId, 2))
    {
      wantedDirection = {0, -1};
    }
    else if (isConnected && sf::Joystick::isButtonPressed(joystickId, 3))
    {
      wantedDirection = {-1, 0};
    }

    if (isConnected && sf::Joystick::isButtonPressed(joystickId, 4))
    {
      direction = {0, 0};
      wantedDirection = {0, 0};
      isMoving = false;
    }
  }

  void chooseNextTile()
  {
    snapToTileCenter();

    sf::Vector2i currentTile = getCurrentTile();

    if (wantedDirection != sf::Vector2i{0, 0})
    {
      sf::Vector2i wantedTile = currentTile + wantedDirection;

      // Wrap horizontally
      if (wantedTile.x < 0)
      {
        wantedTile.x = map.getWidth() - 1;
      }
      else if (wantedTile.x >= map.getWidth())
      {
        wantedTile.x = 0;
      }

      if (!map.isWall(wantedTile.x, wantedTile.y))
      {
        direction = wantedDirection;
      }
    }

    if (direction == sf::Vector2i{0, 0})
    {
      return;
    }

    sf::Vector2i nextTile = currentTile + direction;

    // Wrap horizontally
    if (nextTile.x < 0)
    {
      nextTile.x = map.getWidth() - 1;
    }
    else if (nextTile.x >= map.getWidth())
    {
      nextTile.x = 0;
    }

    if (map.isWall(nextTile.x, nextTile.y))
    {
      direction = {0, 0};
      return;
    }

    targetCenter = getTileCenter(nextTile.x, nextTile.y);

    isMoving = true;
    startedMoving = true;
  }

  void moveToTargetTile(float deltaTime)
  {
    if (!isMoving)
    {
      return;
    }

    sf::Vector2f center = getPacmanCenter();

    float moveAmount = speed * deltaTime;

    float dx = targetCenter.x - center.x;
    float dy = targetCenter.y - center.y;

    float distance = std::sqrt(dx * dx + dy * dy);

    if (distance <= moveAmount)
    {
      x = targetCenter.x - radius;
      y = targetCenter.y - radius;

      // INSTANT tunnel wrap
      sf::Vector2i tile = getCurrentTile();

      if (tile.x <= 0 && direction.x < 0)
      {
        x = (map.getWidth() - 1) * tileSize;
      }
      else if (tile.x >= map.getWidth() - 1 && direction.x > 0)
      {
        x = 0;
      }

      isMoving = false;
      return;
    }

    if (distance > 0.f)
    {
      center.x += (dx / distance) * moveAmount;
      center.y += (dy / distance) * moveAmount;

      x = center.x - radius;
      y = center.y - radius;
    }
  }

  sf::Vector2f getPacmanCenter() const
  {
    return {
        x + radius,
        y + radius
    };
  }

  sf::Vector2i getCurrentTile() const
  {
    sf::Vector2f center = getPacmanCenter();

    return {
        static_cast<int>(center.x / tileSize),
        static_cast<int>(center.y / tileSize)
    };
  }

  sf::Vector2f getTileCenter(int tileX, int tileY) const
  {
    return {
        tileX * tileSize + tileSize / 2.f,
        tileY * tileSize + tileSize / 2.f
    };
  }

  void snapToTileCenter()
  {
    sf::Vector2i tile = getCurrentTile();
    sf::Vector2f tileCenter = getTileCenter(tile.x, tile.y);

    x = tileCenter.x - radius;
    y = tileCenter.y - radius;

    sprite.setPosition(tileCenter);
  }
};


#endif // PAC_MAN_PAC_MAN_H
