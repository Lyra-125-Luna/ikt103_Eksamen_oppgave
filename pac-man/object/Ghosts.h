#ifndef PAC_MAN_GHOSTS_H
#define PAC_MAN_GHOSTS_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <limits>
#include <cstdlib>
#include <ctime>

#include "object/object.h"
#include "map/map.h"
#include "object/pac-man.h"

enum class GhostAI
{
  DirectChase,
  Ambush,
  Random,
  SlowChase,
  Patrol
};

class ghost : public Object
{
public:
  ghost(float x, float y, Map& map, pac_man& pacman, sf::Color color, float speed, GhostAI aiType)
      : Object(x, y), map(map), pacman(pacman), speed(speed), aiType(aiType)
  {
    srand(static_cast<unsigned>(time(0)));

    shape.setRadius(radius);
    shape.setFillColor(color);

    snapToTileCenter();
    shape.setPosition({this->x, this->y});
  }

  void logic(float deltaTime) override
  {
    if (!isMoving)
    {
      chooseNextTile();
    }

    moveToTargetTile(deltaTime);

    shape.setPosition({x, y});
  }

  void draw(sf::RenderWindow& window) override
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

  Map& map;
  pac_man& pacman;

  GhostAI aiType = GhostAI::DirectChase;

  sf::Vector2i direction{0, 0};
  sf::Vector2i previousTile{-1, -1};

  bool isMoving = false;
  sf::Vector2f targetCenter{0.f, 0.f};

  sf::Vector2i patrolTarget{1, 1};
  sf::Vector2i randomTarget{-1, -1};

  float speed = 50.f;

  void chooseNextTile()
  {
    snapToTileCenter();

    sf::Vector2i currentTile = getCurrentTile();
    std::vector<sf::Vector2i> availableDirections = getAvailableDirections(currentTile);

    if (availableDirections.empty())
    {
      direction = {0, 0};
      isMoving = false;
      return;
    }

    sf::Vector2i reverseDirection = {-direction.x, -direction.y};

    bool canContinueForward = false;

    for (sf::Vector2i dir : availableDirections)
    {
      if (dir == direction)
      {
        canContinueForward = true;
        break;
      }
    }

    int nonReverseOptions = 0;

    for (sf::Vector2i dir : availableDirections)
    {
      if (dir != reverseDirection)
      {
        nonReverseOptions++;
      }
    }

    bool isIntersection = nonReverseOptions >= 2;
    bool isDeadEnd = nonReverseOptions == 0;

    if (canContinueForward && !isIntersection && !isDeadEnd)
    {
      // Corridor: keep current direction.
    }
    else
    {
      direction = chooseDirection(currentTile, availableDirections);
    }

    if (direction == sf::Vector2i{0, 0})
    {
      isMoving = false;
      return;
    }

    sf::Vector2i nextTile = currentTile + direction;

    if (map.isWall(nextTile.x, nextTile.y))
    {
      direction = {0, 0};
      isMoving = false;
      return;
    }

    previousTile = currentTile;

    targetCenter = getTileCenter(nextTile.x, nextTile.y);
    isMoving = true;
  }

  std::vector<sf::Vector2i> getAvailableDirections(sf::Vector2i currentTile)
  {
    std::vector<sf::Vector2i> directions = {
        {1, 0},
        {-1, 0},
        {0, 1},
        {0, -1}
    };

    std::vector<sf::Vector2i> available;

    for (sf::Vector2i dir : directions)
    {
      sf::Vector2i nextTile = currentTile + dir;

      if (!map.isWall(nextTile.x, nextTile.y))
      {
        available.push_back(dir);
      }
    }

    return available;
  }

  sf::Vector2i chooseDirection(
      sf::Vector2i currentTile,
      const std::vector<sf::Vector2i>& availableDirections)
  {
    sf::Vector2i targetTile = chooseTargetTile(currentTile);
    sf::Vector2i reverseDirection = {-direction.x, -direction.y};

    sf::Vector2i bestDirection{0, 0};
    int bestDistance = std::numeric_limits<int>::max();

    for (sf::Vector2i candidateDirection : availableDirections)
    {
      sf::Vector2i nextTile = currentTile + candidateDirection;

      // Avoid immediately returning to the previous tile unless needed.
      if (availableDirections.size() > 1 && nextTile == previousTile)
      {
        continue;
      }

      // Avoid reversing unless needed.
      if (availableDirections.size() > 1 && candidateDirection == reverseDirection)
      {
        continue;
      }

      int dx = targetTile.x - nextTile.x;
      int dy = targetTile.y - nextTile.y;
      int distance = dx * dx + dy * dy;

      if (distance < bestDistance)
      {
        bestDistance = distance;
        bestDirection = candidateDirection;
      }
    }

    // Fallback: choose any direction that is not the previous tile.
    if (bestDirection == sf::Vector2i{0, 0})
    {
      for (sf::Vector2i candidateDirection : availableDirections)
      {
        sf::Vector2i nextTile = currentTile + candidateDirection;

        if (nextTile != previousTile)
        {
          bestDirection = candidateDirection;
          break;
        }
      }
    }

    // Final fallback: take the first available direction.
    if (bestDirection == sf::Vector2i{0, 0})
    {
      bestDirection = availableDirections.front();
    }

    return bestDirection;
  }

