
#ifndef PAC_MAN_OBJECT_H
#define PAC_MAN_OBJECT_H

class Object
{
public:

    Object()
    {
        x = y = xspeed = yspeed = 0;
    }

    Object(float x, float y) : x(x), y(y), xspeed(0), yspeed(0) {}

    int red;
    int blue;

    // Common logic for moving objects
    virtual void logic(float deltaTime, const sf::Event& event)
    {
        x += xspeed * deltaTime;
        y += yspeed * deltaTime;
    }

    // draw() is pure virtual since the Object class doesn't have a visual representation
    virtual void draw(sf::RenderWindow &window) = 0;

    float x;
    float xspeed;

    float y;
    float yspeed;

    float getX() const { return x; }
};


#endif //PAC_MAN_OBJECT_H