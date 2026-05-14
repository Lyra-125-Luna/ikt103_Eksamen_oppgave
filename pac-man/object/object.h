#ifndef PAC_MAN_OBJECT_H
#define PAC_MAN_OBJECT_H

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

class Object
{
public:
    Object()
    {
        x = y = xspeed = yspeed = 0;
    }

    Object(float x, float y) : x(x), y(y), xspeed(0), yspeed(0) {}

    virtual ~Object() = default;

    int red = 0;
    int blue = 0;

    // Common logic for moving objects.
    virtual void logic(float deltaTime)
    {
        x += xspeed * deltaTime;
        y += yspeed * deltaTime;
    }

    // draw() is pure virtual because Object has no visual representation.
    virtual void draw(sf::RenderWindow &window) = 0;

    float x = 0.f;
    float xspeed = 0.f;

    float y = 0.f;
    float yspeed = 0.f;

    float getX() const { return x; }
};

#endif // PAC_MAN_OBJECT_H
