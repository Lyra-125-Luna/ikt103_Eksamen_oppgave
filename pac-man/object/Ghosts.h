#ifndef PAC_MAN_GHOSTS_H
#define PAC_MAN_GHOSTS_H

#include <SFML/Graphics.hpp>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <queue>
#include <string>
#include <vector>

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

enum class GhostState
{
  Normal,
  Scared,
  Dead
};

enum class GhostSpriteType
{
  Red,
  Pink,
  Blue,
  Green,
  Orange
};

class ghost : public Object
{
public:
  ghost(float x, float y, Map& map, pac_man& pacman, GhostSpriteType spriteType, float speed, GhostAI aiType)
      : Object(x, y),
        bodySprite(bodyTexture),
        eyeSprite(eyeRightTexture),
        map(map),
        pacman(pacman),
        speed(speed),
        normalSpeed(speed),
        aiType(aiType),
        spriteType(spriteType)
  {
    srand(static_cast<unsigned>(time(0)));

    ghostHouseTile = findGhostHouseTile();

    loadSprites();

    snapToTileCenter();

    bodySprite.setPosition(getGhostCenter());
    eyeSprite.setPosition(getGhostCenter());
  }

  void logic(float deltaTime) override
  {
    updateState(deltaTime);

    if (!isMoving)
    {
      chooseNextTile();
    }

    moveToTargetTile(deltaTime);

    if (state == GhostState::Dead && hasReachedGhostHouse())
    {
      revive();
    }

    updateSprites();
  }

  void draw(sf::RenderWindow& window) override
  {
    window.draw(bodySprite);
    window.draw(eyeSprite);
  }

  sf::FloatRect getBounds() const
  {
    return bodySprite.getGlobalBounds();
  }

  void makeScared(float duration)
  {
    if (state == GhostState::Dead)
    {
      return;
    }

    state = GhostState::Scared;
    scaredTimer = duration;
    speed = normalSpeed * 0.65f;

    bodySprite.setTexture(scaredTexture, true);
    eyeSprite.setTexture(eyeScaredTexture, true);

    forceChooseNewDirection();
  }

  void makeDead()
  {
    if (state == GhostState::Dead)
    {
      return;
    }

    state = GhostState::Dead;
    scaredTimer = 0.f;
    speed = normalSpeed * 1.6f;

    bodySprite.setTexture(deadTexture, true);
    updateEyeTexture();

    forceChooseNewDirection();
  }

  bool isScared() const
  {
    return state == GhostState::Scared;
  }

  bool isDead() const
  {
    return state == GhostState::Dead;
  }

  bool isNormal() const
  {
    return state == GhostState::Normal;
  }

protected:
  static constexpr float radius = 10.f;
  static constexpr float tileSize = 25.f;

  sf::Vector2i ghostHouseTile{9, 10};

  sf::Texture bodyTexture;
  sf::Texture scaredTexture;
  sf::Texture deadTexture;

  sf::Texture eyeRightTexture;
  sf::Texture eyeLeftTexture;
  sf::Texture eyeUpTexture;
  sf::Texture eyeDownTexture;
  sf::Texture eyeScaredTexture;
  sf::Texture eyeWaitingTexture;

  sf::Sprite bodySprite;
  sf::Sprite eyeSprite;

  Map& map;
  pac_man& pacman;

  GhostAI aiType = GhostAI::DirectChase;
  GhostState state = GhostState::Normal;
  GhostSpriteType spriteType = GhostSpriteType::Red;

  sf::Vector2i direction{0, 0};
  sf::Vector2i previousTile{-1, -1};

  bool isMoving = false;
  sf::Vector2f targetCenter{0.f, 0.f};

  sf::Vector2i patrolTarget{1, 1};
  sf::Vector2i randomTarget{-1, -1};

  float speed = 50.f;
  float normalSpeed = 50.f;

  float scaredTimer = 0.f;

  void forceChooseNewDirection()
  {
    isMoving = false;
    previousTile = {-1, -1};
  }

