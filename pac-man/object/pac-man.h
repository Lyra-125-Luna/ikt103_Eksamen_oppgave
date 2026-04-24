#ifndef PAC_MAN_PAC_MAN_H
#define PAC_MAN_PAC_MAN_H

#include <SFML/Graphics.hpp>
#include <random>
#include <iostream>


#include "object/object.h"

class pac_man : public Object
{
public:

  // x and y are passed on to the Object constructor
  pac_man(float x, float y, float maxY, float maxX) : Object(x, y), max_Y(maxY), max_X(maxX)
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

    // check connection state
    bool isConnected = sf::Joystick::isConnected(joystickId);


    if (isConnected && !wasConnected)
    {
      std::cout << "Joystick connected\n";
    }
    else if (!isConnected && wasConnected)
    {
      std::cout << "Joystick disconnected\n";
    }


    wasConnected = isConnected;


    float diameter = radius * 2;

    //key pressed
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) && xspeed == 0)
    {
      xspeed = 60;
      yspeed = 0;
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

    if (isConnected && sf::Joystick::isButtonPressed(joystickId, 0))
    {
      std::cout << "Button 0 pressed\n";
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


};


#endif //PAC_MAN_PAC_MAN_H