  sf::Vector2i chooseTargetTile(sf::Vector2i currentTile)
  {
    sf::Vector2i pacmanTile = pacman.getGridPosition();
    sf::Vector2i pacmanDirection = pacman.getDirection();

    switch (aiType)
    {
      case GhostAI::DirectChase:
        return directChaseTarget(pacmanTile);

      case GhostAI::Ambush:
        return ambushTarget(pacmanTile, pacmanDirection);

      case GhostAI::Random:
        return randomTargetTile();

      case GhostAI::SlowChase:
        return cautiousTarget(currentTile, pacmanTile);

      case GhostAI::Patrol:
        return patrolTargetTile(currentTile);
    }

    return pacmanTile;
  }

  sf::Vector2i directChaseTarget(sf::Vector2i pacmanTile)
  {
    // Red: directly targets Pac-Man.
    return pacmanTile;
  }

  sf::Vector2i ambushTarget(sf::Vector2i pacmanTile, sf::Vector2i pacmanDirection)
  {
    // Pink: targets 4 tiles in front of Pac-Man.
    if (pacmanDirection == sf::Vector2i{0, 0})
    {
      return pacmanTile;
    }

    return pacmanTile + pacmanDirection * 4;
  }

  sf::Vector2i randomTargetTile()
  {
    // Cyan: commits to a random target for a while.
    sf::Vector2i currentTile = getCurrentTile();

    int dx = randomTarget.x - currentTile.x;
    int dy = randomTarget.y - currentTile.y;
    int distanceSquared = dx * dx + dy * dy;

    if (randomTarget == sf::Vector2i{-1, -1} || distanceSquared <= 4)
    {
      randomTarget = {
          rand() % 20,
          rand() % 20
      };
    }

    return randomTarget;
  }

  sf::Vector2i cautiousTarget(sf::Vector2i currentTile, sf::Vector2i pacmanTile)
  {
    // Green: chases when far away, retreats when close.
    int dx = pacmanTile.x - currentTile.x;
    int dy = pacmanTile.y - currentTile.y;
    int distanceSquared = dx * dx + dy * dy;

    if (distanceSquared > 36)
    {
      return pacmanTile;
    }

    return {18, 18};
  }

  sf::Vector2i patrolTargetTile(sf::Vector2i currentTile)
  {
    // White: patrols between corners.
    int dx = patrolTarget.x - currentTile.x;
    int dy = patrolTarget.y - currentTile.y;
    int distanceSquared = dx * dx + dy * dy;

    if (distanceSquared <= 2)
    {
      if (patrolTarget == sf::Vector2i{1, 1})
      {
        patrolTarget = {18, 1};
      }
      else if (patrolTarget == sf::Vector2i{18, 1})
      {
        patrolTarget = {18, 18};
      }
      else if (patrolTarget == sf::Vector2i{18, 18})
      {
        patrolTarget = {1, 18};
      }
      else
      {
        patrolTarget = {1, 1};
      }
    }

    return patrolTarget;
  }

  void moveToTargetTile(float deltaTime)
  {
    if (!isMoving)
    {
      return;
    }

    sf::Vector2f center = getGhostCenter();

    float moveAmount = speed * deltaTime;

    float dx = targetCenter.x - center.x;
    float dy = targetCenter.y - center.y;

    float distance = std::sqrt(dx * dx + dy * dy);

    if (distance <= moveAmount)
    {
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

  sf::Vector2f getGhostCenter() const
  {
    return {
        x + radius,
        y + radius
    };
  }

  sf::Vector2i getCurrentTile() const
  {
    sf::Vector2f center = getGhostCenter();

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

#endif // PAC_MAN_GHOSTS_H