  sf::Vector2i findGhostHouseTile()
  {
    sf::Vector2i mapCenter{map.getWidth() / 2, map.getHeight() / 2};
    sf::Vector2i bestTile = mapCenter;
    int bestDistance = std::numeric_limits<int>::max();
    bool foundGhostHouseFloor = false;

    for (int y = 0; y < map.getHeight(); y++)
    {
      for (int x = 0; x < map.getWidth(); x++)
      {
        if (map.getTileAt(x, y) != 3)
        {
          continue;
        }

        int dx = mapCenter.x - x;
        int dy = mapCenter.y - y;
        int distance = dx * dx + dy * dy;

        if (distance < bestDistance)
        {
          bestDistance = distance;
          bestTile = {x, y};
          foundGhostHouseFloor = true;
        }
      }
    }

    if (foundGhostHouseFloor)
    {
      return bestTile;
    }

    bestDistance = std::numeric_limits<int>::max();

    for (int y = 0; y < map.getHeight(); y++)
    {
      for (int x = 0; x < map.getWidth(); x++)
      {
        if (map.getTileAt(x, y) != 2)
        {
          continue;
        }

        int dx = mapCenter.x - x;
        int dy = mapCenter.y - y;
        int distance = dx * dx + dy * dy;

        if (distance < bestDistance)
        {
          bestDistance = distance;
          bestTile = {x, y};
        }
      }
    }

    if (bestDistance != std::numeric_limits<int>::max())
    {
      return bestTile;
    }

    return clampToNearestOpenTile(mapCenter);
  }

  bool hasReachedGhostHouse()
  {
    sf::Vector2i currentTile = getCurrentTile();

    if (currentTile == ghostHouseTile)
    {
      return true;
    }

    return map.getTileAt(currentTile.x, currentTile.y) == 3;
  }

  void loadSprites()
  {
    std::string bodyPath = getBodyTexturePath();
    bodyTexture.loadFromFile(bodyPath);
    scaredTexture.loadFromFile("data/assets/ghosts/ghostscared.png");
    deadTexture.loadFromFile("data/assets/ghosts/ghostdead.png");
    eyeRightTexture.loadFromFile("data/assets/ghosts/eyeright.png");
    eyeLeftTexture.loadFromFile("data/assets/ghosts/eyeleft.png");
    eyeUpTexture.loadFromFile("data/assets/ghosts/eyeup.png");
    eyeDownTexture.loadFromFile("data/assets/ghosts/eyedown.png");
    eyeScaredTexture.loadFromFile("data/assets/ghosts/eyescared.png");
    eyeWaitingTexture.loadFromFile("data/assets/ghosts/eyewaiting.png");

    bodySprite.setTexture(bodyTexture, true);
    eyeSprite.setTexture(eyeRightTexture, true);

    setupSpriteSize(bodySprite, bodyTexture);
    setupSpriteSize(eyeSprite, eyeRightTexture);
  }

  std::string getBodyTexturePath() const
  {
    switch (spriteType)
    {
      case GhostSpriteType::Red:
        return "data/assets/ghosts/ghostred.png";

      case GhostSpriteType::Pink:
        return "data/assets/ghosts/ghostpink.png";

      case GhostSpriteType::Blue:
        return "data/assets/ghosts/ghostblue.png";

      case GhostSpriteType::Green:
        return "data/assets/ghosts/ghostgreen.png";

      case GhostSpriteType::Orange:
        return "data/assets/ghosts/ghostorange.png";
    }

    return "data/assets/ghosts/ghostred.png";
  }

  void setupSpriteSize(sf::Sprite& sprite, const sf::Texture& texture)
  {
    float diameter = radius * 2.f;
    sf::Vector2u size = texture.getSize();

    if (size.x == 0 || size.y == 0)
    {
      return;
    }

    sprite.setOrigin({
        static_cast<float>(size.x) / 2.f,
        static_cast<float>(size.y) / 2.f
    });

    sprite.setScale({
        diameter / static_cast<float>(size.x),
        diameter / static_cast<float>(size.y)
    });
  }

