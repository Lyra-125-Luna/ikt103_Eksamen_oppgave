#ifndef PAC_MAN_PAC_MAN_H
#define PAC_MAN_PAC_MAN_H

#include <SFML/Graphics.hpp>
#include <random>


#include "object/object.h"

class pac_man : public Object
{
public:

  // x and y are passed on to the Object constructor
  pac_man(float x, float y) : Object(x, y)
  {
    srand(static_cast<unsigned>(time(0)));
    shape.setRadius(radius);
    shape.setFillColor(sf::Color::Yellow);
    reset();
  }

  void collishon_handler(const sf::FloatRect &paddleRect)
  {
    sf::FloatRect ballRect = shape.getGlobalBounds();

    if (ballRect.findIntersection(paddleRect).has_value())
    {
      xspeed = -xspeed;

      if (xspeed > 0)
        x = paddleRect.position.x + paddleRect.size.x + 1.f;
      else
        x = paddleRect.position.x - shape.getRadius() * 2 - 1.f;
    }

  }


  void logic(float deltaTime, const sf::Event& event) override
  {
    float diameter = radius * 2;


    if (y <= 0)
    {
      y = 0.f;
      yspeed = std::abs(yspeed);
    }
    if (y + diameter >= 240.f)
    {
      y = 240.f - diameter;
      yspeed = -std::abs(yspeed);
    }

    Object::logic(deltaTime, event);
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

  void reset()
  {
    x = 160.f;
    y = 120.f;
    justReset = true;

    float speedX = 100.f + static_cast<float>(rand() % 100);
    float speedY = 100.f + static_cast<float>(rand() % 100);


    xspeed = goRight ? speedX : -speedX;
    yspeed = (rand() % 2 == 0) ? speedY : -speedY;

    goRight = !goRight;
  }

  bool hasJustReset() const
  { return justReset; }
  void clearResetFlag()
  { justReset = false; }


  float get_speed()
  {

    static std::mt19937 rng(std::random_device{}());  // seeded once
    std::uniform_real_distribution<float> dist(100.f, 250.f);

    float speed = dist(rng);

    // Randomly flip direction so ball doesn't always go the same way
    if (rng() % 2 == 0)
      speed = -speed;

    return speed;
  }


protected:

  sf::CircleShape shape;

  static constexpr float radius = 10.f;

  bool goRight = false;
  bool justReset = true;


};


#endif //PAC_MAN_PAC_MAN_H