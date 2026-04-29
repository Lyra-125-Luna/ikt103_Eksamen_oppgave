
#ifndef PAC_MAN_ENEMY_GOST_1_H
#define PAC_MAN_ENEMY_GOST_1_H

#include <SFML/Graphics.hpp>
#include <random>
#include <iostream>


#include "object/object.h"

class gost_1 : public Object
{
public:

  // x and y are passed on to the Object constructor
  gost_1(float x, float y, float maxY, float maxX) : Object(x, y), max_Y(maxY), max_X(maxX)
  {
    srand(static_cast<unsigned>(time(0)));
    shape.setRadius(radius);
    shape.setFillColor(sf::Color::Yellow);
  }

  void collishon_handler(const sf::FloatRect &wall)
  {
    if (y == 0)
    {
      xspeed = 0;
      yspeed = 0;
    }

    if (y == max_Y)
    {
      yspeed = 0;
    };

    if (x >= max_X)
    {
      x == 0;
    }
    if (x <= -0.1f)
    {
      x == max_X;
    }


  }



  void logic(float deltaTime)
  {


    bool isConnected = sf::Joystick::isConnected(joystickId);

    isConnected = sf::Joystick::isConnected(joystickId);
    wasConnected = false;

    if (startup)
    {
      if (isConnected && !wasConnected) {

        std::cout << "Joystick connected\n";
      }
      else if (!isConnected && wasConnected) {
        std::cout << "Joystick disconnected\n";
      }
      startup = false;

    }


    wasConnected = isConnected;




    float diameter = radius * 2;

    //key pressed
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) && xspeed == 0)
    {
      yspeed = 0;
      xspeed = 60;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) && xspeed == 0)
    {
      xspeed = -60;
      yspeed = 0;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) && yspeed == 0)
    {
      yspeed = -60;
      xspeed = 0;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)&& yspeed == 0)
    {
      yspeed = 60;
      xspeed = 0;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
    {
      yspeed = 0;
      xspeed = 0;
    }

    // controller

      // down
    if (isConnected && sf::Joystick::isButtonPressed(joystickId, 0) && yspeed == 0)
    {
      yspeed = 60;
      xspeed = 0;

    }

    // rigth
    if (isConnected && sf::Joystick::isButtonPressed(joystickId, 1) && xspeed == 0)
    {
      yspeed = 0;
      xspeed = 60;
    }

    // up
    if (isConnected && sf::Joystick::isButtonPressed(joystickId, 2) && yspeed == 0)
    {

      yspeed = -60;
      xspeed = 0;
    }

      // left
    if (isConnected && sf::Joystick::isButtonPressed(joystickId, 3) && xspeed == 0)
    {
      yspeed = 0;
      xspeed = -60;
    }

    if (isConnected && sf::Joystick::isButtonPressed(joystickId, 4))
    {
      yspeed = 0;
      xspeed = 0;
    }



    Object::logic(deltaTime);
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


 unsigned int joystickId = 0;
  bool wasConnected = false;

  float max_Y = 0.f;
  float max_X = 0.f;

  bool startup = true;

};


#endif //PAC_MAN_ENEMY_GOST_1_H