  void updateSprites()
  {
    sf::Vector2f center = getGhostCenter();

    bodySprite.setPosition(center);
    eyeSprite.setPosition(center);

    updateEyeTexture();
  }

  void updateEyeTexture()
  {
    if (state == GhostState::Scared)
    {
      eyeSprite.setTexture(eyeScaredTexture, true);
      setupSpriteSize(eyeSprite, eyeScaredTexture);
      return;
    }

    if (direction == sf::Vector2i{1, 0})
    {
      eyeSprite.setTexture(eyeRightTexture, true);
      setupSpriteSize(eyeSprite, eyeRightTexture);
    }
    else if (direction == sf::Vector2i{-1, 0})
    {
      eyeSprite.setTexture(eyeLeftTexture, true);
      setupSpriteSize(eyeSprite, eyeLeftTexture);
    }
    else if (direction == sf::Vector2i{0, -1})
    {
      eyeSprite.setTexture(eyeUpTexture, true);
      setupSpriteSize(eyeSprite, eyeUpTexture);
    }
    else if (direction == sf::Vector2i{0, 1})
    {
      eyeSprite.setTexture(eyeDownTexture, true);
      setupSpriteSize(eyeSprite, eyeDownTexture);
    }
    else
    {
      eyeSprite.setTexture(eyeWaitingTexture, true);
      setupSpriteSize(eyeSprite, eyeWaitingTexture);
    }
  }

  void updateState(float deltaTime)
  {
    if (state != GhostState::Scared)
    {
      return;
    }

    scaredTimer -= deltaTime;

    if (scaredTimer <= 0.f)
    {
      revive();
    }
  }

  void revive()
  {
    state = GhostState::Normal;
    scaredTimer = 0.f;
    speed = normalSpeed;

    bodySprite.setTexture(bodyTexture, true);
    updateEyeTexture();

    forceChooseNewDirection();
  }

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

    if (state == GhostState::Dead)
    {
      direction = chooseBfsDirection(currentTile, ghostHouseTile, availableDirections, true);
    }
    else if (state == GhostState::Scared)
    {
      direction = chooseScaredDirection(currentTile, availableDirections);
    }
    else
    {
      bool shouldDecide = shouldPickNewDirection(availableDirections);

      if (shouldDecide)
      {
        sf::Vector2i targetTile = chooseTargetTile(currentTile);
        direction = chooseBfsDirection(currentTile, targetTile, availableDirections, false);
      }
    }

