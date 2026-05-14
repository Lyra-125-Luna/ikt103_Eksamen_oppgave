#ifndef PAC_MAN_PAC_MAN_H
#define PAC_MAN_PAC_MAN_H

#include <SFML/Graphics.hpp>
#include <random>
#include <iostream>
#include <cmath>

#include "object/object.h"
#include "map/map.h"

class pac_man : public Object
{
public:

  sf::Vector2i getGridPosition() const
  {
    return getCurrentTile();
  }

  sf::Vector2i getDirection() const
  {
    return direction;
  }

  pac_man(float x, float y, float maxY, float maxX, Map& map)
      : Object(x, y), max_Y(maxY), max_X(maxX), map(map)
  {
    srand(static_cast<unsigned>(time(0)));

    shape.setRadius(radius);
    shape.setFillColor(sf::Color::Yellow);

    // Start Pac-Man centered on the nearest tile.
    snapToTileCenter();

    shape.setPosition({x, y});
  }

  void collishon_handler(const sf::FloatRect &wall)
  {
    // Not used right now.
  }

  void logic(float deltaTime)
  {
    handleConnectionMessage();
    handleInput();

    if (!isMoving)
    {
      chooseNextTile();
    }

    moveToTargetTile(deltaTime);

    shape.setPosition({x, y});
  }

  void draw(sf::RenderWindow &window) override
  {
    window.draw(shape);
  }

  sf::FloatRect getBounds() const
  {
    return shape.getGlobalBounds();
  }

protected:

  sf::CircleShape shape;

  static constexpr float radius = 10.f;
  static constexpr float tileSize = 25.f;

  unsigned int joystickId = 0;
  bool wasConnected = false;

  float max_Y = 0.f;
  float max_X = 0.f;

  Map& map;

  bool startup = true;

  sf::Vector2i direction{0, 0};
  sf::Vector2i wantedDirection{0, 0};

  bool isMoving = false;

  sf::Vector2f targetCenter{0.f, 0.f};

  float speed = 60.f;

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
      wantedDirection = {0, 1};   // down
    }
    else if (isConnected && sf::Joystick::isButtonPressed(joystickId, 1))
    {
      wantedDirection = {1, 0};   // right
    }
    else if (isConnected && sf::Joystick::isButtonPressed(joystickId, 2))
    {
      wantedDirection = {0, -1};  // up
    }
    else if (isConnected && sf::Joystick::isButtonPressed(joystickId, 3))
    {
      wantedDirection = {-1, 0};  // left
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

    // First try the direction the player wants.
    if (wantedDirection != sf::Vector2i{0, 0})
    {
      sf::Vector2i wantedTile = currentTile + wantedDirection;

      if (!map.isWall(wantedTile.x, wantedTile.y))
      {
        direction = wantedDirection;
      }
    }

    // If current direction is blocked, stop.
    if (direction == sf::Vector2i{0, 0})
    {
      return;
    }

    sf::Vector2i nextTile = currentTile + direction;

    if (map.isWall(nextTile.x, nextTile.y))
    {
      direction = {0, 0};
      return;
    }

    // The next tile is valid. Move toward its center.
    targetCenter = getTileCenter(nextTile.x, nextTile.y);
    isMoving = true;
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
      // Arrived exactly at the next tile center.
      x = targetCenter.x - radius;
      y = targetCenter.y - radius;

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
  }
};

#endif // PAC_MAN_PAC_MAN_H