    if (direction == sf::Vector2i{0, 0})
    {
      direction = availableDirections.front();
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

  bool shouldPickNewDirection(const std::vector<sf::Vector2i>& availableDirections) const
  {
    if (direction == sf::Vector2i{0, 0})
    {
      return true;
    }

    sf::Vector2i reverseDirection = {-direction.x, -direction.y};

    bool canContinueForward = false;
    int nonReverseOptions = 0;

    for (sf::Vector2i dir : availableDirections)
    {
      if (dir == direction)
      {
        canContinueForward = true;
      }

      if (dir != reverseDirection)
      {
        nonReverseOptions++;
      }
    }

    bool isIntersection = nonReverseOptions >= 2;
    bool isDeadEnd = nonReverseOptions == 0;

    return !canContinueForward || isIntersection || isDeadEnd;
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

  sf::Vector2i chooseBfsDirection(
      sf::Vector2i currentTile,
      sf::Vector2i targetTile,
      const std::vector<sf::Vector2i>& availableDirections,
      bool allowReverse
  )
  {
    int width = map.getWidth();
    int height = map.getHeight();

    if (width <= 0 || height <= 0)
    {
      return fallbackDirection(currentTile, targetTile, availableDirections, allowReverse);
    }

    if (targetTile.x < 0 || targetTile.y < 0 || targetTile.x >= width || targetTile.y >= height || map.isWall(targetTile.x, targetTile.y))
    {
      targetTile = clampToNearestOpenTile(targetTile);
    }

    std::vector<int> visited(width * height, 0);
    std::vector<sf::Vector2i> parent(width * height, {-1, -1});

    auto indexOf = [width](sf::Vector2i tile)
    {
      return tile.y * width + tile.x;
    };

    std::queue<sf::Vector2i> queue;
    queue.push(currentTile);
    visited[indexOf(currentTile)] = 1;

    std::vector<sf::Vector2i> directions = {
        {1, 0},
        {-1, 0},
        {0, 1},
        {0, -1}
    };

    while (!queue.empty())
    {
      sf::Vector2i tile = queue.front();
      queue.pop();

      if (tile == targetTile)
      {
        break;
      }

      for (sf::Vector2i dir : directions)
      {
        sf::Vector2i next = tile + dir;

        if (next.x < 0 || next.y < 0 || next.x >= width || next.y >= height)
        {
          continue;
        }

        if (map.isWall(next.x, next.y))
        {
          continue;
        }

        int idx = indexOf(next);

        if (visited[idx])
        {
          continue;
        }

        visited[idx] = 1;
        parent[idx] = tile;
        queue.push(next);
      }
    }

    if (!visited[indexOf(targetTile)])
    {
      return fallbackDirection(currentTile, targetTile, availableDirections, allowReverse);
    }

    sf::Vector2i step = targetTile;

    while (parent[indexOf(step)] != currentTile && parent[indexOf(step)] != sf::Vector2i{-1, -1})
    {
      step = parent[indexOf(step)];
    }

    sf::Vector2i chosenDirection = step - currentTile;

    if (!allowReverse && availableDirections.size() > 1)
    {
      sf::Vector2i reverseDirection = {-direction.x, -direction.y};

      if (chosenDirection == reverseDirection)
      {
        return fallbackDirection(currentTile, targetTile, availableDirections, false);
      }
    }

    return chosenDirection;
  }

  sf::Vector2i fallbackDirection(
      sf::Vector2i currentTile,
      sf::Vector2i targetTile,
      const std::vector<sf::Vector2i>& availableDirections,
      bool allowReverse
  )
  {
    sf::Vector2i reverseDirection = {-direction.x, -direction.y};
    sf::Vector2i bestDirection{0, 0};
    int bestDistance = std::numeric_limits<int>::max();

    for (sf::Vector2i candidateDirection : availableDirections)
    {
      if (!allowReverse && availableDirections.size() > 1 && candidateDirection == reverseDirection)
      {
        continue;
      }

      sf::Vector2i nextTile = currentTile + candidateDirection;

      int dx = targetTile.x - nextTile.x;
      int dy = targetTile.y - nextTile.y;
      int distance = dx * dx + dy * dy;

      if (distance < bestDistance)
      {
        bestDistance = distance;
        bestDirection = candidateDirection;
      }
    }

    if (bestDirection == sf::Vector2i{0, 0})
    {
      bestDirection = availableDirections.front();
    }

    return bestDirection;
  }

  sf::Vector2i chooseScaredDirection(sf::Vector2i currentTile, const std::vector<sf::Vector2i>& availableDirections)
  {
    sf::Vector2i pacmanTile = pacman.getGridPosition();
    sf::Vector2i reverseDirection = {-direction.x, -direction.y};

    sf::Vector2i bestDirection{0, 0};
    int bestDistance = -1;

    for (sf::Vector2i candidateDirection : availableDirections)
    {
      if (availableDirections.size() > 1 && candidateDirection == reverseDirection)
      {
        continue;
      }

      sf::Vector2i nextTile = currentTile + candidateDirection;

      int dx = pacmanTile.x - nextTile.x;
      int dy = pacmanTile.y - nextTile.y;
      int distance = dx * dx + dy * dy;

      if (distance > bestDistance)
      {
        bestDistance = distance;
        bestDirection = candidateDirection;
      }
    }

    if (bestDirection == sf::Vector2i{0, 0})
    {
      bestDirection = availableDirections.front();
    }

    return bestDirection;
  }

  sf::Vector2i clampToNearestOpenTile(sf::Vector2i wantedTile)
  {
    int width = map.getWidth();
    int height = map.getHeight();

    sf::Vector2i bestTile{1, 1};
    int bestDistance = std::numeric_limits<int>::max();

    for (int y = 0; y < height; y++)
    {
      for (int x = 0; x < width; x++)
      {
        if (map.isWall(x, y))
        {
          continue;
        }

        int dx = wantedTile.x - x;
        int dy = wantedTile.y - y;
        int distance = dx * dx + dy * dy;

        if (distance < bestDistance)
        {
          bestDistance = distance;
          bestTile = {x, y};
        }
      }
    }

    return bestTile;
  }

  sf::Vector2i chooseTargetTile(sf::Vector2i currentTile)
  {
    sf::Vector2i pacmanTile = pacman.getGridPosition();
    sf::Vector2i pacmanDirection = pacman.getDirection();

    switch (aiType)
    {
      case GhostAI::DirectChase:
        return pacmanTile;

      case GhostAI::Ambush:
        if (pacmanDirection == sf::Vector2i{0, 0})
        {
          return pacmanTile;
        }
        return pacmanTile + pacmanDirection * 4;

      case GhostAI::Random:
        return randomTargetTile();

      case GhostAI::SlowChase:
        return cautiousTarget(currentTile, pacmanTile);

      case GhostAI::Patrol:
        return patrolTargetTile(currentTile);
    }

    return pacmanTile;
  }

  sf::Vector2i randomTargetTile()
  {
    sf::Vector2i currentTile = getCurrentTile();

    int dx = randomTarget.x - currentTile.x;
    int dy = randomTarget.y - currentTile.y;
    int distanceSquared = dx * dx + dy * dy;

    if (randomTarget == sf::Vector2i{-1, -1} || distanceSquared <= 4 || map.isWall(randomTarget.x, randomTarget.y))
    {
      randomTarget = clampToNearestOpenTile({
          rand() % std::max(1, map.getWidth()),
          rand() % std::max(1, map.getHeight())
      });
    }

    return randomTarget;
  }

  sf::Vector2i cautiousTarget(sf::Vector2i currentTile, sf::Vector2i pacmanTile)
  {
    int dx = pacmanTile.x - currentTile.x;
    int dy = pacmanTile.y - currentTile.y;
    int distanceSquared = dx * dx + dy * dy;

    if (distanceSquared > 36)
    {
      return pacmanTile;
    }

    return clampToNearestOpenTile({map.getWidth() - 2, map.getHeight() - 2});
  }

  sf::Vector2i patrolTargetTile(sf::Vector2i currentTile)
  {
    int dx = patrolTarget.x - currentTile.x;
    int dy = patrolTarget.y - currentTile.y;
    int distanceSquared = dx * dx + dy * dy;

    if (patrolTarget == sf::Vector2i{1, 1} || map.isWall(patrolTarget.x, patrolTarget.y))
    {
      patrolTarget = clampToNearestOpenTile({1, 1});
    }

    if (distanceSquared <= 2)
    {
      sf::Vector2i topLeft = clampToNearestOpenTile({1, 1});
      sf::Vector2i topRight = clampToNearestOpenTile({map.getWidth() - 2, 1});
      sf::Vector2i bottomRight = clampToNearestOpenTile({map.getWidth() - 2, map.getHeight() - 2});
      sf::Vector2i bottomLeft = clampToNearestOpenTile({1, map.getHeight() - 2});

      if (patrolTarget == topLeft)
      {
        patrolTarget = topRight;
      }
      else if (patrolTarget == topRight)
      {
        patrolTarget = bottomRight;
      }
      else if (patrolTarget == bottomRight)
      {
        patrolTarget = bottomLeft;
      }
      else
      {
        patrolTarget = topLeft;
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

#endif
